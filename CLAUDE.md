# LWinDesk - Development Guide

## Project Overview
LWinDesk is a Windows 11-like Wayland desktop environment for Linux.

## Architecture
- **Compositor** (`compositor/`) - C, built on wlroots. Handles Wayland protocol, window management, input, snap zones, virtual desktops.
- **Shell** (`shell/`) - C++/Qt6/QML. The visual UI: taskbar, start menu, notifications, quick settings, snap overlay, lock screen.
- **Session** (`session/`) - Session files for GDM/display manager integration.
- **Packaging** (`packaging/`) - Debian packaging for .deb distribution.

## Build
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

## Test (nested in existing Wayland/X11 session)
```bash
./build/compositor/lwindesk-compositor -s ./build/shell/lwindesk-shell
```

## Key Design Decisions
- wlroots 0.17 for compositor (scene-graph API, Ubuntu 24.04)
- Qt6/QML for shell UI (GPU-accelerated, declarative)
- IPC between compositor and shell via Wayland protocols
- Layer shell for taskbar/overlays
- 48px bottom taskbar matching Windows 11

## Code Style
- C: Linux kernel style, 80-char lines
- C++/QML: Qt conventions
- Tabs for C indentation, spaces for C++/QML
