#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080

int main(int argc, char const **argv) 
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported\n";
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return -1;
    }

	if (argc <= 1) 
	{
		// Send a basic HTTP GET request
		const char *request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
		send(sock, request, strlen(request), 0);
		std::cout << "HTTP request sent\n";
	} 
	else 
	{
		// Create a custom HTTP GET request using std::string
		std::string custom_request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" + std::string(argv[1]);

		// Convert the std::string to a char array for sending
		const char *request = custom_request.c_str();
		send(sock, request, strlen(request), 0);
		std::cout << "Custom HTTP request sent with data: " << argv[1] << "\n";
	}

    // Read response from server
    int valread = read(sock, buffer, 1024);
    std::cout << "Server response:\n" << buffer << std::endl;

    // Close the socket
    close(sock);

    return 0;
}
