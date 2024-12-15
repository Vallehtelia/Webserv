#pragma once

#include <unordered_map>
#include <string>
#include <random>
#include <chrono>
#include <mutex>
#include <iostream>

class SessionManager {
public:
	inline static SessionManager& getInstance() {
		static SessionManager instance; // Single instance for the entire application
		return instance;
	}
	void clearSessions();
    std::string createSession();
    bool isValidSession(const std::string& sessionId) const;
    void invalidateSession(const std::string& sessionId); // might be needed or not
    std::unordered_map<std::string, std::string>& getSession(const std::string& sessionId);
	void printSessions() const;

private:
	mutable std::mutex _mutex;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> _sessions;
    std::string generateSessionId() const;

	SessionManager();
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;
	~SessionManager();
};
