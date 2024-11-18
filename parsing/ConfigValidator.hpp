#pragma once

#include <string>
#include <istream>

class ConfigValidator {
public:
    static bool validateConfigFile(const std::string &filename);

private:
	ConfigValidator() = delete;
	ConfigValidator(const ConfigValidator&) = delete;
	ConfigValidator& operator=(const ConfigValidator&) = delete;
	~ConfigValidator() = delete;

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

