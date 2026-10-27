# IOneXa System Architecture

- **Product:** IOneXa Developer Workbench
- **Target OS:** Windows 11 x64 / Windows 10 x64
- **Language / Standards:** C++20, ISO C++ standard library, Qt 6.8+

---

## 1. High-Level Architecture

IOneXa is composed of decoupled modular layers designed for high responsiveness, memory efficiency, and process safety:

```
+-------------------------------------------------------------------------+
|                              IOneXa UI                                  |
|   (Qt 6 Widgets: Dark Design System, Device Canvas, Inspector, Console) |
+------------------------------------+------------------------------------+
                                     |
+------------------------------------+------------------------------------+
|                         Session Coordinator                             |
|          (Finite State Machine: IDLE -> READY -> RUNNING -> STOPPED)    |
+-------------------+--------------------+--------------------+-------+
                    |                    |                    |
+-------------------+---+  +-------------+------+  +----------+-----------+
|    Project Scanner    |  |  Process Supervisor|  |  Runtime Engine Host |
| (Read-Only Inspection)|  | (Win32 Job Objects)|  | (WebView2 Container) |
+-----------------------+  +--------------------+  +----------------------+
```

---

## 2. Layer Responsibilities

### 2.1 Core & Session Lifecycle (`src/core/`)
- Manages strict session states: `Idle`, `Inspecting`, `Ready`, `Starting`, `Running`, `Stopping`, `Stopped`, and `Error`.
- Emits structured state transition events without blocking the main GUI thread.
- Enforces typed errors (`SessionError`, `ProcessError`, `ProjectError`).

### 2.2 Project Discovery & Inspection (`src/project/`)
- Scans user directories in **read-only mode**. Never modifies imported user project files.
- Detects package managers (`npm`, `yarn`, `pnpm`, `bun`), Expo SDK versions, and web support packages (`react-native-web`, `react-dom`, `@expo/metro-runtime`).
- Generates a `CapabilityManifest` categorizing modules into `Supported`, `Partial`, `Unsupported`, and `Unverified`.

### 2.3 Process Supervisor (`src/process/`)
- Launches Metro/Expo processes using direct arguments without shell string interpolation.
- Assigns child processes to a dedicated Windows Job Object configured with `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`.
- Guarantees termination of all descendant Node/npm child processes upon application stop or crash.
- Streams stdout and stderr line-by-line with millisecond-precision timestamps.

### 2.4 Runtime Engine Host (`src/engines/`)
- Interfaces with Microsoft Edge WebView2 Evergreen runtime embedded into a Qt `QWidget` via native Win32 `HWND`.
- Communicates with Chromium DevTools Protocol (CDP) for browser console capture, network inspection, and device metric emulation.
- Renders real Expo Web applications with live Fast Refresh support.

### 2.5 User Interface (`src/ui/`)
- Dark-themed ergonomic desktop workspace matching professional developer workflows.
- Viewport canvas with mobile aspect ratios (iPhone 14/15, Android, Tablet), orientation toggle, and zoom.
- Persistent label: `"Expo Web Preview — Not an iOS Runtime"`.
