
#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>

class	Socket
{
	private:
		int			_port;
		std::string	_ip;
		int			_socket_fd;
		bool		_active;
	public:
		Socket(std::string port, std::string host);
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

void set_non_blocking(int sockfd);

#endif
