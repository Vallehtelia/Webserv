
#include "LocationConfig.hpp"

LocationConfig::LocationConfig(): path(""), root(""), allow_methods(), autoindex(false), index(""), redirect(""), cgi_path(""), cgi_ext()
{
}

LocationConfig::~LocationConfig()
{
}

LocationConfig::LocationConfig(const LocationConfig &other)
{
	*this = other;
}

LocationConfig &LocationConfig::operator=(const LocationConfig &other)
{
	if (this != &other)
	{
		path = other.path;
		root = other.root;
		allow_methods = other.allow_methods;
		autoindex = other.autoindex;
		index = other.index;
		redirect = other.redirect;
		cgi_path = other.cgi_path;
		cgi_ext = other.cgi_ext;
	}
	return *this;
}

// void	LocationConfig::printConfig() const
// {
// 	std::cout << "Location config:" << std::endl;
// 	std::cout << "Path: " << path << std::endl;
// 	std::cout << "Allow methods:" << std::endl;
// 	for (std::vector<std::string>::const_iterator it = allow_methods.begin(); it != allow_methods.end(); it++)
// 	{
// 		std::cout << *it << std::endl;
// 	}
// 	std::cout << "Autoindex: " << autoindex << std::endl;
// 	std::cout << "Index: " << index << std::endl;
// 	std::cout << "Redirect: " << redirect << std::endl;
// 	std::cout << "Cgi path: " << cgi_path << std::endl;
// 	std::cout << "Cgi extensions:" << std::endl;
// 	for (std::vector<std::string>::const_iterator it = cgi_ext.begin(); it != cgi_ext.end(); it++)
// 	{
// 		std::cout << *it << std::endl;
// 	}
// }

std::string	LocationConfig::getLocation() const
{
	return path;
}

std::string	LocationConfig::getRoot() const
{
	return root;
}

std::vector<std::string>	LocationConfig::getAllowMethods() const
{
	return allow_methods;
}

bool	LocationConfig::isAutoindex() const
{
	return autoindex;
}

std::string	LocationConfig::getIndex() const
{
	return index;
}

std::string	LocationConfig::getRedirect() const
{
	return redirect;
}

std::string	LocationConfig::getCgiPath() const
{
	return cgi_path;
}
