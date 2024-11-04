
#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP


#include <string>
#include <cstdio>
#include "../response/Response.hpp"
#include "Request.hpp"
#include <fstream>


class RequestHandler {
public:
	RequestHandler();
	~RequestHandler();
    void handleRequest(Request& req, Response& res);
    std::string readFileContent(std::string& filePath);
private:
    void handleGetRequest(const Request& req, Response& res);
    void handlePostRequest(const Request& req, Response& res);
    void handleDeleteRequest(const Request& req, Response& res);
    bool validFile(const Request& req);
    std::string createJsonResponse();
    std::string getContentType(const std::string& uri) const;
	std::string _body;
    std::string _statusLine;
    std::string _headers;
    std::string _filePath;
	std::string _uri;
	std::string _method;
};

#endif
