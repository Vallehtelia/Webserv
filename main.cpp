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
#include "request/Request.hpp"
#include "response/Response.hpp"
#include "./parsing/ServerConfig.hpp"
#include <unordered_map>

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
    int new_socket;
	int epoll_fd;
	// Event array for epoll_wait
    struct epoll_event events[MAX_EVENTS];
	// Map to store data being read from each socket
    std::unordered_map<int, std::vector<char>> client_data;

    if (ac != 2)
	{
		std::cerr << "Error: wrong number of arguments" << std::endl;
		return 1;
	}
    checkConfFile(av[1]);
    std::cout << "\033[1;32mParsing file: " << av[1] << "\033[0m" << std::endl;
    parseData(av[1], server);
    for (std::vector<ServerConfig>::iterator it = server.begin(); it != server.end(); it++)
		it->printConfig();

    std::vector<ServerConfig>::iterator it = server.begin();
    Socket server(it->getListenPort(), it->getHost());

    // Configure server address and port
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT); // ehka helpompi et se on aina vaan sama
    //serv_addr.sin_port = htons(std::stoi(it->getListenPort())); // otin tahan vaan ekan serverin portin, taa serv addr pitaa varmaan olla kans joku array tjtn et voidaan kuunnella useempia portteja samanaikasesti
	serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Create socket
    server.setSocketFd(socket(AF_INET, SOCK_STREAM, 0));
    if (server.getSocketFd() <= 0) {
        std::cout << "Failed to set socket: " << strerror(errno) << "\n";
        return 1;
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server.getSocketFd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cout << "setsockopt failed: " << strerror(errno) << "\n";
        return 1;
    }

    // Bind socket to address and port
    if (bind(server.getSocketFd(), (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Failed to bind: " << strerror(errno) << "\n";
        return 1;
    }

	// Start listening for connections
    if (listen(server.getSocketFd(), 10) < 0) {
        std::cout << "Failed to listen: " << strerror(errno) << "\n";
        close(server.getSocketFd());
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
    set_non_blocking(server.getSocketFd());

	// Add the server socket to the epoll instance
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server.getSocketFd();
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server.getSocketFd(), &event) == -1) 
	{
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    std::cout << CYAN << "Server is listening on port " << it->getListenPort() << "...\n";
    std::cout << "open 'localhost:" << std::stoi(it->getListenPort()) << "' on browser\n" << DEFAULT;

	// Main loop
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
            Request req;
			if (events[i].data.fd == server.getSocketFd()) 
			{
				// Accept incoming connection
				// client_len = sizeof(client_addr);
				new_socket = accept(server.getSocketFd(), (struct sockaddr*)&client_addr, (socklen_t *)&client_addr);
				if (new_socket < 0) {
					std::cout << "Failed to create client fd: " << strerror(errno) << "\n";
					close(server.getSocketFd());
					continue;
				}

				// Set the new socket to non-blocking mode and add to epoll instance
                set_non_blocking(new_socket);
                event.events = EPOLLIN;
                event.data.fd = new_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &event) == -1) {
                    perror("epoll_ctl");
                    exit(EXIT_FAILURE);
                }
				// Initialize data buffer for the new socket
                client_data[new_socket] = std::vector<char>();
			}
			else if (events[i].events & EPOLLIN) 
			{
				// Read incoming data
				char buffer[4000] = {0};
				int bytes_read = 0;
                size_t request_length = 0;
                std::string rawRequest;
                while ((bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0)
                {
 
                    rawRequest.append(buffer, bytes_read);
                    std::cout << "read " << bytes_read << " bytes" << std::endl;
                    request_length += bytes_read;
                }
                if (request_length == 0)
                    close(events[i].data.fd);
                else
                {
                    std::cout << "DONE READIN, READ " << request_length << " BYTES" << std::endl;
                    std::cout << "DEBUG BYTES_READ: " << bytes_read << std::endl;
                    std::cout << "RAW REQUEST: " << "\033[94m" << rawRequest << "\033[0m" << std::endl;
					Request req(rawRequest);
                    req.printRequest();
					Response res;
        			res.createResponse(req);
                    res.printResponse();
					std::string http_response = res.getResponseString();
					if (send(events[i].data.fd, http_response.c_str(), http_response.length(), 0) < 0) {
						std::cout << "Failed to send: " << strerror(errno) << "\n";
						close(events[i].data.fd);
						close(socket1.getSocketFd());
						return 1;
					}
					close(events[i].data.fd);
				}
				*/
			}
		}
    }
    close(server.getSocketFd());
	close(epoll_fd);
    return 0;
}
