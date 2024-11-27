
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <variant>
#include "LocationConfig.hpp"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m"
#define DEFAULT "\033[0m"

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

using ValueType = std::variant<int, double, std::string>;

class ServerConfig
{
	private:
		std::unordered_map<std::string, ValueType> 	_configData;
		std::map<int, std::string>					_error_pages;
		std::vector<LocationConfig>					_locations;

	public:
		ServerConfig();
		~ServerConfig();

		void 		setConfig(const std::string& key, const ValueType& value);
		ValueType	getConfig(const std::string& key) const;

		// setters
		void	setListenPort(int port);
		void	setServerName(const std::string& server_name);
		void	setHost(const std::string& host);
		void	setRoot(const std::string& root);
		void	setClientMaxBodySize(int size);
		void	setEpollMaxEvents(int events);
		void	setIndex(const std::string& index);
		void	addLocation(LocationConfig location);
		void	addErrorPage(int code, const std::string &path);

		template<typename T>
    	T getConfigValue(const std::string& key) const;

		int			getListenPort() const;
		std::string getServerName() const;
		std::string getHost() const;
		std::string getRoot() const;
		int 		getBodySize() const;
		int 		getMaxEvents() const;
		std::string	getIndex() const;

		std::string	getLocation(std::string key) const;
    	std::string getErrorPage(int code) const;

		void	printConfig() const;
};

bool	checkConfFile(char *filename);
void	parseData(char *filename, std::vector<ServerConfig> &server);
#endif