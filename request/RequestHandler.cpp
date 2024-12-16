#include "../response/Response.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"

RequestHandler::RequestHandler() {}
RequestHandler::~RequestHandler() {}


void RequestHandler::handleRequest( Request& req, Response& res)
{
	_location = req.getLocation();


    if (req.getState() == State::ERROR)
    {
        res.setResponse(req.getErrorCode(), "text/html", "");
        return ;
    }
    prepareHandler(req);

    if (_location.getLocation() == "/cgi")
    {
        readCgiOutputFile();
    }
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
        res.setResponse(405, "text/html", "");
    }
}


void RequestHandler::readCgiOutputFile() {
    std::ifstream inputFile(_filePath);

    if (!inputFile.is_open()) {
        std::cerr << RED << "Error: Could not open file for reading." << DEFAULT << std::endl;
        return;
    }

    std::string contentType;
    std::string line;

    std::getline(inputFile, contentType);

    std::size_t colonPos = contentType.find(':');
    if (colonPos != std::string::npos) {
        _contentType = contentType.substr(colonPos + 1);
        _contentType.erase(0, _contentType.find_first_not_of(" \t\r\n"));
    }

    std::string bodyContent((std::istreambuf_iterator<char>(inputFile)),
                             std::istreambuf_iterator<char>());

    _body = bodyContent;
    inputFile.close();

    // std::cout << "----- CGI RESPONSE -----" << std::endl;
    // std::cout << "content-type: " << _contentType << std::endl;
    // std::cout << "body: \n" << _body << std::endl;
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

bool isStaticResource(const std::string& uri) {
    return uri.find("/assets/") == 0 || uri.find("/styles/") == 0 ||
           uri.find("/scripts/") == 0 || uri == "/favicon.ico";
}

void RequestHandler::prepareHandler(const Request &req)
{
    _location = req.getLocation();
	_uri = req.getUri();
	_method = req.getMethod();
	_contentType = getContentType(_uri);
    _filePath = getFilepath(_uri);
    _responseFileUrl = _uri;
	_headers = req.getHeaders();
	_cookies = req.getCookies();

    // Session management
	auto& sessionManager = SessionManager::getInstance();
	sessionManager.printSessions();
    if (_cookies.find("session_id") != _cookies.end()) {
        std::string sessionId = _cookies["session_id"];
        if (sessionManager.isValidSession(sessionId)) {
            _sessionId = sessionId;
			sessionManager.validateAndExtendSession(sessionId);
        } else {
            std::cout << "Invalid session: " << sessionId << std::endl;
            _sessionId = sessionManager.createDummySession();
			std::cout << "Creating a new session with ID: " << _sessionId << std::endl;
			_responseCookies.push_back("session_id=" + _sessionId + "; Path=/; HttpOnly");
        }
    } else {
        _sessionId = sessionManager.createDummySession();
		std::cout << "No session_id found, creating a new session with ID: " << _sessionId << std::endl;
		_responseCookies.push_back("session_id=" + _sessionId + "; Path=/; HttpOnly");
    }
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

std::string trimSlashes(const std::string& str) {
    if (str.empty()) return str;

    size_t start = 0;
    size_t end = str.size() - 1;

    if (str[start] == '/') ++start;
    if (str[end] == '/') --end;

    return str.substr(start, end - start + 1);
}

void removePrefix(std::string& str, const std::string& prefix) {
    std::string normalizedPrefix = prefix;
    if (!prefix.empty() && prefix.back() == '/') {
        normalizedPrefix.pop_back();
    }
    if (str.rfind(normalizedPrefix, 0) == 0) {
        str.erase(0, normalizedPrefix.length());
    }
}

std::string RequestHandler::getFilepath(std::string filepath)
{
    filepath = urlDecode(filepath);
    std::string tmp = _location.getLocation();
    removePrefix(tmp, _location.getLocation());
    std::string locationRoot = _location.getRoot();
    removePrefix(locationRoot, "/");

    removePrefix(filepath, _location.getLocation());
    std::filesystem::path baseDir = std::filesystem::current_path();
    std::string baseDirStr = baseDir.string();
    std::string path;
    path = baseDirStr + locationRoot + filepath;
    return path;
}




bool RequestHandler::validFile(const std::string& filePath) {
    try {
        std::filesystem::path fullPath = std::filesystem::path(filePath);
        const std::filesystem::path baseDir = std::filesystem::current_path();
        std::filesystem::path dirPath = std::filesystem::canonical(baseDir);

        std::cout << "validating file: " << fullPath << std::endl;
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
        else if (_method == "POST") {
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

std::string RequestHandler::generateDirectoryListing(const std::string& directoryPath) {
    std::ostringstream jsonStream;


    jsonStream << "{\n\t\"directoryPath\":\"" << _responseFileUrl << "\",\n\t\"files\": [\n";

    try {
        bool isFirstEntry = true;
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
            if (!isFirstEntry) {
                jsonStream << ",\n";
            }
            jsonStream << "\t\t{\n";
            jsonStream << "\t\t\t\"name\": \"" << entry.path().filename().string() << "\",\n";
            jsonStream << "\t\t\t\"type\": \""
                       << (entry.is_directory() ? "directory" : "file") << "\",\n";
            jsonStream << "\t\t\t\"fileurl\": \"" << _responseFileUrl + "/" + entry.path().filename().string() << "\",\n";

            if (!entry.is_directory()) {
                jsonStream << "\t\t\t\"size\": " << entry.file_size() << "\n";
            } else {
                jsonStream << "\t\t\t\"size\": null\n";
            }

            jsonStream << "\t\t}";
            isFirstEntry = false;
        }

        if (!isFirstEntry) {
            jsonStream << "\n";
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
        return "{\"error\": \"Unable to access directory.\"}";
    }
    jsonStream << "\t]\n}";

    std::string result = jsonStream.str();
    return result;
}

// new helpers for sessions:
void logCookies(const std::unordered_map<std::string, std::string>& cookies) {
    std::cout << "Cookies received:" << std::endl;
    for (const auto& [key, value] : cookies) {
        std::cout << "  " << key << " = " << value << std::endl;
    }
}

void logSessionData(const std::string& sessionId, const SessionData& sessionData) {
    std::cout << "Session ID: " << sessionId << std::endl;
    for (const auto& [key, value] : sessionData.data) {
        std::cout << "  " << key << " = " << value << std::endl;
    }
    std::time_t expirationTimeT = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now() +
        (sessionData.expirationTime - std::chrono::system_clock::now()));
    std::cout << "  Expiration Time: "
              << std::put_time(std::localtime(&expirationTimeT), "%Y-%m-%d %H:%M:%S") << std::endl;
}

void RequestHandler::handleGetRequest(Response& res) {
	std::cout << YELLOW << "GET REQUEST " << _uri << DEFAULT << std::endl;

	auto& sessionManager = SessionManager::getInstance();
	if (sessionManager.hasSession(_sessionId)) {
    	auto& sessionData = sessionManager.getSession(_sessionId);

		// Update "last_visited" in session data
		if (sessionData.data.find("last_visited") == sessionData.data.end()) {
			sessionData.data["last_visited"] = _uri;
		} else {
			sessionData.data["last_visited"] = _uri; // Update last visited page
		}
	} else {
		_sessionId = sessionManager.createDummySession();
		auto& newSessionData = sessionManager.getSession(_sessionId);
		newSessionData.data["last_visited"] = _uri;
	}

    if (std::filesystem::is_directory(_filePath)){
        if (_location.isAutoindex()) {
            std::string directoryListing = generateDirectoryListing(_filePath);
            res.setResponse(200, "text/html", directoryListing);
        } else {
			std::string	tmp = res.getSocket().getServer().getIndex();
			_body = readFileContent(tmp);
            res.setResponse(200, "text/html", _body);
        }
    }
    else if (validFile(_filePath)) {
        if (_location.getLocation() == "/cgi")
            res.setResponse(200, _contentType, _body, _responseCookies);
        else
        {
       	    _body = readFileContent(_filePath);
            res.setResponse(200, getContentType(_filePath), _body, _responseCookies);
        }
    } else {
        std::cerr << RED << "FILE IS NOT FOUND AT: " << _filePath << DEFAULT << std::endl;
        res.setResponse(404, "text/html", _body);
    }
}

void RequestHandler::handleDeleteRequest(Response& res)
{
    if (_location.getLocation() == "/cgi")
    {
            res.setResponse(200, _contentType, _body);
    }
    else if (validFile(_filePath))
    {
        if (remove(_filePath.c_str()) == 0)
        {
            res.setResponse(200, "application/json",
                R"({"status":"success","message":"File deleted successfully","filePath":")" + _filePath + R"("})");
        }
        else
        {
            res.setResponse(500, "application/json",
                R"({"status":"error","message":"Failed to delete the file","filePath":")" + _filePath + R"("})");
        }
    }
    else
    {
        res.setResponse(404, "application/json",
            R"({"status":"error","message":"File not found","filePath":")" + _filePath + R"("})");
    }
}



void RequestHandler::handlePostRequest(const Request& req, Response& res)
{
    std::cout << YELLOW << "POST REQUEST " << _uri << DEFAULT << std::endl;
    if (req.isMultiPart()) {
        handleMultipartRequest(req, res);
    }
    else if (req.getContentType() == "application/json") {
        handleJsonData(req, res);
    }
    else {
        res.setResponse(400, "text/html", "Bad Request: Unsupported Content Type");
    }
}


void RequestHandler::handleJsonData(const Request &req, Response &res)
{
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
    const auto& multipartData = req.getMultipartData();
    std::vector<std::string> uploadedFiles;
    bool isCgi = false;

    for (const auto& part : multipartData) {
        if (part.filename.empty()) {
            continue ;
        }
        else if (req.getUri().find("cgi") != std::string::npos) {
            readCgiOutputFile();
            res.setResponse(200, _contentType, _body);
            isCgi = true;
        }
        else {
            handleFileUpload(part, res);
            if (_statusCode == 200)
            {
                uploadedFiles.push_back("{\"filename\" : \"" + part.filename + "\", \"fileurl\" : \"" + trimSlashes(_uri) + "/" + trimSlashes(part.filename) + "\"}");
            }
			if (_statusCode != 200 && _statusCode != 201)
			{
				return ;
			}
        }
    }
    if (!isCgi) {
        _body = createJsonResponse(uploadedFiles);
        res.setResponse(200, "application/json", _body);
    }
}



void RequestHandler::handleFileUpload(const MultipartData& part, Response& res)
{
    if (part.filename.empty()) {
        res.setResponse(400, "application/json", R"({"error": "File name is missing."})");
        return;
    }
    _uploadPath = getFilepath("/" + part.filename);
    if (validFile(_uploadPath)) {
    	std::ofstream file(_uploadPath, std::ios::binary);
		if (file)
		{
        	file.write(part.data.data(), part.data.size());
        	file.close();
			_statusCode = 200;
		}
    } else {
		res.setResponse(_statusCode, "text/html", "error");
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


