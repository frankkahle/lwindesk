# LWinDesk

A Windows 11-like Wayland desktop environment for Linux.

LWinDesk is a from-scratch desktop environment built on **wlroots** (compositor) and **Qt6/QML** (shell UI), designed to give Linux users a familiar Windows 11 experience. It installs as a session option alongside GNOME/KDE on any Ubuntu system.

![Status](https://img.shields.io/badge/status-early%20development-orange)
![License](https://img.shields.io/badge/license-TBD-lightgrey)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Wayland-blue)

---

## Features

### Compositor (C / wlroots)
- Wayland compositor with scene-graph rendering
- XDG shell window management
- **Windows 11 snap layouts** — 6 zones (left, right, quadrants) + maximize with edge detection
- **Virtual desktops** with Super+1-9 switching
- Interactive window move with snap-on-drop
- Keyboard shortcuts matching Windows 11 (Super+D, Super+Arrow, Alt+F4)

### Shell (C++ / Qt6 / QML)
- **Taskbar** — 48px bottom bar with centered app icons, system tray, and clock
- **Start Menu** — Windows 11 "Eleven" layout with search, pinned apps grid, recommended section, and user profile
- **Notification Center** — slide-in panel with calendar
- **Quick Settings** — Wi-Fi, Bluetooth, volume, brightness toggles
- **Snap Overlay** — visual zone preview while dragging windows
- **Lock Screen** — time/date display with unlock prompt
- **Virtual Desktop Switcher** — Task View with desktop thumbnails
- **Widget Panel** — extensible widget sidebar
- **App Launcher** — scans system .desktop files with search filtering

### Session Integration
- Installs as a selectable session at the GDM login screen
- Packaged as a `.deb` for Ubuntu 24.04+
- Runs alongside GNOME — choose LWinDesk or GNOME at login

---

## Architecture

```
lwindesk/
├── compositor/          # Wayland compositor (C, wlroots 0.18)
│   ├── include/         # Headers (server, view, input, output, snap, workspace)
│   └── src/             # Implementation
├── shell/               # Desktop shell UI (C++, Qt6/QML)
│   ├── src/             # C++ backend (models, managers, IPC)
│   └── qml/             # QML frontend
│       ├── taskbar/     # Taskbar, buttons, system tray, clock
│       ├── startmenu/   # Start menu, app grid, search
│       ├── notifications/
│       ├── quicksettings/
│       ├── snapoverlay/
│       ├── lockscreen/
│       ├── virtualdesktops/
│       ├── widgets/
│       └── common/      # Shared components (LWPanel, LWButton)
├── session/             # Session files for display manager
├── packaging/           # Debian packaging
└── data/                # Icons, themes, wallpapers, sounds
```

---

## Building

### Prerequisites (Ubuntu 24.04)

```bash
sudo apt install -y \
  build-essential cmake pkg-config \
  libwlroots-dev libwayland-dev wayland-protocols \
  libxkbcommon-dev libinput-dev libdrm-dev libgbm-dev \
  libegl-dev libgles-dev libpixman-1-dev libseat-dev \
  libsystemd-dev libdbus-1-dev \
  qt6-base-dev qt6-declarative-dev qt6-wayland-dev qt6-svg-dev \
  qml6-module-qtquick qml6-module-qtquick-controls \
  qml6-module-qtquick-layouts qml6-module-qtquick-window \
  xwayland
```

### Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

### Run (nested, for testing)

```bash
./build/compositor/lwindesk-compositor -s ./build/shell/lwindesk-shell
```

### Install

```bash
sudo make install
# LWinDesk will appear as a session option at the GDM login screen
```

---

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Super` | Start Menu |
| `Super+D` | Show Desktop (minimize all) |
| `Super+Left/Right` | Snap window left/right |
| `Super+Up` | Maximize |
| `Super+Down` | Restore |
| `Super+Q` | Close window |
| `Alt+F4` | Close window |
| `Super+1-9` | Switch virtual desktop |

---

## Roadmap

- [x] Project scaffold and architecture
- [ ] Compositor builds and runs
- [ ] Shell renders taskbar with clock
- [ ] Start menu opens/closes with app grid
- [ ] Window snap zones functional
- [ ] Layer shell integration (taskbar anchored to screen)
- [ ] IPC between compositor and shell
- [ ] System tray (StatusNotifier D-Bus)
- [ ] Notification daemon (org.freedesktop.Notifications)
- [ ] XWayland support for legacy X11 apps
- [ ] .deb packaging
- [ ] Theming system
- [ ] Screen lock with PAM authentication
- [ ] Multi-monitor support
- [ ] Settings app

---

## Tech Stack

| Component | Technology |
|---|---|
| Compositor | C, wlroots 0.18, libwayland |
| Shell UI | C++17, Qt 6, QML |
| Rendering | wlr_scene (compositor), Qt Quick Scene Graph (shell) |
| Display Protocol | Wayland |
| Legacy App Support | XWayland |
| IPC | Wayland protocols + D-Bus |
| Packaging | .deb (dpkg/apt) |
| Target | Ubuntu 24.04 LTS |

---

## Contributing

LWinDesk is in early development. Contributions welcome.

```bash
# Clone
git clone git@github.com:frankkahle/lwindesk.git
cd lwindesk

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Test nested
./compositor/lwindesk-compositor -s ./shell/lwindesk-shell
```

---

## License

TBD
