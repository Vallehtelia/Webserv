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

void SessionManager::cleanUpExpiredSessions() {
    std::lock_guard<std::mutex> lock(_mutex);
    auto now = std::chrono::steady_clock::now();

    for (auto it = _sessions.begin(); it != _sessions.end(); ) {
        const auto& sessionId = it->first;
        const auto& sessionData = it->second; // avoids using []operator for performance (no lookup necessary)

		// Possibly add culling of older sessions IF number of active sessions exceeds
		// a certain large number, to avoid bombarding with setting more and more cookies
		// by repeating a few requests then deleting the cookie at clients end
		// currently no way to know which sessions are 'really active'
        if (now > sessionData.expirationTime) {
            // Print details of the expired session
            std::cout << "Expired session: " << sessionId
                      << ", is dummy: " << (sessionData.isDummy ? "true" : "false") << " was deleted." << std::endl;

            // Erase the session from the map
            it = _sessions.erase(it); // safer in the loop than passing to another function.
        } else {
            ++it;
        }
    }
}

static unsigned int seedWithTime() {
    std::random_device rd;
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return rd() ^ static_cast<unsigned int>(duration.count());
}

bool SessionManager::hasSession(const std::string& sessionId) const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _sessions.find(sessionId) != _sessions.end();
}

std::string SessionManager::createDummySession() { // add more time before cleanup later
    std::string sessionId = generateSessionId();
    _sessions[sessionId] = { {}, std::chrono::steady_clock::now() + std::chrono::minutes(1), true };
    return sessionId;
}

bool SessionManager::isValidSession(const std::string& sessionId) const {
	std::lock_guard<std::mutex> lock(_mutex);
    bool valid = _sessions.find(sessionId) != _sessions.end();
    std::cout << "Session validation for ID: " << sessionId << " - " << (valid ? "Valid" : "Invalid") << std::endl;
    return valid;
}

void SessionManager::validateAndExtendSession(const std::string& sessionId) {
	std::lock_guard<std::mutex> lock(_mutex);
	_sessions[sessionId].expirationTime = std::chrono::steady_clock::now() + std::chrono::hours(1);
	_sessions[sessionId].isDummy = false;
}

SessionData& SessionManager::getSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(_mutex);
    return _sessions.at(sessionId); // Throws std::out_of_range if sessionId is not found
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
	for (const auto& [sessionId, sessionData] : _sessions) { // Use sessionData directly
		std::cout << "Session ID: " << sessionId << std::endl;

		// Iterate over the data map inside SessionData
		for (const auto& [key, value] : sessionData.data) {
			std::cout << "  " << key << ": " << value << std::endl;
		}

        // Print expiration time in human-readable format
        std::time_t expirationTimeT = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now() +
            (sessionData.expirationTime - std::chrono::steady_clock::now()));

        std::cout << "  Expiration Time: " 
                  << std::put_time(std::localtime(&expirationTimeT), "%Y-%m-%d %H:%M:%S") 
                  << std::endl;

		std::cout << "  Considered dummy: " << sessionData.isDummy << std::endl;
	}
}

