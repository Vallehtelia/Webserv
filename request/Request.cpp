#include "Request.hpp"
#include <iostream>

Request::Request() : method(""), path(""), version(""), body("") {}

Request::Request(const std::string& rawRequest) {
    parse(rawRequest);
}

Request::~Request() {}

std::string Request::getMethod() const {
    return method;
}

std::string Request::getPath() const {
    return path;
}

std::string Request::getVersion() const {
    return version;
}


std::string Request::getBody() const {
    return body;
}


void Request::parse(const std::string& rawRequest) {
    
    std::istringstream requestStream(rawRequest);
    std::string requestLine;

    std::getline(requestStream, requestLine);
    std::istringstream requestLineStream(requestLine);
    requestLineStream >> method >> path >> version;

    std::string headerLine;
    int contentLength = 0;
    
    while (std::getline(requestStream, headerLine) && headerLine != "\r") {
        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos) {
            std::string key = headerLine.substr(0, colonPos);
            std::string value = headerLine.substr(colonPos + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            headers[key] = value;
            if (key == "Content-Length") {
                contentLength = std::stoi(value);
            }
        }
    }

    if (contentLength > 0) {
        body.resize(contentLength);
        requestStream.read(&body[0], contentLength);


        if (method == "POST" && headers["Content-Type"].find("multipart/form-data") != std::string::npos) {
            size_t boundaryPos = headers["Content-Type"].find("boundary=");
            if (boundaryPos != std::string::npos) {
                std::string boundary = headers["Content-Type"].substr(boundaryPos + 9);
                parseMultipartData(boundary);
            }
        }
    }
}

void checkline(std::string line)
{
std::cout << "Line characters: ";
for (char c : line) {
    std::cout << (c == '\n' ? std::string("\\n") : c == '\r' ? std::string("\\r") : std::string(1, c));
}
std::cout << std::endl;
}

std::vector<std::string> splitByBoundary(std::string data, std::string boundary) {
    std::vector<std::string> parts;
    std::string part;
    size_t start = 0;

    while ((start = data.find(boundary, start)) != std::string::npos) {
        start += boundary.length();
        size_t end = data.find(boundary, start);
        part = data.substr(start, end - start);
        parts.push_back(part);
        std::cout  << "added: \033[32m [" << part << "] \033[0m" << std::endl;
        start = end;
    }
    return parts;
}



MultipartData createData(std::string &part) {
    MultipartData multipartData;
    
    std::istringstream partStream(part);
    std::string line;
    while (std::getline(partStream, line))
    {
        if (line == "\r")
            break ;
        if (line.empty())
            continue ;
        line.erase(0, line.find_first_not_of(" "));
        line.erase(line.find_last_not_of("\r\n") + 1);
        checkline(line);
        if (line.find("Content-Disposition:") != std::string::npos) {
            size_t nameStart = line.find("name=\"") + 6;
            size_t nameEnd = line.find("\"", nameStart);
            multipartData.name = line.substr(nameStart, nameEnd - nameStart);
            
            size_t filenameStart = line.find("filename=\"");
            if (filenameStart != std::string::npos) {
                filenameStart += 10;
                size_t filenameEnd = line.find("\"", filenameStart);
                multipartData.filename = line.substr(filenameStart, filenameEnd - filenameStart);
            }
        }
        else if (line.find("Content-Type:") != std::string::npos) {
        size_t typeStart = line.find("Content-Type:") + 14;
        multipartData.contentType = line.substr(typeStart);
    }
    }
    std::string content((std::istreambuf_iterator<char>(partStream)),
                        std::istreambuf_iterator<char>());
    
    multipartData.data.insert(multipartData.data.end(), content.begin(), content.end());
    
    return multipartData;
}

void printVectorAsHex(const std::vector<char>& vec) {
    for (unsigned char c : vec) {
        std::cout << std::hex << static_cast<int>(c) << " ";
    }
    std::cout << std::dec << std::endl;
}

void Request::parseMultipartData(const std::string& boundary) {
    std::cout << "hello from multidata" << std::endl;
    std::string delimiter =  "--" + boundary;
    std::string data(body.begin(), body.end());
    std::vector<std::string> parts = splitByBoundary(data, delimiter);
    int i = 0;
    for (std::string &part : parts)
    {   
        std::cout << "PART: " << part << std::endl;
        //std::cout << delimiter << std::endl;
        multipartData.push_back(createData(part));
        std::cout << "PART NAME: " << multipartData[i].name << std::endl;
        std::cout << "FILE NAME: " << multipartData[i].filename << std::endl;
        std::cout << "CONTENT TYPE: " << multipartData[i].contentType << std::endl;
        std::cout << "PART DATA: ";
        printVectorAsHex(multipartData[i].data);
        std::cout << std::endl;
        i++;
    }
}