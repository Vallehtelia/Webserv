#pragma once

#include <string>
#include <istream>

class ConfigValidator {
public:
    // Public static method to validate the entire configuration file
    static bool validateConfigFile(const std::string &filename);

private:
    // Private constructor to prevent instantiation
    ConfigValidator() = delete;

    // Private static methods for validation logic
    static bool validateServerBlock(std::istream &input);
    static bool validateLocationBlock(std::istream &input);
    static void trim(std::string &str);
    static bool validateDirective(const std::string &line);
    static bool validateAllowMethods(const std::string &line);
    static bool validateRoot(const std::string &line);
    static bool validateIndex(const std::string &line);
    static bool validateCgiPath(const std::string &line);
	static bool validateAutoindex(const std::string& line);
    static std::string extractLocationPath(const std::string &line);

	static std::string removeRootAndSemicolon(const std::string& line);
};

