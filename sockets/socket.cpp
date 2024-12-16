#include "./socket.hpp"
#include "../epoll/epoll.hpp"
#include <chrono>
#include <thread>

extern bool running;

Socket::Socket(int port, std::string host) : _socket_fd(0), _active(false), _server()
{
	this->_port = port;
	this->_ip = host;
}

Socket::Socket(Socket const &other)
{
	this->_port = other._port;
	this->_ip = other._ip;
	this->_socket_fd = other._socket_fd;
	this->_active = other._active;
	this->_server = other._server;
}

Socket &Socket::operator=(Socket const &other)
{
	this->_port = other._port;
	this->_ip = other._ip;
	this->_socket_fd = other._socket_fd;
	this->_active = other._active;
	this->_server = other._server;
	return (*this);
}

Socket::~Socket() {}

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

void	Socket::setServer(ServerConfig server)
{
	this->_server = server;
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

ServerConfig	Socket::getServer() const
{
	return (this->_server);
}

bool	initSocket(std::vector<ServerConfig> &server, std::vector<Socket> &sockets)
{
	for (const auto &config : server)
    {
		Socket sock(config.getListenPort(), config.getHost());

		sock.setSocketFd(socket(AF_INET, SOCK_STREAM, 0));
		if (sock.getSocketFd() <= 0)
		{
			std::cerr << RED << "Failed to set socket: " << strerror(errno) << DEFAULT << "\n";
			cleanup(sockets, -1);
			return false;
		}
		if (set_non_blocking(sock.getSocketFd()) == false)
		{
			close(sock.getSocketFd());
			cleanup(sockets, -1);
			return false;
		}

		struct sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(sock.getPort());
		addr.sin_addr.s_addr = inet_addr(sock.getIp().c_str());

		int timeoutMaxSec = 60;
		// try looping bind for 60 seconds and sleep for 1 second on fail using c++ chrono
		auto start = std::chrono::system_clock::now();
		while (bind(sock.getSocketFd(), (struct sockaddr*)&addr, sizeof(addr)) < 0 && running)
		{
			auto end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end - start;
			// print elapsed time in int form
			std::cerr << CLEAR_TERMINAL << RED << "Failed to bind due timeout, waiting for successful bind max 1min" << DEFAULT << "\n";
			std::cout << "Elapsed time: " << static_cast<int>(elapsed_seconds.count()) << " seconds" << std::endl;
			if (elapsed_seconds.count() >= timeoutMaxSec)
			{
				std::cerr << RED << "Failed to bind: " << strerror(errno) << " or having timeout" " on port: " << sock.getPort() << DEFAULT << "\n";
				close(sock.getSocketFd());
				cleanup(sockets, -1);
				return false;
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		// if (bind(sock.getSocketFd(), (struct sockaddr*)&addr, sizeof(addr)) < 0)
		// {
		// 	std::cerr << RED << "Failed to bind: " << strerror(errno) << " or having timeout" " on port: " << sock.getPort() << DEFAULT << "\n";
		// 	close(sock.getSocketFd());
		// 	cleanup(sockets, -1);
		// 	return false;
		// }

		if (listen(sock.getSocketFd(), SOMAXCONN) < 0) // SOMAXCONN is the system defined maximum for the backlog
		{
			std::cerr << RED << "Failed to listen: " << strerror(errno) << DEFAULT << "\n";
			close(sock.getSocketFd());
			cleanup(sockets, -1);
			return false;
		}
		sock.setServer(config);
		sockets.push_back(sock);
	}
	return true;
}
