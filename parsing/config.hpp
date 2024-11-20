#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include "ServerConfig.hpp"
#include "ConfigValidator.hpp"

bool	initConfig(const std::string &configFile, std::vector<ServerConfig> &server);

#endif
