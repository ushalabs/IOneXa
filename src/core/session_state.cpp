#include "session_state.hpp"

namespace ionexa::core {

std::string sessionStateToString(SessionState state) {
    switch (state) {
    case SessionState::Idle: return "Idle";
    case SessionState::Inspecting: return "Inspecting";
    case SessionState::Ready: return "Ready";
    case SessionState::Starting: return "Starting";
    case SessionState::Running: return "Running";
    case SessionState::Stopping: return "Stopping";
    case SessionState::Stopped: return "Stopped";
    case SessionState::Error: return "Error";
    default: return "Unknown";
    }
}

SessionStateMachine::SessionStateMachine(SessionState initialState)
    : m_state(initialState) {}

SessionState SessionStateMachine::currentState() const {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_state;
}

bool SessionStateMachine::canTransitionTo(SessionState newState) const {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (m_state == newState) {
        return false;
    }

    switch (m_state) {
    case SessionState::Idle:
        return newState == SessionState::Inspecting || newState == SessionState::Error;

    case SessionState::Inspecting:
        return newState == SessionState::Ready || newState == SessionState::Idle || newState == SessionState::Error;

    case SessionState::Ready:
        return newState == SessionState::Starting || newState == SessionState::Inspecting || newState == SessionState::Idle || newState == SessionState::Error;

    case SessionState::Starting:
        return newState == SessionState::Running || newState == SessionState::Stopping || newState == SessionState::Error;

    case SessionState::Running:
        return newState == SessionState::Stopping || newState == SessionState::Error;

    case SessionState::Stopping:
        return newState == SessionState::Stopped || newState == SessionState::Error;

    case SessionState::Stopped:
        return newState == SessionState::Ready || newState == SessionState::Starting || newState == SessionState::Idle || newState == SessionState::Error;

    case SessionState::Error:
        return newState == SessionState::Idle || newState == SessionState::Ready;

    default:
        return false;
    }
}

bool SessionStateMachine::transitionTo(SessionState newState) {
    SessionState oldState;
    StateChangeCallback cb;

    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (!canTransitionTo(newState)) {
            return false;
        }
        oldState = m_state;
        m_state = newState;
        cb = m_callback;
    }

    if (cb) {
        cb(oldState, newState);
    }
    return true;
}

void SessionStateMachine::setOnStateChanged(StateChangeCallback callback) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_callback = std::move(callback);
}

void SessionStateMachine::reset() {
    SessionState oldState;
    StateChangeCallback cb;

    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        oldState = m_state;
        m_state = SessionState::Idle;
        cb = m_callback;
    }

    if (cb && oldState != SessionState::Idle) {
        cb(oldState, SessionState::Idle);
    }
}

} // namespace ionexa::core
