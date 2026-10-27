# IOneXa

**Windows-Native Mobile Application Development and Testing Workbench for Expo and React Native**

*Developed by Usha Labs*

---

> [!IMPORTANT]
> **Truth in Engineering Disclosure:**  
> IOneXa executes real Expo and React Native projects using Expo's official web bundler target (`react-native-web` / Metro) embedded in a Windows-native shell powered by Microsoft Edge WebView2 and Qt 6.  
> **This is an Expo Web Preview, NOT an iOS runtime or iOS simulator.** It does not boot an iOS kernel, run compiled `.ipa` binaries, or emulate Apple-specific platform services (such as APNs, StoreKit, Apple Pay, Keychain, or native UIKit controls). For genuine iOS verification, tests must be executed on a physical iPhone or a macOS-hosted Apple Simulator.

---

## 1. Overview

IOneXa provides Windows developers with an integrated, zero-friction desktop workbench to inspect, run, preview, debug, and test Expo/React Native mobile applications directly on Windows 11/10 without requiring a Mac or heavy emulation overhead.

### Key Capabilities
- **Real Project Execution:** Runs unmodified Expo projects using their own local Metro bundler and Node toolchain.
- **Process Supervisor:** Windows Job Object-managed process lifecycle ensuring zero orphaned Metro or Node processes upon stop or crash.
- **Interactive Mobile Viewport:** Device presets, responsive dimensions, orientation toggling, and DPI scaling.
- **Diagnostics & Debugging:** Unified console streaming Metro stdout/stderr and WebView2 browser console logs, network activity traces, and structured error boundaries.
- **Compatibility Analyzer:** Scans project dependencies to identify platform-specific divergence (`.ios.*` vs `.web.*`), unsupported native modules, and honest readiness status.

---

## 2. Architecture & Tech Stack

- **Desktop Shell:** C++20, Qt 6.8.2 (Widgets architecture with native Win32 HWND embedding).
- **Embedded Web Engine:** Microsoft Edge WebView2 (Evergreen runtime, pre-installed on Windows 10/11) communicating over Chromium DevTools Protocol (CDP).
- **Process Management:** Win32 `CreateProcessW` bound to Windows Job Objects (`JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`).
- **Project Toolchain:** Node.js, npm / yarn / pnpm / bun, `@expo/metro-runtime`.
- **Build System:** CMake (>= 3.25) with Ninja and MSVC 2022 (v143).

---

## 3. Repository Structure

```text
IOneXa/
  CMakeLists.txt            # Root build specification
  CMakePresets.json         # Standard MSVC x64 presets
  src/
    app/                    # Application entry point, window management
    core/                   # Configuration, session state machine, errors
    project/                # Project scanner, dependency & manifest analysis
    process/                # Windows Job Object process supervisor
    engines/                # Engine contracts (Expo Web via WebView2)
    devices/                # Viewport presets, orientation, scaling
    diagnostics/            # Console aggregation, CDP bridge, log exporter
    ui/                     # Dark-themed Qt desktop UI components
  tests/
    unit/                   # C++ unit tests (session state, scanner, process)
    fixtures/
      expo-basic/           # Standard Expo fixture with interactive state
      expo-router/          # Expo Router navigation fixture
      expo-platform-variants/ # Platform-split code fixture
  vendor/
    webview2/               # Microsoft.Web.WebView2 C++ SDK headers & libs
  docs/
    ADR/                    # Architectural Decision Records
    PHASE_REPORTS/          # Verifiable Phase-by-Phase Completion Reports
    ARCHITECTURE.md
    COMPATIBILITY.md
    SECURITY.md
    TEST_PLAN.md
```

---

## 4. Getting Started

### Prerequisites on Windows
- Windows 11 x64 or Windows 10 x64 (Build 19041+)
- Node.js (v18+ LTS, v20+, or v24+)
- Visual Studio 2022 C++ Build Tools with CMake & Ninja
- Qt 6.8.2+ MSVC 2022 x64 (`C:\Qt\6.8.2\msvc2022_64`)
- Microsoft Edge WebView2 Runtime (Evergreen)

### Building from Source

```cmd
:: Open VS Developer Command Prompt or initialize vcvars64.bat
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

:: Set Qt directory
set CMAKE_PREFIX_PATH=C:\Qt\6.8.2\msvc2022_64

:: Configure with CMake and Ninja
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

:: Build target
cmake --build build --config Release

:: Run Unit Tests
ctest --test-dir build --output-on-failure
```

---

## 5. Development Phases & Roadmap

| Phase | Title | Status | Git Commit |
|---|---|---|---|
| Phase 0 | Feasibility Spike and Runtime Decision | In Progress | Pending |
| Phase 1 | Repository Foundation and Build System | Planned | Pending |
| Phase 2 | Runtime Engine Contracts and Session Lifecycle | Planned | Pending |
| Phase 3 | Expo Project Import and Environment Diagnostics | Planned | Pending |
| Phase 4 | Metro Launch and Windows Process Management | Planned | Pending |
| Phase 5 | Live Expo Application Preview Engine | Planned | Pending |
| Phase 6 | Modern Windows Workspace and Design System | Planned | Pending |
| Phase 7 | Fast Refresh Integration and Live Reload | Planned | Pending |
| Phase 8 | Responsive Device Viewports and Input Simulation | Planned | Pending |
| Phase 9 | Developer Console and Runtime Diagnostics | Planned | Pending |
| Phase 10 | Network Inspector and Debugging Integration | Planned | Pending |
| Phase 11 | Expo Module and iOS Compatibility Analyzer | Planned | Pending |
| Phase 12 | Real-World Expo Feature Test Suite | Planned | Pending |
| Phase 13 | Automated Preview Testing and Regression Suite | Planned | Pending |
| Phase 14 | Reliability Performance and Security Hardening | Planned | Pending |
| Phase 15 | Experimental Native React Native Runtime Research | Planned | Pending |
| Phase 16 | Native iOS Verification Handoff and Reports | Planned | Pending |
| Phase 17 | Windows Packaging and Developer Documentation | Planned | Pending |
| Phase 18 | Real Application Pilot and Version 0.1 Release | Planned | Pending |

---

## 6. License & Attribution

Copyright © 2026 Usha Labs. All rights reserved.  
Uses Qt under LGPLv3 / Commercial compliance. Microsoft Edge WebView2 SDK is licensed under the Microsoft Software License.
