#include "ServerConfig.hpp"

ServerConfig::ServerConfig(): _listen_port("8080"), _server_name("default_server"), _host("localhost"), _root("/var/www/html"), _client_max_body_size("1m"), _index("index.html")
{
}

ServerConfig::~ServerConfig()
{
}

void	ServerConfig::setListenPort(std::string port)
{
	_listen_port = port;
}

void	ServerConfig::setServerName(std::string server_name)
{
	_server_name = server_name;
}

void	ServerConfig::setHost(std::string host)
{
	_host = host;
}

void	ServerConfig::setRoot(std::string root)
{
	_root = root;
}

void	ServerConfig::setClientMaxBodySize(int size)
{
	_client_max_body_size = std::to_string(size) + "m";
}

void	ServerConfig::setIndex(std::string index)
{
	_index = index;
}

void	ServerConfig::addErrorPage(int code, const std::string &path)
{
	_error_pages[code] = path;
}

void	ServerConfig::addLocation(LocationConfig location)
{
	_locations.push_back(location);
}

std::string	ServerConfig::getListenPort() const
{
	return _listen_port;
}

std::string	ServerConfig::getServerName() const
{
	return _server_name;
}

std::string	ServerConfig::getHost() const
{
	return _host;
}

std::string	ServerConfig::getRoot() const
{
	return _root;
}

std::string	ServerConfig::getBodySize() const
{
	return _client_max_body_size;
}

std::string	ServerConfig::getIndex() const
{
	return _index;
}

std::string	ServerConfig::getErrorPage(int code) const
{
	std::map<int, std::string>::const_iterator it = _error_pages.find(code);
	if (it != _error_pages.end())
		return it->second;
	return "";
}

std::string	ServerConfig::getLocation(std::string key) const
{
	for (std::vector<LocationConfig>::const_iterator it = _locations.begin(); it != _locations.end(); it++)
	{
		if (it->getLocation() == key)
			return it->getRoot();
	}
	return "";
}

void	ServerConfig::printConfig() const
{
	std::cout << std::endl << "\033[1;32mServer config:\033[0m" << std::endl;
	std::cout << "Listen port: " << _listen_port << std::endl;
	std::cout << "Server name: " << _server_name << std::endl;
	std::cout << "Host: " << _host << std::endl;
	std::cout << "Root: " << _root << std::endl;
	std::cout << "Client max body size: " << _client_max_body_size << std::endl;
	std::cout << "Index: " << _index << std::endl;
	std::cout << "Error pages:" << std::endl;
	for (std::map<int, std::string>::const_iterator it = _error_pages.begin(); it != _error_pages.end(); it++)
	{
		std::cout << "Error code: " << it->first << " Path: " << it->second << std::endl;
	}
	std::cout << "Locations:" << std::endl;
	std::cout << "Number of locations: " << _locations.size() << std::endl;
	for (std::vector<LocationConfig>::const_iterator it = _locations.begin(); it != _locations.end(); it++)
	{
		std::cout << "\033[1;32mLocation: " << it->getLocation() << "\033[0m" << std::endl;
		std::cout << "Root: " << it->getRoot() << std::endl;
		std::vector<std::string> allow_methods = it->getAllowMethods();
		if (allow_methods.size() > 0)
		{
			for (std::vector<std::string>::const_iterator it = allow_methods.begin(); it != allow_methods.end(); it++)
			{
				std::cout << *it << " ";
			}
			std::cout << std::endl;
		}
		std::cout << "Autoindex: " << it->isAutoindex() << std::endl;
		std::cout << "Index: " << it->getIndex() << std::endl;
		std::cout << "Redirect: " << it->getRedirect() << std::endl;
		std::cout << "Cgi path: " << it->getCgiPath() << std::endl;
	}
}
