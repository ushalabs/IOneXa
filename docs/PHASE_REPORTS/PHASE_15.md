# Phase 15 Report: Experimental Native React Native Runtime Research

- **Phase number / exact title:** Phase 15: "Experimental Native React Native Runtime Research"
- **Status:** PASS
- **Date:** 2026-10-28
- **Host OS:** Windows 11 x64 (MSVC 19.44 / CMake 3.31.6 / Ninja 1.12.1 / Qt 6.8.2)
- **Git Commit SHA:** Pending commit

---

## 1. What Was Implemented

1. **Architecture Decision Record (`docs/ADR/0002-native-windows-react-native-runtime.md`):**
   - Conducted and documented an exhaustive technical feasibility assessment of Microsoft's React Native for Windows (RNW) runtime.
   - Evaluated 6 key architectural dimensions:
     - **Project Invasiveness:** RNW violates IOneXa's core tenet of *Zero Project Mutation* by requiring `windows/` solution scaffolding and invasive project changes.
     - **Expo Ecosystem Support:** The vast majority of Expo modules (`expo-router`, `expo-camera`, `expo-av`, `expo-location`, `expo-notifications`, etc.) lack native C++/WinUI Windows implementations, causing immediate build/linking failures for real-world Expo projects.
     - **Build Overhead & Toolchain:** RNW incurs massive MSBuild compilation overhead (5 to 15+ minutes cold builds, requiring Windows 10/11 SDK, C++/WinRT, and WinUI 3) compared to instant (<1.5 second) Metro web bundling.
     - **Release Cadence & Version Lag:** RNW release cycles lag upstream React Native and Expo SDK versioning by several minor releases.
     - **Crash Isolation:** Native C++ WinUI exceptions crash the entire host process, whereas WebView2 and Metro operate in sandboxed, supervised out-of-process boundaries.
     - **Fidelity & Truth in Engineering:** RNW compiles to native Windows desktop controls (XAML `Button`, `ScrollView`), **NOT** Apple iOS UIKit controls. It offers no iOS fidelity advantages while adding severe fragility.

2. **Experimental Feature Flag Isolation (`src/core/app_settings.hpp` & `.cpp`):**
   - Integrated `experimentalNativeWindowsRuntime` boolean property in `AppSettings` (defaulting to `false`).
   - Ensured full JSON persistence and configuration round-tripping.
   - Preserved zero impact on default build dependencies and executable packaging.

3. **Automated Unit Tests (`tests/unit/test_app_settings.hpp`):**
   - Verified default disabled state of `experimentalNativeWindowsRuntime`.
   - Verified persistence, serialization, and deserialization round-trip.
   - All 18 unit test suites pass 100% under CTest.

---

## 2. Files Changed / Created

- `docs/ADR/0002-native-windows-react-native-runtime.md`
- `src/core/app_settings.hpp`
- `src/core/app_settings.cpp`
- `tests/unit/test_app_settings.hpp`
- `docs/PHASE_REPORTS/PHASE_15.md`

---

## 3. Verification & Test Evidence

```
Test project C:/Users/HP/Desktop/Projects/IOneXa/build
    Start 1: UnitTests
1/1 Test #1: UnitTests ........................   Passed   10.65 sec

100% tests passed, 0 tests failed out of 1
Total Test time (real) =  10.66 sec
```

Suites passed:
- `TestSessionState`: PASS
- `TestErrors`: PASS
- `TestCapabilityManifest`: PASS
- `TestStructuredLogger`: PASS
- `TestTaskCoordinator`: PASS
- `TestAppSettings`: PASS
- `TestProjectScanner`: PASS
- `TestMetroSupervisor`: PASS
- `TestWebView2Engine`: PASS
- `TestUiComponents`: PASS
- `TestFastRefresh`: PASS
- `TestViewportPresets`: PASS
- `TestDeveloperConsole`: PASS
- `TestNetworkInspector`: PASS
- `TestCompatibilityAnalyzer`: PASS
- `TestFeatureSuite`: PASS
- `TestRegressionSuite`: PASS
- `TestHardeningStress`: PASS

---

## 4. Truth in Engineering Audit

- ADR 0002 explicitly documents why React Native for Windows does not provide iOS emulation.
- Even if native Windows controls are rendered, they remain Windows XAML controls, not iOS UIKit elements.
- The persistent banner `"Expo Web Preview — Not an iOS Runtime"` remains the truthful product definition.
