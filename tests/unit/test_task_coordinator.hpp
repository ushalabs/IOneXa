#pragma once

#include <QTest>
#include "core/task_coordinator.hpp"

using namespace ionexa::core;

class TestTaskCoordinator : public QObject {
    Q_OBJECT

private slots:
    void testAsyncExecution() {
        TaskCoordinator coordinator;
        std::atomic<bool> workExecuted{false};
        std::atomic<bool> completionCalled{false};

        auto handle = coordinator.runAsync(
            [&](const std::atomic<bool>& isCancelled) {
                Q_UNUSED(isCancelled);
                workExecuted.store(true);
            },
            [&](bool wasCancelled) {
                Q_UNUSED(wasCancelled);
                completionCalled.store(true);
            }
        );

        QVERIFY(handle != nullptr);
        QVERIFY(handle->waitFor(2000));
        QVERIFY(handle->isDone());
        QVERIFY(!handle->isCancelled());
        QVERIFY(workExecuted.load());
        QVERIFY(completionCalled.load());
    }

    void testCancellation() {
        TaskCoordinator coordinator;
        std::atomic<bool> cancelledObserved{false};

        auto handle = coordinator.runAsync(
            [&](const std::atomic<bool>& isCancelled) {
                for (int i = 0; i < 50; ++i) {
                    if (isCancelled.load()) {
                        cancelledObserved.store(true);
                        return;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
            }
        );

        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        handle->cancel();
        QVERIFY(handle->isCancelled());

        QVERIFY(handle->waitFor(2000));
        QVERIFY(handle->isDone());
        QVERIFY(cancelledObserved.load());
    }
};
