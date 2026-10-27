# ADR 0001: Runtime Execution Strategy & Decision Record

- **Status:** Accepted
- **Date:** 2026-10-27
- **Author:** Lead Systems Architect & Engineering Team (IOneXa)
- **Target OS:** Windows 11 x64 (Windows 10 x64 fallback where supported)

---

## 1. Context and Problem Statement

Developers building React Native and Expo applications on Windows frequently require a fast, interactive local preview without maintaining macOS hardware or launching heavy Android emulators for early-stage development and iteration. However, developer tools in this space often make misleading claims regarding "iOS emulation" on Windows.

IOneXa aims to be a **Windows-first mobile application development and testing workbench**, optimized for Expo projects and explicitly honest about fidelity.

We must select and validate the primary execution runtime engine for IOneXa v0.1 and establish strict truth-in-labeling boundaries.

---

## 2. Candidate Execution Engines Evaluated

### Engine A: Expo Web via Embedded Chromium Renderer (Recommended MVP Candidate)
- **Mechanism:** Execute the real Expo/React Native project source via Expo's official web bundler target (`react-native-web`, launched via `npx expo start --web`). The application is rendered inside an embedded desktop web runtime integrated into a Qt 6 desktop application.
- **Renderer Selection:**
  - *Qt WebEngine:* Evaluated. The installed Qt 6.8.2 MSVC toolchain on the target host does not include Qt WebEngine pre-compiled. Bundling Qt WebEngine requires substantial disk footprint (>250MB binaries) and complex dependency management.
  - *Microsoft Edge WebView2:* Evaluated. The Evergreen WebView2 Runtime is pre-installed across Windows 10 and Windows 11 (present on target host at `C:\Program Files (x86)\Microsoft\EdgeWebView\Application\*\msedgewebview2.exe`). WebView2 provides direct Chromium DevTools Protocol (CDP) access for console logging, network interception, script injection, device metrics emulation, and seamless HWND embedding inside a Qt 6 window.
- **Strengths:** Zero mutation of user project files; supports the standard Expo toolchain (`@expo/metro-runtime`, Expo Router, fast refresh); instant startup; low system footprint; robust crash isolation.
- **Limitations:** Browser-native differences from iOS; touch events and web APIs are shims; native iOS-only modules (`react-native-iap`, APNs, Apple Pay, Keychain, native UIKit components) do not execute.

### Engine B: React Native for Windows (Experimental / Secondary Track)
- **Mechanism:** Run React Native using Microsoft's native Windows platform integration (UWP / WinUI 3 / C++).
- **Strengths:** Native Windows OS rendering closer to RN architecture than Web DOM.
- **Limitations:** Highly invasive to generic Expo projects. Expo projects cannot simply run on RNW without native project modification (`npx react-native-windows-init`), version pinning (RNW versions lag or differ from Expo SDK React Native versions), and lack of Windows implementations for the majority of Expo ecosystem modules. Non-starter for generic zero-config import of existing Expo projects.
- **Decision:** Positioned strictly as an experimental Phase 15 research track behind a feature flag. Not viable for v0.1 MVP.

### Engine C: Remote / Native iOS Verification (Future Auxiliary Integration)
- **Mechanism:** Delegation to physical iOS devices, macOS-hosted iOS Simulators, or EAS cloud build pipelines.
- **Strengths:** True iOS fidelity.
- **Limitations:** Requires external hardware, Apple developer credentials, or network services. Cannot serve as the zero-setup, offline-capable local Windows preview.
- **Decision:** Reserved for optional Phase 16 handoff.

---

## 3. Weighted Decision Matrix

| Evaluation Criteria | Weight | Engine A (Expo Web + WebView2) | Engine B (RN for Windows) | Engine C (Remote iOS) |
|---|---|---|---|---|
| **Actual Runnable Evidence** | 35% | **9 / 10** (Proven runnable on target) | 3 / 10 (Requires heavy project porting) | N/A (Not local Windows) |
| **Compatible Expo Projects** | 20% | **9 / 10** (Standard Expo web target) | 2 / 10 (Almost no zero-config apps) | 10 / 10 (Real iOS) |
| **Setup & Maintenance Overhead** | 15% | **9 / 10** (Uses preinstalled WebView2 & Node) | 2 / 10 (Heavy C++/WinUI toolchain setup) | 3 / 10 (Mac hardware/cloud dependency) |
| **Debugging & Fast Refresh** | 15% | **9 / 10** (Metro Fast Refresh + CDP) | 4 / 10 (Complex native debugging) | 6 / 10 (Remote latency) |
| **Security & Packaging** | 10% | **9 / 10** (Chromium sandbox + Qt) | 5 / 10 (AppContainer / UWP signing) | 5 / 10 (Credential exposure risks) |
| **Resource Footprint** | 5% | **8 / 10** (Shared Edge runtime) | 6 / 10 (Native memory overhead) | 9 / 10 (Offloaded) |
| **Weighted Score** | 100% | **8.85 / 10** | **3.20 / 10** | **N/A** |

---

## 4. Architectural Decision

1. **Primary MVP Engine:** IOneXa v0.1 adopts **Engine A (Expo Web via Microsoft Edge WebView2 embedded in a Qt 6 desktop shell)**.
2. **Desktop Shell:** C++20 with **Qt 6.8.2** (Widgets shell, leveraging Windows HWND container for WebView2).
3. **Orchestration:** Supervised Node/npm/Expo process tree using Windows Job Objects for leak-free lifecycle and cancellation.
4. **Research Track:** Engine B (React Native for Windows) is isolated to Phase 15 as an experimental spike without impacting the MVP delivery path.

---

## 5. Explicit Truth-in-Labeling Rules

All UI screens, status bars, console logs, exported diagnostics, and documentation must adhere to the following principles:

1. **Persistent Badge:** The active preview canvas must display a clear, persistent tag:
   `"Expo Web Preview — Not an iOS Runtime"`.
2. **No Deceptive Marketing:** Never refer to IOneXa as an "iOS Simulator for Windows" or "iOS Emulator". It is an **Expo / React Native Developer Workbench on Windows**.
3. **Diagnostic Distinction:** Compatibility analysis must classify modules into `supported`, `partial`, `unsupported`, and `unverified`. Modules not verifiable on Windows (e.g. APNs, StoreKit, Apple Pay) must be explicitly flagged as `unverified — requires genuine iOS device/simulator`.
4. **No Mock Data / Static Stubs:** Previews must run the real application code. If a project fails to start, surface the actual error stack rather than falling back to an unrepresentative static mockup.
