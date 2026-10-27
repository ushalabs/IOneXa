# IOneXa Developer Guide & Architecture Manual

- **Publisher:** Usha Labs
- **Repository:** `https://github.com/ushalabs/IOneXa`
- **Target OS:** Windows 11 x64 / Windows 10 x64 (Build 1809+)
- **Target Frameworks:** Expo SDK 50+, React Native 0.73+, React Native Web

---

## 1. Product Overview & Architectural Philosophy

### 1.1 What is IOneXa?
**IOneXa** is a high-performance, Windows-native developer workbench engineered specifically for mobile application developers building on **Expo** and **React Native**. It bridges the gap between raw command-line tools and heavy virtualization by offering an instant, lightweight, interactive local preview workbench with deep device diagnostics directly on Windows.

### 1.2 Truth in Engineering Disclosure
Unlike commercial tools that make misleading claims regarding "iOS emulation on Windows":
- **IOneXa executes the real Expo / React Native codebase using Expo's official web bundler target (`@expo/metro-runtime` and `react-native-web`) embedded inside a native Microsoft Edge WebView2 canvas and Qt 6 desktop frame.**
- It is **NOT** an iOS emulator, iOS simulator, or Mach/Darwin kernel translation layer.
- It renders standard DOM elements framed inside calibrated mobile device viewports (iPhone 14/15, 15 Pro Max, SE, Pixel 7, Galaxy S23, iPad Mini).
- For native hardware APIs not supported on web (e.g. APNs, StoreKit, CoreBluetooth), IOneXa provides an automated **Native iOS Verification Handoff Guide** for seamless testing on physical iPhones (via Expo Go / QR code) or macOS Xcode simulators.

---

## 2. Host Environment & Prerequisites

### 2.1 Supported Operating Systems
- **Windows 11 x64** (Build 22000 or newer, recommended Build 26100+)
- **Windows 10 x64** (Version 1809 or newer, Build 17763+)

### 2.2 Toolchain Requirements
| Component | Minimum Version | Recommended / Tested Host Version |
|---|---|---|
| **C++ Compiler** | MSVC 19.30 (VS 2022) | MSVC 19.44 (Visual Studio 2022 v17.14) |
| **C++ Standard** | C++20 | ISO C++20 (`-std:c++20`) |
| **GUI Framework** | Qt 6.5 LTS | Qt 6.8.2 MSVC 2022 x64 (`C:\Qt\6.8.2\msvc2022_64`) |
| **Build System** | CMake 3.25 | CMake 3.31.6 + Ninja 1.12.1 |
| **Node.js** | Node.js 18.0.0 LTS | Node.js v24.12.0 + npm 11.6.2 |
| **Web Runtime** | Edge WebView2 | Evergreen Runtime (Preinstalled on Win 10/11) |

---

## 3. Building from Source

### 3.1 Initialize Visual Studio Developer Environment
Open PowerShell or Command Prompt and initialize the 64-bit developer environment:
```cmd
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
```

### 3.2 Configure CMake
From the project root:
```powershell
cmake -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.8.2\msvc2022_64"
```

### 3.3 Compile Binaries
```powershell
ninja -C build
```
This generates:
- `build/ionexa.exe`: The primary desktop workbench application.
- `build/ionexa_core.lib`: The static core engine library.
- `build/ionexa_unit_tests.exe`: The automated unit test suite.

### 3.4 Execute Automated Unit & Regression Tests
```powershell
ctest --test-dir build --output-on-failure
```
All 19 test suites must pass 100%.

---

## 4. Subsystem Architecture

```
+-----------------------------------------------------------------------+
|                              IOneXa UI                                |
|  +------------+  +--------------------------------+  +--------------+ |
|  |  NavRail   |  |   Viewport Frame & Device      |  |  Inspector   | |
|  |  (Open,    |  |   Bezel (Dynamic Island, Notch)|  |  Panel       | |
|  |   Start,   |  |   +--------------------------+ |  |  (Metrics,   | |
|  |   Stop,    |  |   |    Edge WebView2 Canvas  | |  |   Compat,    | |
|  |   Inspect) |  |   +--------------------------+ |  |   Handoff)   | |
|  +------------+  +--------------------------------+  +--------------+ |
|  +------------------------------------------------------------------+ |
|  | Bottom Tabs: [Console & Diagnostics] [Network Inspector]         | |
+--+------------------------------------------------------------------+-+
                                   |
                  +----------------+----------------+
                  |                                 |
+-----------------+---------------+ +---------------+-----------------+
|        Process Supervision      | |        Runtime Engines          |
|  - Windows Job Objects          | |  - Fast Refresh Bridge (HMR)   |
|  - ProcessSupervisor (node/npm) | |  - Network Inspector (HAR 1.2)  |
|  - MetroSupervisor (port 8081)  | |  - Compatibility Analyzer       |
|  - Thread-safe Joined Monitors  | |  - iOS Handoff Generator        |
|  - Security Policy Gatekeeper   | |  - CDP Device Metrics Override  |
+---------------------------------+ +---------------------------------+
```

### 4.1 Process Supervisor & Windows Job Objects
- **Zero Orphan Guarantee:** Metro bundler and Node.js child processes are bound to an OS-level Windows Job Object with `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`. Even in abnormal workbench termination, child processes are instantly terminated by the Windows kernel.
- **Port Conflict Resolution:** `MetroSupervisor::findAvailablePort` dynamically checks loopback ports starting from 8081 up to 8131, avoiding conflicts with existing development servers.

### 4.2 Live Preview Engine & WebView2
- **HWND Integration:** Embedded via Microsoft Edge WebView2 COM API (`vendor/webview2`).
- **CDP Emulation:** Communicates with Chromium via DevTools Protocol (`Emulation.setDeviceMetricsOverride`, `Emulation.setTouchEmulationEnabled`, `Network.setUserAgentOverride`).
- **Script Injection:** Injects runtime hooks capturing unhandled window errors, DOM console logs, and fetch/XHR traffic without touching user code on disk.

### 4.3 Fast Refresh Bridge
- Parses Metro stdout/stderr streams with high-efficiency regular expressions to detect bundle completion, durations, and compilation errors.
- Actively polls Metro loopback socket via `QTcpSocket` to detect server health and trigger automatic reconnection.

### 4.4 Compatibility Analyzer & iOS Handoff
- Evaluates project dependencies against an embedded knowledge base categorizing packages into `Supported`, `Partial`, `Unsupported`, and `Unverified`.
- Scans project source files for platform-specific patterns (`*.ios.*`, `*.web.*`).
- Generates step-by-step physical iPhone (Expo Go) and macOS Xcode Simulator handoff plans with actionable checklist gates.

---

## 5. Packaging & Distribution

To create a clean, portable Windows distribution:
```powershell
powershell -ExecutionPolicy Bypass -File scripts/package_windows.ps1
```
The packaging pipeline:
1. Deploys all required Qt 6 shared libraries and plugins via `windeployqt.exe`.
2. Bundles `WebView2Loader.dll`.
3. Emits `README_PORTABLE.txt` and calculates `SHA256SUMS.txt`.
4. Compresses everything into a self-contained archive:
   `dist/IOneXa-v0.1.0-windows-x64.zip` (~13 MB).

---

## 6. Developer Troubleshooting Guide

| Issue / Symptom | Root Cause | Remediation |
|---|---|---|
| **Port 8081 already in use** | Another Metro bundler or local server is running on port 8081. | IOneXa automatically finds the next available port (e.g. 8082). You can also set a custom port in App Settings. |
| **Node.js not detected** | Node.js is not present in system `PATH`. | Install Node.js v18+ from `https://nodejs.org` and restart IOneXa. |
| **WebView2 fails to initialize** | Edge WebView2 Runtime is missing or corrupted. | Download and install the **Evergreen WebView2 Bootstrapper** from Microsoft. |
| **Fast Refresh disconnected (Stale)** | Metro server process was terminated or loopback network blocked. | Check Developer Console for Metro errors. Click **Start Preview** to relaunch bundler. |
| **Antivirus blocks Node execution** | Windows Defender real-time scanning slows pipe creation. | Add project workspace folder to Windows Defender exclusion list. |
| **High-DPI blurriness** | DPI scaling mode mismatch. | IOneXa runs with `Qt::HighDpiScaleFactorRoundingPolicy::PassThrough` enabled automatically. |
