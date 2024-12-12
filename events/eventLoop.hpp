#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <map>
#include <unordered_map>
#include <vector>

#define MAX_EVENTS 1024

class Request;
class Socket;

int	acceptConnection(int fd, int epoll_fd);
void cleanupTempFiles();
int	handleClientData(int fd, Request &req, struct epoll_event &event, std::unordered_map<int, std::vector<char>> &client_data, const Socket &socket);
void	event_loop(const std::vector<Socket> &sockets, int epoll_fd);

#endif
