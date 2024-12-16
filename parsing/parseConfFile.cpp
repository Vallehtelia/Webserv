
#include "ServerConfig.hpp"
#include <iostream>
#include <fstream>
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
	if (trimmedLine.empty() || trimmedLine[0] == '#') return 0;
	size_t spacePos = trimmedLine.find(' ');
	if (spacePos == std::string::npos) {
		std::cout << "Invalid server configuration value: " << trimmedLine << std::endl;
		return 1;
	}
	std::string key = trimmedLine.substr(0, spacePos);
	std::string value = trim(trimmedLine.substr(spacePos + 1));
	if (key == "listen" || key == "client_max_body_size" || key == "max_events") {
		server.setConfig(key, std::stoi(value));
		return 0;
	}
	else if (key == "server_name" || key == "host" || key == "root" || key == "index") {
		server.setConfig(key, value.erase(value.find_last_of(";")));
		return 0;
	}
	else if (key == "error_page") {
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

static void	parseLocationData(LocationConfig &location, std::string line, int *data)
{
	std::string		value;
	int				i = 0;
	int 			j = 0;

	value = line;
	switch (*data)
	{
		case 0:
			value = value.substr(4, value.find_first_of(";") - 4);
			location.root = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);
			break ;
		case 1:
			i = 13;
			while (value[i] != ';' && value[i] != '\0')
			{
				while (std::isspace(value[i]))
					i++;
				if (value[i] == ';')
					break ;
				j = i;
				while (value[j] != ' ' && value[j] != ';' && value[j] != '\0')
					j++;
				location.allow_methods.push_back(value.substr(i, j - i));
				i = j;
			}
			break;
		case 2:
			value = value.substr(10, value.find_first_of(";") - 10);
			value = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);
			if (value.compare("on") == 0)
				location.autoindex = true;
			else
				location.autoindex = false;
			break ;
		case 3:
			value = value.substr(6, value.find_first_of(";") - 6);
			location.index = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);
			break ;
		case 4:
			value = value.substr(9, value.find_first_of(";") - 9);
			location.redirect = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);
			break ;
		case 5:
			value = value.substr(9, value.find_first_of(";") - 9);
			location.cgi_path = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);
			break ;
		default:
			break ;
	}
}

static void	parseLocationBlock(LocationConfig &location, std::ifstream &file, std::string line)
{
	std::string		data[6] = {"root", "allow_methods", "autoindex", "index", "redirect", "cgi_path"};
	std::string		value;

	value = line.substr(line.find_first_not_of(" \t"), line.find_last_not_of(" \t") - line.find_first_not_of(" \t") + 1);
	location.path = value.substr(value.find_first_of(" \t"), value.find_first_of("{") - value.find_first_of(" \t"));
	location.path = location.path.substr(location.path.find_first_not_of(" \t"), location.path.find_last_not_of(" \t") - location.path.find_first_not_of(" \t") + 1);
	while (std::getline(file, line))
	{
		line = line.substr(line.find_first_not_of(" \t"), line.find_last_not_of(" \t") - line.find_first_not_of(" \t") + 1);
		if (line.empty())
			continue ;
		if (line.compare(0, 1, "}") == 0)
			break ;
		for (int j = 0; j < 6; j++)
		{
			if (line.compare(0, data[j].length(), data[j]) == 0)
			{
				parseLocationData(location, line, &j);
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
		if (line.empty())
			continue ;
		line = line.substr(line.find_first_not_of(" \t"), line.find_last_not_of(" \t") - line.find_first_not_of(" \t") + 1);
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

// New function to check the presence of all configuration keys
std::vector<std::string> validateServerBlock(const ServerConfig &server) {
    std::vector<std::string> confKeys = {
		"listen",
		"server_name",
		"host",
		"root",
        "client_max_body_size",
		"max_events",
        "index"};

    std::vector<std::string> missingKeys;

    for (const std::string &key : confKeys) {
        try {
            server.getConfig(key);
        } catch (const std::out_of_range &e) {
            missingKeys.push_back(key);
        }
    }
    return missingKeys;
}

std::vector<int> validateErrorPageKeys(const ServerConfig &server) {
    // Define the required error page status codes
    std::vector<int> requiredErrorPageKeys = {
        400, 403, 404, 405, 409, 500, 504
    };

    std::vector<int> missingKeys;

    for (int key : requiredErrorPageKeys) {
        if (server.getErrorPage(key).empty()) {
            missingKeys.push_back(key);
        }
    }
    return missingKeys;
}

std::vector<std::string> validateLocationConfigs(const ServerConfig &server) {
    std::vector<std::string> missingKeys;
    const std::vector<LocationConfig> &locations = server.getLocations();

    for (size_t i = 0; i < locations.size(); ++i) {
        const LocationConfig &location = locations[i];

        if (location.root.empty()) {
            missingKeys.push_back("root missing in location " + std::to_string(i) + " (" + location.path + ")");
        }

        if (location.allow_methods.empty()) {
            missingKeys.push_back("allow_methods missing in location " + std::to_string(i) + " (" + location.path + ")");
        }
    }
    return missingKeys;
}

// New function to validate the new server
bool validateNewServer(const ServerConfig &server) {

	std::vector<std::string> missingKeys = validateServerBlock(server);
	std::vector<int> missingErrorPageKeys = validateErrorPageKeys(server);
	std::vector<std::string> missingLocationKeys = validateLocationConfigs(server);

	if (!missingKeys.empty())
	{
		std::cerr << "Missing keys: ";
		for (const std::string &key : missingKeys) {
			std::cerr << key << " ";
		}
		std::cerr << std::endl;
	}

	if (!missingErrorPageKeys.empty())
	{
		std::cerr << "Missing error page keys:" << std::endl;
		for (int key : missingErrorPageKeys) {
			std::cerr << key << std::endl;
		}
	}

	if (!missingLocationKeys.empty())
	{
		std::cerr << "Missing location keys: ";
		for (const std::string &key : missingLocationKeys) {
			std::cerr << key << " ";
		}
		std::cerr << std::endl;
	}

	if (!missingErrorPageKeys.empty() || !missingKeys.empty() || !missingLocationKeys.empty()) {
		return false;
	}
	return true;
}

bool	parseData(const std::string &filename, std::vector<ServerConfig> &server)
{
	std::ifstream	file(filename);
	std::string		line;
	int				new_serv = 0;

	(void)server;
	while (std::getline(file, line))
	{
		if (line.empty())
			continue ;
		line = line.substr(line.find_first_not_of(" \t"), line.find_last_not_of(" \t") - line.find_first_not_of(" \t") + 1);
		if ((line.find("server") != std::string::npos && line.length() == 6) || new_serv == 1)
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
			if (!validateNewServer(new_server)) {
				std::cout << "Not valid..." << std::endl;
				return false;
			}
			server.push_back(new_server);
		}
	}
	return true;
}
