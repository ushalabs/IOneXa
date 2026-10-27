#pragma once

#include <string>
#include <functional>
#include <mutex>

namespace ionexa::core {

enum class SessionState {
    Idle,
    Inspecting,
    Ready,
    Starting,
    Running,
    Stopping,
    Stopped,
    Error
};

std::string sessionStateToString(SessionState state);

class SessionStateMachine {
public:
    using StateChangeCallback = std::function<void(SessionState oldState, SessionState newState)>;

    explicit SessionStateMachine(SessionState initialState = SessionState::Idle);

    [[nodiscard]] SessionState currentState() const;
    [[nodiscard]] bool canTransitionTo(SessionState newState) const;
    bool transitionTo(SessionState newState);

    void setOnStateChanged(StateChangeCallback callback);
    void reset();

private:
    mutable std::recursive_mutex m_mutex;
    SessionState m_state;
    StateChangeCallback m_callback;
};

} // namespace ionexa::core
