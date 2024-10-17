
#include <iostream>
#include <string>
#include <vector>

class LocationConfig {
public:
    std::string path;
    std::string root;
    std::vector<std::string> allow_methods;
    bool autoindex;
    std::string index;
    std::string redirect;
    std::string cgi_path;
    std::vector<std::string> cgi_ext;

    LocationConfig();
    ~LocationConfig();
    LocationConfig(const LocationConfig &other);
    LocationConfig &operator=(const LocationConfig &other);
    void printConfig() const;
    std::string getLocation() const;
    std::string getRoot() const;
    std::vector<std::string> getAllowMethods() const;
    bool isAutoindex() const;
    std::string getIndex() const;
    std::string getRedirect() const;
    std::string getCgiPath() const;
};
