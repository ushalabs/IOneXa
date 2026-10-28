# IOneXa — Baseline Truth Audit and Runtime Reality Report
**Document ID:** `docs/native-ios/BASELINE_AUDIT.md`  
**Date:** October 28, 2026  
**Status:** Complete  
**Base Commit:** `90007dc416833c8d4dee32af9c69fab98043025e`  
**Working Branch:** `architecture/native-ios-execution`  
**Auditor:** Antigravity (Principal React Native/Expo & Windows Systems Engineer)

---

## 1. Executive Summary & Audit Purpose

This document provides a skeptical, proof-gated engineering truth audit of the `ushalabs/IOneXa` repository at commit `90007dc416833c8d4dee32af9c69fab98043025e` ("GUI Phase 7: Regression Testing and GUI Release Candidate").

Prior phase reports (Phases 0–18 and GUI Phases 1–7) claimed a fully functional, production-ready "Windows Mobile Development Workbench" for Expo and React Native applications, asserting complete passing acceptance tests across 26 test suites.

**The primary audit finding:** The existing implementation did **not** execute native iOS applications, did **not** run an iOS Simulator, and in fact did **not even execute the claimed Expo Web preview in the graphical UI**. The codebase contained critical uninstantiated pointers, re-entrant event loops, mocked test assertions, and hardcoded Expo Web invocations.

This document records the exact state of the codebase, reproduces defects with precise line references, classifies all prior claims, and establishes the foundation for the Phase 19–25 Native iOS Architecture Reset.

---

## 2. Environment and Baseline State

| Property | Value Observed on Target System |
|---|---|
| **Operating System** | Windows 11 Pro x64 (Version 24H2 / Build 26100) |
| **Repository URL** | `https://github.com/ushalabs/IOneXa` |
| **Default Branch** | `master` |
| **Audit Branch** | `architecture/native-ios-execution` |
| **Base HEAD Commit** | `90007dc416833c8d4dee32af9c69fab98043025e` |
| **C++ Toolchain** | MSVC 14.44.35207 (Visual Studio 2022 BuildTools x64) |
| **C++ Standard** | C++20 (`/std:c++20`) |
| **Build Generator** | Ninja via CMake 3.25+ |
| **Qt Version** | Qt 6.8.2 MSVC 2022 64-bit (`C:/Qt/6.8.2/msvc2022_64`) |
| **Node.js** | v24.12.0 |
| **npm** | 11.6.2 |
| **EAS CLI** | 18.6.0 global (outdated; latest 26.0.0; auth: `Not logged in`) |
| **WebView2** | Microsoft Edge WebView2 Evergreen Runtime installed |

---

## 3. Critical Code Defect Audit & Reproduction

### Defect 1: Uninstantiated `m_webView` in `MainWindow` (Dead Webview Pointer)
- **Files:** `src/ui/main_window.hpp` (line 145), `src/ui/main_window.cpp` (lines 71, 99, 867, 882)
- **Observed Code:**
  ```cpp
  // src/ui/main_window.hpp line 145:
  engines::WebView2Widget* m_webView{nullptr};
  ```
- **Finding:** `m_webView` is declared as a pointer initialized to `nullptr`. Nowhere in `MainWindow::MainWindow()` or `MainWindow::setupUi()` is `m_webView = new engines::WebView2Widget(...)` ever executed. Every guarded call (`if (m_webView) m_webView->reload();`) silently evaluates to false.
- **Classification:** **BROKEN**

### Defect 2: `setWebView()` in `DeviceEmulatorWindow` Is Never Called
- **Files:** `src/ui/device_emulator_window.hpp` (line 45, 100), `src/ui/device_emulator_window.cpp` (line 336)
- **Observed Code:**
  ```cpp
  // src/ui/device_emulator_window.cpp line 336:
  void DeviceEmulatorWindow::setWebView(engines::WebView2Widget* webView) {
      m_webView = webView;
      // ...
  }
  ```
- **Finding:** A repository-wide regex search for `setWebView` confirms that `setWebView()` is never invoked anywhere across the entire repository. As a result, `DeviceEmulatorWindow::m_webView` remains `nullptr` for the lifetime of the application.
- **Classification:** **BROKEN**

### Defect 3: The Standalone Emulator Renders an Empty Blank Frame
- **Files:** `src/ui/device_emulator_window.cpp` (lines 278–310)
- **Finding:** `DeviceEmulatorWindow` contains a layout with an iPhone bezel drawing routine, dynamic island, and a container widget (`m_deviceViewport`). Because `m_webView` was never instantiated or set, the viewport is completely empty. The previous GUI Phase 7 screenshots and claims of "Expo Web running in standalone window" were purely empty QFrames with painted borders.
- **Classification:** **BROKEN**

### Defect 4: Backend Process Hardcoded to Expo Web (`--web`)
- **Files:** `src/process/metro_supervisor.cpp` (lines 57–58)
- **Observed Code:**
  ```cpp
  std::wstring appPath = L"cmd.exe";
  std::wstring args = L"/c npx.cmd expo start --web --port " + std::to_wstring(m_activePort);
  ```
- **Finding:** The supervisor hardcodes launching Expo Web (`npx.cmd expo start --web`). This is a web-only bundler target utilizing `react-native-web` to compile React Native primitives into HTML DOM elements. It does not produce or launch iOS Simulator builds (`.app`), cannot execute native iOS modules, and cannot satisfy the master product contract.
- **Classification:** **BROKEN / ARCHITECTURALLY INVALID**

### Defect 5: TCP Socket Connection Falsely Claimed as Metro Dev Server Readiness
- **Files:** `src/process/metro_supervisor.cpp` (lines 124–131)
- **Observed Code:**
  ```cpp
  QTcpSocket socket;
  socket.connectToHost(QHostAddress::LocalHost, static_cast<quint16>(port));
  if (socket.waitForConnected(500)) {
      socket.disconnectFromHost();
      isReady = true;
      break;
  }
  ```
- **Finding:** Readiness is determined solely by whether any process accepts a TCP handshake on loopback. It does not probe the Metro HTTP endpoint (`/status`), does not verify JavaScript bundle compilation, and does not check development client connectivity.
- **Classification:** **BROKEN**

### Defect 6: Re-Entrant Reload Loop in UI Event Handlers
- **Files:** `src/ui/device_emulator_window.cpp` (lines 412–416), `src/ui/main_window.cpp` (lines 115, 863–873)
- **Observed Code:**
  ```cpp
  // DeviceEmulatorWindow::reload():
  void DeviceEmulatorWindow::reload() {
      if (m_webView) m_webView->reload();
      emit reloadRequested(); // [Signal 1]
  }

  // MainWindow constructor:
  connect(m_emulatorWindow, &DeviceEmulatorWindow::reloadRequested, this, &MainWindow::onReloadClicked);

  // MainWindow::onReloadClicked():
  void MainWindow::onReloadClicked() {
      // ...
      if (m_emulatorWindow) {
          m_emulatorWindow->reload(); // [Re-enters and triggers Signal 1 again!]
      }
  }
  ```
- **Finding:** When a reload is triggered from the emulator window, it emits `reloadRequested`, invoking `MainWindow::onReloadClicked()`, which immediately calls `m_emulatorWindow->reload()` again, triggering an uncontrolled recursive signal loop.
- **Classification:** **BROKEN**

### Defect 7: `dockEmulator()` Does Not Dock
- **Files:** `src/ui/main_window.cpp` (lines 918–924)
- **Observed Code:**
  ```cpp
  void MainWindow::dockEmulator() {
      if (!m_emulatorWindow) return;
      m_emulatorWindow->hide();
      m_emulatorWindow->setDocked(true);
      appendLog("[Layout] Device emulator docked into Workbench.");
      updateHomeDashboard();
  }
  ```
- **Finding:** Docking merely hides the window and sets a boolean flag. It does not reparent or insert the emulator viewport into the central Workbench view.
- **Classification:** **DEFERRED (GUI Issue, not an architectural gate)**

### Defect 8: Unit Test Failure in Clean Environment (`TestMetroSupervisor`)
- **Observed Execution:**
  ```text
  Internal ctest changing into directory: C:/Users/HP/Desktop/Projects/IOneXa/build
  Test project C:/Users/HP/Desktop/Projects/IOneXa/build
      Start 1: UnitTests
  1/1 Test #1: UnitTests ........................***Failed   24.22 sec
  PASSED: TestSessionState
  ...
  FAILED: TestMetroSupervisor (code 1)
  ...
  0% tests passed, 1 tests failed out of 1
  ```
- **Finding:** `TestMetroSupervisor::testStartAndStopLifecycle()` spawns `npx.cmd expo start --web` in `tests/fixtures/expo-basic` and loops with `QTest::qWait(500)` up to 15 seconds. On cold execution, Node/Expo startup exceeds 15 seconds, causing `QVERIFY(logSpy.count() > 0)` to fail and CTest to fail completely.
- **Classification:** **BROKEN**

### Defect 9: Mocked and Non-Executing Acceptance Tests
- **Files:** `tests/unit/test_feature_suite.hpp`, `tests/unit/test_acceptance_deskflow.hpp`
- **Finding:**
  - `test_feature_suite.hpp` uses `QFile::exists` to verify file existence in `tests/fixtures/expo-router` and parses query parameters with C++ `QUrlQuery`. It never executes JavaScript or renders screens.
  - `test_acceptance_deskflow.hpp` merely verifies `package.json` contains `"deskflow"`, displays an empty `DeviceEmulatorWindow` for 50ms, and captures a PNG of the empty widget frame.
  - Neither test suite proves that React Native code executes, that an app runs, or that iOS behavior is validated.
- **Classification:** **UNVERIFIED / OVERCLAIMED**

### Defect 10: Missing Native iOS Project Inspection
- **Files:** `src/project/project_scanner.cpp` (lines 102–120)
- **Finding:** The scanner strictly verified web dependencies: `react-native-web`, `react-dom`, `@expo/metro-runtime`. It did not parse `app.json` for `expo.ios.bundleIdentifier`, did not check `eas.json` for iOS simulator build profiles, did not inventory native Expo modules, and did not check EAS project association (`expo.extra.eas.projectId`).
- **Classification:** **UNVERIFIED / INCOMPLETE**

### Defect 11: Web-Biased Compatibility Matrix
- **Files:** `src/project/compatibility_analyzer.cpp` (lines 38–60)
- **Finding:** The analyzer flagged native iOS features (e.g. `react-native-keychain`, `react-native-iap`, `expo-camera`) as "Unsupported" or "Partial" because browsers lack Keychain Services or StoreKit. On a genuine iOS Simulator, Keychain Services, StoreKit testing, CADisplayLink, and worklets are fully supported native APIs. The database assumed a browser runtime rather than native iOS.
- **Classification:** **BROKEN / MISALIGNED**

---

## 4. Prior Claims Classification Truth Table

| Subsystem / Claim | Prior Claimed Status | Actual Verified Status | Classification | Evidence & Impact |
|---|---|---|---|---|
| **Primary Runtime** | Native Mobile Preview | Hardcoded `npx expo start --web` | **BROKEN** | Expo Web is not an iOS runtime. |
| **WebView2 Rendering** | Active embedded browser preview | `m_webView` is null; `setWebView` never called | **BROKEN** | No pixels ever rendered; dead pointers. |
| **Device Emulator Window** | Standalone calibrated mobile preview | Empty QFrame with painted bezels | **BROKEN** | Blank window; zero app execution. |
| **Metro Process Supervisor** | Robust lifecycle supervisor | Port TCP connection used as ready signal | **BROKEN** | Fails in CTest; no bundle readiness check. |
| **UI Reload Handling** | Clean refresh triggers | Recursive re-entrant reload loop | **BROKEN** | Double reload and infinite signal recursion. |
| **Docking / Undocking** | Seamless docking into Workbench | Only hides window and sets boolean | **DEFERRED** | Non-blocking GUI cosmetic flaw. |
| **Unit Test Suite** | 26 passing test suites (100% pass) | CTest fails on `TestMetroSupervisor` | **BROKEN** | 1 test failure fails entire test binary. |
| **Feature Acceptance** | Proven Expo Router & navigation | Static `QFile::exists` and `QUrlQuery` | **UNVERIFIED** | Zero JS execution in test suite. |
| **DeskFlow Acceptance** | Full real-world project pilot | Inspected `package.json`, shot empty QFrame | **UNVERIFIED** | DeskFlow was never actually run. |
| **Native iOS Simulator** | Claimed in marketing/docs | Never implemented | **BROKEN** | No macOS host, no `.app` build, no simulator. |
| **Project Scanner** | Full Expo project analysis | Only checks web dependencies | **PARTIAL** | Reads `package.json`, lacks `eas.json`/bundle ID. |
| **Theme & Windowing** | JetBrains/Android Studio Qt UI | Clean QWidget layout and palette | **VERIFIED** | Reusable desktop GUI shell. |
| **Windows Packaging** | Portable packaging engine | `package_windows.ps1` builds zip | **VERIFIED** | Reusable deployment script. |

---

## 5. False Acceptance Assumptions Identified

1. **False Assumption 1: Expo Web can substitute for iOS testing.**
   *Reality:* `react-native-web` compiles components into standard HTML DOM (`<div>`, `<span>`, `<input>`). It bypasses UIKit, CoreAnimation, native iOS networking, native navigation stacks (`react-native-screens`), native gesture recognizers, and all native iOS modules. An Expo Web build running in Chromium provides zero evidence that the app will build or run on iOS.

2. **False Assumption 2: An empty QFrame with painted iPhone borders constitutes an emulator.**
   *Reality:* Visual chrome (bezels, camera notch, dynamic island) drawn with `QPainter` has no connection to the mobile runtime. Rendering an outline without an executing binary is cosmetically deceptive.

3. **False Assumption 3: TCP port listening proves dev server readiness.**
   *Reality:* A port responds to TCP SYN as soon as Node opens a socket, long before Metro parses JavaScript, discovers assets, or compiles the initial bundle. It provides zero guarantee that the development client can fetch a bundle.

4. **False Assumption 4: File presence on Windows proves application execution.**
   *Reality:* Checking that `_layout.js` exists on disk does not prove that Expo Router resolves routes, mounts components, or handles navigation events.

---

## 6. Provider Candidates for Native iOS Execution

Windows hardware cannot natively execute Apple Mach-O binaries or run the official Apple iOS Simulator due to Darwin kernel, Metal, and UIKit dependencies. Therefore, authentic iOS execution requires remote execution on supported macOS/Apple infrastructure:

### Provider Option A: Expo EAS Simulator (Primary Candidate)
- **Mechanism:** Expo-hosted genuine iOS Simulator instances running on remote macOS infrastructure, controlled via EAS CLI (`eas simulator:*`).
- **Artifact:** Uses genuine native iOS Simulator `.app` builds compiled via EAS Build (`"ios": { "simulator": true }`).
- **Metro / Fast Refresh:** Documented remote dev client connection via tunnel or egress proxy.
- **Current Local Status:** EAS CLI 18.6.0 is installed; latest is 26.0.0. User account is currently `Not logged in`. Entitlement to EAS Simulator is early-access and must be verified in Phase 20 via `eas simulator:availability --json`.

### Provider Option B: Appetize.io (Secondary / Fallback Candidate)
- **Mechanism:** Commercial cloud iOS Simulator streaming platform.
- **Artifact:** Requires genuine iOS Simulator `.app` bundles archived as ZIP or TAR.GZ (explicitly rejects production `.ipa`).
- **Viewer / Controls:** Documented embeddable iframe viewer and JavaScript SDK for touch, typing, rotation, and lifecycle control.
- **Current Local Status:** Publicly accessible cloud API. Requires an active API token and usage plan. Embedding in Windows WebView2 is officially supported and documented.

### Provider Option C: React Native Windows / Fabric (Quarantined Optional R&D)
- **Mechanism:** Native Windows runtime using WinUI 3 / Fabric.
- **Status:** **NOT iOS.** Does not execute iOS Simulator binaries. Strictly quarantined to optional Phase 26 research and excluded from the iOS verification path.

---

## 7. Feasible Minimum Native Simulator Proof (Phase 21 Gate)

To achieve genuine native iOS verification, Phase 21 must construct an isolated native verification fixture and prove the following observable actions on a real remote iOS Simulator:
1. Valid iOS Simulator `.app` build ID and artifact provenance.
2. Active remote iOS Simulator session ID and confirmed running status.
3. Live rendering of `IOneXa Native iOS Verification Fixture`.
4. Working React Native `Pressable` counter with verified state change.
5. Working native `TextInput` invoking the authentic iOS onscreen keyboard.
6. Triggering a native iOS system alert (`Alert.alert()`) rendered by UIKit.
7. Explicit verification that `.web.tsx` web fallback code is **not** running.
8. Clean remote session termination and teardown.

---

## 8. Conclusion and Next Steps

The baseline audit confirms that previous claims of iOS testing on Windows were entirely unverified and architecturally flawed. The repository truth audit is complete. Phase 19 transitions directly into establishing the Architecture Decision Record (`ARCHITECTURE_DECISION_RECORD.md`) and the provider-agnostic interface definitions.
