#include "eventLoop.hpp"
#include "../epoll/epoll.hpp"
#include "../request/Request.hpp"
#include "../request/RequestHandler.hpp"
#include "../cgi/cgi_request.hpp"
#include "../response/Response.hpp"
#include "../sockets/Socket.hpp"

int	acceptConnection(int fd, int epoll_fd)
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


int	handleClientData(int fd, Request &req, struct epoll_event &event, std::unordered_map<int, std::vector<char>> &client_data)
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
			res.printResponse();
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

void	event_loop(const std::vector<Socket> &sockets, int epoll_fd)
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
				handleClientData(fd, req, events[i], client_data);
		}
	}
}
