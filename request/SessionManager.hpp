#pragma once

#include <unordered_map>
#include <string>
#include <random>
#include <chrono>
#include <mutex>
#include <iostream>
#include <iomanip>

struct SessionData {
    std::unordered_map<std::string, std::string> data;
    std::chrono::time_point<std::chrono::steady_clock> expirationTime;
    bool isDummy; // Mark session as dummy
};

class SessionManager {
public:
	inline static SessionManager& getInstance() { // has to be in header, singleton
		static SessionManager instance; // Single instance for the entire application
		return instance;
	}
	void clearSessions();
	void cleanUpExpiredSessions();
    bool hasSession(const std::string& sessionId) const;
	std::string createDummySession();
    bool isValidSession(const std::string& sessionId) const;
	void validateAndExtendSession(const std::string& sessionId);
    SessionData& getSession(const std::string& sessionId);
	void printSessions() const;

private:
	mutable std::mutex _mutex;
    std::unordered_map<std::string, SessionData> _sessions;
    std::string generateSessionId() const;

	SessionManager();
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;
	~SessionManager();
};
