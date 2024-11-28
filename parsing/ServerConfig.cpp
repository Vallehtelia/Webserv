#include "ServerConfig.hpp"
#include <stdexcept>

ServerConfig::ServerConfig()
{
}

ServerConfig::~ServerConfig()
{
}

// Setters that store values directly in _configData

void ServerConfig::setListenPort(int port) {
    _configData["listen"] = port;
}

void ServerConfig::setServerName(const std::string& server_name) {
    _configData["server_name"] = server_name;
}

void ServerConfig::setHost(const std::string& host) {
    _configData["host"] = host;
}

void ServerConfig::setRoot(const std::string& root) {
    _configData["root"] = root;
}

void ServerConfig::setClientMaxBodySize(int size) {
    _configData["client_max_body_size"] = size; // Store as integer
}

void ServerConfig::setEpollMaxEvents(int events) {
    _configData["max_events"] = events;
}

void ServerConfig::setIndex(const std::string& index) {
    _configData["index"] = index;
}

void	ServerConfig::addErrorPage(int code, const std::string &path)
{
	_error_pages[code] = path;
}

void	ServerConfig::addLocation(LocationConfig location)
{
	_locations.push_back(location);
}

// Template function for retrieving values with type checking
template<typename T>
T ServerConfig::getConfigValue(const std::string& key) const {
    auto it = _configData.find(key);
    if (it != _configData.end()) {
        if (std::holds_alternative<T>(it->second)) {
            return std::get<T>(it->second);
        } else {
            throw std::runtime_error("Type mismatch for key: " + key);
        }
    } else {
        throw std::runtime_error("Key not found: " + key);
    }
}

// Specialized getters
int ServerConfig::getListenPort() const {
    return getConfigValue<int>("listen");
}

std::string ServerConfig::getServerName() const {
    return getConfigValue<std::string>("server_name");
}

std::string ServerConfig::getHost() const {
    return getConfigValue<std::string>("host");
}

std::string ServerConfig::getRoot() const {
    return getConfigValue<std::string>("root");
}

int ServerConfig::getBodySize() const {
    return getConfigValue<int>("client_max_body_size");
}

int ServerConfig::getMaxEvents() const {
    try {
        return getConfigValue<int>("max_events");
    } catch (const std::runtime_error&) {
        std::cerr << "Warning: 'max_events' not set, using default value." << std::endl;
        return 10; // or another sensible default
    }
}

std::string ServerConfig::getIndex() const {
    return getConfigValue<std::string>("index");
}

std::string ServerConfig::getErrorPage(int code) const {
    auto it = _error_pages.find(code);
    if (it != _error_pages.end()) {
        return it->second;
    }
    return "";  // Return an empty string if the error page code is not found
}

const std::map<int, std::string>	&ServerConfig::getErrorPages() const
{
	return _error_pages;
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

// NEW:
// Generic setter to insert any value type
void ServerConfig::setConfig(const std::string& key, const ValueType& value) {
	_configData[key] = value;
}

// Accessor to get the value by key
ValueType	ServerConfig::getConfig(const std::string& key) const {
	if (_configData.find(key) != _configData.end()) {
		return _configData.at(key);
	} else {
		throw std::out_of_range("Key not found!");
	}
}

// Print configurations for debugging
void	ServerConfig::printConfig() const {
	for (const auto& pair : _configData) {
		std::cout << "Key: " << pair.first << ", Value: ";
		std::visit([](auto&& arg) { std::cout << arg << std::endl; }, pair.second);
	}
	for (std::map<int, std::string>::const_iterator Er = _error_pages.begin(); Er != _error_pages.end(); Er++)
		std::cout << "Error code: " << Er->first << ", Page: " << Er->second << std::endl;
}

/*
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
*/
