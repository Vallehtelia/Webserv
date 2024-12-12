#include "./socket.hpp"
#include "../epoll/epoll.hpp"

Socket::Socket(int port, std::string host) : _socket_fd(0), _active(false), _server()
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
		std::cout << "Creating socket on port " << config.getListenPort() << " and host " << config.getHost() << "...\n";
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

		if (bind(sock.getSocketFd(), (struct sockaddr*)&addr, sizeof(addr)) < 0)
		{
			std::cerr << RED << "Failed to bind: " << strerror(errno) << " On port: " << sock.getPort() << DEFAULT << "\n";
			close(sock.getSocketFd());
			cleanup(sockets, -1);
			return false;
		}

		//std::cout << "SOMAXCONN: " << SOMAXCONN << std::endl;
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
