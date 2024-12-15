#pragma once

#include <unordered_map>
#include <string>
#include <random>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>

struct SessionData {
    std::unordered_map<std::string, std::string> data;
	std::chrono::time_point<std::chrono::system_clock> creationTime; // Use system_clock
    std::chrono::time_point<std::chrono::system_clock> expirationTime; // Use system_clock
    bool isDummy; // Mark session as dummy
};

const size_t MAX_ACTIVE_SESSIONS = 1000000;
const size_t TARGET_SESSIONS = 900000;

class SessionManager {
public:
	inline static SessionManager& getInstance() { // has to be in header, singleton
		static SessionManager instance; // Single instance for the entire application
		return instance;
	}
	void clearSessions();
	void cleanUpExpiredSessions();
	void cullSessions();
    bool hasSession(const std::string& sessionId) const;
	std::string createDummySession();
    bool isValidSession(const std::string& sessionId) const;
	void validateAndExtendSession(const std::string& sessionId);
    SessionData& getSession(const std::string& sessionId);
	void printSessions() const;

private:
    std::unordered_map<std::string, SessionData> _sessions;
    std::string generateSessionId() const;

	SessionManager();
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;
	~SessionManager();
};
