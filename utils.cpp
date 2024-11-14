#include "utils.hpp"


std::string stateToString(State state) {
    switch (state) {
        case State::REQUEST_LINE: return "REQUEST_LINE";
        case State::HEADERS: return "HEADERS";
        case State::UNCHUNK: return "UNCHUNK";
        case State::BODY: return "BODY";
        case State::MULTIPARTDATA: return "MULTIPARTDATA";
        case State::COMPLETE: return "COMPLETE";
        case State::ERROR: return "ERROR";
        case State::INCOMPLETE: return "INCOMPLETE";
        default: return "UNKNOWN";
    }
}

State StateFromString(const std::string& stateStr) {
    if (stateStr == "REQUEST_LINE") return State::REQUEST_LINE;
    if (stateStr == "HEADERS") return State::HEADERS;
    if (stateStr == "UNCHUNK") return State::UNCHUNK;
    if (stateStr == "BODY") return State::BODY;
    if (stateStr == "MULTIPARTDATA") return State::MULTIPARTDATA;
    if (stateStr == "COMPLETE") return State::COMPLETE;
    if (stateStr == "ERROR") return State::ERROR;
    if (stateStr == "INCOMPLETE") return State::INCOMPLETE;
    return State::ERROR;
}

void printVectorAsHex(const std::vector<char>& vec) {
    for (unsigned char c : vec) {
        std::cout << std::hex << static_cast<int>(c) << " ";
    }
    std::cout << std::dec << std::endl;
}


void checkline(std::string line)
{
std::cout << "Line characters: ";
for (char c : line) {
    std::cout << (c == '\n' ? std::string("\\n") : c == '\r' ? std::string("\\r") : std::string(1, c));
}
std::cout << std::endl;
}

void removeCarriageReturn(std::string& str) {
    size_t pos = str.find_last_not_of("\r");
    
    if (pos != std::string::npos) {
        str.erase(pos + 1);
    }
}