cmake_minimum_required(VERSION 3.22)
project(lwindesk VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build test suite" OFF)

# Find dependencies
find_package(PkgConfig REQUIRED)

# Compositor dependencies (wlroots + Wayland)
pkg_check_modules(WLROOTS REQUIRED wlroots-0.18)
pkg_check_modules(WAYLAND_SERVER REQUIRED wayland-server)
pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
pkg_check_modules(XKBCOMMON REQUIRED xkbcommon)
pkg_check_modules(LIBINPUT REQUIRED libinput)
pkg_check_modules(PIXMAN REQUIRED pixman-1)
pkg_check_modules(DRM REQUIRED libdrm)
pkg_check_modules(GBM REQUIRED gbm)
pkg_check_modules(LIBSEAT REQUIRED libseat)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

# Shell dependencies (Qt6)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick WaylandClient Svg)
qt_standard_project_setup()

# Subdirectories
add_subdirectory(compositor)
add_subdirectory(shell)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install session files
install(FILES session/lwindesk.desktop
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/wayland-sessions)
install(FILES session/lwindesk-session
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
