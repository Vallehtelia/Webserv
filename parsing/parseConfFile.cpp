
#include "ServerConfig.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <thread>
#include <chrono>

bool	checkConfFile(const std::string &filename)
{
	std::ifstream	file(filename);

	if (!file.is_open())
	{
		std::cerr << RED << "Error: could not open file" << DEFAULT << std::endl;
		return false;
	}
	std::cout << GREEN << "File found [OK]\033[0m" << DEFAULT << std::endl;
	return true;
}

// Function to trim leading and trailing whitespace
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

// Function to parse configuration data from the input line
static bool parseServerData(ServerConfig& server, const std::string& line) {
	std::string trimmedLine = trim(line);

	// Check if the line is empty or starts with a comment (you can extend this to support comments)
	if (trimmedLine.empty() || trimmedLine[0] == '#') return 0;

	// Find the position of the first space to separate the key and value
	size_t spacePos = trimmedLine.find(' ');
	if (spacePos == std::string::npos) {
		std::cout << "Invalid server configuration value: " << trimmedLine << std::endl;
		return 1;  // No space means invalid line
	}

	// Extract the key and value from the line
	std::string key = trimmedLine.substr(0, spacePos);
	std::string value = trim(trimmedLine.substr(spacePos + 1));  // Remove extra spaces from the value

	// Process the key and assign the value
	if (key == "listen" || key == "client_max_body_size" || key == "max_events") {
		server.setConfig(key, std::stoi(value));  // Assume listen is an integer (port)
		return 0;
	}
	else if (key == "server_name" || key == "host" || key == "root" || key == "index") {
		server.setConfig(key, value.erase(value.find_last_of(";")));  // server_name is a string
		return 0;
	}
	else if (key == "error_page") {
		// Assuming error_page contains the error code and file path
		size_t spacePos = value.find(' ');
		if (spacePos != std::string::npos)
		{
			try
			{
				int code = std::stoi(value.substr(0, spacePos));
				std::string page = value.substr(spacePos + 1);
				page.erase(page.find_last_of(";"));
				server.addErrorPage(code, page);
			}
			catch (const std::exception &e)
			{
				std::cerr << RED << "Invalid error_page format: " << value << " (" << e.what() << ")" << DEFAULT << std::endl;
				return 1;
			}
		}
		return 0;
	}
	else {
		std::cerr << "Unknown configuration key: " << key << std::endl;
		return 1;
	}
}


static void parseLocationData(LocationConfig &location, const std::string &key, const std::string &line) {
    std::string value;

    // Extract value based on key's length
    size_t value_start = key.length();
    if (value_start >= line.size()) {
        std::cerr << "Malformed configuration line: " << line << std::endl;
        return;
    }

    // Trim value and remove trailing semicolon
    value = trim(line.substr(value_start));
    size_t semicolon_pos = value.find(';');
    if (semicolon_pos != std::string::npos) {
        value = value.substr(0, semicolon_pos);
    } else {
	    std::cerr << "Malformed configuration line: " << line << std::endl;
        return;	
	}

    // Handle key-specific logic
    if (key == "root") {
        location.root = value;
    } else if (key == "allow_methods") {
        // Split space-separated methods
        std::istringstream iss(value);
        std::string method;
        while (iss >> method) {
            location.allow_methods.push_back(method);
        }
    } else if (key == "autoindex") {
        location.autoindex = (value == "on");
    } else if (key == "index") {
        location.index = value;
    } else if (key == "redirect") {
        location.redirect = value;
    } else if (key == "cgi_path") {
        location.cgi_path = value;
    } else {
        std::cerr << "Unknown configuration key: " << key << std::endl;
    }
}

static void	parseLocationBlock(LocationConfig &location, std::ifstream &file, std::string line)
{
	std::string		data[6] = {"root", "allow_methods", "autoindex", "index", "redirect", "cgi_path"};
	std::string		value;

	value = line;
	location.path = value.substr(8, value.find_first_of("{") - 8);
	while (std::getline(file, line))
	{
		line = trim(line);
		if (line.compare(0, 1, "}") == 0)
			break ;
		for (int j = 0; j < 6; j++)
		{
			if (line.compare(0, data[j].length(), data[j]) == 0)
			{
				parseLocationData(location, data[j], line);
				break ;
			}
		}
	}
}

static int	parseServerBlock(std::ifstream &file, ServerConfig &server)
{
	std::string				line;
	std::vector<std::string> confKeys = {"listen", "server_name", "host", "root", "client_max_body_size", "max_events", "index", "error_page"};

	while (std::getline(file, line))
	{
		trim(line);
		if (line.empty() || line[0] == '#' || line == "{") continue;
		if (line.compare(0, 6, "server") == 0 && line.length() == 6)
			return 1;
		for (int j = 0; j < 8; j++)
		{
			if (line.compare(line.find_first_not_of(" \t"), confKeys[j].length(), confKeys[j]) == 0)
			{
				parseServerData(server, line);
				break ;
			}
		}
		if (line.compare(0, 8, "location") == 0)
		{
			LocationConfig location;
			parseLocationBlock(location, file, line);
			server.addLocation(std::move(location));
		}
	}
	return 0;
}


void	parseData(const std::string &filename, std::vector<ServerConfig> &server)
{
	std::ifstream	file(filename);
	std::string		line;
	int				new_serv = 0;

	(void)server;

    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

	while (std::getline(file, line))
	{
		line = trim(line);
		
		if (line.empty() || line[0] == '#') continue;
	
        size_t server_pos = line.find("server");
        if (server_pos == 0 ||
            new_serv == 1)
        {
			std::cout << CYAN << "Server block found!" << std::endl << "Parsing." << DEFAULT << std::endl;
			int i = 0;
			while (i < 20)
			{
				std::cout << GREEN << "+" << DEFAULT;
				std::cout.flush();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				i++;
			}
			std::cout << std::endl;
			new_serv = 0;
			ServerConfig new_server;
			new_serv = parseServerBlock(file, new_server);
			server.push_back(new_server);
		}
		else
		{
			std::cerr << "Error: Invalid server block" << std::endl;
			return;
		}
	}
}
