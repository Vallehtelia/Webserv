#include "Request.hpp"
#include <iostream>

Request::Request() : currentState(State::REQUEST_LINE), method(""), uri(""), version(""), contentLength(0), body("") {
    chunked = false;
    received = false;
}

Request::Request(const std::string& rawRequest) : currentState(State::REQUEST_LINE), requestStream(rawRequest), contentLength(0)  {
}

Request::~Request() {}

std::string Request::getMethod() const {
    return method;
}

std::string Request::getUri() const {
    return uri;
}

std::string Request::getVersion() const {
    return version;
}


std::string Request::getBody() const {
    return body;
}

void Request::reset()
{
    currentState = State::REQUEST_LINE;
    chunked = false;
    received = false;
    rawRequest.clear();
    requestStream.str("");
    requestStream.clear();
    rawChunkedData.clear();
    boundary.clear();
    method.clear();
    uri.clear();
    version.clear();
    contentLength = 0;
    body_size = 0;
    headers.clear();
    body.clear();
    multipartData.clear();
}

std::string Request::stateToString(State state) {
    switch (state) {
        case State::REQUEST_LINE: return "REQUEST_LINE";
        case State::HEADERS: return "HEADERS";
         case State::UNCHUNK: return "UNCHUNK";
        case State::BODY: return "BODY";
        case State::MULTIPARTDATA: return "MULTIPARTDATA";
        case State::COMPLETE: return "COMPLETE";
        case State::ERROR: return "ERROR";
        case State::INCOMPLETE: return "INCOMPLETE";
        default: return "UNKNOWN";
    }
}

State Request::StateFromString(const std::string& stateStr) {
    if (stateStr == "REQUEST_LINE") return State::REQUEST_LINE;
    if (stateStr == "HEADERS") return State::HEADERS;
    if (stateStr == "UNCHUNK") return State::UNCHUNK;
    if (stateStr == "BODY") return State::BODY;
    if (stateStr == "MULTIPARTDATA") return State::MULTIPARTDATA;
    if (stateStr == "COMPLETE") return State::COMPLETE;
    if (stateStr == "ERROR") return State::ERROR;
    if (stateStr == "INCOMPLETE") return State::INCOMPLETE;

    // Return a default state if no match found
    return State::ERROR; // or another appropriate default state
}


std::string Request::getState()
{
    return stateToString(currentState);
}

void Request::setState(State state)
{
    currentState = state;
}

void Request::handleError(const std::string& errorMsg) {
    std::cerr << "Error: " << errorMsg << std::endl;
    currentState = State::ERROR;
}

void Request::parseRequest(std::string &rawRequest) {
    requestStream.str(rawRequest);
    if (currentState == State::INCOMPLETE)
        currentState = State::BODY;
    while (currentState != State::COMPLETE && currentState != State::ERROR && currentState != State::INCOMPLETE) {
        //std::cout << "STATE: " << stateToString(currentState) << std::endl;
        switch (currentState) {
            case State::REQUEST_LINE:
                parseRequestLine();
                break;
            case State::HEADERS:
                parseHeaders();
                break;
            case State::UNCHUNK:
                parseBody();
                break;
            case State::BODY:
                parseBody();
                break;
            case State::MULTIPARTDATA:
                parseMultipartData();
                break;
            case State::INCOMPLETE:
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
        lineStream >> method >> uri >> version;
        if (!method.empty() && !uri.empty() && !version.empty()) {
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
            if (key == "Transfer-Encoding")
            {
                if (value == "chunked\r")
                {
                    chunked = true;
                }
            }
            std::cout << key  << value << std::endl;
        }
    }
    std::cout << "CHUNKED: " << chunked << std::endl;
    if (chunked == true)
        currentState = State::UNCHUNK;
    else if (contentLength > 0) {
        currentState = State::BODY;
    } else {
        currentState = State::COMPLETE;
    }
}


void Request::parseChunks()
{
    std::vector<char> buffer(std::istreambuf_iterator<char>(requestStream), {});
    std::string chunkBuffer(buffer.begin(), buffer.end());
    std::cout << "PARSING CHUNKS" << std::endl;
    static bool inChunk = false;
    static size_t chunkSize = 0;
    rawChunkedData.append(std::string(buffer.begin(), buffer.end()));
    checkline(rawChunkedData);
    if (!inChunk) {
        size_t pos = rawChunkedData.find("\r\n");
        if (pos == std::string::npos) {
            currentState = State::INCOMPLETE; // Wait for more data
            return;
        }
        std::string chunkSizeStr = rawChunkedData.substr(0, pos);
        std::cout << "CHUNKSIZE STR: " << chunkSizeStr << std::endl;
        std::cout << "RAWBUFFER LEN: " << rawChunkedData.length() << std::endl;
        chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
        std::cout << "CHUNKSIZE SIZE_T: " << chunkSize << std::endl;
        rawChunkedData.erase(0, pos + 2);
        if (chunkSize == 0) {
            currentState = State::BODY;
            chunked = false;
            std::cout << "CHUNKS PARSED" << std::endl;
            return;
        }
        inChunk = true;
    }
    if (inChunk) {
        if (rawChunkedData.length() < chunkSize) {
            currentState = State::INCOMPLETE;
            return;
        }
        std::string chunkData = rawChunkedData.substr(0, chunkSize);
        body.append(chunkData);
        rawChunkedData.erase(0, chunkSize + 2);
        inChunk = false;
    }
}




void Request::parseBody()
{
    std::cout << "PARSING BODY" << std::endl;
    std::vector<char> buffer(std::istreambuf_iterator<char>(requestStream), {});
    body.append(std::string(buffer.begin(), buffer.end()));
    std::cout << "BODY SIZE: " << body.size() << std::endl;
    //std::cout << "METHOD: " << method << " URI: " << uri << std::endl;
    //std::cout << " BODY SIZE: " << buffer.size() << " CONTENT LENGTH: " << contentLength << "BODY_SIZE: " << body_size << std::endl; 
    if ((body.size() >= contentLength))
    {   
        std::cout << "CHECKING FOR MULTIPART DATA" << std::endl;
        if (method == "POST" && headers["Content-Type"].find("multipart/form-data") != std::string::npos) {
            size_t boundaryPos = headers["Content-Type"].find("boundary=");
            if (boundaryPos != std::string::npos) {
                boundary = headers["Content-Type"].substr(boundaryPos + 9);
                currentState = State::MULTIPARTDATA;
                return;
                }
        }
        else
        {
            currentState = State::COMPLETE;
            std::cout << "REQUEST PARSING COMPLETE" << std::endl;
        }
    }
    else
        currentState = State::INCOMPLETE;
    std::cout << "STATE:" << stateToString(currentState) << std::endl;
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
    //printMultipartdata();
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
    std::cout << "Content-length: " << contentLength << std::endl;
    std::cout << "Body size: " << body.size() << std::endl;
	std::cout << "uri: " << uri << std::endl;
	std::cout << "version: " << version << std::endl;
    for (const auto& pair : headers) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    if (body.size() < 2000)
	    std::cout << "body: \n" << body << std::endl;
	std::cout << "---------------" << "\033[0m" << std::endl;
}