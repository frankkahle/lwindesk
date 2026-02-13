qt_add_executable(lwindesk-shell
    src/main.cpp
    src/shellmanager.cpp
    src/taskbarmodel.cpp
    src/startmenumodel.cpp
    src/notificationmanager.cpp
    src/systemtraymanager.cpp
)

qt_add_qml_module(lwindesk-shell
    URI LWinDesk
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/taskbar/Taskbar.qml
        qml/taskbar/TaskbarButton.qml
        qml/taskbar/SystemTray.qml
        qml/taskbar/ClockWidget.qml
        qml/startmenu/StartMenu.qml
        qml/startmenu/AppGrid.qml
        qml/startmenu/SearchBar.qml
        qml/notifications/NotificationCenter.qml
        qml/notifications/NotificationPopup.qml
        qml/quicksettings/QuickSettings.qml
        qml/search/SearchOverlay.qml
        qml/snapoverlay/SnapOverlay.qml
        qml/lockscreen/LockScreen.qml
        qml/virtualdesktops/DesktopSwitcher.qml
        qml/common/LWButton.qml
        qml/common/LWPanel.qml
)

target_link_libraries(lwindesk-shell PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::WaylandClient
    Qt6::Svg
)

install(TARGETS lwindesk-shell DESTINATION bin)
