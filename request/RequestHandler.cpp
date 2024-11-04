#include "../response/Response.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"

RequestHandler::RequestHandler() {}
RequestHandler::~RequestHandler() {}


void RequestHandler::handleRequest(Request& req, Response& res)
{
	prepareHandler(req);
    if (_method == "GET")
	{
        handleGetRequest(res);
    }
	else if (_method == "POST")
	{
        handlePostRequest(req, res);
    }
	else if (_method == "DELETE")
	{
        handleDeleteRequest(res);
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


void RequestHandler::prepareHandler(const Request &req)
{

    std::string _headers;
	_uri = req.getUri();
	_method = req.getMethod();
	_contentType = getContentType(_uri);
    _filePath = getFilepath(_uri);
}

static std::string urlDecode(const std::string& src) {
    std::string decoded;
    for (size_t i = 0; i < src.length(); ++i) {
        if (src[i] == '%' && i + 2 < src.length()) {
            std::string hex = src.substr(i + 1, 2);
            decoded += static_cast<char>(std::stoi(hex, nullptr, 16));
            i += 2;
        } else if (src[i] == '+') {
            decoded += ' ';
        } else {
            decoded += src[i];
        }
    }
    return decoded;
}

std::string RequestHandler::getFilepath(std::string filepath)
{
	filepath = urlDecode(filepath);
	if (filepath.front() == '/' && filepath.length() > 1) {
            filepath.erase(0, 1);
    }
    std::filesystem::path baseDir = std::filesystem::current_path() / "html";
    std::filesystem::path path;
    if (_method == "GET") {
        if (filepath == "/") {
            path = baseDir / "index.html";
        } 
        else if (_contentType == "text/html" || _contentType == "text/css" || _contentType == "application/javascript") {
            path = baseDir / filepath;
        } 
        else {
            path = baseDir / "uploads" / filepath;
        }
    } 
    else if (_method == "POST" || _method == "PUT" || _method == "DELETE") {
        path = baseDir / "uploads" / filepath;
    } 
    else {
		std::cout << "somethings off!" << std::endl;
        path = std::filesystem::path(filepath);
    }
	std::cout << "FILEPATH CREATED: " << path.string() << std::endl;
    return path.string();  
}


bool RequestHandler::validFile(const std::string& filePath) {
    try {
        std::filesystem::path fullPath = std::filesystem::path(filePath); 
        const std::filesystem::path baseDir = std::filesystem::current_path() / "html";
        std::filesystem::path dirPath = std::filesystem::canonical(baseDir);

        std::cout << "Base Directory: " << baseDir << std::endl;
        std::cout << "Canonical Directory Path: " << dirPath << std::endl;
        std::cout << "Full Path: " << fullPath << std::endl;

        if (std::filesystem::relative(fullPath, dirPath).string().find("..") == 0) {
            std::cerr << "Error: Access outside base directory is prohibited." << std::endl;
			_statusCode = 403;
            return false;
        }
        if (_method == "GET" || _method == "DELETE") {
            if (!std::filesystem::exists(fullPath)) {
                std::cerr << "Error: File does not exist." << std::endl;
				_statusCode = 404;
                return false;
            }
            if ((std::filesystem::status(fullPath).permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none) {
                std::cerr << "Error: No read permission for the file." << std::endl;
				_statusCode = 403;
                return false;
            }
			_statusCode = 200;
            return true;
        } 
        else if (_method == "POST" || _method == "PUT") {
            std::filesystem::path dir = fullPath.parent_path();
            if (!std::filesystem::exists(dir)) {
                std::cerr << "Error: Directory does not exist." << std::endl;
				_statusCode = 404;
                return false;
            }
            if ((std::filesystem::status(dir).permissions() & std::filesystem::perms::owner_write) == std::filesystem::perms::none) {
                std::cerr << "Error: No write permission for the directory." << std::endl;
				_statusCode = 403;
                return false;
            }
            // if (_method == "POST" && std::filesystem::exists(fullPath)) {
            //     std::cerr << "Error: File already exists, cannot overwrite in POST request." << std::endl;
			// 	_statusCode = 409;
            //     return false;
            // }
            const auto freeSpace = std::filesystem::space(dir).available;
            const size_t maxUploadSize = 50 * 1024 * 1024;  // 50 MB limit
            if (freeSpace < maxUploadSize) {
                std::cerr << "Error: Not enough disk space for upload." << std::endl;
				_statusCode = 500;
                return false;
            }
			_statusCode = 200;
            return true;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
		_statusCode = 500;
        return false;
    }
    std::cerr << "Error: Unsupported method for file operation: " << _method << std::endl;
	_statusCode = 405;
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

void RequestHandler::handleGetRequest(Response& res) {

    std::cout << "HANDLE GET" << std::endl;
    if (validFile(_filePath)) {
        std::cout << "FILE IS LEGIT" << std::endl;
       	_body = readFileContent(_filePath);
        res.setResponse(200, getContentType(_filePath), _body);
    } else {
        std::cout << "FILE IS NOT FOUND" << std::endl;
        res.setResponse(404, "text/html", _body);
    }
}

void RequestHandler::handleDeleteRequest(Response& res)
{

    if (validFile(_filePath))
    {
        if (remove(_filePath.c_str()) == 0)
            res.setResponse(200, "text/plain", "SUCCESS!");
        else
            res.setResponse(500, "text/html", "");
    }
    else
        res.setResponse(404, "text/html", "");
}


void RequestHandler::handlePostRequest(const Request& req, Response& res)
{
    std::cout << "HANDLING POST REQUEST" << std::endl;
	std::cout << "CONTENT TYPE: " << req.getContentType() << std::endl;
    if (req.isMultiPart()) {
        handleMultipartRequest(req, res);
    } 
    else if (req.getContentType() == "application/json\r") {
		std::cout << "hola" << std::endl;
        handleJsonData(req, res);
    } 
    // else if (req.getContentType() == "text/plain") {
    //     handlePlainText(req, res);
    // } 
    else {
		std::cout << "here?"  << std::endl;
        res.setResponse(400, "text/html", "Bad Request: Unsupported Content Type");
    }
}

void RequestHandler::handleJsonData(const Request &req, Response &res)
{
	std::cout << "HANDLE JSON DATA" << std::endl;
	if (validFile(_filePath))
	{
        std::ofstream file(_filePath, std::ios::out | std::ios::trunc | std::ios::binary);
        if (file)
        {
            file << req.getBody();
            file.close();
            res.setResponse(200, "application/json", req.getBody());
        }
    }
	else
		res.setResponse(_statusCode, "text/html", _body);
}



void RequestHandler::handleMultipartRequest(const Request &req, Response &res) {
	std::cout << "HANDLE MULTIPART REQUEST" << std::endl;
    const auto& multipartData = req.getMultipartData();
    std::unordered_map<std::string, std::string> formData;

    for (const auto& part : multipartData) {
        if (part.filename.empty()) {
            handleFormField(part, formData);
        } else {
            handleFileUpload(part, res);
			if (_statusCode != 200 && _statusCode != 201)
			{
				//res.setResponse(_statusCode, "text/html", "error");
				return ;
			}
        }
    }
    _body = createJsonResponse(formData);
    res.setResponse(200, "application/json", _body);
}

void RequestHandler::handleFileUpload(const MultipartData& part, Response& res)
{
    if (part.filename.empty()) {
        res.setResponse(400, "application/json", R"({"error": "File name is missing."})");
        return;
    }
    std::string uploadPath = getFilepath(part.filename);
    if (validFile(uploadPath)) {
    	std::ofstream file(uploadPath, std::ios::binary);
		if (file)
		{
			std::cout << "WRITING TO FILE" << uploadPath << std::endl;
        	file.write(part.data.data(), part.data.size());
        	file.close();
        	std::string responseBody = "{\"status\": \"File uploaded successfully\", \"filename\": \"" + part.filename + "\"}";
        	res.setResponse(200, "application/json", responseBody);
			_statusCode = 200;
		}
    } else {
		res.setResponse(_statusCode, "text/html", "error");
		return ;
    }
}

void RequestHandler::handleFormField(const MultipartData& part, std::unordered_map<std::string, std::string>& formData) {
    if (part.filename.empty()) {
        std::string fieldName = part.name;
        std::string fieldValue(part.data.begin(), part.data.end());
        formData[fieldName] = fieldValue;
        std::cout << "Form field received - " << fieldName << ": " << fieldValue << std::endl;
    }
}

std::string RequestHandler::createJsonResponse(const std::unordered_map<std::string, std::string>& formData) {
    std::stringstream json;
    json << "{";

    for (auto it = formData.begin(); it != formData.end(); ++it) {
        if (it != formData.begin()) json << ", ";
        json << "\"" << it->first << "\": \"" << it->second << "\"";
    }

    json << "}";
    return json.str();
}


