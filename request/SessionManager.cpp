#include "SessionManager.hpp"

SessionManager::SessionManager() {
    std::cout << "SessionManager initialized: _sessions.size() = " << _sessions.size() << std::endl;
}

SessionManager::~SessionManager() {
	clearSessions();
    std::cout << "SessionManager destructed." << std::endl;
}

void SessionManager::clearSessions() {
	_sessions.clear();
	std::cout << "After clearSessions: _sessions.size() = " << _sessions.size() << std::endl;
}

static unsigned int seedWithTime() {
    std::random_device rd;
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return rd() ^ static_cast<unsigned int>(duration.count());
}

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
	unsigned int seed = seedWithTime();
	static thread_local std::mt19937 gen(seed);
    static std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

	std::string sessionId;
    for (int i = 0; i < 16; ++i) {
        int index = dist(gen);
        sessionId += charset[index];
    }
	std::cout << "Generated session ID: " << sessionId << std::endl;
	return sessionId;
}

void SessionManager::printSessions() const {
	for (const auto& [sessionId, data] : _sessions) {
		std::cout << "Session ID: " << sessionId << std::endl;
		for (const auto& [key, value] : data) {
			std::cout << "  " << key << ": " << value << std::endl;
		}
	}
}

