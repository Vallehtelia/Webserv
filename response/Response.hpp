#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <cstdio>
#include "../request/Request.hpp"


class Response {
public:
    Response();
    ~Response();

    std::string getResponseString() const;
    void printResponse();
    void setResponse(int code, const std::string& contentType, const std::string &body);
    void setStatusLine(std::string &statusline);
    void setBody(std::string &body);
    void setHeaders(std::string &headers);
    void setUri(std::string &URI);
private:
    std::string _statusLine;
    std::string _headers;
    std::string _body;
    std::string _uri;
    int         _contentLength;
    std::string _contentType;
    int _statusCode;
    std::string getErrorPage() const;
    void setStatusLine();
    std::string readFileContent(std::string& filePath);
    void setError();
};

#endif
