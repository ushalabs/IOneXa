#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <functional>
#include <deque>

namespace ionexa::core {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

enum class LogSource {
    System,
    Metro,
    BrowserConsole,
    Network,
    Engine
};

inline std::string logLevelToString(LogLevel level) {
    switch (level) {
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info: return "INFO";
    case LogLevel::Warning: return "WARN";
    case LogLevel::Error: return "ERROR";
    default: return "UNKNOWN";
    }
}

inline std::string logSourceToString(LogSource source) {
    switch (source) {
    case LogSource::System: return "SYSTEM";
    case LogSource::Metro: return "METRO";
    case LogSource::BrowserConsole: return "CONSOLE";
    case LogSource::Network: return "NETWORK";
    case LogSource::Engine: return "ENGINE";
    default: return "UNKNOWN";
    }
}

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    LogSource source;
    std::string message;
    std::string context;

    [[nodiscard]] std::string format() const;
};

class StructuredLogger {
public:
    using LogSubscriber = std::function<void(const LogEntry& entry)>;

    static StructuredLogger& instance();

    explicit StructuredLogger(size_t maxEntries = 5000);

    void log(LogLevel level, LogSource source, const std::string& message, const std::string& context = "");
    void debug(LogSource source, const std::string& message, const std::string& context = "");
    void info(LogSource source, const std::string& message, const std::string& context = "");
    void warn(LogSource source, const std::string& message, const std::string& context = "");
    void error(LogSource source, const std::string& message, const std::string& context = "");

    [[nodiscard]] std::vector<LogEntry> getEntries(LogLevel minLevel = LogLevel::Debug,
                                                   LogSource* filterSource = nullptr) const;

    void addSubscriber(LogSubscriber subscriber);
    void clear();
    [[nodiscard]] size_t size() const;

private:
    size_t m_maxEntries;
    mutable std::mutex m_mutex;
    std::deque<LogEntry> m_entries;
    std::vector<LogSubscriber> m_subscribers;
};

} // namespace ionexa::core
