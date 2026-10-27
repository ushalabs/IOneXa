#pragma once

#include <string>
#include <stdexcept>
#include <utility>

namespace ionexa::core {

enum class ErrorCategory {
    General,
    ProjectInspection,
    ProcessManagement,
    SessionLifecycle,
    Rendering,
    Compatibility
};

inline std::string errorCategoryToString(ErrorCategory category) {
    switch (category) {
    case ErrorCategory::General: return "General";
    case ErrorCategory::ProjectInspection: return "ProjectInspection";
    case ErrorCategory::ProcessManagement: return "ProcessManagement";
    case ErrorCategory::SessionLifecycle: return "SessionLifecycle";
    case ErrorCategory::Rendering: return "Rendering";
    case ErrorCategory::Compatibility: return "Compatibility";
    default: return "Unknown";
    }
}

class IOneXaError : public std::runtime_error {
public:
    IOneXaError(ErrorCategory category, int code, std::string message, std::string details = "")
        : std::runtime_error(message)
        , m_category(category)
        , m_code(code)
        , m_details(std::move(details)) {}

    [[nodiscard]] ErrorCategory category() const noexcept { return m_category; }
    [[nodiscard]] int code() const noexcept { return m_code; }
    [[nodiscard]] const std::string& details() const noexcept { return m_details; }

    [[nodiscard]] std::string format() const {
        std::string res = "[" + errorCategoryToString(m_category) + " E" + std::to_string(m_code) + "] " + what();
        if (!m_details.empty()) {
            res += " (Details: " + m_details + ")";
        }
        return res;
    }

private:
    ErrorCategory m_category;
    int m_code;
    std::string m_details;
};

class ProjectError : public IOneXaError {
public:
    ProjectError(int code, std::string message, std::string details = "")
        : IOneXaError(ErrorCategory::ProjectInspection, code, std::move(message), std::move(details)) {}
};

class ProcessError : public IOneXaError {
public:
    ProcessError(int code, std::string message, std::string details = "")
        : IOneXaError(ErrorCategory::ProcessManagement, code, std::move(message), std::move(details)) {}
};

class SessionError : public IOneXaError {
public:
    SessionError(int code, std::string message, std::string details = "")
        : IOneXaError(ErrorCategory::SessionLifecycle, code, std::move(message), std::move(details)) {}
};

} // namespace ionexa::core
