# IOneXa v0.1.0 Release Notes

**Windows-Native Mobile Application Development and Testing Workbench for Expo and React Native**  
**Publisher:** Usha Labs  
**Release Date:** October 28, 2026  
**Git Tag:** `v0.1.0`  
**Target Platform:** Windows 11 x64 / Windows 10 x64 (Build 1809+)  

---

## 1. Executive Summary

Usha Labs is proud to announce the **v0.1.0 General Availability** release of **IOneXa**, a Windows-native development and testing workbench engineered from the ground up for **Expo** and **React Native** developers.

IOneXa solves the developer pain of developing cross-platform mobile apps on Windows without requiring expensive macOS hardware, complex dual-boot configurations, or slow, heavy Android emulators for early-stage iteration. By embedding a high-performance Microsoft Edge WebView2 canvas directly inside a Qt 6 desktop application, IOneXa executes live, unmodified Expo projects via their native Metro bundler with sub-second Fast Refresh, realistic mobile viewport framing, and comprehensive diagnostics.

---

## 2. Truth in Engineering Certification

> **Usha Labs Truth in Engineering Commitment:**  
> IOneXa executes real Expo / React Native projects via Expo's supported Web bundler target (`@expo/metro-runtime` and `react-native-web`) embedded inside Microsoft Edge WebView2 and Qt 6.  
> **IOneXa is NOT an iOS emulator or macOS simulator.** It does not emulate an Apple Mach/Darwin kernel, execute compiled `.ipa` binaries, or emulate native Apple services (e.g. APNs, StoreKit, Apple Pay, or Keychain). For downstream native verification, IOneXa includes an automated **Native iOS Verification Handoff Guide** for testing on physical iPhones via Expo Go or macOS Xcode.

---

## 3. Key Features Delivered Across 18 Engineering Phases

### 3.1 Core Architecture & Process Supervision
- **Windows Job Object Supervision:** Every Metro bundler and Node.js process is strictly bound to an OS-level Windows Job Object (`JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`), ensuring zero orphaned processes or lingering port locks upon exit or crash.
- **Dynamic Port Conflict Resolution:** Automatically probes and allocates safe, unprivileged loopback ports (8081-8131) to prevent port collisions with existing tools.
- **Session State Machine:** Thread-safe state coordinator managing `Idle`, `Inspecting`, `Ready`, `Starting`, `Running`, `Stopping`, `Stopped`, and `Error` lifecycles.

### 3.2 Live Preview & Viewport Emulation
- **Hardware-Accelerated Embedded Web View:** Microsoft Edge WebView2 COM integration hosted inside a native Win32 HWND container.
- **Calibrated Device Profiles:** 7 pre-calibrated mobile viewport presets:
  - iPhone 14/15 (390 x 844, 3x scale)
  - iPhone 15 Pro Max (430 x 932, 3x scale)
  - iPhone SE 3rd Gen (375 x 667, 2x scale)
  - Google Pixel 7 (412 x 915, 2.625x scale)
  - Samsung Galaxy S23 (360 x 780, 3x scale)
  - iPad Mini 6th Gen (744 x 1133, 2x scale)
  - Custom user-defined dimensions with safety clamping
- **Device Hardware Frame Styling:** Realistic dark-mode device frames featuring Dynamic Island pills and Home Indicator bezels.
- **CDP Emulation Controls:** Live orientation toggling (Portrait <-> Landscape), zoom scaling (50% to 300%), and touch event simulation (`Emulation.setTouchEmulationEnabled`).

### 3.3 Fast Refresh & Developer Experience
- **Live Fast Refresh Bridge:** Real-time log scraping and socket health polling detecting bundle duration, recompilation triggers, and syntax errors.
- **Automatic Stale Session Detection:** Health monitor detects stalled or terminated dev servers and automatically triggers reconnection loops.
- **Zero Project Mutation:** Opens and previews Expo projects strictly in-place without adding extra files or altering `package.json`.

### 3.4 Developer Console & Diagnostics
- **Multi-Source Console:** Aggregates Metro stdout/stderr, WebView2 browser console logs (`console.log`, `warn`, `error`), and IOneXa system events.
- **Real-Time Log Filtering:** Severity filtering (`All`, `Debug`, `Info`, `Warn`, `Error`), case-insensitive live text search, and color-coded severity badges.
- **Log Export & Clipboard:** One-click JSON, HAR, and plain text log exports for team collaboration.

### 3.5 Network Inspector
- **Dual-Layer Traffic Capture:** Intercepts both native WebView2 web resources (`AddWebResourceRequestedFilter`) and DOM client `window.fetch` requests.
- **Security Redaction Engine:** Automatically masks sensitive headers (`Authorization`, `Cookie`, `Set-Cookie`, `X-API-Key`, tokens, secrets) in HAR and JSON exports.
- **HAR 1.2 Export:** Compliant with the HTTP Archive 1.2 specification for import into external API debuggers.

### 3.6 Expo Module & iOS Compatibility Analyzer
- **Embedded Module Registry:** Categorizes hundreds of Expo and React Native packages into `Supported`, `Partial`, `Unsupported`, and `Unverified`.
- **Platform-Specific File Scanner:** Recursively scans project source code for `*.ios.*`, `*.web.*`, and `*.native.*` extensions.
- **Overall Compatibility Score:** Computes weighted compatibility ratings and generates actionable migration recommendations.

### 3.7 Native iOS Verification Handoff Guide
- **Expo Go LAN / Tunnel Instructions:** One-click generation of terminal commands (`npx expo start --tunnel`) and QR-code pairing steps for physical iPhone testing without a Mac.
- **macOS Xcode Simulator Instructions:** Generates CocoaPods install and EAS build commands (`eas build --platform ios`).
- **Pre-Flight Signoff Checklist:** Formats interactive Markdown task lists (`- [ ]`) flagging all unsupported native modules as blocking release gates.

### 3.8 Security & Reliability Hardening
- **SecurityPolicy Gatekeeper:** Enforces strict loopback navigation origins, blocking dangerous schemes (`javascript:`, `file:`, `data:`, `ms-appx:`) and non-loopback external navigation.
- **Path Traversal Protection:** Validates canonical path boundaries, preventing access outside the project root.
- **Memory Bounded Logging:** Ring-buffer architecture capping log memory usage.

---

## 4. Verification & Quality Metrics

- **Unit & Regression Test Suites:** 19 out of 19 test suites passing (100% pass rate).
- **Automated Stress Testing:** 25 repeated supervisor start/stop cycles, 25 session lifecycle stress loops, and 3000 rapid log injections with zero memory leaks or dangling processes.
- **Real Application Pilot:** Verified across `expo-basic` (state, forms, assets) and `expo-router` (file-based navigation, parameter resolution, stack layout, persistence).

---

## 5. System Requirements & Installation

- **OS:** Windows 11 x64 or Windows 10 x64 (Version 1809+).
- **Runtime:** Microsoft Edge WebView2 Evergreen Runtime (pre-installed on Windows).
- **Toolchain:** Node.js v18.0.0+ with npm.
- **Installation:** Download `IOneXa-v0.1.0-windows-x64.zip`, extract to any folder, and double-click `ionexa.exe`. No installer or administrator privileges required.

---

## 6. Acknowledgments

Developed with care by **Usha Labs**.
Special thanks to the React Native and Expo communities, the Qt Project, and the Microsoft Edge WebView2 team.
