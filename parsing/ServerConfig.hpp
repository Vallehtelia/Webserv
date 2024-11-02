
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "LocationConfig.hpp"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m"
#define DEFAULT "\033[0m"

class ServerConfig
{
	private:
		std::string	_listen_port;
		std::string	_server_name;
		std::string	_host;
		std::string	_root;
		std::string	_client_max_body_size;
		std::string	_index;
		std::map<int, std::string>	_error_pages;
		std::vector<LocationConfig>	_locations;
	public:
		ServerConfig();
		~ServerConfig();

		// setters
		void	setListenPort(std::string port);
		void	setServerName(std::string server_name);
		void	setHost(std::string host);
		void	setRoot(std::string root);
		void	setClientMaxBodySize(int size);
		void	setIndex(std::string index);
		void	addLocation(LocationConfig location);
		void	addErrorPage(int code, const std::string &path);

		// getters
		std::string getListenPort() const;
		std::string getServerName() const;
		std::string getHost() const;
		std::string getRoot() const;
		std::string getBodySize() const;
		std::string getIndex() const;
		std::string	getErrorPage(int code) const;
		std::string getLocation(std::string key) const;

		void	printConfig() const;
};

bool	checkConfFile(char *filename);
void	parseData(char *filename, std::vector<ServerConfig> &server);
