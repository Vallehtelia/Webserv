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
    void setResponse(int code, const std::string& contentType, size_t contentLength);
    void setStatusLine(std::string &statusline);
    void setBody(std::string &body);
    void setHeaders(std::string &headers);
    void setUri(std::string &URI);
    void setFilePath(std::string &filepath);
    void setError();
private:
    std::string _statusLine;
    std::string _headers;
    std::string _body;
    std::string _filePath;
    std::string _uri;
    int         _contentLength;
    std::string _contentType;
    int _statusCode;
    std::string getErrorPage() const;
    void setStatusLine();
    std::string readFileContent(std::string& filePath);
    //std::string buildHeaders() const;

};

#endif

    // 
    // void setError();
    // std::string createJsonResponse();
    // std::string readFileContent(const std::string& filePath) const;
    // 
    // std::string getContentType(const std::string& path) const;
    // bool validFile(const Request &req);


    // void    createResponse(const Request& req);
    // void handleGetRequest(const Request& req);
    // void handleDeleteRequest(const Request &req);
    // void handlePostRequest(const Request &req);