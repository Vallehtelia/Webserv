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
    int client_fd;
	int epoll_fd;
    std::unordered_map<int, std::vector<char>> client_data;
    State currentState;
    currentState = State::REQUEST_LINE;

    if (ac != 2)
	{
		std::cerr << "Error: wrong number of arguments" << std::endl;
		return 1;
	}
    checkConfFile(av[1]);
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
            req.setState(currentState);
			if (events[i].data.fd == socket1.getSocketFd()) 
			{
				// Accept incoming connection
				client_len = sizeof(client_addr);
				client_fd = accept(socket1.getSocketFd(), (struct sockaddr*)&client_addr, &client_len);
				if (client_fd < 0) {
					std::cout << "Failed to create client fd: " << strerror(errno) << "\n";
                    currentState = State::REQUEST_LINE;
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
                std::cout << "RECEIVING DATA FROM FD: " << client_fd << std::endl;
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
					//client_data[client_fd].insert(client_data[client_fd].end(), buffer, buffer + bytes_read);
                    std::string rawRequest(buffer, bytes_read);
					//std::cout << "RAW BUFFER: " << "\033[94m" << rawRequest << "\033[0m" << std::endl;
					req.parseRequest(rawRequest);
                    currentState = req.StateFromString(req.getState());
                    //req.printRequest();
                    if (req.getState() == "COMPLETE")
                    {
                        currentState = State::REQUEST_LINE;
					    Response res;
                        req.printRequest();
        			    res.createResponse(req);
                        res.printResponse();
					    // Get the full HTTP response string from the Response class
					    std::string http_response = res.getResponseString();
					    // Send the response back to the client
                        
					    if (send(events[i].data.fd, http_response.c_str(), http_response.length(), 0) < 0) {
						    std::cout << "Failed to send: " << strerror(errno) << "\n";
						    close(events[i].data.fd);
						    close(socket1.getSocketFd());
						    return 1;
					    }
                        std::cout << "RESPONSE SENT" << std::endl;
                        requests[events[i].data.fd].reset();
					    close(events[i].data.fd);
                    }
				}
			}
		}
		/*
        bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0) {
            std::cout << "Failed to receive: " << strerror(errno) << "\n";
            close(socket1.getSocketFd());
            return 1;
        }
        buffer[bytes_received] = '\0';
        std::cout << buffer << std::endl;
        std::string rawRequest(buffer, bytes_received);
        Request req(rawRequest);
        std::cout << "Received request:\n" << std::endl;
        std::cout << "method: " << req.getMethod() << std::endl;
        std::cout << "path: " << req.getPath() << std::endl;
        std::cout << "version: " << req.getVersion() << std::endl;
        std::cout << "body: " << req.getBody() << std::endl;
        std::cout << "---------------" << std::endl;
        std::cout << "Serving file: " << req.getPath() << std::endl;
        std::cout << "---------------" << std::endl;
        Response res;
        res.createResponse(req);
        // Let the Response class handle everything

        // Get the full HTTP response string from the Response class
        std::string http_response = res.getResponseString();
        // Send the response back to the client
        if (send(client_fd, http_response.c_str(), http_response.length(), 0) < 0) {
            std::cout << "Failed to send: " << strerror(errno) << "\n";
            close(client_fd);
            close(socket1.getSocketFd());
            return 1;
        }
        close(client_fd);
		*/
    }

    // Close the other sockets
    close(socket1.getSocketFd());
	close(epoll_fd);
    return 0;
}
