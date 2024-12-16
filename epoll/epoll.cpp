
#include "./epoll.hpp"
#include "../sockets/socket.hpp"

bool set_non_blocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << RED << "Error: receiving fcntl flags, fd: " << sockfd << DEFAULT << std::endl;
        return false;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
        std::cerr << RED << "Error: setting fcntl flags, fd: " << sockfd << DEFAULT << std::endl;
        return false;
    }
	return true;
}

void cleanup(std::vector<Socket> &sockets, int epoll_fd)
{
	for (const auto &sock : sockets)
		close(sock.getSocketFd());
	if (epoll_fd != -1) // valgrind warning
		close(epoll_fd);
}

int	setup_epoll(std::vector<Socket> &sockets)
{
	int epoll_fd = epoll_create(1);
    if (epoll_fd == -1) {
        std::cerr << RED << "epoll_create failed!" << DEFAULT << std::endl;
        return -1;
    }

	for (const auto &sock : sockets)
	{
		epoll_event event = {};
		event.events = EPOLLIN;
		event.data.fd = sock.getSocketFd();
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock.getSocketFd(), &event) == -1)
		{
			std::cerr << RED << "Error: epoll_ctl failed!" << DEFAULT << std::endl;
			close(epoll_fd);
			return -1;
		}

		std::cout << CYAN << "Server is listening on port " << sock.getPort() << "...\n";
		std::cout << "open " << sock.getIp() << ":" << sock.getPort() << " on browser" << DEFAULT << std::endl;
	}
	return epoll_fd;
}
