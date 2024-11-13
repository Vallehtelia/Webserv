
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
        Request(const Request &other);
        Request &operator=(const Request &rhs);
        ~Request();
        void                                reset();
        void                                printRequest();
        void                                parseRequest(std::string &rawReques);
        void                                setPath(std::string newPath);
        void                                setState(State state);
        void                                setReceived(bool state);
        std::string                         getMethod() const;
        std::string                         getUri() const;
        std::string                         getVersion() const;
        std::string                         getBody() const;
        std::map<std::string, std::string>  getHeaders() const;
        const std::vector<MultipartData>    &getMultipartData() const { return multipartData; };
        std::string                         getState();
        std::string                         stateToString(State state);
        State                               StateFromString(const std::string& stateStr);
        std::string                         getContentType() const;
        bool                                isMultiPart() const;
    private:

        bool                                received;
        bool                                chunked;
        bool                                _isMultiPart;
        State                               currentState;
        size_t                              contentLength;
        size_t                              body_size = 0;
        std::string                         rawRequest;
        std::string                         body;
        std::string                         rawChunkedData;
        std::string                         boundary;
        std::string                         method;
        std::string                         uri;
        std::string                         version;
        std::string                         contentType;
        std::istringstream                  requestStream;
        std::map<std::string, std::string>  headers;
        std::vector<MultipartData>          multipartData;

        MultipartData                       createData(std::string &rawData);
        void                                parseMultipartData();
        void                                parseHeaders();
        void                                parseBody();
        void                                handleError(const std::string& errorMsg);
        void                                parseRequestLine();
        void                                printMultipartdata();
        void                                createMultipartBody(MultipartData &multipartData, std::istringstream &rawMultipartData);
        void                                createMultipartHeaders(MultipartData &multipartData, std::istringstream &rawDataStream);
        void                                parseChunks();

} ;

# endif
