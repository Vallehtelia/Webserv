#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <cstdio>
#include "../request/Request.hpp"

#include "../sockets/socket.hpp"


class Response {
public:
    Response(const Socket &socket);
    ~Response();

    void        printResponse();
    void        setResponse(int statusCode, const std::string& contentType, 
					const std::string &body, const std::vector<std::string>& cookies = {});
    void        setStatusLine(std::string &statusline);
    void        setBody(std::string &body);
    void        setHeaders(std::string &headers);
    void        setUri(std::string &URI);
    std::string getResponseString() const;
	std::string printResponseString() const;
private:
    int         _statusCode;
    int         _contentLength;
    std::string _statusLine;
    std::string _headers;
    std::string _body;
    std::string _uri;
    std::string _contentType;

    void        setStatusLine();
    void        setError();
    std::string getErrorPage() const;
    std::string readFileContent(std::string& filePath);
    Socket      _socket;
};

#endif
