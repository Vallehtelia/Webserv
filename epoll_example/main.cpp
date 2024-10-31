#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

#define MAX_EVENTS 10
#define PORT 8080

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

int main(void) 
{
    int server_fd, new_socket, epoll_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Set up server address structure
    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // the address
    address.sin_port = htons(PORT); // port by network byte order

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) 
	{
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) 
	{
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) 
	{
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Create epoll instance
    epoll_fd = epoll_create(1); // was epoll_create1(0), arg size must be > 0 but is ignored
    if (epoll_fd == -1) 
	{
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }

    // Set the server socket to non-blocking mode
    set_non_blocking(server_fd);

    // Add the server socket to the epoll instance
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) 
	{
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    // Event array for epoll_wait
    struct epoll_event events[MAX_EVENTS];
	// Map to store data being read from each socket
    std::unordered_map<int, std::vector<char>> socket_data;

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
            if (events[i].data.fd == server_fd) {
                // Incoming connection
                new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                if (new_socket == -1) {
                    perror("accept");
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
                socket_data[new_socket] = std::vector<char>();
            }
			else if (events[i].events & EPOLLIN) {
				
				int sockfd = events[i].data.fd;
				char temp[1024];
				int bytes_read;

				while ((bytes_read = read(sockfd, temp, sizeof(temp))) > 0) {
					socked_data[sockfd].insert(socked_data[sockfd].end(), temp, temp + bytes_read);
				}

				if (bytes_read == -1 && errno != EAGAIN) {
					// Error occurred
					perror("read");
					close(sockfd);
					socket_data.erase(sockfd);
				} 
				else if (bytes_read == 0) {
					// Write some shit
					std::ofstream output_file("uploads/received_file.jpg", std::ios::binary);
                    output_file.write(socket_data[sockfd].data(), socket_data[sockfd].size());
                    output_file.close();
					// End of data, send the response
					std::string headers = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(buffer.size()) + "\r\n\r\n";
					write(events[i].data.fd, headers.c_str(), headers.size());
					write(events[i].data.fd, buffer.data(), buffer.size());
					close(events[i].data.fd);
				}
			}
			/*
			else if (events[i].events & EPOLLIN) 
			{
				// Read incoming data
				char buffer[1024] = {0};
				int bytes_read = read(events[i].data.fd, buffer, sizeof(buffer) - 1);
				if (bytes_read <= 0) {
					// Close connection if read fails or end of data
					close(events[i].data.fd);
				} else {
					// Respond with the client's custom data or default data
					buffer[bytes_read] = '\0'; // Ensure null-terminated string
					std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(strlen(buffer)) + "\r\n\r\n" + buffer;
					write(events[i].data.fd, response.c_str(), response.size());
					close(events[i].data.fd);
				}
			}
			*/
        }
    }
    close(server_fd);
    close(epoll_fd);
    return 0;
}
