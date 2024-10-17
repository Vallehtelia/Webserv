#include "socket.hpp"
#include "./parsing/ServerConfig.hpp"

Socket::Socket(std::string port, std::string host) : _socket_fd(0), _active(false)
{
	this->_port = std::stoi(port);
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
