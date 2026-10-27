#include "structured_logger.hpp"
#include <iomanip>
#include <sstream>

namespace ionexa::core {

std::string LogEntry::format() const {
    auto in_time_t = std::chrono::system_clock::to_time_t(timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()) % 1000;

    std::tm time_info = {};
#ifdef _WIN32
    localtime_s(&time_info, &in_time_t);
#else
    localtime_r(&in_time_t, &time_info);
#endif

    std::ostringstream ss;
    ss << "[" << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S")
       << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
       << "[" << logLevelToString(level) << "] "
       << "[" << logSourceToString(source) << "] "
       << message;

    if (!context.empty()) {
        ss << " {" << context << "}";
    }

    return ss.str();
}

StructuredLogger& StructuredLogger::instance() {
    static StructuredLogger s_instance;
    return s_instance;
}

StructuredLogger::StructuredLogger(size_t maxEntries)
    : m_maxEntries(maxEntries) {}

void StructuredLogger::log(LogLevel level, LogSource source, const std::string& message, const std::string& context) {
    LogEntry entry{
        std::chrono::system_clock::now(),
        level,
        source,
        message,
        context
    };

    std::vector<LogSubscriber> currentSubscribers;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.push_back(entry);
        if (m_entries.size() > m_maxEntries) {
            m_entries.pop_front();
        }
        currentSubscribers = m_subscribers;
    }

    for (const auto& sub : currentSubscribers) {
        if (sub) {
            sub(entry);
        }
    }
}

void StructuredLogger::debug(LogSource source, const std::string& message, const std::string& context) {
    log(LogLevel::Debug, source, message, context);
}

void StructuredLogger::info(LogSource source, const std::string& message, const std::string& context) {
    log(LogLevel::Info, source, message, context);
}

void StructuredLogger::warn(LogSource source, const std::string& message, const std::string& context) {
    log(LogLevel::Warning, source, message, context);
}

void StructuredLogger::error(LogSource source, const std::string& message, const std::string& context) {
    log(LogLevel::Error, source, message, context);
}

std::vector<LogEntry> StructuredLogger::getEntries(LogLevel minLevel, LogSource* filterSource) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<LogEntry> result;
    for (const auto& entry : m_entries) {
        if (static_cast<int>(entry.level) >= static_cast<int>(minLevel)) {
            if (filterSource == nullptr || entry.source == *filterSource) {
                result.push_back(entry);
            }
        }
    }
    return result;
}

void StructuredLogger::addSubscriber(LogSubscriber subscriber) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_subscribers.push_back(std::move(subscriber));
}

void StructuredLogger::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries.clear();
}

size_t StructuredLogger::size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_entries.size();
}

} // namespace ionexa::core
