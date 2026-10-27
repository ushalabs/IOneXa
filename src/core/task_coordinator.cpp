#include "task_coordinator.hpp"

namespace ionexa::core {

TaskHandle::TaskHandle()
    : m_cancelled(std::make_shared<std::atomic<bool>>(false))
    , m_done(std::make_shared<std::atomic<bool>>(false)) {}

TaskHandle::~TaskHandle() {
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void TaskHandle::cancel() {
    if (m_cancelled) {
        m_cancelled->store(true);
    }
}

bool TaskHandle::isCancelled() const noexcept {
    return m_cancelled && m_cancelled->load();
}

bool TaskHandle::isDone() const noexcept {
    return m_done && m_done->load();
}

bool TaskHandle::waitFor(int timeoutMs) {
    const auto start = std::chrono::steady_clock::now();
    while (!isDone()) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count() >= timeoutMs) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    if (m_thread.joinable()) {
        m_thread.join();
    }
    return true;
}

TaskCoordinator::~TaskCoordinator() {
    cancelAll();
    waitForAll(1000);
}

std::shared_ptr<TaskHandle> TaskCoordinator::runAsync(
    std::function<void(const std::atomic<bool>& isCancelled)> work,
    std::function<void(bool wasCancelled)> onComplete) {

    auto handle = std::make_shared<TaskHandle>();
    auto isCancelled = handle->m_cancelled;
    auto isDone = handle->m_done;

    handle->m_thread = std::thread([work = std::move(work),
                                     onComplete = std::move(onComplete),
                                     isCancelled,
                                     isDone]() {
        if (work) {
            work(*isCancelled);
        }
        isDone->store(true);
        if (onComplete) {
            onComplete(isCancelled->load());
        }
    });

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push_back(handle);
    }

    return handle;
}

void TaskCoordinator::cancelAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& task : m_tasks) {
        if (task) {
            task->cancel();
        }
    }
}

void TaskCoordinator::waitForAll(int timeoutMs) {
    std::vector<std::shared_ptr<TaskHandle>> tasks;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        tasks = m_tasks;
    }

    for (auto& task : tasks) {
        if (task) {
            task->waitFor(timeoutMs);
        }
    }
}

} // namespace ionexa::core
