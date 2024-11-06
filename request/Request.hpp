
#ifndef REQUEST_HPP
# define REQUEST_HPP


#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <iomanip> 


#define URICHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-.~:/?#[]@!$&'()*+,;="
#define FIELDCHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-"


void checkline(std::string line);

enum class State {
    REQUEST_LINE,
    HEADERS,
    UNCHUNK,
    BODY,
    MULTIPARTDATA,
    COMPLETE,
    ERROR,
    INCOMPLETE
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
        std::string getUri() const;
        std::string getVersion() const;
        void    reset();
        std::map<std::string, std::string> getHeaders() const {
        return headers;
    }
        void setPath(std::string newPath);

        std::string getBody() const;
        void parseRequest(std::string &rawReques);
        void handleRequest(void);
        void parseMultipartData();
        const std::vector<MultipartData>& getMultipartData() const { return multipartData; };
        void printRequest();
        std::string getState();
        void setState(State state);
        std::string stateToString(State state);
        State StateFromString(const std::string& stateStr);
        bool isMultiPart() const;
        std::string getContentType() const;
    private:
        State currentState;
        bool    chunked;
        bool    received;
        std::string rawRequest;
        std::istringstream requestStream;
        std::string rawChunkedData;
        std::string boundary;
        std::string method;
        std::string uri;
        std::string version;
        size_t contentLength;
        size_t body_size = 0;
        bool _isMultiPart;
        std::map<std::string, std::string> headers;
        std::string body;
        std::string contentType;
        std::vector<MultipartData> multipartData;
        //std::vector<char> bodyBuffer;
        void parseHeaders();
        void parseBody();
        void handleError(const std::string& errorMsg);
        void parseRequestLine();
        void printMultipartdata();
        MultipartData createData(std::string &rawData);
        void createMultipartBody(MultipartData &multipartData, std::istringstream &rawMultipartData);
        void createMultipartHeaders(MultipartData &multipartData, std::istringstream &rawDataStream);
        void parseChunks();

} ;

# endif
