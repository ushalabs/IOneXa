# IOneXa Dependency Manifest & Licensing

- **Target OS:** Windows 11 x64 (Primary) / Windows 10 x64 (Supported)

---

## 1. Native Build Toolchain

| Dependency | Required Version | Role | License | Location on Host |
|---|---|---|---|---|
| **MSVC C++** | VS 2022 v143 (19.44+) | C++20 Compiler & Linker | Commercial / Community | `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools` |
| **CMake** | >= 3.25 (3.31.6 installed) | Build Configuration | BSD-3-Clause | Bundled in VS Build Tools |
| **Ninja** | >= 1.10 (1.12.1 installed) | High-Speed Build Tool | Apache-2.0 | Bundled in VS Build Tools |
| **Qt Framework** | 6.8.2 MSVC 2022 x64 | Desktop UI & Event Loop | LGPLv3 / Commercial | `C:\Qt\6.8.2\msvc2022_64` |
| **WebView2 SDK** | 1.0.4191.47 | Chromium Embedding API | Microsoft Software License | `vendor/webview2` |
| **WebView2 Runtime** | Evergreen (155.0+) | Chromium Browser Engine | Microsoft Pre-Installed OS | `C:\Program Files (x86)\Microsoft\EdgeWebView` |

---

## 2. Project Execution Toolchain

| Dependency | Required Version | Role | License |
|---|---|---|---|
| **Node.js** | v18+, v20+, or v24+ | JavaScript/Metro Runtime | MIT |
| **npm / yarn / pnpm** | Latest | Package Resolution | Artistic-2.0 / BSD / MIT |
| **Expo CLI** | SDK 50+ | Metro Dev Bundler | MIT |
| **react-native-web** | >= 0.19 | Shared React Component Rendering | MIT |

---

## 3. License Compliance

- **Qt 6:** IOneXa links dynamically against Qt 6 libraries in accordance with LGPLv3 terms. No Qt source code is modified.
- **WebView2:** Utilizes the official Microsoft.Web.WebView2 loader library according to Microsoft redistribution terms.
