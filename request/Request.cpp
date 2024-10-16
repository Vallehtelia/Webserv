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

    // Read the first line (request line)
    std::getline(requestStream, requestLine);
    std::istringstream requestLineStream(requestLine);
    requestLineStream >> method >> path >> version;

    // Read headers
    std::string headerLine;
    int contentLength = 0;
    
    while (std::getline(requestStream, headerLine) && headerLine != "\r") {
        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos) {
            std::string key = headerLine.substr(0, colonPos);
            std::string value = headerLine.substr(colonPos + 1);
            // Trim leading whitespace from value
            value.erase(0, value.find_first_not_of(" \t"));
            headers[key] = value;
            //std::cout << key << " : " << value << std::endl;
            // Check if this is the Content-Length header
            if (key == "Content-Length") {
                contentLength = std::stoi(value);  // Parse content length
            }
        }
    }

    // If there is a body, extract it using Content-Length
    if (contentLength > 0) {
        body.resize(contentLength);  // Resize the body string to fit the content
        requestStream.read(&body[0], contentLength);  // Read the exact content length

        // Handle multipart form-data (POST request)
        if (method == "POST" && headers["Content-Type"].find("multipart/form-data") != std::string::npos) {
            size_t boundaryPos = headers["Content-Type"].find("boundary=");
            if (boundaryPos != std::string::npos) {
                std::string boundary = headers["Content-Type"].substr(boundaryPos + 9); // "boundary=" is 9 characters
                boundary.erase(0, boundary.find_first_not_of("\r\n "));
                boundary.erase(boundary.find_last_not_of("\r\n ") + 1);
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


void Request::parseMultipartData(const std::string& boundary) {
    std::string delimiter =  "--" + boundary;
    std::string closingBoundary = delimiter + "--";
    //std::cout << body << std::endl;
    size_t start = 0, end = 0;

    while ((start = body.find(delimiter, end)) != std::string::npos)
    {
        end = body.find(delimiter, start + delimiter.length());
        std::string part = body.substr(start + delimiter.length(), end - delimiter.length());
        part.erase(0, part.find_first_not_of("\r\n"));
        part.erase(part.find_last_not_of("\r\n") + 1);
        checkline(part);
        if (part ==  "--")
            break;
        std::istringstream partStream(part);
        std::string line;
        std::string partHeaders;
        std::string partContent;
        bool isContent = false;
        MultipartData multipartPart;  // Create a new multipartPart instance for each part
    while (std::getline(partStream, line)) {
    std::string sanitizedLine = line;
    sanitizedLine.erase(0, sanitizedLine.find_first_not_of("\r\n"));
    sanitizedLine.erase(sanitizedLine.find_last_not_of("\r\n") + 1);
    // Trim leading/trailing whitespace including spaces
    if (line.empty() || line == "\r" || line == "\n")
    {
        isContent = true;
        continue;
    }
    if (sanitizedLine == closingBoundary)
        break ;
    if (!isContent) {
        partHeaders += line + "\n";
    } else {
        partContent += line + "\n";
    }
}

        std::cout << "Part Headers: " << partHeaders << std::endl;
        std::cout << "Part Content: " << partContent << std::endl;

        // Extract "Content-Disposition" for the field name or file name
        size_t contentDispositionPos = partHeaders.find("Content-Disposition:");
        if (contentDispositionPos != std::string::npos) {
            size_t namePos = partHeaders.find("name=\"", contentDispositionPos);
            if (namePos != std::string::npos) {
                size_t nameEnd = partHeaders.find("\"", namePos + 6);
                multipartPart.name = partHeaders.substr(namePos + 6, nameEnd - (namePos + 6));

                // Check if it's a file (i.e., has a filename)
                size_t filenamePos = partHeaders.find("filename=\"", nameEnd);
                if (filenamePos != std::string::npos) {
                    size_t filenameEnd = partHeaders.find("\"", filenamePos + 10);
                    multipartPart.filename = partHeaders.substr(filenamePos + 10, filenameEnd - (filenamePos + 10));
                }

                    multipartPart.contentType = headers["Content-Type"];

                // Set content (body) for the field or file
                multipartPart.data = std::vector<char>(partContent.begin(), partContent.end());

                // Add to the multipartData vector
                multipartData.push_back(multipartPart);
                // Debug output for each part added
                std::cout << "Added part: " << multipartPart.name 
                          << " with filename: " << multipartPart.filename 
                          << " and content type: " << multipartPart.contentType << std::endl;
            }
        }
}
}
