
#ifndef REQUEST_HPP
# define REQUEST_HPP


#include <iostream>
#include <map>
#include <sstream> 
#include <vector>
#include <iomanip> 

#include "RequestHandler.hpp"

#define URICHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-.~:/?#[]@!$&'()*+,;="
#define FIELDCHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-"

enum class State {
    REQUEST_LINE,
    HEADERS,
    BODY,
    MULTIPARTDATA,
    COMPLETE,
    ERROR
};

enum class multipartState {
    SPLIT_DATA,
    CREATE_DATA,
    BODY,
    MULTIPARTDATA,
    COMPLETE,
    ERROR
};

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
        void parseRequest(std::string &rawReques);
        void handleRequest(void);
        void parseMultipartData();
        const std::vector<MultipartData>& getMultipartData() const { return multipartData; };
        void printRequest();
        std::string getState();
    private:
        State currentState;
        std::string rawRequest;
        std::istringstream requestStream;
        std::string boundary;
        std::string method;
        std::string path;
        std::string version;
        size_t contentLength;
        std::map<std::string, std::string> headers;
        std::string body;
        std::vector<MultipartData> multipartData;
        void parseHeaders();
        void parseBody();
        void handleError(const std::string& errorMsg);
        void parseRequestLine();
        void printMultipartdata();
        MultipartData createData(std::string &rawData);
        void createMultipartBody(MultipartData &multipartData, std::istringstream &rawMultipartData);
        void createMultipartHeaders(MultipartData &multipartData, std::istringstream &rawDataStream);

} ;

# endif