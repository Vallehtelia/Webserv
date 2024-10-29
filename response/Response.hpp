#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <cstdio>
#include "../request/Request.hpp"


class Response {
public:
    Response();
    ~Response();

    void    createResponse(const Request& req);
    std::string getResponseString() const;
    void handleGetRequest(const Request& req);
    void handleDeleteRequest(const Request &req);
    void handlePostRequest(const Request &req);
    void printResponse();
private:
    std::string statusLine;
    std::string headers;
    std::string body;
    std::string filePath;
    int statusCode;
    std::string contentType();
    std::string getContentType(const std::string& path) const;
    std::string buildHeaders() const;
    bool validFile(const Request &req);
    std::string readFileContent(const std::string& filePath) const;
    std::string getStatusLine() const;
    void setResponse(int code, const std::string& contentType, size_t contentLength);
    std::string getErrorPage();
    void setError();
    std::string createJsonResponse();
};

#endif
