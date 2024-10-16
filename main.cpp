#include "socket.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <map>
#include "request/Request.hpp"
#include "response/Response.hpp"


int main() {
    Socket socket1;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t client_len;
    int client_fd;
    char buffer[1024];
    size_t bytes_received;

    // Configure server address and port
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Create socket
    socket1.setSocketFd(socket(AF_INET, SOCK_STREAM, 0));
    if (socket1.getSocketFd() <= 0) {
        std::cout << "Failed to set socket: " << strerror(errno) << "\n";
        return 1;
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(socket1.getSocketFd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cout << "setsockopt failed: " << strerror(errno) << "\n";
        return 1;
    }

    // Bind socket to address and port
    if (bind(socket1.getSocketFd(), (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Failed to bind: " << strerror(errno) << "\n";
        return 1;
    }

    if (listen(socket1.getSocketFd(), 10) < 0) {
        std::cout << "Failed to listen: " << strerror(errno) << "\n";
        close(socket1.getSocketFd());
        return 1;
    }

    std::cout << "Server is listening on port 8080...\n";

    while (true) {
        // Accept incoming connection
        client_len = sizeof(client_addr);
        client_fd = accept(socket1.getSocketFd(), (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cout << "Failed to create client fd: " << strerror(errno) << "\n";
            close(socket1.getSocketFd());
            return 1;
        }

        bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0) {
            std::cout << "Failed to receive: " << strerror(errno) << "\n";
            close(socket1.getSocketFd());
            return 1;
        }
        buffer[bytes_received] = '\0';
        std::cout << buffer << std::endl;

        Request req(buffer);
        req.parse(buffer);
        std::cout << "Received request:\n" << std::endl;
        std::cout << "method: " << req.getMethod() << std::endl;
        std::cout << "path: " << req.getPath() << std::endl;
        std::cout << "version: " << req.getVersion() << std::endl;
        std::cout << "body: " << req.getBody() << std::endl;
        std::cout << "---------------" << std::endl;

        std::cout << "Serving file: " << req.getPath() << std::endl;

        Response res;
        res.createResponse(req);
        // Let the Response class handle everything

        // Get the full HTTP response string from the Response class
        std::string http_response = res.getResponseString();
        // Send the response back to the client
        if (send(client_fd, http_response.c_str(), http_response.length(), 0) < 0) {
            std::cout << "Failed to send: " << strerror(errno) << "\n";
            close(client_fd);
            close(socket1.getSocketFd());
            return 1;
        }
        close(client_fd);
    }

    // Close server socket
    close(socket1.getSocketFd());
    return 0;
}
