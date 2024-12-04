
#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP


#include <string>
#include <cstdio>
#include "../response/Response.hpp"
#include "Request.hpp"
#include <fstream>
#include <unordered_map>
#include <filesystem>

#include "../parsing/LocationConfig.hpp"


class RequestHandler {
public:
	RequestHandler();
	~RequestHandler();
    void handleRequest(Request& req, Response& res);
    std::string readFileContent(std::string& filePath);
private:
	void prepareHandler(const Request &req);
    void handleGetRequest(Response& res);
    void handlePutRequest(const Request& req, Response& res);
    void handleDeleteRequest(Response& res);
	void handlePostRequest(const Request& req, Response& res);
    std::string createJsonResponse(const std::vector<std::string> uploadedFiles);
	void handleFormField(const MultipartData& part);
	void handleFileUpload(const MultipartData& part, Response& res);
	void handleJsonData(const Request &req, Response &res);
    std::string getContentType(const std::string& uri) const;
	void handleMultipartRequest(const Request &req, Response &res);
	bool validFile(const std::string& filePath);
	std::string getFilepath(std::string filepath);
	std::string				_body;
    std::string				_statusLine;
    std::string				_headers;
    std::string	_filePath;
	std::string				_uri;
	std::string				_method;
	std::string				_contentType;
	int						_statusCode;
	LocationConfig 			_location;
};

#endif
