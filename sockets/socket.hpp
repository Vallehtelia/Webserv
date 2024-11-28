
#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <vector>
#include "../parsing/ServerConfig.hpp"

class	Socket
{
	private:
		int			_port;
		std::string	_ip;
		int			_socket_fd;
		bool		_active;
	public:
		Socket(int port, std::string host);
		Socket(Socket const &other);
		Socket &operator=(Socket const &other);
		~Socket();

		void	setPort(int	port);
		void	setIp(std::string ip);
		void	setSocketFd(int socketFd);
		void	setActiveMode(bool mode);

		int			getPort() const;
		std::string	getIp() const;
		int			getSocketFd() const;
		bool			getActiveMode() const;
};

bool set_non_blocking(int sockfd);
bool	initSocket(std::vector<ServerConfig> &server, std::vector<Socket> &sockets);

#endif
