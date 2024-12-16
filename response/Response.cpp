#include "Response.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include "../request/Request.hpp"

Response::Response(const Socket &socket) : _socket(socket)  {}
Response::~Response() {}


std::string Response::getResponseString() const
{
    return _statusLine + _headers + "\r\n" + _body;
}

// Excluding body when printing bcs Docker prints binary data
std::string Response::printResponseString() const
{
    return _statusLine + _headers + "\r\n" + "<body excluded>";
}

void Response::printResponse()
{
    std::cout << std::endl;
    std::cout << "RESPONSE:" << "\033[32m" << std::endl;
    std::cout << printResponseString() << std::endl;
    std::cout << "\033[0m" << std::endl;
}

void Response::setStatusLine(std::string &statusline)
{
    _statusLine = statusline;
}

void Response::setHeaders(std::string &headers)
{
    _headers = headers;
}

void Response::setBody(std::string &body)
{
    _body = body;
}

void Response::setUri(std::string &URI)
{
    _uri = URI;
}

// works with or without cookies
void Response::setResponse(int statusCode, const std::string& contentType,
		const std::string &body, const std::vector<std::string>& cookies) {
    _body = body;
    _statusCode = statusCode;
    setStatusLine();
    _contentLength = _body.size();
    _contentType = contentType;
    if (_statusCode != 200)
    {
        setError();
        _contentLength = _body.size();
    }
    _headers = "Content-Type: " + _contentType + "\r\n" +
              "Content-Length: " + std::to_string(_contentLength) + "\r\n" +
              "Access-Control-Allow-Origin: *" + "\r\n" +
              "Access-Control-Allow-Methods: GET, POST, OPTIONS" + "\r\n" +
              "Access-Control-Allow-Headers: Content-Type" + "\r\n";

	// Add cookies as Set-Cookie headers
    for (const std::string& cookie : cookies)
    {
        _headers += "Set-Cookie: " + cookie + "\r\n";
    }
}

void Response::setStatusLine()
{
    switch (_statusCode) {
        case 200:
            _statusLine = "HTTP/1.1 200 OK\r\n";
            break;
        case 201:
            _statusLine = "HTTP/1.1 201 Created\r\n";
            break;
        case 400:
            _statusLine = "HTTP/1.1 400 Bad Request\r\n";
            break;
        case 403:
            _statusLine = "HTTP/1.1 403 Forbidden\r\n";
            break;
        case 404:
            _statusLine = "HTTP/1.1 404 Not Found\r\n";
            break;
        case 405:
            _statusLine = "HTTP/1.1 405 Method Not Allowed\r\n";
            break;
        case 409:
            _statusLine = "HTTP/1.1 409 Conflict\r\n";
            break;
		case 500:
			_statusLine = "HTTP/1.1 500 Internal Server Error\r\n";
			break;
        default:
            _statusLine = "HTTP/1.1 504 Gateway Timeout\r\n";
            break;
    }
}


std::string Response::getErrorPage() const
{
    switch (_statusCode) {
        case 400:
            return "./website/html/error_pages/400.html";
        case 403:
            return "./website/html/error_pages/403.html";
        case 404:
            return "./website/html/error_pages/404.html";
        case 405:
            return "./website/html/error_pages/405.html";
        case 409:
            return "./website/html/error_pages/409.html";
		case 500:
			return "./website/html/error_pages/500.html";
        default:
            return "./website/html/error_pages/504.html";
    }
}

void    Response::setError()
{
    std::string errorPage =  _socket.getServer().getErrorPage(_statusCode);
    std::cout << "fetching error page: " << errorPage << std::endl;
	_body = readFileContent(errorPage);
}

Socket		Response::getSocket() const
{
	return (this->_socket);
}

std::string Response::readFileContent(std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (file) {
        return  std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
    else
    {
        std::cout << "\033[31m" << "failed to read file: " << filePath <<"\033[0m" << std::endl;
        return "";
    }
}

