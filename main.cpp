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
#include "./parsing/ServerConfig.hpp"
#include "request/Request.hpp"
#include "request/RequestHandler.hpp"
#include "response/Response.hpp"
#include "./parsing/ServerConfig.hpp"
#include "./cgi/cgi_request.hpp"
#include "utils.hpp"

#define MAX_EVENTS 10 // taa varmaa conffii
#define PORT 8002 // ja taa


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
    struct sockaddr_in serv_addr, client_addr;
    socklen_t client_len;
    int client_fd;
	int epoll_fd;
    std::unordered_map<int, std::vector<char>> client_data;

    if (ac != 2)
	{
		std::cerr << "Error: wrong number of arguments" << std::endl;
		return 1;
	}
    if (!checkConfFile(av[1]))
        return 1;
    std::cout << "\033[1;32mParsing file: " << av[1] << "\033[0m" << std::endl;
    parseData(av[1], server);
    // for (std::vector<ServerConfig>::iterator it = server.begin(); it != server.end(); it++)
	// {
	// 	it->printConfig();
	// }

    std::vector<ServerConfig>::iterator it = server.begin();
    Socket socket1(it->getListenPort(), it->getHost());

    // Configure server address and port
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT); // ehka helpompi et se on aina vaan sama
    //serv_addr.sin_port = htons(std::stoi(it->getListenPort())); // otin tahan vaan ekan serverin portin, taa serv addr pitaa varmaan olla kans joku array tjtn et voidaan kuunnella useempia portteja samanaikasesti
	serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Create socket
    socket1.setSocketFd(socket(AF_INET, SOCK_STREAM, 0));
    if (socket1.getSocketFd() <= 0) {
        std::cout << "Failed to set socket: " << strerror(errno) << "\n";
        return 1;
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(socket1.getSocketFd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cout << "setsockopt failed: " << strerror(errno) << "\n";
        return 1;
    }

    // Bind socket to address and port
    if (bind(socket1.getSocketFd(), (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Failed to bind: " << strerror(errno) << "\n";
        return 1;
    }

	// Start listening for connections
    if (listen(socket1.getSocketFd(), 10) < 0) {
        std::cout << "Failed to listen: " << strerror(errno) << "\n";
        close(socket1.getSocketFd());
        return 1;
    }

	// Create epoll instance
    epoll_fd = epoll_create(1); // was epoll_create1(0), arg size must be > 0 but is ignored
    if (epoll_fd == -1)
	{
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }

	// Set the server socket to non-blocking mode
    set_non_blocking(socket1.getSocketFd());

	// Add the server socket to the epoll instance
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = socket1.getSocketFd();
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket1.getSocketFd(), &event) == -1)
	{
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

	// Event array for epoll_wait
    struct epoll_event events[MAX_EVENTS];

    std::cout << CYAN << "Server is listening on port " << it->getListenPort() << "...\n";
    std::cout << "open 'localhost:" << std::stoi(it->getListenPort()) << "' on browser\n" << DEFAULT;

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
            Request &req = requests[events[i].data.fd];
			if (events[i].data.fd == socket1.getSocketFd())
			{
				// Accept incoming connection
				client_len = sizeof(client_addr);
				client_fd = accept(socket1.getSocketFd(), (struct sockaddr*)&client_addr, &client_len);
				if (client_fd < 0) {
					std::cout << "Failed to create client fd: " << strerror(errno) << "\n";
					close(socket1.getSocketFd());
					return 1;
				}
				// Set the new socket to non-blocking mode and add to epoll instance
                set_non_blocking(client_fd);
                event.events = EPOLLIN;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                    perror("epoll_ctl");
                    exit(EXIT_FAILURE);
                }
                client_data[client_fd] = std::vector<char>();
                std::cout << "ACCEPTED CONNECTION FD: " << client_fd << std::endl;
			}
			else if (events[i].events & EPOLLIN)
			{
				// Read incoming data
                int fd = events[i].data.fd;
                std::cout << "RECEIVING DATA FROM FD: " << fd << std::endl;
				char buffer[4000] = {0};
				int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
				if (bytes_read <= 0) {
					// Close connection if read fails or end of data
					close(events[i].data.fd);
                	client_data.erase(client_fd);
                    currentState = State::REQUEST_LINE;
				}
                else
                {
                    std::string rawRequest(buffer, bytes_read);
					req.parseRequest(rawRequest);
                    req.printRequest();
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
                            int execute_result = cgireg.execute();
                            if (execute_result == 0)
                            {
                                req.setPath("/cgi_output.html");
                            }
                        }
                        }
					    Response res;
                        RequestHandler requestHandler;
        			    requestHandler.handleRequest(req, res);
                        res.printResponse();
					    std::string http_response = res.getResponseString();

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
    close(socket1.getSocketFd());
	close(epoll_fd);
    return 0;
}
