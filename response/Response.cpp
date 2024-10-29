#include "Response.hpp"
#include <fstream>
#include <iostream>
#include <map>

Response::Response() {}
Response::~Response() {}

void Response::createResponse(const Request& req)
{
    body = req.getBody();
    if (req.getMethod() == "GET") {
        handleGetRequest(req);
    }
    else if (req.getMethod() == "DELETE")
    {
        handleDeleteRequest(req);
    }
    else if (req.getMethod() == "POST")
    {
        handlePostRequest(req);
    }
    else
    {
        setResponse(405, "text/html", body.length());
    }
}

std::string Response::getResponseString() const
{
    return statusLine + headers + "\r\n" + body;
}

std::string Response::getContentType(const std::string& path) const {
    std::map<std::string, std::string> mime_types = {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".json", "application/json"}
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

bool Response::validFile(const Request &req)
{
    std::string method = req.getMethod();
    if (method == "GET")
    {
        if (req.getPath() == "/")
            filePath = "./html/index.html";
        else
            filePath = "./html" + req.getPath();
        std::cout << "FILEPATH: " << filePath << std::endl;
    }
    else
        filePath = "./html/uploads" + req.getPath(); 
    if (method == "POST" || method == "PUT")
    {
        std::ofstream file(filePath);
        return file.is_open();
    }
    else if (method == "GET" || method == "DELETE")
    {
        std::ifstream file(filePath);
        return file.is_open();
    }
    return false;
}

std::string Response::readFileContent(const std::string& filePath) const {
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


std::string Response::createJsonResponse() {
    std::ostringstream response;
    response << "{\n"
             << "  \"status\": \"success\",\n"
             << "  \"message\": \"Files uploaded successfully\",\n"
             << "}";
    return response.str();
}


void Response::handlePostRequest(const Request& req) {

    const auto& multipartData = req.getMultipartData();

    if (!multipartData.empty()) {
        for (const auto &part : multipartData) {
            if (!part.filename.empty()) {
                std::ofstream file("./html/uploads/" + part.filename, std::ios::binary);
                if (file) {
                    file.write(part.data.data(), part.data.size());
                    file.close();
                }
                else {
                    setResponse(500, "text/html", body.length());
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
        body = createJsonResponse();
        setResponse(200, "application/json", body.length());
    }
    else if (validFile(req))
    {
        std::ofstream file(filePath, std::ios::binary);
        if (file)
        {
            file << body;
            file.close();
            body = createJsonResponse();
            setResponse(200, "application/json", body.length());
        }
    }
    else {
        setResponse(404, "text/html", body.length());
    }
}


void Response::handleGetRequest(const Request& req) {

    std::cout << "HANDLE GET" << std::endl;
    if (validFile(req)) {
        std::cout << "FILE IS LEGIT" << std::endl;
        body = readFileContent(filePath);
        setResponse(200, getContentType(filePath), body.length());
    } else {
        std::cout << "FILE IS NOT LEGIT" << std::endl;
        setResponse(404, "text/html", body.length());
    }
    statusLine = getStatusLine();
}


void Response::handleDeleteRequest(const Request &req)
{

    if (validFile(req))
    {
        if (remove(filePath.c_str()) == 0)
            setResponse(200, "text/plain", 0);
        else
            setResponse(500, "text/html", 0);
    }
    else
        setResponse(404, "text/html", 0);
}


void    Response::setError()
{
    std::string errorPage = getErrorPage();
    std::cout << "fetching error page: " << errorPage << std::endl;
    body = readFileContent(errorPage);
}

void Response::setResponse(int code, const std::string& contentType, size_t contentLength) {
    statusCode = code;
    statusLine = getStatusLine();
    if (statusCode != 200)
    {
        setError();
        contentLength = body.size();
    }
    headers = "Content-Type: " + contentType + "\r\n" +
              "Content-Length: " + std::to_string(contentLength) + "\r\n";
}

std::string Response::getStatusLine() const {
    switch (statusCode) {
        case 200:
            return "HTTP/1.1 200 OK\r\n";
        case 404:
            return "HTTP/1.1 404 Not Found\r\n";
        case 405:
            return "HTTP/1.1 405 Method Not Allowed\r\n";
        default:
            return "HTTP/1.1 500 Internal Server Error\r\n";
    }
}

std::string Response::getErrorPage()
{
    switch (statusCode) {
        case 404:
            return "./html/error_pages/404.html";
        case 405:
            return "./html/error_pages/405.html";
        default:
            return "./html/error_pages/500.html";
    }
}


void Response::printResponse()
{
   std::cout << std::endl;
   std::cout << "RESPONSE:" << "\033[32m" << std::endl;
   std::cout << headers << std::endl;
   std::cout << statusLine << std::endl;
   std::cout << statusCode << std::endl;
   std::cout << body << "\033[0m" << std::endl;

}