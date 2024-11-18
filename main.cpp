#include "socket.hpp"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include "./parsing/ConfigValidator.hpp"
#include "./parsing/ServerConfig.hpp"
#include "request/Request.hpp"
#include "request/RequestHandler.hpp"
#include "response/Response.hpp"
#include "./cgi/cgi_request.hpp"
#include "utils.hpp"

#define MAX_EVENTS 10 // taa varmaa conffii
// #define PORT 8002 // ja taa

// Function to set a socket to non-blocking mode
void set_non_blocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl 1");
        exit(EXIT_FAILURE);
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
        perror("fcntl 2");
        exit(EXIT_FAILURE);
    }
}

int main(int ac, char **av)
{
    std::vector<ServerConfig>	server; // Taa sisaltaa kaiken tiedon, Server name, port, host, root, client bodysize, index path, error pages in a map, locations in vector.
	int epoll_fd;
    std::unordered_map<int, std::vector<char>> client_data;

    if (ac != 2)
	{
		std::cerr << "Error: wrong number of arguments" << std::endl;
		return 1;
	}
    if (!checkConfFile(av[1]))
        return 1;
    std::cout << "\033[1;32mValidating file: " << av[1] << "\033[0m" << std::endl;
	if (!ConfigValidator::validateConfigFile(av[1])) {
		std::cout << "Warning: invalid configuration file" << std::endl;
	} else {
		std::cout << "Valid configuration file found." << std::endl;
	}
	std::cout << "\033[1;32mParsing file: " << av[1] << "\033[0m" << std::endl;
    parseData(av[1], server);
	server[0].printConfig();
    std::vector<Socket> sockets;
    for (const ServerConfig &config : server)
    {
		std::cout << "Creating socket on port " << config.getListenPort() << " and host " << config.getHost() << "...\n";
        Socket sock(config.getListenPort(), config.getHost());
		sockets.push_back(sock);
    }

	// Create epoll instance
    epoll_fd = epoll_create(1); // was epoll_create1(0), arg size must be > 0 but is ignored
    if (epoll_fd == -1)
	{
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }

    // Create socket
	for (Socket &sock : sockets)
	{
		sock.setSocketFd(socket(AF_INET, SOCK_STREAM, 0));
		if (sock.getSocketFd() <= 0)
		{
			std::cerr << RED << "Failed to set socket: " << strerror(errno) << DEFAULT << "\n";
			return 1;
		}
		set_non_blocking(sock.getSocketFd());

		struct sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(sock.getPort());
		addr.sin_addr.s_addr = inet_addr(sock.getIp().c_str());

		if (bind(sock.getSocketFd(), (struct sockaddr*)&addr, sizeof(addr)) < 0)
		{
			std::cerr << RED << "Failed to bind: " << strerror(errno) << " On port: " << sock.getPort() << DEFAULT << "\n";
			return 1;
		}

		if (listen(sock.getSocketFd(), 10) < 0)
		{
			std::cerr << RED << "Failed to listen: " << strerror(errno) << DEFAULT << "\n";
			close(sock.getSocketFd());
			return 1;
		}

		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = sock.getSocketFd();
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock.getSocketFd(), &event) == -1)
		{
			perror("epoll_ctl");
			exit(EXIT_FAILURE);
		}

		std::cout << CYAN << "Server is listening on port " << sock.getPort() << "...\n";
		std::cout << "open " << sock.getIp() << ":" << sock.getPort() << " on browser\n" << DEFAULT;
	}

	// Event array for epoll_wait
    struct epoll_event events[MAX_EVENTS];

	// Main loop
    std::unordered_map<int, Request> requests;
    while (true)
	{
		int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1)
		{
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
		for (int i = 0; i < num_events; ++i)
		{
			int fd = events[i].data.fd;
            Request &req = requests[fd];

			auto it = std::find_if(sockets.begin(), sockets.end(),
				[fd](const Socket &sock) { return sock.getSocketFd() == fd; });
			if (it != sockets.end())
			{
				// Accept incoming connection
				struct sockaddr_in client_addr;
				socklen_t client_len = sizeof(client_addr);
				int client_fd = accept(fd, (struct sockaddr*)&client_addr, &client_len);
				if (client_fd < 0)
				{
					std::cerr << RED << "Failed to accept connection: " << strerror(errno) << DEFAULT << "\n";
					continue;
				}

				set_non_blocking(client_fd);
				struct epoll_event event;
				event.events = EPOLLIN;
				event.data.fd = client_fd;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
				{
					std::cerr << RED << "Failed to add client to epoll: " << strerror(errno) << DEFAULT << "\n";
					close(client_fd);
					continue;
				}
				std::cout << "ACCEPTED CONNECTION ON PORT " << it->getPort() << " FD: " << client_fd << std::endl;
			}

			else if (events[i].events & EPOLLIN)
			{
				// Read incoming data
                int fd = events[i].data.fd;
                std::cout << "RECEIVING DATA FROM FD: " << fd << std::endl;
				char buffer[4000] = {0};
				int bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
				if (bytes_read < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        std::cerr << "errno EAGAIN or EWOULDBLOCK\n";
                        continue;
                    }
                    else
                    {
                        perror("recv");
                        // Close connection if read fails or end of data
                        close(fd);
                        client_data.erase(fd);
                    }
				}
                else if (bytes_read == 0)
                {
                    close(fd);
                    client_data.erase(fd);
                }
                else
                {
                    std::string rawRequest(buffer, bytes_read);
					req.parseRequest(rawRequest);
                    // req.printRequest();
                    if (req.getState() == State::COMPLETE || req.getState() == State::ERROR)
                    {
                        if (req.getState() != State::ERROR)
                        {
                            std::string path = req.getUri();
                            bool    cgi_req = (path.find("/cgi-bin/") != std::string::npos || (path.size() > 3 && path.substr(path.size() - 3) == ".py"));
                            if (cgi_req)
                            {
                                std::string queryString = findQueryStr(req.getUri());
                                std::string directPath;
                                directPath = findPath(req.getUri());
                                std::cout << "DIRECT PATH: " << directPath << std::endl;
                                cgiRequest cgireg(directPath, req.getMethod(), queryString, req.getVersion(), req.getBody());
                                int execute_result = cgireg.execute(); // exit status so we need to give correct error page so current one is broken
                                if (execute_result == 0)
                                {
                                    req.setPath("/cgi_output.html");
                                }
                            }
                        }
                        //req.printRequest();
					    Response res;
                        RequestHandler requestHandler;
        			    requestHandler.handleRequest(req, res);
                        //res.printResponse();
					    // Get the full HTTP response string from the Response class
					    std::string http_response = res.getResponseString();
					    // Send the response back to the client

                        // std::cout << http_response.length() << "lenght here!!!\n";
                        // res.printResponse();
                        size_t total_sent = 0;
                        size_t message_length = http_response.length();
                        const char *message_ptr = http_response.c_str();
                        while (total_sent < message_length)
                        {
                            ssize_t bytes_sent = send(events[i].data.fd, message_ptr + total_sent, message_length - total_sent, 0);
                            if (bytes_sent < 0)
                            {
                                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                    // Socket is not ready, could add a delay or handle as needed
                                    continue;
                                } else {
                                    perror("send");
                                    close(events[i].data.fd);
                                    break;
                                }
                            }
                            total_sent += bytes_sent;
                        }
                        std::cout << "RESPONSE SENT" << std::endl;
                        requests[events[i].data.fd].reset();
					    close(events[i].data.fd);
                        std::string tempInFilePath = "./html/tmp/cgi_output.html";
                        std::string tempOutFilePath = "./html/tmp/cgi_input.html";
                        if (std::ifstream(tempInFilePath))
                        {
                            if (std::remove(tempInFilePath.c_str()) != 0)
                                std::cerr << "Failed to delete temp file: " << strerror(errno) << "\n";
                        }
                        if (std::ifstream(tempOutFilePath))
                        {
                            if (std::remove(tempOutFilePath.c_str()) != 0)
                                std::cerr << "Failed to delete temp file: " << strerror(errno) << "\n";
                        }
                    }
				}
			}
		}
    }
    // Close the other sockets
	for (Socket &sock : sockets)
	{
		close(sock.getSocketFd());
	}
	close(epoll_fd);
    return 0;
}
