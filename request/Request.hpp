
#ifndef REQUEST_HPP
# define REQUEST_HPP


#include <iostream>
#include <map>
#include <sstream> 
#include <vector>

#include "RequestHandler.hpp"

struct MultipartData
{
	std::string					name;
	std::string					filename;
	std::string					contentType;
	std::vector<char>			data = {};
	std::string					boundary;
    std::map<std::string, std::string>    headers;
};

class Request {
    public:
        Request();
        Request(const std::string &rawRequest);
        ~Request();
        std::string getMethod() const;
        std::string getPath() const;
        std::string getVersion() const;
        std::map<std::string, std::string> getHeaders() const {
        return headers;
    }
        std::string getBody() const;
        void parseRequest(const std::string& rawRequest);
        void handleRequest(void);
        void parseMultipartData(const std::string& boundary);
        const std::vector<MultipartData>& getMultipartData() const { return multipartData; };
        void printRequest();
    private:
        std::string rawRequest;
        std::string method;
        std::string path;
        std::string version;
        size_t contentLength;
        std::map<std::string, std::string> headers;
        std::string body;
        std::vector<MultipartData> multipartData;
        void parseHeaders(std::istringstream& requestStream);
        void parseBody(std::istringstream& requestStream);

} ;

# endif