
#include "config.hpp"

bool	initConfig(const std::string &configFile, std::vector<ServerConfig> &server)
{
	if (!checkConfFile(configFile))
		return false;

	std::cout << GREEN << "Validating file: " << configFile << DEFAULT << std::endl;
	if (!ConfigValidator::validateConfigFile(configFile))
	{
		std::cout << "Warning: invalid configuration file" << std::endl;
		return false;
	}

	std::cout << GREEN << "Parsing file: " << configFile << DEFAULT << std::endl;

	if (!parseData(configFile, server))
	{
		std::cerr << RED << "Fatal error in server configuration, aborting..." << DEFAULT << std::endl;
		return false;
	}
	for (auto it = begin(server); it != end(server); it++)
		it->printConfig();
	return true;
}
