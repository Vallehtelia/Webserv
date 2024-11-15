#include "ConfigValidator.hpp"
#include <iostream>
#include <regex>
#include <sstream>

ConfigValidator::ConfigValidator(const std::string& filename) : configFile(filename) {}

bool ConfigValidator::validate() {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << configFile << std::endl;
        return false;
    }

    std::string line;
    bool inLocation = false;
    std::set<std::string> locationPaths;
    std::unordered_map<std::string, int> directiveCounts;

    while (std::getline(file, line)) {
        trim(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line.find("location") == 0) {
            std::string locationPath = extractLocationPath(line);
            if (locationPaths.find(locationPath) != locationPaths.end()) {
                std::cerr << "Duplicate location block for: " << locationPath << std::endl;
                return false;
            }
            locationPaths.insert(locationPath);
            inLocation = true;
        }

        if (inLocation) {
            if (line.find("allow_methods") == 0) {
                if (!validateAllowMethods(line)) {
                    std::cerr << "Invalid allow_methods directive: " << line << std::endl;
                    return false;
                }
            }
            else if (line.find("root") == 0) {
                if (!validateRoot(line)) {
                    std::cerr << "Invalid root path: " << line << std::endl;
                    return false;
                }
            }
            else if (line.find("index") == 0) {
                if (!validateIndex(line)) {
                    std::cerr << "Invalid index directive: " << line << std::endl;
                    return false;
                }
            }
            else if (line.find("cgi_path") == 0) {
                if (!validateCgiPath(line)) {
                    std::cerr << "Invalid cgi_path directive: " << line << std::endl;
                    return false;
                }
            }
        }

        if (line.find("listen") == 0 || line.find("server_name") == 0 ||
            line.find("host") == 0 || line.find("client_max_body_size") == 0 ||
            line.find("max_events") == 0 || line.find("error_page") == 0) {
            if (!validateDirective(line)) {
                std::cerr << "Invalid directive: " << line << std::endl;
                return false;
            }
        }

        if (line == "}") {
            inLocation = false;
        }
    }

    file.close();
    return true;
}

void ConfigValidator::trim(std::string& str) {

    const std::string whitespace = " \t\n\r";
    size_t first = str.find_first_not_of(whitespace);
    size_t last = str.find_last_not_of(whitespace);
    if (first != std::string::npos && last != std::string::npos) {
        str = str.substr(first, (last - first + 1));
    } else {
        str.clear();
    }
}

bool ConfigValidator::validateDirective(const std::string& line) {
    std::regex listenRegex(R"(^listen \d+$)");
    std::regex serverNameRegex(R"(^server_name \S+$)");
    std::regex hostRegex(R"(^host \S+$)");
    std::regex bodySizeRegex(R"(^client_max_body_size \d+$)");
    std::regex maxEventsRegex(R"(^max_events \d+$)");
    std::regex errorPageRegex(R"(^error_page \d+ \S+$)");

    if (std::regex_match(line, listenRegex) ||
        std::regex_match(line, serverNameRegex) ||
        std::regex_match(line, hostRegex) ||
        std::regex_match(line, bodySizeRegex) ||
        std::regex_match(line, maxEventsRegex) ||
        std::regex_match(line, errorPageRegex)) {
        return true;
    }

    return false;
}

bool ConfigValidator::validateAllowMethods(const std::string& line) {
    std::regex methodRegex(R"(^allow_methods (GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH|TRACE)( (GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH|TRACE))*$)");
    return std::regex_match(line, methodRegex);
}

bool ConfigValidator::validateRoot(const std::string& line) {
    std::regex rootRegex(R"(^root \S+$)");
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

std::string ConfigValidator::extractLocationPath(const std::string& line) {
    std::regex locationRegex(R"(^location (\S+) \{)");
    std::smatch match;
    if (std::regex_search(line, match, locationRegex)) {
        return match[1].str();
    }
    return "";
}
