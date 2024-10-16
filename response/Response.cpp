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
        body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
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
    filePath = "." + req.getPath();
    if (req.getPath() == "/") {
        filePath = "./html/index.html";
    } else {
        filePath = "./html" + req.getPath();
    }
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
        return "";
    }
}


void Response::handlePostRequest(const Request& req) {

    const auto& multipartData = req.getMultipartData();


    if (!multipartData.empty()) {
        for (const auto& part : multipartData) {
            if (!part.filename.empty()) {

                std::ofstream file("./html/" + part.filename, std::ios::binary);
                if (file) {
                    file.write(part.data.data(), part.data.size());
                    file.close();
                }
                else {

                    statusCode = 500;
                    body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
                    setResponse(500, "text/html", body.length());
                    return;
                }
            } else {
                std::ofstream file("./html/" + part.name, std::ios::binary);
                if (file)
                {
                    file.write(part.data.data(), part.data.size());
                    file.close();
                }
                std::cout << "Form field: " << part.name << ", Value: " << std::string(part.data.begin(), part.data.end()) << std::endl;
            }
        }

        statusCode = 200;
        body = "<html><body><h1>File uploaded successfully!</h1></body></html>";
        setResponse(200, "text/html", body.length());
    } 
    else if (validFile(req))
    {
        std::ofstream file(filePath, std::ios::binary);
        if (file)
        {
            file << body;
            file.close();
            body = "<html><body><h1>File uploaded successfully!</h1></body></html>";
            setResponse(200, "text/html", body.length());
        }
    }
    else {

        statusCode = 400;
        body = "<html><body><h1>400 Bad Request</h1></body></html>";
        setResponse(400, "text/html", body.length());
    }
    std::cout << "added" << std::endl;
    //statusLine = getStatusLine();
}


void Response::handleGetRequest(const Request& req) {

    if (validFile(req)) {
        body = readFileContent(filePath);
        statusCode = 200;
        setResponse(200, getContentType(filePath), body.length());
    } else {
        statusCode = 404;
        body = "<html><body><h1>404 Not Found</h1></body></html>";
        setResponse(404, getContentType(filePath), body.length());
    }
    statusLine = getStatusLine();
}


void Response::handleDeleteRequest(const Request &req)
{

    if (validFile(req))
    {
        if (remove(filePath.c_str()) == 0)
        {
            body = "<html><body><h1>file deleted</h1></body></html>";
            setResponse(200, "text/plain", 0);
        }
        else
        {
            body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
            setResponse(500, "text/plain", 0);
        }
    }
    else
    {
        body = "<html><body><h1>404 Not Found</h1><p>The requested file does not exist.</p></body></html>";
        setResponse(404, "text/plain", 0);
    }
}

void Response::setResponse(int code, const std::string& contentType, size_t contentLength) {
    statusCode = code;
    statusLine = getStatusLine();
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