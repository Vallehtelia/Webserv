
#include "ServerConfig.hpp"
#include <iostream>
#include <fstream>
#include <cctype>
#include <thread>
#include <chrono>

bool	checkConfFile(char *filename)
{
	std::ifstream	file(filename);

	if (!file.is_open())
	{
		std::cerr << RED << "Error: could not open file" << DEFAULT << std::endl;
		return false;
	}
	std::cout << GREEN << "File found [OK]\033[0m" << DEFAULT << std::endl;
	return true;
}

static void	parseServerdata(ServerConfig &server, std::string line, int data)
{
	std::string		value;
	int				code = 0;
	int				i = 10;

	value = line.substr(line.find_first_not_of(" \t"), line.find_last_not_of(" \t") - line.find_first_not_of(" \t") + 1);
	switch (data)
	{
		case 0:
			server.setListenPort(value.substr(6, value.find_first_of(";") - 6));
			break ;
		case 1:
			server.setServerName(value.substr(11, value.find_first_of(";") - 11));
			break ;
		case 2:
			server.setHost(value.substr(5, value.find_first_of(";") - 5));
			break ;
		case 3:
			server.setRoot(value.substr(5, value.find_first_of(";") - 5));
			break ;
		case 4:
			server.setClientMaxBodySize(std::stoi(value.substr(22, value.find_first_of(";") - 22)));
			break ;
		case 5:
			server.setIndex(value.substr(6, value.find_first_of(";") - 6));
			break ;
		case 6:
			while (std::isspace(value[i]))
				i++;
			code = std::stoi(value.substr(i, 3));
			i += 3;
			while (std::isspace(value[i]))
				i++;
			server.addErrorPage(code, value.substr(i, value.find_first_of(";") - i));
			break ;
		default:
			break ;
	}
}

static void	parseLocationData(LocationConfig &location, std::string line, int *data)
{
	std::string		value;
	int				i = 0;
	int 			j = 0;

	value = line;
	switch (*data)
	{
		case 0:
			value = value.substr(4, value.find_first_of(";") - 4);
			location.root = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);
			break ;
		case 1:
			i = 13;
			while (value[i] != ';' && value[i] != '\0')
			{
				while (std::isspace(value[i]))
					i++;
				if (value[i] == ';')
					break ;
				j = i;
				while (value[j] != ' ' && value[j] != ';' && value[j] != '\0')
					j++;
				location.allow_methods.push_back(value.substr(i, j - i));
				i = j;
			}
		case 2:
			value = value.substr(10, value.find_first_of(";") - 10);
			value = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);
			if (value.compare("on") == 0)
				location.autoindex = true;
			else
				location.autoindex = false;
			break ;
		case 3:
			value = value.substr(6, value.find_first_of(";") - 6);
			location.index = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);
			break ;
		case 4:
			value = value.substr(9, value.find_first_of(";") - 9);
			location.redirect = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);
			break ;
		case 5:
			value = value.substr(9, value.find_first_of(";") - 9);
			location.cgi_path = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);
			break ;
		default:
			break ;
	}
}

static void	parseLocationBlock(LocationConfig &location, std::ifstream &file, std::string line)
{
	std::string		data[6] = {"root", "allow_methods", "autoindex", "index", "redirect", "cgi_path"};
	std::string		value;

	value = line;
	location.path = value.substr(8, value.find_first_of("{") - 8);
	while (std::getline(file, line))
	{
		line = line.substr(line.find_first_not_of(" \t"), line.find_last_not_of(" \t") - line.find_first_not_of(" \t") + 1);
		if (line.empty())
			continue ;
		if (line.compare(0, 1, "}") == 0)
			break ;
		for (int j = 0; j < 6; j++)
		{
			if (line.compare(0, data[j].length(), data[j]) == 0)
			{
				parseLocationData(location, line, &j);
				break ;
			}
		}
	}
}

static int	parseServerBlock(std::ifstream &file, ServerConfig &server)
{
	std::string		line;
	std::string		data[7] = {"listen", "server_name", "host", "root", "client_max_body_size", "index", "error_page"};

	while (std::getline(file, line))
	{
		if (line.empty())
			continue ;
		// int i = 0;
		// std::cout << "Line found: " << line << std::endl;
		line = line.substr(line.find_first_not_of(" \t"), line.find_last_not_of(" \t") - line.find_first_not_of(" \t") + 1);
		if (line.compare(0, 6, "server") == 0 && line.length() == 6)
			return 1;
		for (int j = 0; j < 7; j++)
		{
			if (line.compare(line.find_first_not_of(" \t"), data[j].length(), data[j]) == 0)
			{
				parseServerdata(server, line, j);
				break ;
			}
		}
		if (line.compare(0, 8, "location") == 0)
		{
			LocationConfig location;
			parseLocationBlock(location, file, line);
			server.addLocation(std::move(location));
		}
	}
	return 0;
}

void	parseData(char *filename, std::vector<ServerConfig> &server)
{
	std::ifstream	file(filename);
	std::string		line;
	int				new_serv = 0;

	(void)server;
	while (std::getline(file, line))
	{
		if (line.empty())
			continue ;
		line = line.substr(line.find_first_not_of(" \t"), line.find_last_not_of(" \t") - line.find_first_not_of(" \t") + 1);
		if ((line.find("server") != std::string::npos && line.length() == 6) || new_serv == 1)
		{
			std::cout << CYAN << "Server block found!" << std::endl << "Parsing." << DEFAULT << std::endl;
			int i = 0;
			while (i < 20)
			{
				std::cout << GREEN << "+" << DEFAULT;
				std::cout.flush();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				i++;
			}
			std::cout << std::endl;
			new_serv = 0;
			ServerConfig new_server;
			new_serv = parseServerBlock(file, new_server);
			server.push_back(new_server);
		}
	}
}
