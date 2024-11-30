#ifndef EPOLL_HPP
#define EPOLL_HPP

#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include "../parsing/ServerConfig.hpp"
#include <sys/epoll.h>

class Socket;

int set_non_blocking(int sockfd);
void cleanup(std::vector<Socket> &sockets, int epoll_fd);
int	setup_epoll(std::vector<Socket> &sockets);

#endif
