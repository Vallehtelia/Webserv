
#include "config.hpp"
#include <bits/stdc++.h>

bool	portCheck(std::vector<ServerConfig> &server)
{
	std::vector<int> ports;

	for (std::vector<ServerConfig>::iterator it = server.begin(); it != server.end(); it++)
	{
		int port = it->getListenPort();
		if (std::find(ports.begin(), ports.end(), port) != ports.end())
		{
			std::cerr << RED << "Error: servers share the same port" << DEFAULT << std::endl;
			return false;
		}
		ports.push_back(port);
	}
	return true;
}

bool	initConfig(const std::string &configFile, std::vector<ServerConfig> &server)
{
	if (!checkConfFile(configFile))
		return false;

	std::cout << GREEN << "Validating file: " << configFile << DEFAULT << std::endl;
	if (!ConfigValidator::validateConfigFile(configFile))
	{
		std::cerr << RED << "Warning: invalid configuration file" << DEFAULT << std::endl;
		return false;
	}

	std::cout << GREEN << "Parsing file: " << configFile << DEFAULT << std::endl;
	parseData(configFile, server);

	// check if servers share the same port
	return(portCheck(server));
}
