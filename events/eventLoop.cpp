#include "eventLoop.hpp"
#include "../epoll/epoll.hpp"
#include "../request/Request.hpp"
#include "../request/RequestHandler.hpp"
#include "../response/Response.hpp"
#include "../sockets/socket.hpp"

int acceptConnection(int fd, int epoll_fd)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Yritä hyväksyä uusi yhteys
    int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0)
    {
    	if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Ei uusia yhteyksiä juuri nyt, tämä ei ole kriittinen virhe
            return -1;
        }
        std::cerr << RED << "Failed to accept connection." << DEFAULT << "\n";
        return -1;
    }

    // Aseta uusi client_fd ei-blokkaavaksi
    if (!set_non_blocking(client_fd))
    {
        std::cerr << RED << "Failed to set client_fd non-blocking: " << client_fd << DEFAULT << "\n";
        close(client_fd);
        return -1;
    }

    // Lisää uusi client_fd epollin valvontaan
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET; // Lisää mahdollisesti EPOLLET (edge-triggered)
	// Caution: In edge-triggered mode, you must process all pending data
	// until recv() or send() returns EAGAIN. Onks meilla nain?
    event.data.fd = client_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
    {
        std::cerr << RED << "Failed to add client to epoll: " << client_fd << DEFAULT << "\n";
        close(client_fd);
        return -1;
    }

    std::cout << GREEN << "Accepted new connection: client_fd = " << client_fd << DEFAULT << "\n";
    return client_fd;
}

/*
* @brief Remove temporary cgi files
*/
void cleanupTempFiles()
{
	std::string tempInFilePath = "./html/tmp/cgi_output.html";
	std::string tempOutFilePath = "./html/tmp/cgi_input.html";
	if (std::ifstream(tempInFilePath))
	{
		if (std::remove(tempInFilePath.c_str()) != 0)
			std::cerr << "Failed to delete temp file: " << tempInFilePath << std::endl;
	}
	if (std::ifstream(tempOutFilePath))
	{
		if (std::remove(tempOutFilePath.c_str()) != 0)
			std::cerr << "Failed to delete temp file: " << tempOutFilePath << "\n";
	}
}

/*
* @brief Close file descriptor if open
*
* @param fd File descriptor to close
*/
static void	closeOnce(int &fd)
{
	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}
}

/*
* @brief Send data to the client and handle timeouts
*
* @param httpRespose HTTP response to send
* @param event epoll_event object
*/
static void	sendData(std::string httpRespose, epoll_event &event)
{
	size_t		totalSent = 0;
	size_t		messageLength = httpRespose.length();
	const char	*messagePtr = httpRespose.c_str();
	int			maxRetries = 10; // testissa vaan ettei jaa infinite looppiin lahettamaan vaan aiheuttaa sitten timeoutin.
	int			retries = 0;
	
	while (totalSent < messageLength) // tankin pitais toimii edgetriggered modessa
	{
		size_t	bytesSent = send(event.data.fd, messagePtr + totalSent, messageLength - totalSent, 0);
		
		if (bytesSent > 0)
		{
			// Successfully sent some bytes
			totalSent += bytesSent;
			retries = 0; // Reset retries on successful send
		}
		else if (bytesSent == 0)
		{
			// No data was sent; retry until the maximum retries are reached
			if (++retries >= maxRetries)
			{
				std::cerr << RED << "Send timed out after multiple retries" << DEFAULT << std::endl;
				closeOnce(event.data.fd);
				return;
			}
		}
		else // bytesSent < 0
		{
			// Treat all failures as temporary; let epoll retry
			std::cerr << YELLOW << "Temporary send failure, deferring to EPOLLOUT event." << DEFAULT << std::endl;
			return; // Exit and wait for the next EPOLLOUT
		}
	}
	std::cout << "Response successfully sent to client." << std::endl;
}

/*
* @brief Check if the received data is valid or end of file
*
* @param fd File descriptor of the client socket
* @param bytesRead Number of bytes read
* @param clientData Map of client file descriptors to data
* @return 1 if the data is invalid or end of file, 0 otherwise
*/
static int	checkReceivedData(int &fd, int bytesRead, std::unordered_map<int, std::vector<char>> &clientData)
{
	if (bytesRead == 0)
	{
		// You don't really need to check the errno here
		// Connection closed by the client
		std::cerr << "Connection closed by client (fd: " << fd << ")" << std::endl;;
		closeOnce(fd);
		clientData.erase(fd);
		return 1;
	}
	else // bytesRead < 0
	{
		// Generic failure case, handle as an unspecified error and wait for EPOLLIN to trigger again
		std::cerr << RED << "Recv failed for fd: " << fd << DEFAULT << std::endl;
		// closeOnce(fd);
		// clientData.erase(fd);
		return 1;
	}
}

/*
* @brief Check if the request is a CGI request
*
* @param path Path of the request
* @return true if the request is a CGI request, false otherwise
*/
// static bool	isCgi(const std::string &path)
// {
// 	return (path.find("/cgi-bin/") != std::string::npos || (path.size() > 3 && path.substr(path.size() - 3) == ".py"));
// }

/*
* @brief Handles the client data and sends the response back to the client
*
* @param fd File descriptor of the client socket
* @param req Request object
* @param event epoll_event object
* @param client_data Map of client file descriptors to data
* @param socket Server socket
* @return int 0 on success, -1 on failure
*/
int	handleClientData(int fd, Request &req, struct epoll_event &event, std::unordered_map<int, std::vector<char>> &client_data, const Socket &socket)
{
    std::cout << "RECEIVING DATA FROM FD: " << fd << std::endl;
	while (true) // should work with edge-triggered mode, since we read all data until recv() returns -1
	{
		char buffer[4000] = {0};
		int bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
		if (bytes_read <= 0)
		{
			checkReceivedData(fd, bytes_read, client_data);
			return -1;
			// temp errors not checked, just close the connection
			// let epoll retry the recv
		}
		else
		{
			std::string rawRequest(buffer, bytes_read);
			req.parseRequest(rawRequest, socket);

			if (req.getState() == State::INCOMPLETE)
				continue;
			
			if (req.getState() == State::COMPLETE || req.getState() == State::ERROR)
			{
				req.printRequest();
				if (req.getState() != State::ERROR)
				{
					if (req.getLocation().getLocation() == "/cgi")
						handleCgiRequest(req, socket);
				}
				//req.printRequest();
				Response res(socket);
				// std::cout << "URI FROM EVENT LOOP: " << req.getUri() << std::endl;
				RequestHandler requestHandler;
				requestHandler.handleRequest(req, res);
				res.printResponse();
				// Get the full HTTP response string from the Response class
				std::string http_response = res.getResponseString();
				// Send the response back to the client
				// std::cout << http_response.length() << "lenght here!!!\n";
				// res.printResponse();
				sendData(http_response, event);
				std::cout << "RESPONSE SENT" << std::endl;
				req.reset();
				closeOnce(event.data.fd);
				break;
			}
			cleanupTempFiles();
		}
	}
	return 0;
}

/*
* @brief Add new file descriptor to client_to_server_map
*
* @param client_to_server_map Map of client to server file descriptors that are connected
* @param fd File descriptor of the server socket
* @param new_fd File descriptor of the new client socket
* @param requests Map of client file descriptors to Request objects
* @param client_data Map of client file descriptors to data
*/
static void	addNewFd(std::unordered_map<int, int> &client_to_server_map,
						const int &fd,
						const int &new_fd,
						std::unordered_map<int, Request> &requests,
						std::unordered_map<int, std::vector<char>> &client_data)
{
	if (client_to_server_map.find(new_fd) == client_to_server_map.end())
		client_to_server_map[new_fd] = fd;
	else
		std::cerr << RED << "client_fd: " << new_fd << " already exists in map" << DEFAULT << std::endl;
	requests[new_fd] = Request();
	client_data[new_fd] = std::vector<char>();
}

/*
* @brief Cleanup client connection
*
* @param fd File descriptor to close
* @param epoll_fd File descriptor for epoll
* @param client_to_server_map Map of client to server file descriptors
* @param requests Map of client file descriptors to Request objects
* @param client_data Map of client file descriptors to data
*/
static void	cleanupClientConnection(int fd,
									int epoll_fd,
									std::unordered_map<int, int> &client_to_server_map,
									std::unordered_map<int, Request> &requests,
									std::unordered_map<int, std::vector<char>> &client_data)
{
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
	closeOnce(fd);
	client_to_server_map.erase(fd);
	requests.erase(fd);
	client_data.erase(fd);
}

/*
* @brief Event loop for handling incoming connections and data
*
* @param sockets Vector of server sockets
* @param epoll_fd File descriptor for epoll
*/
void event_loop(const std::vector<Socket> &sockets, int epoll_fd)
{
    struct epoll_event events[MAX_EVENTS];
    std::unordered_map<int, Request> requests;
    std::unordered_map<int, std::vector<char>> client_data;
    std::unordered_map<int, int> client_to_server_map; // Asiakas-socket -> Server-socket

    while (true)
    {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1)
        {
            std::cerr << RED << "Error: epoll_wait failed" << DEFAULT << std::endl;
            break;
        }

        for (int i = 0; i < num_events; ++i)
        {
            int fd = events[i].data.fd;

            // 1. Tarkista, onko fd server-socket
            auto it = std::find_if(sockets.begin(), sockets.end(),
                                   [fd](const Socket &sock) { return sock.getSocketFd() == fd; });

            if (it != sockets.end()) // Löytyikö server-socket
            {
                int new_fd = acceptConnection(fd, epoll_fd);
                if (new_fd > 0)
					addNewFd(client_to_server_map, fd, new_fd, requests, client_data);
                continue;
            }

            // 2. EPOLLIN-tapahtuma: Käsittele asiakasdataa
            if (events[i].events & EPOLLIN)
            {
                // Etsi asiakassocketin hyväksynyt server-socket
                auto server_fd_it = client_to_server_map.find(fd);
                if (server_fd_it != client_to_server_map.end())
                {
                    int server_fd = server_fd_it->second;

                    // Etsi oikea server-socket
                    auto server_socket_it = std::find_if(sockets.begin(), sockets.end(),
                                                         [server_fd](const Socket &sock) {
                                                             return sock.getSocketFd() == server_fd;
                                                         });

                    if (server_socket_it != sockets.end()) // Löytyi oikea server-socket
                    {
                        const Socket &socket = *server_socket_it;
                        handleClientData(fd, requests[fd], events[i], client_data, socket);
						client_to_server_map.erase(fd);
                    }
                    else
                    {
                        std::cerr << "Failed to find server socket for client_fd: " << fd << std::endl;
						cleanupClientConnection(fd, epoll_fd, client_to_server_map, requests, client_data);
                    }
                }
                else
				{
					std::cerr << RED << "Unknown client_fd: " << fd << " - closing connection." << DEFAULT << std::endl;
					for (const auto &entry : client_to_server_map)
					{
						std::cout << "client_fd: " << entry.first << ", server_fd: " << entry.second << std::endl;
					}
					cleanupClientConnection(fd, epoll_fd, client_to_server_map, requests, client_data);
				}
            }
        }
    }
}
