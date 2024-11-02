#include "Request.hpp"
#include <iostream>

Request::Request() : method(""), path(""), version(""), body("") {}

Request::Request(const std::string& rawRequest) : currentState(State::REQUEST_LINE), requestStream(rawRequest), contentLength(0)  {
    parseRequest();
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



static std::string stateToString(State state) {
    switch (state) {
        case State::REQUEST_LINE: return "REQUEST_LINE";
        case State::HEADERS: return "HEADERS";
        case State::BODY: return "BODY";
        case State::MULTIPARTDATA: return "MULTIPARTDATA";
        case State::COMPLETE: return "COMPLETE";
        case State::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}


std::string Request::getState()
{
    return stateToString(currentState);
}

void Request::handleError(const std::string& errorMsg) {
    std::cerr << "Error: " << errorMsg << std::endl;
    currentState = State::ERROR;
}

void Request::parseRequest(std::string &rawRequest) {
    requestStream = rawRequest;
    while (currentState != State::COMPLETE && currentState != State::ERROR) {
        std::cout << "STATE: " << stateToString(currentState) << std::endl;
        switch (currentState) {
            case State::REQUEST_LINE:
                parseRequestLine();
                break;
            case State::HEADERS:
                parseHeaders();
                break;
            case State::BODY:
                parseBody();
                break;
            case State::MULTIPARTDATA:
                parseMultipartData();
                break;
            case State::COMPLETE:
                break;
            case State::ERROR:
                std::cerr << "Error in parsing request\n";
                break;
        }
    }
}



static bool isValidRequestLine(const std::string& requestLine) {
    for (char c : requestLine) {
        if (!isalnum(c) && std::string("-._~:/?#[]@!$&'()*+,;=% ").find(c) == std::string::npos) {
            return false;
        }
    }
    return true;
}

void Request::parseRequestLine() {
    std::string requestLine;
    if (std::getline(requestStream, requestLine)) {
        if (!isValidRequestLine(requestLine))
            handleError("Invalid characters in request line.");
        std::istringstream lineStream(requestLine);
        lineStream >> method >> path >> version;
        if (!method.empty() && !path.empty() && !version.empty()) {
            currentState = State::HEADERS;
        } else {
            handleError("Invalid request line format.");
        }
    } else {
        handleError("Request line missing.");
    }
}

void Request::parseHeaders() {
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
    if (contentLength > 0) {
        currentState = State::BODY;
    } else {
        currentState = State::COMPLETE;
    }
}

void Request::parseBody()
{
        std::vector<char> bodyBuffer(contentLength);
        requestStream.read(bodyBuffer.data(), contentLength);
        body = std::string(bodyBuffer.begin(), bodyBuffer.end());
        if (body.length == reques.contentLength)
        {
            if (method == "POST" && headers["Content-Type"].find("multipart/form-data") != std::string::npos) {
                size_t boundaryPos = headers["Content-Type"].find("boundary=");
                if (boundaryPos != std::string::npos) {
                    boundary = headers["Content-Type"].substr(boundaryPos + 9);
                    currentState = State::MULTIPARTDATA;
                    return;
                }
            else
                currentState = State::COMPLETE;
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
        if (part == "--\r\n")
            break ;
        parts.push_back(part);
        start = end;
    }
    return parts;
}


void    Request::createMultipartHeaders(MultipartData &multipartData, std::istringstream &rawDataStream)
{
    // READ THE HEADERS LINE BY LINE
    std::string line;
    while (std::getline(rawDataStream, line))
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
}

void    Request::createMultipartBody(MultipartData &multipartData, std::istringstream &rawMultipartData)
{
    // PUT THE REST OF THE MULTIPARTDATA TO ITS BODY
    std::vector<char> buffer(std::istreambuf_iterator<char>(rawMultipartData), {});
    multipartData.data.insert(multipartData.data.end(), buffer.begin(), buffer.end());
}

MultipartData Request::createData(std::string &rawData) {
    MultipartData multipartData;
    
    rawData.erase(0, rawData.find_first_not_of("\r\n"));
    std::istringstream rawDataStream(rawData);
    createMultipartHeaders(multipartData, rawDataStream);
    createMultipartBody(multipartData, rawDataStream);
    return multipartData;
}

void Request::parseMultipartData() {
    std::string delimiter =  "--" + boundary;
    std::string data(body.begin(), body.end());
    delimiter.erase(delimiter.find_last_not_of("\r") + 1);
    
    std::vector<std::string> parts = splitByBoundary(data, delimiter);
    for (std::string &part : parts)
        multipartData.push_back(createData(part));
    printMultipartdata();
    currentState = State::COMPLETE;
}



static void printVectorAsHex(const std::vector<char>& vec) {
    for (unsigned char c : vec) {
        std::cout << std::hex << static_cast<int>(c) << " ";
    }
    std::cout << std::dec << std::endl;
}

void Request::printMultipartdata()
{
    int i = 0;
    for (const auto &part : multipartData)
    {   
        std::cout << "MULTIPART DATA:" << std::endl;
        std::cout << "\033[32m" << "PART: " << i << std::endl;
        std::cout << "PART NAME: " << part.name << std::endl;
        std::cout << "FILE NAME: " << part.filename << std::endl;
        std::cout << "CONTENT TYPE: " << part.contentType << std::endl;
        std::cout << "PART DATA: ";
        printVectorAsHex(part.data);
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