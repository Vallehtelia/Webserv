#include "../response/Response.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"

RequestHandler::RequestHandler() {}
RequestHandler::~RequestHandler() {}


void RequestHandler::handleRequest(Request& req, Response& res)
{
    if (req.getMethod() == "GET")
	{
        handleGetRequest(req, res);
    }
	else if (req.getMethod() == "POST")
	{
        handlePostRequest(req, res);
    }
	else if (req.getMethod() == "DELETE")
	{
        handleDeleteRequest(req, res);
    }
	else
	{
        res.setResponse(405, "text/html", 0);
    }
}

std::string RequestHandler::getContentType(const std::string& path) const {
    std::map<std::string, std::string> mime_types = {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".json", "application/json"},
        {".mp3", "audio/mpeg"},
        {".mp4", "video/mp4"} 

    };

    size_t dot_pos = path.find_last_of(".");
    if (dot_pos != std::string::npos) {
        std::string ext = path.substr(dot_pos);
        if (mime_types.count(ext)) {
            return mime_types.at(ext);
        }
    }

    return "text/plain";
}


bool RequestHandler::validFile(const Request &req)
{
    std::string tempContentType = getContentType(req.getUri());
    std::string method = req.getMethod();
    std::cout << getContentType(req.getUri()) << std::endl;
    if (method == "GET")
    {
        if (req.getUri() == "/")
                _filePath = "./html/index.html";
        else if (tempContentType == "text/html" || tempContentType == "text/css" || tempContentType == "application/javascript")
        {
            _filePath = "./html" + req.getUri();
        }
        else
            _filePath = "./html/uploads" + req.getUri();
        std::cout << "FILEPATH: " << _filePath << std::endl;
        std::ifstream file(_filePath);
        return file.is_open();
    }
    else if (method == "POST" || method == "PUT")
    {
        std::cout << "DOES IT GET HERE?" << std::endl;
        _filePath = "./html/uploads" + req.getUri();
        std::cout << "FILEPATH: " << _filePath << std::endl;
        std::ofstream file(_filePath);
        return file.is_open();
    }
    else if (method == "GET" || method == "DELETE")
    {
        std::ifstream file(_filePath);
        return file.is_open();
    }
    else
        return false;
}


std::string RequestHandler::readFileContent(std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (file) {
        return  std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
    else
    {
        std::cout << "\033[31m" << "failed to read file" <<"\033[0m" << std::endl;
        return "";
    }
}

std::string RequestHandler::createJsonResponse() {
    std::ostringstream response;
    response << "{\n"
             << "  \"status\": \"success\",\n"
             << "  \"message\": \"Files uploaded successfully\",\n"
             << "}";
    return response.str();
}


void RequestHandler::handleGetRequest(const Request& req, Response& res) {

    std::cout << "HANDLE GET" << std::endl;
    if (validFile(req)) {
        std::cout << "FILE IS LEGIT" << std::endl;
       	_body = readFileContent(_filePath);
		res.setBody(_body);
        res.setResponse(200, getContentType(_filePath), _body.length());
    } else {
        std::cout << "FILE IS NOT LEGIT" << std::endl;
        res.setResponse(404, "text/html", _body.length());
    }
}



void RequestHandler::handleDeleteRequest(const Request& req, Response& res)
{

    if (validFile(req))
    {
        if (remove(_filePath.c_str()) == 0)
            res.setResponse(200, "text/plain", 0);
        else
            res.setResponse(500, "text/html", 0);
    }
    else
        res.setResponse(404, "text/html", 0);
}


void RequestHandler::handlePostRequest(const Request& req, Response& res) {

    const auto& multipartData = req.getMultipartData();

    std::cout << "HANDLING POST REQUEST" << std::endl;
    std::cout << "MULTIPART DATA EMPTY: " << (multipartData.empty() ? "TRUE" : "FALSE") << std::endl;
    if (!multipartData.empty()) {
        for (const auto &part : multipartData) {
            if (!part.filename.empty()) {
                std::ofstream file("./html/uploads/" + part.filename, std::ios::binary);
                if (file) {
                    file.write(part.data.data(), part.data.size());
                    file.close();
                }
                else {
                    res.setResponse(500, "text/html", _body.length());
                    return;
                }
            } else {
                std::ofstream file("./html/uploads/" + part.name, std::ios::binary);
                if (file)
                {
                    file.write(part.data.data(), part.data.size());
                    file.close();
                }
            }
        }
		_body = createJsonResponse();
        res.setBody(_body);
        res.setResponse(200, "application/json", _body.length());
    }
    else if (validFile(req))
    {
        std::cout << "FILE IS VALID" << std::endl;
        std::ofstream file(_filePath, std::ios::out | std::ios::trunc | std::ios::binary);
        if (file)
        {
            file << req.getBody();
            file.close();
            res.setResponse(200, "application/json", _body.length());
        }
    }
    else {
        res.setResponse(404, "text/html", _body.length());
    }
}


