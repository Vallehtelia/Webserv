#include "Response.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include "../request/Request.hpp"

Response::Response() {}
Response::~Response() {}


std::string Response::getResponseString() const
{
    return _statusLine + _headers + "\r\n" + _body;
}

void Response::printResponse()
{
    std::cout << std::endl;
    std::cout << "RESPONSE:" << "\033[32m" << std::endl;
    // std::cout << getResponseString() << std::endl;
    std::cout << _body.size() << std::endl;
    std::cout << _contentLength << std::endl;
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


void Response::setResponse(int statusCode, const std::string& contentType, const std::string &body) {
    std::cout << "hello from setResponse" << std::endl;
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
    _headers = "Content-Type4: " + _contentType + "\r\n" +
              "Content-Length: " + std::to_string(_contentLength) + "\r\n";
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
            _statusLine = "HTTP/1.1 400 Bad Request";
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
        default:
            _statusLine = "HTTP/1.1 500 Internal Server Error\r\n";
            break;
    }
}


std::string Response::getErrorPage() const
{
    switch (_statusCode) {
        case 400:
            return "./html/error_pages/400.html";
        case 403:
            return ".html/error_pages/403.html";
        case 404:
            return "./html/error_pages/404.html";
        case 405:
            return "./html/error_pages/405.html";
        case 409:
            return "./html/error_pages/409.html";
        default:
            return "./html/error_pages/500.html";
    }
}

void    Response::setError()
{
    std::string errorPage = getErrorPage();
    std::cout << "fetching error page: " << errorPage << std::endl;
	_body = readFileContent(errorPage);
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

