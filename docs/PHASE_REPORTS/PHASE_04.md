# Phase 4 Report: Metro Launch and Windows Process Management

- **Phase number / exact title:** Phase 4: "Metro Launch and Windows Process Management"
- **Status:** PASS
- **Date:** 2026-10-27
- **Host OS:** Windows 11 x64 (MSVC 19.44 / CMake 3.31.6 / Ninja 1.12.1 / Qt 6.8.2)
- **Git Commit SHA:** pending until commit

---

## 1. What Was Implemented

1. **Metro & Expo Process Supervisor (`src/process/metro_supervisor.hpp` & `.cpp`):**
   - High-level orchestrator built on `ProcessSupervisor` and Win32 Job Objects.
   - Dynamic port selection & conflict resolution (`findAvailablePort`): probes loopback ports with `QTcpServer`, automatically skipping occupied ports to prevent collision crashes.
   - Safe command dispatch: executes `cmd.exe /c npx.cmd expo start --web --port <port>` with native wchar working directory handling for spaces in paths.
   - Bi-directional pipe streaming: implemented background worker threads reading both `stdout` and `stderr` simultaneously to prevent buffer saturation deadlocks.
   - Structured logging integration: routes process lines to `StructuredLogger` with millisecond timestamps and source tags.
   - Startup readiness detector (`monitorReadiness`): asynchronously probes the bound loopback port until TCP connectivity is confirmed, then transitions status to `Ready`.
   - Windows process tree containment: assigns spawned processes to Job Objects with `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`, ensuring clean termination of `cmd.exe`, `node.exe`, and all descendant worker threads upon `stop()` or crash.

2. **Automated Unit & Integration Tests (`tests/unit/test_metro_supervisor.hpp`):**
   - Verified port conflict resolution: bound a mock server to port 8200; confirmed `findAvailablePort` detected the conflict and safely advanced to port 8201.
   - Verified actual end-to-end Metro process lifecycle:
     - Scanned `tests/fixtures/expo-basic`.
     - Spawned Metro supervisor on dynamic loopback port.
     - Confirmed process ID assignment (`processId() > 0`).
     - Verified real stdout/stderr log emission (`logSpy.count() > 0`).
     - Executed graceful stop; verified transition to `Stopped`.
     - Verified zero orphaned child processes.

---

## 2. Files Changed / Created

- `src/process/metro_supervisor.hpp`
- `src/process/metro_supervisor.cpp`
- `src/process/process_supervisor.cpp` (added stderr background reader thread)
- `tests/unit/test_metro_supervisor.hpp`
- `tests/unit/main.cpp`
- `CMakeLists.txt`
- `docs/PHASE_REPORTS/PHASE_04.md`

---

## 3. Key Architectural Decisions

- **Dual-Pipe Background Readers:** Read both `stdout` and `stderr` in separate detached worker threads to prevent Win32 pipe deadlocks when node/npm writes warning banners to standard error.
- **Loopback Port Conflict Isolation:** Replaces hardcoded default ports with dynamic loopback probing, enabling seamless concurrent runs without port contention.
- **Job Object Teardown Guarantee:** Guarantees that stopping the supervisor instantly terminates the entire tree of spawned Node and cmd worker processes.

---

## 4. Commands Run

```cmd
:: Build all targets with MetroSupervisor integration
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cmake --build build --config Release

:: Execute CTest suite
ctest --test-dir build --output-on-failure

:: Verify absence of orphaned processes
tasklist /fi "imagename eq node.exe"
```

---

## 5. Unit / Integration / Manual Tests

| Scenario | Mode | Expected | Result |
|---|---|---|---|
| Dynamic Port Conflict Resolution | Automated (QtTest) | Occupied port skipped, next free port found | **PASS** |
| Process Supervisor Launch | Automated (QtTest) | `processId() > 0`, state is `Starting` | **PASS** |
| Real Log Streaming | Automated (QtTest) | `logOutput` signal fires with stdout/stderr lines | **PASS** |
| Clean Teardown | Automated (QtTest) | `stop()` terminates tree, state is `Stopped` | **PASS** |
| Orphaned Process Verification | Host Inspection | No orphaned Metro worker processes left | **PASS** |
| Full Regression Suite | CTest Runner | All 8 test suites pass | **PASS** (100% Passed) |

---

## 6. Concrete Proof (Build & Test Logs)

```text
Internal ctest changing into directory: C:/Users/HP/Desktop/Projects/IOneXa/build
Test project C:/Users/HP/Desktop/Projects/IOneXa/build
    Start 1: UnitTests
1/1 Test #1: UnitTests ........................   Passed    3.37 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   3.40 sec

============================================
ALL 8 IONEXA UNIT TEST SUITES PASSED (100%)!
============================================
```

---

## 7. Known Limitations

- Readiness is monitored via loopback TCP handshake; full HTTP body verification is performed during live preview embedding in Phase 5.

---

## 8. Security Review & Project File Mutations

- **Zero Project Mutations:** No fixture files were changed.
- **Localhost Binding:** Development server binds explicitly to loopback `127.0.0.1` and does not broadcast across the LAN.
- **Safe Working Directories:** Working directory paths with spaces are handled without shell string concatenation vulnerabilities.

---

## 9. Build and Runtime Version Details

- MSVC: 19.44.35229 (Visual Studio 2022 v143)
- CMake: 3.31.6
- Ninja: 1.12.1
- Qt: 6.8.2 MSVC 2022 x64
- Node.js: v24.12.0

---

## 10. Acceptance Criteria Checklist

- [x] Metro and Expo process supervisor implemented.
- [x] Dynamic loopback port discovery and conflict avoidance verified.
- [x] Startup readiness detection implemented.
- [x] Bi-directional stdout and stderr streaming without pipe deadlocks.
- [x] Process-tree cleanup via Windows Job Objects verified with zero orphans.
- [x] Paths with spaces supported natively.
- [x] All 8 unit test suites pass 100% under CTest.
- [x] Phase report authored in `docs/PHASE_REPORTS/PHASE_04.md`.

---

## 11. Git Commit

- **Exact Subject:** `Phase 4: "Metro Launch and Windows Process Management"`
- **Commit SHA:** pending until commit

---

## 12. Next Phase Dependencies

- **Phase 5: Live Expo Application Preview Engine**
  - Embed Microsoft Edge WebView2 runtime into Qt MainWindow canvas, connect Metro readiness to live navigation, implement navigation tracking, renderer crash recovery, and truth-in-labeling UI badges.
