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
        location = rhs.location;
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

LocationConfig  Request::getLocation() const {
    return location;
}

std::map<std::string, std::string>  Request::getQueryParams() const
{
    return queryParams;
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
    location = LocationConfig();
}


void Request::handleError(const std::string& errorMsg) {
    std::cerr << "\033[31m" << "Error: " << errorMsg << "\033[0m" << std::endl;
    currentState = State::ERROR;
}

void Request::parseRequest(std::string &rawRequest,const Socket &socket) {
    requestStream.str(rawRequest);
    if (currentState == State::INCOMPLETE)
        currentState = State::BODY;
    if (chunked)
        currentState = State::UNCHUNK;
    while (currentState != State::COMPLETE && currentState != State::ERROR && currentState != State::INCOMPLETE) {
        // std::cout << "REQUEST PARSING: " << stateToString(currentState) << std::endl;
        switch (currentState) {
            case State::REQUEST_LINE:
                parseRequestLine(socket);
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

LocationConfig Request::findLocation(const std::string &uri, const Socket &socket) {
    LocationConfig bestMatch; // Default location config to return if no match
    size_t bestMatchLength = 0; // Track the length of the best match
    std::vector<LocationConfig> locations = socket.getServer().getLocations();

    for (std::vector<LocationConfig>::iterator it = locations.begin(); it != locations.end(); ++it) {
        std::string locationPath = it->getLocation();

        // Ensure no trailing slash in location unless it’s "/"
        if (locationPath != "/" && locationPath.back() == '/')
            locationPath = locationPath.substr(0, locationPath.length() - 1);

        // Check if locationPath is a prefix of uri
        if (uri.find(locationPath) == 0) {
            // Ensure match is valid:
            // - If location is "/", always match.
            // - If location is not "/", match only if uri follows the pattern "/locationPath/..." or matches exactly.
            if (locationPath == "/" || uri == locationPath || uri[locationPath.length()] == '/') {
                if (locationPath.length() > bestMatchLength) {
                    bestMatch = *it;
                    bestMatchLength = locationPath.length();
                }
            }
        }
    }

    return bestMatch;
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



void Request::parseRequestLine(const Socket &socket) {
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
            location = findLocation(uri, socket);
            currentState = State::HEADERS;
        } else {
            handleError("Invalid request line format.");
        }
    } else {
        handleError("Request line missing.");
    }
}

bool isMethodAllowed(const std::vector<std::string>& allowedMethods, const std::string& method) {
    return std::find(allowedMethods.begin(), allowedMethods.end(), method) != allowedMethods.end();
}

void Request::prepareRequest()
{
    if (!isMethodAllowed(location.getAllowMethods(), method))
    {
        return handleError("METHOD not allowed in" + location.getLocation());
    }
    if ((method == "POST" || method == "PUT"))
    {
        if (headers.find("content-length") != headers.end()) {
            contentLength = std::stoi(headers["content-length"]);
        } else {
            handleError("Content-Length missing on a POST request");
            return ;
        }
        // if (contentLength > 500000)
        // {
        //     return handleError("body size manually limited to 500000 in prepareRequest()");
        // }
    }
    if (headers.find("transfer-encoding") != headers.end())
        if (headers["transfer-encoding"] == "chunked")
            chunked = true;
    if (headers.find("content-type") != headers.end())
        contentType = headers["content-type"];
    if ((method == "POST" || method == "PUT") && headers["content-type"].find("multipart/form-data") != std::string::npos)
    {
        size_t boundaryPos = headers["content-type"].find("boundary=");
        if (boundaryPos != std::string::npos)
        {
            boundary = headers["content-type"].substr(boundaryPos + 9);
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
}

bool Request::isValidHeaderKey(const std::string& key) {
    return std::regex_match(key, std::regex("^[A-Za-z0-9\\-]+$"));
}

bool Request::isValidHeaderValue(const std::string& key, const std::string& value) {
    if (key == "content-length") {
        return std::regex_match(value, std::regex("^\\d+$"));
    }
    if (key == "content-type") {
        return std::regex_match(value, std::regex("^[a-zA-Z0-9\\-]+/[a-zA-Z0-9\\-]+(;\\s*[^;]+=[^;]+)*$"));
    }
    return true;
}

void Request::validateHeaders() {

    for (auto& pair : headers) {
        std::string key = pair.first;
        std::string value = pair.second;
        if (!isValidHeaderKey(key)) {
            handleError("Invalid header key format: " + key);
            return;
        }
        if (!isValidHeaderValue(key, value)) {
            handleError("Invalid header value format for " + key + ": " + value);
            return;
        }
    }
}

void Request::parseHeaders() {
    std::string headerLine;
    while (std::getline(requestStream, headerLine) && headerLine != "\r") {
        removeCarriageReturn(headerLine);
        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos) {
            std::string key = headerLine.substr(0, colonPos);
            std::transform(key.begin(), key.end(), key.begin(), ::tolower); // transform header keys to lowercase to remove case sensitivity
            std::string value = headerLine.substr(colonPos + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            headers[key] = value;
        }
    }
    validateHeaders();
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
        chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
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
    std::cout << "---------------------------------------------" << std::endl;
	location.printLocation();
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
    // printMultipartdata();
	std::cout << "---------------------------------------------" << "\033[0m" << std::endl;
}






