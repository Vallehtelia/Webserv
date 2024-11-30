#include <sys/resource.h>

#include "./Socket.hpp"
#include "../epoll/epoll.hpp"

Socket::Socket(int port, std::string host) : _socket_fd(0), _active(false)
{
	this->_port = port;
	this->_ip = host;
	std::cout << "Socket default constructor\n";
}

Socket::Socket(Socket const &other)
{
	std::cout << "Socket copy constructor\n";
	this->_port = other._port;
	this->_ip = other._ip;
	this->_socket_fd = other._socket_fd;
	this->_active = other._active;
}

Socket &Socket::operator=(Socket const &other)
{
	this->_port = other._port;
	this->_ip = other._ip;
	this->_socket_fd = other._socket_fd;
	this->_active = other._active;
	return (*this);
}

Socket::~Socket()
{
	std::cout << "Socket destroyed.\n";
}

void	Socket::setPort(int	port)
{
	this->_port = port;
}

void	Socket::setIp(std::string ip)
{
	this->_ip = ip;
}

void	Socket::setSocketFd(int socketFd)
{
	this->_socket_fd = socketFd;
}

void	Socket::setActiveMode(bool mode)
{
	this->_active = mode;
}

int	Socket::getPort() const
{
	return (this->_port);
}

std::string	Socket::getIp() const
{
return (this->_ip);
}

int	Socket::getSocketFd() const
{
return (this->_socket_fd);
}

bool	Socket::getActiveMode() const
{
return (this->_active);
}


void increaseFileDescriptorLimit(int new_limit) {
    struct rlimit limit;

    // Get current limits
    if (getrlimit(RLIMIT_NOFILE, &limit) == 0) {
        std::cout << "Current limits: soft=" << limit.rlim_cur << ", hard=" << limit.rlim_max << "\n";

        // Update the soft limit to the desired value, within the hard limit
        limit.rlim_cur = std::min(static_cast<rlim_t>(new_limit), limit.rlim_max);

        if (setrlimit(RLIMIT_NOFILE, &limit) == 0) {
            std::cout << "New limits: soft=" << limit.rlim_cur << ", hard=" << limit.rlim_max << "\n";
        } else {
            std::cerr << "Error: Failed to set new file descriptor limit" << std::endl;
        }
    } else {
        std::cerr << "Error: Failed to get file descriptor limit" << std::endl;
    }
}

bool	initSocket(std::vector<ServerConfig> &server, std::vector<Socket> &sockets)
{
	for (const auto &config : server)
    {
		std::cout << "Creating socket on port " << config.getListenPort() << " and host " << config.getHost() << "...\n";
		Socket sock(config.getListenPort(), config.getHost());

		sock.setSocketFd(socket(AF_INET, SOCK_STREAM, 0));
		if (sock.getSocketFd() <= 0)
		{
			std::cerr << RED << "Failed to set socket: " << strerror(errno) << DEFAULT << "\n";
			cleanup(sockets, -1);
			return false;
		}
		int snb = set_non_blocking(sock.getSocketFd());
		if (snb > 0)
		{
			std::cerr << RED << "Failed to set non-blocking: (" << snb << ") " << strerror(errno) << DEFAULT << "\n";
			close(sock.getSocketFd());
			cleanup(sockets, -1);
			return false;
		}

		struct sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(sock.getPort());
		addr.sin_addr.s_addr = inet_addr(sock.getIp().c_str());

		if (bind(sock.getSocketFd(), (struct sockaddr*)&addr, sizeof(addr)) < 0)
		{
			std::cerr << RED << "Failed to bind: " << strerror(errno) << " On port: " << sock.getPort() << DEFAULT << "\n";
			close(sock.getSocketFd());
			cleanup(sockets, -1);
			return false;
		}
		std::cout << "System max listen backlog (somaxconn) is : " << SOMAXCONN << std::endl;
		increaseFileDescriptorLimit(SOMAXCONN);
		if (listen(sock.getSocketFd(), SOMAXCONN) < 0)
		{
			std::cerr << RED << "Failed to listen: " << strerror(errno) << DEFAULT << "\n";
			close(sock.getSocketFd());
			cleanup(sockets, -1);
			return false;
		}
		sockets.push_back(sock);
	}
	return true;
}
