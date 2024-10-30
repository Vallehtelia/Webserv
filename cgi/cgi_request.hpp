
#include <iostream>
#include <unistd.h>
#include <map>
#include <string>
#include <cstring>
#include <fstream>
#include <wait.h>
#include <fcntl.h>

class cgiRequest
{
	private:
		std::string	script_path;
		std::string	request_method;
		std::string	query_str;
		std::string	httpProtocol;
		std::map<std::string, std::string> env;
	public:
		cgiRequest(const std::string &path, const std::string &method, const std::string &queryString, const std::string &protocol);
		~cgiRequest();

		void		setEnvironmentVariables();
		char		**buildEnv();
		int			execute();
		bool		isValidCgi();
		void		printEnv(char **envp); // debugging purpose only
		void		printCgiRequestData(); // debugging purpose only

		std::string	getScriptPath();
		std::string	getRequestMethod();
		std::string	getQueryString();
		std::string	getProtocol();
		std::map<std::string, std::string>	getEnv();
};

std::string findQueryStr(const std::string &path);
std::string findPath(const std::string &path);
void		cleanEnv(char **envp);
