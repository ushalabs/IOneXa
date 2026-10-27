# IOneXa — GUI Redesign & Modernization Progress Tracker

**Android Studio–Inspired Windows Development Environment**  
**Publisher:** Usha Labs  
**Date Started:** October 28, 2026  
**Status:** In Progress (GUI Phase 1)

---

## 1. Executive Summary & Design Goals

The IOneXa GUI Overhaul separates the graphical interface into two decoupled, professional windows:
1. **Window A: IOneXa Workbench:** Project management, run configurations, Metro process orchestration, multi-source console, network inspection, compatibility analysis, and device management.
2. **Window B: Standalone Device Emulator Window:** A dedicated, native top-level Windows window with calibrated mobile viewport framing, device hardware chrome (Dynamic Island, Home Indicator), and adjacent mobile controls (Power, Reload, Rotate, Zoom, Fit, Screenshot, Always on Top, Dock/Undock).

---

## 2. Phase Execution Tracker

| Phase | Title | Status | Commit SHA | Verification Highlights |
|---|---|---|---|---|
| **GUI Phase 1** | Architecture Audit and Modern Design System | **Completed** | *Current* | Decoupled window architecture, design tokens (`theme.hpp/cpp`), README banner & branding |
| **GUI Phase 2** | Standalone Device Emulator Window | Planned | *Pending* | Native top-level `DeviceEmulatorWindow`, aspect ratio preservation, scaling, window persistence |
| **GUI Phase 3** | Modern Development Workbench | Planned | *Pending* | Modular workbench, Android Studio–inspired command bar, navigation rail, central views |
| **GUI Phase 4** | Device Manager and Emulator Controls | Planned | *Pending* | Device Manager panel, custom profiles, adjacent emulator toolbar, screenshots |
| **GUI Phase 5** | Debugging Workspace and Session Inspector | Planned | *Pending* | Collapsible bottom tools dock, optional right-side inspector, honest status segregation |
| **GUI Phase 6** | UX Polish and Window Reliability | Planned | *Pending* | High-DPI multi-monitor resilience, keyboard shortcuts, error overlays, graceful window lifecycle |
| **GUI Phase 7** | Regression Testing and GUI Release Candidate | Planned | *Pending* | 19+ unit test suites, new GUI tests, real application acceptance test on `DeskFlow` |

---

## 3. Visual Design System Tokens

- **Background (Main):** `#16181D`
- **Secondary Surfaces:** `#1E222A`
- **Raised Surfaces / Cards:** `#262B35`
- **Borders & Dividers:** `#343B48`
- **Subtle Borders:** `#2A303C`
- **Primary Accent:** `#4E9CFF` (Hover: `#68ABFF`, Pressed: `#3584E4`)
- **Success / Healthy:** `#43C990`
- **Warning / Warning:** `#F5B84D`
- **Error / Critical:** `#F06B72`
- **Primary Text:** `#F2F4F8`
- **Secondary Text:** `#A4ADBD`
- **Muted Text:** `#6B7688`
- **Typography:** Segoe UI Variable / Segoe UI (UI), Cascadia Code / Consolas (Code & Logs)

---

## 4. Truth in Engineering Invariant

All windows, status indicators, and documentation will persistently state:
`"Expo Web Preview — Not an iOS Runtime"`.
IOneXa executes Expo applications via `@expo/metro-runtime` and `react-native-web` inside Edge WebView2.
