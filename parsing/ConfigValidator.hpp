#pragma once

#include <string>
#include <unordered_map>
#include <set>
#include <fstream>

class ConfigValidator {
public:

    explicit ConfigValidator(const std::string& filename);
    bool validate();

private:
    std::string configFile;

    void trim(std::string& str);
    bool validateDirective(const std::string& line);
    bool validateAllowMethods(const std::string& line);
    bool validateRoot(const std::string& line);
    bool validateIndex(const std::string& line);
    bool validateCgiPath(const std::string& line);
    std::string extractLocationPath(const std::string& line);
};
