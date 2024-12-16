#include "ConfigValidator.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_set>

void ConfigValidator::trim(std::string &str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    if (start == std::string::npos || end == std::string::npos) {
        str.clear();
    } else {
        str = str.substr(start, end - start + 1);
    }
}

std::string ConfigValidator::extractLocationPath(const std::string &line) {
    size_t pos = line.find_first_of(" \t") + 1;
    return line.substr(pos, line.find_first_of(" \t", pos) - pos);
}

bool ConfigValidator::validateAllowMethods(const std::string &line) {
    std::regex methodRegex(R"(^allow_methods\s+(GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH|TRACE)(\s+(GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH|TRACE))*\s*;$)");
    return std::regex_match(line, methodRegex);
}

bool ConfigValidator::validateDirective(const std::string& line) {

	std::regex listenRegex(R"(^listen\s+\d+\s*;$)");
	std::regex serverNameRegex(R"(^server_name\s+\S+;$)");
	std::regex hostRegex(R"(^host\s+\S+;$)");
	std::regex rootRegex(R"(^\s*root\s+(.*?);\s*$)");
	std::regex bodySizeRegex(R"(^client_max_body_size\s+\d+;$)");
	std::regex maxEventsRegex(R"(^max_events\s+\d+;$)");
	std::regex indexRegex(R"(^index \S+$)");
	std::regex errorPageRegex(R"(^error_page\s+\d+\s+\S+;$)");

	if (std::regex_match(line, listenRegex) ||
		std::regex_match(line, serverNameRegex) ||
		std::regex_match(line, hostRegex) ||
		std::regex_match(line, rootRegex) ||
		std::regex_match(line, bodySizeRegex) ||
		std::regex_match(line, maxEventsRegex) ||
		std::regex_match(line, indexRegex) ||
		std::regex_match(line, errorPageRegex)) {
		return true;
	}

	return false;
}

bool ConfigValidator::validateRoot(const std::string& line) {
	std::regex rootRegex(R"(^\s*root\s+(.*?);\s*$)");
	return std::regex_match(line, rootRegex);
}

bool ConfigValidator::validateIndex(const std::string& line) {
    std::regex indexRegex(R"(^index \S+$)");
    return std::regex_match(line, indexRegex);
}

bool ConfigValidator::validateCgiPath(const std::string& line) {

    std::regex cgiPathRegex(R"(^cgi_path \S+ \S+$)");
    return std::regex_match(line, cgiPathRegex);
}

bool ConfigValidator::validateAutoindex(const std::string& line) {

    std::regex autoindexRegex(R"(^\s*autoindex\s+(on|off)\s*;\s*$)");
    return std::regex_match(line, autoindexRegex);
}

// Validates a single location block
bool ConfigValidator::validateLocationBlock(std::istream &input) {
    std::string line;
    while (std::getline(input, line)) {
        trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (line.find("allow_methods") == 0) {
            if (!validateAllowMethods(line)) {
                std::cerr << "Invalid allow_methods directive: " << line << std::endl;
                return false;
            }
        } else if (line.find("root") == 0) {
            if (!validateRoot(line)) {
                std::cerr << "Invalid root directive: " << line << std::endl;
                return false;
            }
        } else if (line.find("index") == 0) {
            if (!validateIndex(line)) {
                std::cerr << "Invalid index directive: " << line << std::endl;
                return false;
            }
        } else if (line.find("cgi_path") == 0) {
            if (!validateCgiPath(line)) {
                std::cerr << "Invalid cgi_path directive: " << line << std::endl;
                return false;
            }
		} else if (line.find("autoindex") == 0) {
            if (!validateAutoindex(line)) {
                std::cerr << "Invalid cgi_path directive: " << line << std::endl;
                return false;
            }
        } else if (line == "}") {
            break;
        } else {
            std::cerr << "Invalid directive in location block: " << line << std::endl;
            return false;
        }
    }
    return true;
}

bool ConfigValidator::validateServerBlock(std::istream &input) {
    std::unordered_set<std::string> locationPaths;
    std::string line;
    while (std::getline(input, line)) {
        trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (line.find("location") == 0) {
            std::string locationPath = extractLocationPath(line);
            if (locationPaths.find(locationPath) != locationPaths.end()) {
                std::cerr << "Duplicate location block for: " << locationPath << std::endl;
                return false;
            }
            locationPaths.insert(locationPath);
            if (!validateLocationBlock(input)) {
                return false;
            }
        } else if (line.find("listen") == 0 ||
					line.find("server_name") == 0 ||
                   	line.find("host") == 0 ||
					line.find("root") == 0 ||
					line.find("client_max_body_size") == 0 ||
                   	line.find("max_events") == 0 ||
					line.find("index") == 0 ||
					line.find("error_page") == 0) {
            if (!validateDirective(line)) {
                std::cerr << "Invalid directive: " << line << std::endl;
                return false;
            }
        } else if (line == "}") {
            break;
        } else {
            std::cerr << "Invalid directive in server block: \"" << line << "\"" << std::endl;
            return false;
        }
    }
    return true;
}

// Validates the entire configuration file
bool ConfigValidator::validateConfigFile(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        trim(line);
        if (line.empty() || line[0] == '#') continue;

		if (line == "server") {
			if (!std::getline(file, line)) {
				std::cerr << "Unexpected end of file after 'server'" << std::endl;
				return false;
			}
			trim(line);
			if (line != "{") {
				std::cerr << "Expected '{' after 'server', but got: " << line << std::endl;
				return false;
			}
			if (!validateServerBlock(file)) {
				return false;
			}
		} else if (line.find("server {") == 0) {
			std::string trimmedRest = line.substr(7);
			trim(trimmedRest);
			if (trimmedRest != "{") {
				std::cerr << "Unexpected content after 'server {': " << trimmedRest << std::endl;
				return false;
			}
			if (!validateServerBlock(file)) {
				return false;
			}
		} else {
			std::cerr << "Unexpected directive outside server block: " << line << std::endl;
			return false;
		}
    }
    file.close();
    return true;
}
