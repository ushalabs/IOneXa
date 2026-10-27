# ADR 0002: Technical Evaluation of React Native for Windows as an Auxiliary Native Execution Engine

- **Status:** Evaluated / Rejected for Primary MVP / Maintained as Experimental Flag
- **Date:** 2026-10-28
- **Author:** Lead Systems Architect & Core Runtime Team (IOneXa)
- **Target OS:** Windows 11 x64 (MSVC 19.44 / CMake 3.31.6 / Ninja 1.12.1 / Qt 6.8.2)

---

## 1. Context and Problem Statement

While IOneXa v0.1 successfully executes Expo and React Native applications via Expo Web (`react-native-web`) embedded inside Microsoft Edge WebView2, certain developers inquire whether Microsoft's **React Native for Windows (RNW)** (`microsoft/react-native-windows`) could serve as a native desktop execution runtime.

The theoretical premise is that RNW executes React Native JavaScript over native Windows UI frameworks (WinUI 3 / XAML / C++ WinRT) rather than the HTML DOM, potentially offering native thread segregation (UI thread vs JS thread) and native Windows controls.

This architectural decision record documents the in-depth technical evaluation of React Native for Windows, measures its viability against IOneXa's core design tenets, and specifies its placement within the product architecture.

---

## 2. Technical Evaluation & Comparison

| Evaluation Dimension | Primary Engine (Expo Web + Edge WebView2) | Experimental RN for Windows (UWP / WinUI 3) |
|---|---|---|
| **Zero Project Mutation** | **100% Zero-Touch.** Works out-of-the-box on standard Expo apps without adding files or modifying `package.json`. | **Invasive.** Requires `npx react-native-windows-init`, creating a `windows/` solution folder with `.sln`, `.vcxproj`, and native assets. |
| **Expo Ecosystem Compatibility** | **95%+ parity.** Expo official web support covers `expo-router`, `expo-image`, `@expo/vector-icons`, `expo-constants`, etc. | **< 20% parity.** The majority of Expo modules lack C++/WinUI implementations, causing build or runtime failures. |
| **Initial Build & Cold Start** | **< 2 seconds.** Metro web bundler serves pre-transpiled bundles instantly to WebView2. | **5 to 15 minutes.** Requires native MSBuild compilation of C++/WinRT translation units, Hermes, and WinUI assemblies. |
| **Version Alignment** | **Aligned.** Tied to Expo SDK version (e.g. Expo SDK 52 / React Native 0.76). | **Lagging / Desynchronized.** RNW releases frequently lag upstream React Native by several minor releases. |
| **Crash Isolation** | **Complete.** WebView2 and Metro run in isolated out-of-process sandboxes managed by Windows Job Objects. | **Process-Fatal.** An unhandled C++ exception in a native WinUI control crashes the host process. |
| **Truth in Engineering Disclosure** | Transparent: Emits web DOM, explicitly tagged *"Expo Web Preview — Not an iOS Runtime"*. | Windows controls (XAML `Button`, `ScrollView`), **NOT** Apple UIKit (`UIButton`, `UIScrollView`). |

---

## 3. Key Findings

### 3.1 Project Invasiveness Conflict
A non-negotiable architectural invariant of IOneXa is **Zero Project Mutation**: developers must be able to open any existing Expo project on Windows and begin previewing without generating unwanted native directories or modifying source dependencies. RNW cannot run an Expo app without generating a `windows/` native solution and injecting Windows-specific package entries.

### 3.2 Massive Expo Module Ecosystem Gap
Most real-world Expo applications depend on Expo SDK packages (`expo-router`, `expo-camera`, `expo-av`, `expo-location`, `expo-notifications`, `expo-secure-store`). While these packages feature web fallbacks via browser APIs (e.g. Web Audio, MediaDevices, Geolocation, Web Crypto), they possess **no native Windows C++ implementations**. Attempting to run a standard Expo project on RNW produces immediate module resolution errors for native code.

### 3.3 Heavy Toolchain Prerequisites
Running RNW requires:
- Visual Studio 2022 with Desktop development with C++
- Windows 10/11 SDK (10.0.19041.0 or newer)
- C++/WinRT Visual Studio Extension
- Windows App SDK / WinUI 3 runtimes

This imposes tens of gigabytes of disk overhead and prohibitive setup friction for developers who merely want a fast, lightweight mobile preview workbench.

### 3.4 Fidelity Reality
Even if an application were compiled with RNW, it would render **Windows XAML elements**, not Apple iOS UIKit elements. It does not reproduce iOS font rasterization, iOS kinetic scroll friction, UIKit navigation controllers, or iOS gesture recognizers. Therefore, RNW provides no fidelity advantage over WebView2 regarding iOS verification, while introducing immense brittleness.

---

## 4. Architectural Decision

1. **Rejection as Default Runtime:** React Native for Windows is **rejected** as the default or general-purpose execution engine for IOneXa.
2. **Preservation of Core MVP Track:** IOneXa continues with **Expo Web via Microsoft Edge WebView2 and Qt 6** as the sole production execution engine for v0.1.
3. **Experimental Flag Isolation:**
   - RNW support is isolated behind an experimental configuration setting:
     `AppSettings::experimentalNativeWindowsRuntime()` (default: `false`).
   - A dedicated capability flag `CapabilityStatus::Unsupported` is maintained in `CapabilityManifest` with explicit documentation for developers.
4. **Zero Impact on Delivery:** No build dependencies on C++/WinRT or WinUI 3 are introduced into the core IOneXa executable.

---

## 5. Truth in Engineering Summary

Neither Expo Web nor React Native for Windows constitutes an iOS simulator. React Native for Windows compiles JavaScript to Windows desktop controls, whereas IOneXa provides an immediate, zero-configuration Expo Web preview with device viewport framing, CDP diagnostics, and Fast Refresh. All documentation and user-facing UI will continue to reinforce:
`"Expo Web Preview — Not an iOS Runtime"`.
