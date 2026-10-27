cmake_minimum_required(VERSION 3.25)
project(IOneXa VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt Automoc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt 6 components
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Network Test)

# Core Library
add_library(ionexa_core STATIC
    src/core/session_state.hpp
    src/core/session_state.cpp
    src/core/errors.hpp
    src/core/capability_manifest.hpp
    src/core/structured_logger.hpp
    src/core/structured_logger.cpp
    src/core/task_coordinator.hpp
    src/core/task_coordinator.cpp
    src/core/app_settings.hpp
    src/core/app_settings.cpp
    src/engines/runtime_engine.hpp
    src/engines/webview2_widget.hpp
    src/engines/webview2_widget.cpp
    src/engines/fast_refresh_bridge.hpp
    src/engines/fast_refresh_bridge.cpp
    src/process/process_supervisor.hpp
    src/process/process_supervisor.cpp
    src/process/metro_supervisor.hpp
    src/process/metro_supervisor.cpp
    src/project/project_scanner.hpp
    src/project/project_scanner.cpp
    src/ui/nav_rail.hpp
    src/ui/nav_rail.cpp
    src/ui/inspector_panel.hpp
    src/ui/inspector_panel.cpp
    src/ui/viewport_manager.hpp
    src/ui/viewport_manager.cpp
)

target_include_directories(ionexa_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/webview2/include
)

target_link_libraries(ionexa_core PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    advapi32.lib
    user32.lib
    ole32.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/webview2/lib/x64/WebView2LoaderStatic.lib
)

# Main Application Executable
add_executable(ionexa WIN32
    src/ui/main_window.hpp
    src/ui/main_window.cpp
    src/app/main.cpp
)

target_link_libraries(ionexa PRIVATE
    ionexa_core
    Qt6::Widgets
    Qt6::Network
    user32.lib
    shell32.lib
    ole32.lib
    advapi32.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/webview2/lib/x64/WebView2LoaderStatic.lib
)

# Copy WebView2Loader.dll next to executable on Windows
add_custom_command(TARGET ionexa POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/webview2/bin/x64/WebView2Loader.dll"
    $<TARGET_FILE_DIR:ionexa>
)

# Automated Unit Tests
enable_testing()

add_executable(ionexa_unit_tests
    tests/unit/test_session_state.hpp
    tests/unit/test_errors.hpp
    tests/unit/test_capability_manifest.hpp
    tests/unit/test_structured_logger.hpp
    tests/unit/test_task_coordinator.hpp
    tests/unit/test_app_settings.hpp
    tests/unit/test_project_scanner.hpp
    tests/unit/test_metro_supervisor.hpp
    tests/unit/test_webview2_engine.hpp
    tests/unit/test_ui_components.hpp
    tests/unit/test_fast_refresh.hpp
    tests/unit/test_viewport_presets.hpp
    tests/unit/main.cpp
)

target_link_libraries(ionexa_unit_tests PRIVATE
    ionexa_core
    Qt6::Test
    Qt6::Widgets
)

target_compile_definitions(ionexa_unit_tests PRIVATE
    PROJECT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME UnitTests COMMAND ionexa_unit_tests)
set_tests_properties(UnitTests PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:C:/Qt/6.8.2/msvc2022_64/bin"
)
