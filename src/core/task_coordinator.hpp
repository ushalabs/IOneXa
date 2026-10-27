#pragma once

#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <future>
#include <vector>
#include <mutex>

namespace ionexa::core {

class TaskHandle {
public:
    TaskHandle();
    ~TaskHandle();

    void cancel();
    [[nodiscard]] bool isCancelled() const noexcept;
    [[nodiscard]] bool isDone() const noexcept;
    bool waitFor(int timeoutMs = 5000);

private:
    friend class TaskCoordinator;
    std::shared_ptr<std::atomic<bool>> m_cancelled;
    std::shared_ptr<std::atomic<bool>> m_done;
    std::thread m_thread;
};

class TaskCoordinator {
public:
    TaskCoordinator() = default;
    ~TaskCoordinator();

    TaskCoordinator(const TaskCoordinator&) = delete;
    TaskCoordinator& operator=(const TaskCoordinator&) = delete;

    std::shared_ptr<TaskHandle> runAsync(
        std::function<void(const std::atomic<bool>& isCancelled)> work,
        std::function<void(bool wasCancelled)> onComplete = nullptr
    );

    void cancelAll();
    void waitForAll(int timeoutMs = 3000);

private:
    std::mutex m_mutex;
    std::vector<std::shared_ptr<TaskHandle>> m_tasks;
};

} // namespace ionexa::core
