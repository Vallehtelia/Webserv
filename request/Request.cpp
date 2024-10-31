#include "Request.hpp"
#include <iostream>

Request::Request() : method(""), path(""), version(""), body("") {}

Request::Request(const std::string& rawRequest) {
    parseRequest(rawRequest);
}

Request::~Request() {}

std::string Request::getMethod() const {
    return method;
}

std::string Request::getPath() const {
    return path;
}

void Request::setPath(std::string newPath) {
    path = newPath;
}

std::string Request::getVersion() const {
    return version;
}


std::string Request::getBody() const {
    return body;
}


#include <iomanip> // For std::hex and std::setfill

// static void printBodyHex(std::vector<char> bodyBuffer)
// {
//         std::cout << "Body (Hex): ";
//         for (unsigned char c : bodyBuffer) {
//             std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
//         }
//         std::cout << std::dec << std::endl;
// }

void Request::parseHeaders(std::istringstream& requestStream)
{
    std::string requestLine;

    // Read the first line (Request Line)
    std::getline(requestStream, requestLine);
    std::istringstream requestLineStream(requestLine);
    requestLineStream >> method >> path >> version;

    // Read headers
    std::string headerLine;
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
}

void Request::parseBody(std::istringstream& requestStream)
{
        std::vector<char> bodyBuffer(contentLength);
        requestStream.read(bodyBuffer.data(), contentLength);  // Read as raw binary

        body = std::string(bodyBuffer.begin(), bodyBuffer.end());
        // Debugging: Print the body content as hexadecimal
        //printBodyHex(bodyBuffer);
        if (method == "POST" && headers["Content-Type"].find("multipart/form-data") != std::string::npos) {
            size_t boundaryPos = headers["Content-Type"].find("boundary=");
            if (boundaryPos != std::string::npos) {
                std::string boundary = headers["Content-Type"].substr(boundaryPos + 9);
                parseMultipartData(boundary);
            }
        }
}

void Request::parseRequest(const std::string& rawRequest) {
    std::istringstream requestStream(rawRequest);
    parseHeaders(requestStream);
    if (contentLength > 0) {
        parseBody(requestStream);
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
        if (part == "--\r\n")
            break ;
        parts.push_back(part);
        start = end;
    }
    return parts;
}



MultipartData createData(std::string &part) {
    MultipartData multipartData;

    part.erase(0, part.find_first_not_of("\r\n"));
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
    // Read the entire remaining content as raw binary data
    std::vector<char> buffer(std::istreambuf_iterator<char>(partStream), {});

    // Copy the buffer into multipartData.data
    multipartData.data.insert(multipartData.data.end(), buffer.begin(), buffer.end());

    //std::cout << "BODY DATA SIZE: " << multipartData.data.size() << "\nDATA: ";
    return multipartData;
}

void printVectorAsHex(const std::vector<char>& vec) {
    for (unsigned char c : vec) {
        std::cout << std::hex << static_cast<int>(c) << " ";
    }
    std::cout << std::dec << std::endl;
}

void Request::parseMultipartData(const std::string& boundary) {
    std::cout << "PARSING MULTIPART DATA" << std::endl;
    std::string delimiter =  "--" + boundary;
    std::string data(body.begin(), body.end());
    delimiter.erase(delimiter.find_last_not_of("\r") + 1);
    std::vector<std::string> parts = splitByBoundary(data, delimiter);
    int i = 0;
    for (std::string &part : parts)
    {
        std::cout << "PART: " << part << std::endl;
        //std::cout << delimiter << std::endl;
        multipartData.push_back(createData(part));
        std::cout << "CREATED MULTIPART DATA:" << std::endl;
        std::cout << "\033[32m" << "PART: " << i << std::endl;
        std::cout << "PART NAME: " << multipartData[i].name << std::endl;
        std::cout << "FILE NAME: " << multipartData[i].filename << std::endl;
        std::cout << "CONTENT TYPE: " << multipartData[i].contentType << std::endl;
        std::cout << "PART DATA: ";
        printVectorAsHex(multipartData[i].data);
        std::cout << "\033[0m" << std::endl;
        i++;
    }
}

void Request::printRequest()
{
	std::cout << "RECEIVED REQUEST:" << "\033[33m" << std::endl;
	std::cout << "method: " << method << std::endl;
	std::cout << "path: " << path << std::endl;
	std::cout << "version: " << version << std::endl;
	std::cout << "body: " << body << std::endl;
	std::cout << "---------------" << "\033[0m" << std::endl;
}
