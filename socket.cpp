#include "socket.hpp"

Socket::Socket() : _port(8080), _ip("69.69.69.1"), _socket_fd(0), _active(false)
{
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
