#ifndef UTILS_HPP
# define UTILS_HPP


enum class State;

#include "request/Request.hpp"

std::string stateToString(State state);
State StateFromString(const std::string& stateStr);
void printVectorAsHex(const std::vector<char>& vec);
void checkline(std::string line);
void removeCarriageReturn(std::string& str);

# endif