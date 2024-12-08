#include "./sockets/socket.hpp"
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
#include <vector>
#include <map>
#include <unordered_map>
#include "./parsing/ConfigValidator.hpp"
#include "./parsing/ServerConfig.hpp"
#include "./parsing/config.hpp"
#include "./epoll/epoll.hpp"
#include "request/Request.hpp"
#include "request/RequestHandler.hpp"
#include "response/Response.hpp"
#include "./request/cgi_request.hpp"
#include "utils.hpp"
#include "./events/eventLoop.hpp"
#include <csignal>

// #define PORT 8002 // ja taa

std::vector<Socket> sockets;
int epoll_fd = -1;
bool running = true;

void	sig_handler(int signum)
{
	std::cerr << RED << "Caught signal " << signum << DEFAULT << std::endl;

	if (epoll_fd != -1)
	{
		cleanup(sockets, epoll_fd);
		std::cerr << RED << "Cleaned up sockets and epoll" << DEFAULT << std::endl;
	}
	running = false;
}

int	main(int ac, char **av)
{
	signal(SIGINT, sig_handler);
    std::vector<ServerConfig>	server; // Taa sisaltaa kaiken tiedon, Server name, port, host, root, client bodysize, index path, error pages in a map, locations in vector.

    if (ac != 2)
	{
		std::cerr << "Error: wrong number of arguments" << std::endl;
		return 1;
	}

	if (!initConfig(av[1], server))
		return 1;

	if (initSocket(server, sockets) == false)
		return 1;
	epoll_fd = setup_epoll(sockets);
	if (epoll_fd == -1)
	{
		cleanup(sockets, epoll_fd);
		return 1;
	}

	while (running)
		event_loop(sockets, epoll_fd);

	cleanup(sockets, epoll_fd);
    return 0;
}
