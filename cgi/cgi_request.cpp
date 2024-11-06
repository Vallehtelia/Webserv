
#include "cgi_request.hpp"

cgiRequest::cgiRequest(const std::string &path, const std::string &method, const std::string &queryString, const std::string &protocol, const std::string &bodyData) : script_path(path), request_method(method), httpProtocol(protocol), body_data(bodyData)
{
	std::cout << "cgi request constructor" << std::endl;
	if (!queryString.empty())
	{
		query_str = queryString;
	}
	else
		query_str = "";
	setEnvironmentVariables();
}

cgiRequest::~cgiRequest()
{
	std::cout << "cgi request deconstructor" << std::endl;
}

void	cgiRequest::setEnvironmentVariables()
{
	env["REQUEST_METHOD"] = request_method;
	env["SCRIPT_PATH"] = script_path;
	env["QUERY_STRING"] = query_str;
	env["HTTP_PROTOCOL"] = httpProtocol;

	if (request_method == "POST")
	{
		env["CONTENT_LENGTH"] = std::to_string(body_data.size());
		env["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
	}
}

void	cgiRequest::printEnv(char **envp) // debugging purposes only
{
	for (int i = 0; envp[i]; i++)
	{
		std::cout << "env[" << i << "] = " << envp[i] << std::endl;
	}
}

void	cgiRequest::printCgiRequestData()
{
	std::cout << "Script path: " << script_path << std::endl;
	std::cout << "Request method: " << request_method << std::endl;
	std::cout << "Query string: " << query_str << std::endl;
	std::cout << "Http protocol: " << httpProtocol << std::endl;

	char **envp = buildEnv();

	if (envp)
	{
		printEnv(envp);
		for (int i = 0; envp[i]; i++)
		{
			delete[] envp[i];
		}
		delete[] envp;
	}
}

char	**cgiRequest::buildEnv()
{
	char	**envp = new char*[env.size() + 1];
	size_t	i = 0;

	for(const auto &pair : env)
	{
		std::string	envString = pair.first + "=" + pair.second;
		envp[i] = new char[envString.size() + 1];
		std::strcpy(envp[i], envString.c_str());
		i++;
	}
	envp[i] = nullptr;
	return envp;
}

void	cleanEnv(char **envp)
{
	if (envp)
	{
		for (int i = 0; envp[i]; i++)
		{
			delete[] envp[i];
		}
		delete[] envp;
	}
}

std::string	cgiRequest::getScriptPath()
{
	return script_path;
}

std::string	cgiRequest::getRequestMethod()
{
	return request_method;
}

std::string	cgiRequest::getQueryString()
{
	return query_str;
}

std::string	cgiRequest::getProtocol()
{
	return httpProtocol;
}

std::string cgiRequest::getBodyData()
{
	return body_data;
}

std::map<std::string, std::string>	cgiRequest::getEnv()
{
	return (env);
}

bool	cgiRequest::isValidCgi()
{
	std::ifstream file;

	file.open(script_path);

	if (file)
	{
		std::cout << "Great success\n";
		return true;
	}
	else
	{
		std::cout << "Cgi script not opening at: " << script_path << std::endl;
		return false;
	}
}

int	cgiRequest::execute()
{
	if (isValidCgi())
	{
		pid_t pid = fork();
		if (pid == -1)
		{
			std::cerr << "Failed to create child process for cgi script" << std::endl;
			return 500;
		}
		else if (pid == 0)
		{
			std::string	outputPath = "./html/tmp/cgi_output.html";
			int	outputFile = open(outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
			if (outputFile == -1)
			{
				std::cerr << "Failed to open output file for CGI script" << std::endl;
				exit(500);
			}
			dup2(outputFile, STDOUT_FILENO);
			close(outputFile);
			if (request_method == "POST" && !body_data.empty())
			{
				std::cerr << "pipeing body" << std::endl; // debugging
				std::cerr << "Body data length: " << body_data.size() << std::endl;
				
				int inputPipe[2];
				if (pipe(inputPipe) == -1) {
					std::cerr << "Failed to create input pipe for CGI script" << std::endl;
					exit(500);
				}

				// Set the write-end of the pipe to non-blocking
				int flags = fcntl(inputPipe[1], F_GETFL, 0);
				if (flags == -1 || fcntl(inputPipe[1], F_SETFL, flags | O_NONBLOCK) == -1) {
					std::cerr << "Failed to set non-blocking mode on input pipe" << std::endl;
					close(inputPipe[0]);
					close(inputPipe[1]);
					exit(500);
				}

				const size_t buffer_size = 8192; // 8 KB buffer
				size_t written = 0;
				while (written < body_data.size()) {
					size_t to_write = std::min(buffer_size, body_data.size() - written);
					ssize_t result = write(inputPipe[1], body_data.c_str() + written, to_write);
					if (result == -1) {
						perror("write to pipe");
						std::cerr << "Write to pipe failed" << std::endl;
						close(inputPipe[0]);
						close(inputPipe[1]);
						exit(500);
					}
					written += result;
				}
				
				close(inputPipe[1]); // Close write-end after writing all data
				dup2(inputPipe[0], STDIN_FILENO); // Redirect stdin to read-end of pipe
				close(inputPipe[0]); // Close original read-end
				std::cerr << "done pipeing data" << std::endl; // debugging
			}
			char	*args[] = {const_cast<char *>(script_path.c_str()), nullptr};
			char	**envp = buildEnv();
			// std::cout << "script path: " << script_path.c_str() << std::endl;
			// for (int i = 0; args[i]; i++)
			// {
			// 	std::cout << "arg[" << i << "]: " << args[i] << std::endl;
			// }
			// for (int i = 0; envp[i]; i++)
			// {
			// 	std::cout << "envp[" << i << "]: " << envp[i] << std::endl;
			// }
			std::cerr << "running execve script: " << script_path.c_str() << std::endl; // debugging
			execve(script_path.c_str(), args, envp);
			perror("execve"); // Lisää tämä rivi, jotta näet tarkemman virheen syyn
			std::cerr << "Execve failed!" << std::endl;
			cleanEnv(envp);
			exit(500);
		}
		else
		{
			waitpid(pid, nullptr, 0);
			// std::cout << output << std::endl;
		}
		return 0;
	}
	return 1;
}



std::string	findQueryStr(const std::string &path)
{
	std::string query;
	size_t i = path.find('?');
	if (i != std::string::npos)
		query = path.substr(i + 1);
	else
		query = "";
	// std::cout << "debugging" << std::endl;
	return(query);
}

std::string	findPath(const std::string &path)
{
	std::string	directPath;
	size_t		i = path.find('?');

	if (i != std::string::npos)
		directPath = path.substr(0, i);
	else
		directPath = path;
	directPath = '.' + directPath;
	return directPath;
}
