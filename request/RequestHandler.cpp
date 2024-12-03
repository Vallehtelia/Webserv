#include "../response/Response.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"

RequestHandler::RequestHandler() {}
RequestHandler::~RequestHandler() {}


void RequestHandler::handleRequest(Request& req, Response& res)
{
    _location = req.getLocation();

	if (req.getUri() == "/")
		req.setPath("/index.html");
	if (req.getState() == State::CGI_ERROR) // Naa johonki siistimmin
	{
		res.setResponse(500, "text/html", "");
		return ;
	}
	if (req.getState() == State::TIMEOUT)
	{
		res.setResponse(504, "text/html", "");
		return ;
	}
	if (req.getState() == State::CGI_NOT_FOUND)
	{
		res.setResponse(404, "text/html", "");
		return ;
	}
	if (req.getState() == State::CGI_NOT_PERMITTED)
	{
		res.setResponse(403, "text/html", "");
		return ;
	}
    if (req.getState() == State::ERROR)
    {
        std::cout << "it gets here!" << std::endl;
        res.setResponse(400, "text/html", "");
        std::cout << "but not here" << std::endl;
        return ;
    }
	prepareHandler(req);
    if (_method == "GET")
	{
        handleGetRequest(res);
    }
	else if (_method == "POST")
	{
        handlePostRequest(req, res);
    }
	else if (_method == "PUT")
	{
        handlePutRequest(req, res);
    }
	else if (_method == "DELETE")
	{
        handleDeleteRequest(res);
    }
	else
	{
        res.setResponse(405, "text/html", "");
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


void removePrefix(std::string& str, const std::string& prefix) {
    if (str.rfind(prefix, 0) == 0) { // Check if the string starts with the prefix
        str.erase(0, prefix.length()); // Remove the prefix
    }
}

std::string RequestHandler::getFilepath(std::string filepath)
{
	filepath = urlDecode(filepath);
    removePrefix(filepath, _location.getRoot());
	if (filepath.front() == '/' && filepath.length() > 1) {
            filepath.erase(0, 1);
    }
	std::string tmp = _location.getLocation();
	removePrefix(tmp, _location.getRoot());
	_location.path = tmp;
	tmp = _location.getRoot();
	removePrefix(tmp, "/");
    std::filesystem::path baseDir = std::filesystem::current_path() / tmp;
    std::filesystem::path path;
    if (filepath.find("cgi") != std::string::npos)
        path = baseDir / "tmp" / filepath;
    else {
            path = baseDir / filepath;
    }
	std::cout << "FILEPATH CREATED: " << path.string() << std::endl;
    return path.string();
}


bool RequestHandler::validFile(const std::string& filePath) {
    try {
        std::filesystem::path fullPath = std::filesystem::path(filePath);
        const std::filesystem::path baseDir = std::filesystem::current_path();
        std::cout << "Full Path: " << fullPath << std::endl;
        std::cout << "Base Directory: " << baseDir << std::endl;

        std::filesystem::path dirPath = std::filesystem::canonical(baseDir);

        std::cout << "Canonical Directory Path: " << dirPath << std::endl;

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

std::string generateDirectoryListing(const std::string& directoryPath) {
    std::ostringstream jsonStream;
    jsonStream << "{\n\t\"files\": [\n";

    try {
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
            jsonStream << "\t\t{\n";
            jsonStream << "\t\t\t\"name\": \"" << entry.path().filename().string() << "\",\n";
            jsonStream << "\t\t\t\"type\": \""
                       << (entry.is_directory() ? "directory" : "file") << "\",\n";

            if (!entry.is_directory()) {
                jsonStream << "\t\t\t\"size\": " << entry.file_size() << "\n";
            } else {
                jsonStream << "\t\t\t\"size\": null\n";
            }

            jsonStream << "\t\t},\n";
        }

        std::string result = jsonStream.str();
        // Remove the trailing comma and newline for valid JSON
        if (result.size() > 2) {
            result.erase(result.size() - 2, 2);
        }
        result += "\n\t]\n}";

        return result;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
        return "{\"error\": \"Unable to access directory.\"}";
    }
}

void RequestHandler::handleGetRequest(Response& res) {

    std::cout << "HANDLE GET" << std::endl;
    std::cout << "----------" << std::endl;
    std::cout << "Path: " << _filePath << std::endl;
    std::cout << "Is directory: " << std::filesystem::is_directory(_filePath) << std::endl;
    std::cout << "----------" << std::endl;
    if (std::filesystem::is_directory(_filePath)){
        std::cout << "it is a directory" << std::endl ;
        if (_location.isAutoindex()) { // Assuming there's a method to check if auto-index is enabled
            std::string directoryListing = generateDirectoryListing(_filePath);
            res.setResponse(200, "text/html", directoryListing);
        } else {
            res.setResponse(403, "text/html", "Auto-indexing is disabled for this directory.");
        }
    }
    else if (validFile(_filePath)) {
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
    else if (req.getContentType() == "application/json") {
        handleJsonData(req, res);
    }
    // else if (req.getContentType() == "text/plain") {
    //     handlePlainText(req, res);
    // }
    else {
        res.setResponse(400, "text/html", "Bad Request: Unsupported Content Type");
    }
}

void RequestHandler::handlePutRequest(const Request& req, Response& res)
{
    std::cout << "HANDLING PUT REQUEST" << std::endl;
	std::cout << "CONTENT TYPE: " << req.getContentType() << std::endl;

    if (req.isMultiPart()) {
        handleMultipartRequest(req, res);
    }
    else if (req.getContentType() == "application/json") {
		std::cout << "Processing JSON Data for PUT" << std::endl;
        handleJsonData(req, res);
    }
    else {
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
    {
        std::cout << "ERROR: COULD NOT OPEN FILE: " << _filePath << std::endl;
		res.setResponse(_statusCode, "text/html", _body);
    }
}



void RequestHandler::handleMultipartRequest(const Request &req, Response &res) {
	std::cout << "HANDLE MULTIPART REQUEST" << std::endl;
    const auto& multipartData = req.getMultipartData();
    std::vector<std::string> uploadedFiles;
    bool isCgi = false;

    _body = "{\n\t\"uploadedFiles\": [\n";
    for (const auto& part : multipartData) {
        if (part.filename.empty()) {
            // handleFormField(part, formData);
            continue ;
        }
        else if (req.getUri().find("cgi") != std::string::npos) {
            // edited file uploaded to tmp folder
            std::string response_body = readFileContent(_filePath);
            res.setResponse(200, getContentType(_filePath), response_body);
            isCgi = true;
        }
        else {
            handleFileUpload(part, res);
            if (_statusCode == 200)
            {
                uploadedFiles.push_back("{\"filename\" : \"" + part.filename + "\", \"fileurl\" : \"uploads/" + part.filename + "\"}");
            }
			if (_statusCode != 200 && _statusCode != 201)
			{
				//res.setResponse(_statusCode, "text/html", "error");
				return ;
			}
        }
    }
    if (!isCgi) {
        _body = createJsonResponse(uploadedFiles);
        res.setResponse(200, "application/json", _body);
    }
}

// {
//   "uploadedFiles": [
//     {
//       "fileName": "image1.jpg",
//       "fileUrl": "https://example.com/uploads/image1.jpg",
//       "fileId": "abc123"
//     },
//     {
//       "fileName": "image2.png",
//       "fileUrl": "https://example.com/uploads/image2.png",
//       "fileId": "xyz789"
//     }
//   ]
// }

void RequestHandler::handleFileUpload(const MultipartData& part, Response& res)
{
    if (part.filename.empty()) {
        res.setResponse(400, "application/json", R"({"error": "File name is missing."})");
        return;
    }
    std::string uploadPath = getFilepath("/uploads/" + part.filename);
    if (validFile(uploadPath)) {
    	std::ofstream file(uploadPath, std::ios::binary);
		if (file)
		{
			std::cout << "WRITING TO FILE" << uploadPath << std::endl;
        	file.write(part.data.data(), part.data.size());
        	file.close();
			_statusCode = 200;
		}
    } else {
		res.setResponse(_statusCode, "text/html", "error");
		return ;
    }
}

void RequestHandler::handleFormField(const MultipartData& part) {
    if (part.filename.empty()) {
        // textToBody += part.name;
        // std::string fieldValue(part.data.begin(), part.data.end());
        // formData[fieldName] = fieldValue;
        // std::cout << "Form field received - " << fieldName << ": " << fieldValue << std::endl;
        return ;
    }
}

std::string RequestHandler::createJsonResponse(const std::vector<std::string> uploadedFiles) {
    std::string json;
    json += "{\n\t\"uploadedFiles\": [\n";

    for (size_t i = 0; i < uploadedFiles.size(); i++)
    {
        json += "\t\t" + uploadedFiles[i];
        if (i != uploadedFiles.size() - 1) {
            json += ",\n";
        }
    }
    json += "\n\t]\n}";
    return json;
}


