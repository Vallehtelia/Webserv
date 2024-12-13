#include "SessionManager.hpp"

std::string SessionManager::createSession() {
	std::lock_guard<std::mutex> lock(_mutex);
	std::string sessionId = generateSessionId();
	_sessions[sessionId] = {}; // Empty session data
	return sessionId;
}

bool SessionManager::isValidSession(const std::string& sessionId) const {
	std::lock_guard<std::mutex> lock(_mutex);
    bool valid = _sessions.find(sessionId) != _sessions.end();
    std::cout << "Session validation for ID: " << sessionId << " - " << (valid ? "Valid" : "Invalid") << std::endl;
    return valid;
}

void SessionManager::invalidateSession(const std::string& sessionId) {
	std::lock_guard<std::mutex> lock(_mutex);
	_sessions.erase(sessionId);
}

std::unordered_map<std::string, std::string>& SessionManager::getSession(const std::string& sessionId) {
	std::lock_guard<std::mutex> lock(_mutex);
	return _sessions[sessionId]; // Returns a reference to the session data
}

std::string SessionManager::generateSessionId() const {
	static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

	std::string sessionId;
	for (int i = 0; i < 16; ++i) { // 16-character session ID
		sessionId += charset[dist(gen)];
	}
	return sessionId;
}

