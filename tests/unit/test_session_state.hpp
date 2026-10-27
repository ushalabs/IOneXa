#pragma once

#include <QTest>
#include "core/session_state.hpp"

using namespace ionexa::core;

class TestSessionState : public QObject {
    Q_OBJECT

private slots:
    void testInitialState() {
        SessionStateMachine sm;
        QVERIFY(sm.currentState() == SessionState::Idle);
        QCOMPARE(QString::fromStdString(sessionStateToString(SessionState::Idle)), QString("Idle"));
    }

    void testValidTransitions() {
        SessionStateMachine sm;
        QVERIFY(sm.canTransitionTo(SessionState::Inspecting));
        QVERIFY(sm.transitionTo(SessionState::Inspecting));
        QVERIFY(sm.currentState() == SessionState::Inspecting);

        QVERIFY(sm.canTransitionTo(SessionState::Ready));
        QVERIFY(sm.transitionTo(SessionState::Ready));
        QVERIFY(sm.currentState() == SessionState::Ready);

        QVERIFY(sm.canTransitionTo(SessionState::Starting));
        QVERIFY(sm.transitionTo(SessionState::Starting));

        QVERIFY(sm.canTransitionTo(SessionState::Running));
        QVERIFY(sm.transitionTo(SessionState::Running));

        QVERIFY(sm.canTransitionTo(SessionState::Stopping));
        QVERIFY(sm.transitionTo(SessionState::Stopping));

        QVERIFY(sm.canTransitionTo(SessionState::Stopped));
        QVERIFY(sm.transitionTo(SessionState::Stopped));
    }

    void testInvalidTransitions() {
        SessionStateMachine sm;
        QVERIFY(!sm.canTransitionTo(SessionState::Running));
        QVERIFY(!sm.transitionTo(SessionState::Running));
        QVERIFY(sm.currentState() == SessionState::Idle);

        QVERIFY(!sm.canTransitionTo(SessionState::Idle));
        QVERIFY(!sm.transitionTo(SessionState::Idle));
    }

    void testStateChangeCallback() {
        SessionStateMachine sm;
        SessionState recordedOld = SessionState::Error;
        SessionState recordedNew = SessionState::Error;
        int callCount = 0;

        sm.setOnStateChanged([&](SessionState oldS, SessionState newS) {
            recordedOld = oldS;
            recordedNew = newS;
            ++callCount;
        });

        QVERIFY(sm.transitionTo(SessionState::Inspecting));
        QCOMPARE(callCount, 1);
        QVERIFY(recordedOld == SessionState::Idle);
        QVERIFY(recordedNew == SessionState::Inspecting);
    }
};
