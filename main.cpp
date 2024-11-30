#include "./sockets/Socket.hpp"
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
#include "./cgi/cgi_request.hpp"
#include "utils.hpp"
#include "./events/eventLoop.hpp"

// #define PORT 8002 // ja taa

int	main(int ac, char **av)
{
    std::vector<ServerConfig>	server; // Taa sisaltaa kaiken tiedon, Server name, port, host, root, client bodysize, index path, error pages in a map, locations in vector.
	int epoll_fd;

    if (ac != 2)
	{
		std::cerr << "Error: wrong number of arguments" << std::endl;
		return 1;
	}

	std::cout << "Starting webserver. Please hold..." << std::endl;
	if (!initConfig(av[1], server))
		return 1;

    std::vector<Socket> sockets;
	if (initSocket(server, sockets) == false)
		return 1;

	epoll_fd = setup_epoll(sockets);
	if (epoll_fd == -1)
	{
		cleanup(sockets, epoll_fd);
		return 1;
	}

	event_loop(sockets, epoll_fd);

    // Close the other sockets
	cleanup(sockets, epoll_fd);
    return 0;
}
