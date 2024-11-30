#include "Request.hpp"
#include <iostream>

Request::Request() : currentState(State::REQUEST_LINE), contentLength(0), body(""), method(""), uri(""), version("") {
    chunked = false;
    received = false;
    _isMultiPart = false;
}

Request::Request(const std::string& rawRequest) : currentState(State::REQUEST_LINE), contentLength(0), requestStream(rawRequest)  {
}

Request &Request::operator=(const Request &rhs) {
    if (this != &rhs) {
        currentState = rhs.currentState;
        contentLength = rhs.contentLength;
        body = rhs.body;
        rawRequest = rhs.rawRequest;
        body_size = rhs.body_size;
        headers = rhs.headers;
        method = rhs.method;
        uri = rhs.uri;
        version = rhs.version;
        chunked = rhs.chunked;
        received = rhs.received;
        _isMultiPart = rhs._isMultiPart;
        rawChunkedData = rhs.rawChunkedData;
        boundary = rhs.boundary;
        multipartData = rhs.multipartData;
    }
    return *this;
}


Request::~Request() {}

std::string Request::getMethod() const {
    return method;
}

std::string Request::getUri() const {
    return uri;
}

void Request::setPath(std::string newPath) {
    uri = newPath;
}

std::string Request::getVersion() const {
    return version;
}


std::string Request::getBody() const {
    return body;
}

void Request::setReceived(bool state)
{
    received = state;
}

std::map<std::string, std::string> Request::getHeaders() const {
    return headers;
}

bool Request::isMultiPart() const
{
    return _isMultiPart;
}

std::string Request::getContentType() const
{
    return contentType;
}

State Request::getState()
{
    return currentState;
}

void Request::setState(State state)
{
    currentState = state;
}

void Request::reset()
{
    currentState = State::REQUEST_LINE;
    chunked = false;
    received = false;
    _isMultiPart = false;
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
    queryParams.clear();
}


void Request::handleError(const std::string& errorMsg) {
    std::cerr << "Error: " << errorMsg << std::endl;
    currentState = State::ERROR;
}

void Request::parseRequest(std::string &rawRequest) {
    requestStream.str(rawRequest);
    if (currentState == State::INCOMPLETE)
        currentState = State::BODY;
    if (chunked)
        currentState = State::UNCHUNK;
    while (currentState != State::COMPLETE && currentState != State::ERROR && currentState != State::INCOMPLETE) {
        // std::cout << "REQUEST PARSING: " << stateToString(currentState) << std::endl;
        switch (currentState) {
            case State::REQUEST_LINE:
                parseRequestLine();
                break;
            case State::HEADERS:
                parseHeaders();
                break;
            case State::UNCHUNK:
                parseChunks();
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
            default:
                break;
        }
    }
}



static bool isValidRequestLine(const std::string& requestLine) {
    std::regex requestLinePattern(R"(^[A-Z]+ [^\s]+ HTTP/1\.1$)");
    return std::regex_match(requestLine, requestLinePattern);
}

void Request::parseQueryString() {
    size_t queryPos = uri.find('?');

    if (queryPos != std::string::npos) {
        std::string param, key, value;
        std::istringstream queryStream(uri.substr(queryPos + 1, uri.size()));
        while (std::getline(queryStream, param, '&'))
        {
            std::istringstream keyval(param);
            if (std::getline(keyval, key, '=') && std::getline(keyval, value))
                queryParams[key] = value;
        }
        uri.erase(queryPos, uri.size());
    }
}


void Request::parseRequestLine() {
    std::string requestLine;
    if (std::getline(requestStream, requestLine)) {
        removeCarriageReturn(requestLine);
        if (!isValidRequestLine(requestLine))
        {
            handleError("Invalid characters in request line.");
            return ;
        }
        std::istringstream lineStream(requestLine);
        lineStream >> method >> uri >> version;
        if (!method.empty() && !uri.empty() && !version.empty()) {
            parseQueryString();
            currentState = State::HEADERS;
        } else {
            handleError("Invalid request line format.");
        }
    } else {
        handleError("Request line missing.");
    }
}

void Request::prepareRequest()
{
    if ((method == "POST" || method == "PUT"))
    {
        if (headers.find("Content-Length") != headers.end()) {
            contentLength = std::stoi(headers["Content-Length"]);
        } else {
            handleError("Content-Length missing on a POST request");
            return ;
        }
    }
    if (headers.find("Transfer-Encoding") != headers.end())
        if (headers["Transfer-Encoding"] == "chunked")
            chunked = true;
    if (headers.find("Content-Type") != headers.end())
        contentType = headers["Content-Type"];
    if ((method == "POST" || method == "PUT") && headers["Content-Type"].find("multipart/form-data") != std::string::npos)
    {
        size_t boundaryPos = headers["Content-Type"].find("boundary=");
        if (boundaryPos != std::string::npos)
        {
            boundary = headers["Content-Type"].substr(boundaryPos + 9);
            boundary =  "--" + boundary;
            boundary.erase(boundary.find_last_not_of("\r") + 1);
            _isMultiPart = true;
        }
    }
    if (chunked == true)
        currentState = State::UNCHUNK;
    else if (contentLength > 0)
    {
        if (method == "GET")
        {
            handleError("body in a GET request");
            return ;
        }
        else
            currentState = State::BODY;
    }
    else
        currentState = State::COMPLETE;
    std::cout << "\n\n HEADERS PRINTED FROM PREPARE_REQUEST()\n\n";
    for (const auto& pair : headers) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
}

bool Request::isValidHeaderKey(const std::string& key) {
    return std::regex_match(key, std::regex("^[A-Za-z0-9\\-]+$"));
}

bool Request::isValidHeaderValue(const std::string& key, const std::string& value) {
    if (key == "Content-Length") {
        return std::regex_match(value, std::regex("^\\d+$"));
    }
    if (key == "Content-Type") {
        return std::regex_match(value, std::regex("^[a-zA-Z0-9\\-]+/[a-zA-Z0-9\\-]+(;\\s*[^;]+=[^;]+)*$"));
    }
    return true;
}

void Request::parseHeaders() {
    std::string headerLine;
    while (std::getline(requestStream, headerLine) && headerLine != "\r") {
        removeCarriageReturn(headerLine);
        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos) {
            std::string key = headerLine.substr(0, colonPos);
            std::string value = headerLine.substr(colonPos + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            if (!isValidHeaderKey(key)) {
                handleError("Invalid header key format: " + key);
                return;
            }
            if (!isValidHeaderValue(key, value)) {
                handleError("Invalid header value format for " + key + ": " + value);
                return;
            }
            headers[key] = value;
            std::cout << key << value << std::endl;
        }
    }
    prepareRequest();
}


void Request::parseChunks()
{
    static bool inChunk = false;
    static size_t chunkSize = 0;

    std::cout << "PARSING CHUNKS" << std::endl;
    std::vector<char> buffer(std::istreambuf_iterator<char>(requestStream), {});
    std::string chunkBuffer(buffer.begin(), buffer.end());
    rawChunkedData.append(std::string(buffer.begin(), buffer.end()));
    checkline(rawChunkedData);
    if (!inChunk) {
        size_t pos = rawChunkedData.find("\r\n");
        if (pos == std::string::npos) {
            currentState = State::INCOMPLETE;
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
            contentLength = body.size();
            checkline(body);
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
    std::vector<char> buffer(std::istreambuf_iterator<char>(requestStream), {});
    // std::cout << "\n\n\n\n BUFFER: \n";
    // std::cout << std::string(buffer.begin(), buffer.end());
    // std::cout << "\n\n BODY SIZE: " << body.size() << " CONTENT-LENGTH: " << contentLength << "\n\n\n";
    body.append(std::string(buffer.begin(), buffer.end()));
    if (body.size() > contentLength)
    {
        handleError("body size and content length dont match");
        std::cout << "\033[33m" << "body size: " << body.size() << " contentLength: " << contentLength << "\033[0m" << std::endl;
        return ;
    }
    else if ((body.size() == contentLength))
    {
        if (_isMultiPart) {
            currentState = State::MULTIPARTDATA;
            return;
        }
        else
        {
            currentState = State::COMPLETE;
            std::cout << "REQUEST PARSING COMPLETE" << std::endl;
        }
    }
    else
        currentState = State::INCOMPLETE;
}


void Request::parseMultipartData()
{
    size_t start = 0;

    while ((start = body.find(boundary, start)) != std::string::npos) {
        start += boundary.length();
        size_t end = body.find(boundary, start);
        std::string part = body.substr(start, end - start);
        if (part == "--\r\n")
        {
            currentState = State::COMPLETE;
            break ;
        }
        multipartData.push_back(createData(part));
        start = end;
    }
}

MultipartData Request::createData(std::string &rawData)
{
    MultipartData multipartData;

    rawData.erase(0, rawData.find_first_not_of("\r\n"));
    std::istringstream rawDataStream(rawData);

    createMultipartHeaders(multipartData, rawDataStream);
    createMultipartBody(multipartData, rawDataStream);
    return multipartData;
}


void    Request::createMultipartHeaders(MultipartData &multipartData, std::istringstream &rawDataStream)
{
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
    constexpr size_t CHUNK_SIZE = 4096;
    char buffer[CHUNK_SIZE];

    while (rawMultipartData) {
        rawMultipartData.read(buffer, CHUNK_SIZE);
        std::streamsize bytesRead = rawMultipartData.gcount();
        if (bytesRead > 0) {
            multipartData.data.insert(multipartData.data.end(), buffer, buffer + bytesRead);
        }
    }
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






