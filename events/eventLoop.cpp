#include "eventLoop.hpp"
#include "../epoll/epoll.hpp"
#include "../request/Request.hpp"
#include "../request/RequestHandler.hpp"
#include "../cgi/cgi_request.hpp"
#include "../response/Response.hpp"
#include "../sockets/socket.hpp"

/*int	acceptConnection(int fd, int epoll_fd)
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0)
	{
		std::cerr << RED << "Failed to accept connection: " << strerror(errno) << DEFAULT << "\n";
		return 1;
	}

	set_non_blocking(client_fd);
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = client_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
	{
		std::cerr << RED << "Failed to add client to epoll: " << strerror(errno) << DEFAULT << "\n";
		close(client_fd);
		return 1;
	}
	return 0;
}*/

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
        std::cerr << RED << "Failed to accept connection: " << strerror(errno) << DEFAULT << "\n";
        return -1;
    }

    // Aseta uusi client_fd ei-blokkaavaksi
    if (!set_non_blocking(client_fd))
    {
        std::cerr << RED << "Failed to set client_fd non-blocking: " << strerror(errno) << DEFAULT << "\n";
        close(client_fd);
        return -1;
    }

    // Lisää uusi client_fd epollin valvontaan
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET; // Lisää mahdollisesti EPOLLET (edge-triggered)
    event.data.fd = client_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
    {
        std::cerr << RED << "Failed to add client to epoll: " << strerror(errno) << DEFAULT << "\n";
        close(client_fd);
        return -1;
    }

    std::cout << GREEN << "Accepted new connection: client_fd = " << client_fd << DEFAULT << "\n";
    return client_fd;
}


void cleanupTempFiles()
{
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

LocationConfig findLocation(const std::string uri, const Socket &socket)
{
	LocationConfig location;
	if (uri == "")
		return location;
	std::vector<LocationConfig> locations;

	ServerConfig server = socket.getServer();
	locations = server.getLocations();
	for (std::vector<LocationConfig>::iterator it = locations.begin(); it != locations.end(); it++)
	{
		if (it->getLocation() == uri)
		{
			location = *it;
			break;
		}
	}
	return location;
}

int	handleClientData(int fd, Request &req, struct epoll_event &event, std::unordered_map<int, std::vector<char>> &client_data, const Socket &socket)
{
    std::cout << "RECEIVING DATA FROM FD: " << fd << std::endl;
	char buffer[4000] = {0};
	int bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_read < 0)
	{
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            std::cerr << "errno EAGAIN or EWOULDBLOCK\n";
            return 1;
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
        if (req.getState() == State::COMPLETE || req.getState() == State::ERROR)
        {
        	req.printRequest();
            if (req.getState() != State::ERROR)
            {
                std::string path = req.getUri();
                bool    cgi_req = (path.find("/cgi-bin/") != std::string::npos || (path.size() > 3 && path.substr(path.size() - 3) == ".py"));
                if (cgi_req)
                {
					std::cout << "THE IMAGE IS CGI" << std::endl;
                    std::string queryString = findQueryStr(req.getUri());
                    std::string directPath;
                    directPath = findPath(req.getUri());
                    std::cout << "DIRECT PATH: " << directPath << std::endl;
                    cgiRequest cgireg(directPath, req.getMethod(), queryString, req.getVersion(), req.getBody());
                    int execute_result = cgireg.execute(); // exit status so we need to give correct error page so current one is broken
                    if (execute_result == 0)
                        req.setPath("/cgi_output.html");
					// else
					// {

					// }
                }
            }
            //req.printRequest();
		    Response res;
            RequestHandler requestHandler;
		    requestHandler.handleRequest(req, res);
			std::cout << "URI: " << req.getUri() << std::endl;
			LocationConfig location = findLocation(req.getUri(), socket);
			//print allowe methods
			std::vector<std::string> allow_methods = location.getAllowMethods();
			for (std::vector<std::string>::const_iterator it = allow_methods.begin(); it != allow_methods.end(); it++)
			{
				std::cout << "ALLOWED METHOD: " << *it << std::endl;
			}
			if (location.getLocation() != "")
			{
				std::cout << "FOUND LOCATION: " << location.getLocation() << std::endl;
				std::cout << "ROOT: " << location.getRoot() << std::endl;
			}
            //res.printResponse();
		    // Get the full HTTP response string from the Response class
		    std::string http_response = res.getResponseString();
		    // Send the response back to the client

            // std::cout << http_response.length() << "lenght here!!!\n";
            res.printResponse();
            size_t total_sent = 0;
            size_t message_length = http_response.length();
            const char *message_ptr = http_response.c_str();
            while (total_sent < message_length)
            {
                ssize_t bytes_sent = send(event.data.fd, message_ptr + total_sent, message_length - total_sent, 0);
                if (bytes_sent < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // Socket is not ready, could add a delay or handle as needed
                        continue;
                    } else {
                        perror("send");
                        close(event.data.fd);
                        break;
                    }
                }
                total_sent += bytes_sent;
            }
            std::cout << "RESPONSE SENT" << std::endl;
            req.reset();
		    close(event.data.fd);
        }
		cleanupTempFiles();
	}
	return 0;
}

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
            std::cerr << RED << "epoll_wait failed" << DEFAULT << std::endl;
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
				{
					std::cout << "ACCEPTED CONNECTION ON PORT " << it->getPort() << std::endl;

					// Tallenna asiakas-socketin ja server-socketin välinen yhteys
					if (client_to_server_map.find(new_fd) == client_to_server_map.end())
					{
						client_to_server_map[new_fd] = fd;
						std::cout << "Mapped client_fd: " << new_fd << " to server_fd: " << fd << std::endl;
					}
					else
					{
						std::cerr << "client_fd: " << new_fd << " already exists in map." << std::endl;
					}

					// Lisää uusi fd requests- ja client_data-karttoihin
					requests[new_fd] = Request();
					client_data[new_fd] = std::vector<char>();
				}
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
                    }
                    else
                    {
                        std::cerr << "Failed to find server socket for client_fd: " << fd << std::endl;
                    }
                }
                else
				{
					std::cerr << "Unknown client_fd: " << fd << " - closing connection." << std::endl;
					for (const auto &entry : client_to_server_map)
					{
						std::cout << "client_fd: " << entry.first << ", server_fd: " << entry.second << std::endl;
					}
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
					close(fd);
					requests.erase(fd);
					client_data.erase(fd);
				}
            }
        }
    }
}

/*void	event_loop(const std::vector<Socket> &sockets, int epoll_fd)
{
	struct epoll_event events[MAX_EVENTS];
	std::unordered_map<int, Request> requests;
	std::unordered_map<int, std::vector<char>> client_data;

	while (true)
	{
		int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1)
		{
            std::cerr << RED << "epoll_wait failed" << DEFAULT << std::endl;
            break;
        }
		for (int i = 0; i < num_events; ++i)
		{
			int fd = events[i].data.fd;
			Request &req = requests[fd];

			auto it = std::find_if(sockets.begin(), sockets.end(),
						[fd](const Socket &sock) { return sock.getSocketFd() == fd; });
			if (it != sockets.end())
			{
				if (acceptConnection(fd, epoll_fd) == 0)
					std::cout << "ACCEPTED CONNECTION ON PORT " << it->getPort() << std::endl;
				continue;
			}
			else if (events[i].events & EPOLLIN)
			{
				handleClientData(fd, req, events[i], client_data);
			}
		}
	}
}*/
