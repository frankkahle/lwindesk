import QtQuick
import QtQuick.Window
import LWinDesk

/*
 * Main shell window - a 48px taskbar anchored to the bottom of the screen.
 * Start menu and panels open as child popups above the taskbar.
 */
Window {
    id: root
    visible: true
    width: Screen.width > 0 ? Screen.width : 1920
    height: 48
    x: 0
    y: (Screen.height > 0 ? Screen.height : 1080) - 48
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    /* Taskbar fills the window */
    Loader {
        id: taskbarLoader
        source: "taskbar/Taskbar.qml"
        anchors.fill: parent
    }

    /* Start Menu window (separate popup above taskbar) */
    Window {
        id: startMenuWindow
        visible: shellManager.startMenuVisible
        width: 640
        height: 720
        x: (root.width - 640) / 2
        y: root.y - 720 - 12
        color: "transparent"
        flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Popup

        Loader {
            active: shellManager.startMenuVisible
            source: "startmenu/StartMenu.qml"
            anchors.fill: parent
        }
    }

    /* Notification Center window */
    Window {
        id: notifWindow
        visible: shellManager.notificationCenterVisible
        width: 380
        height: 600
        x: root.width - 380 - 12
        y: root.y - 600 - 12
        color: "transparent"
        flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Popup

        Loader {
            active: shellManager.notificationCenterVisible
            source: "notifications/NotificationCenter.qml"
            anchors.fill: parent
        }
    }

    /* Quick Settings window */
    Window {
        id: quickSettingsWindow
        visible: shellManager.quickSettingsVisible
        width: 360
        height: 400
        x: root.width - 360 - 12
        y: root.y - 400 - 12
        color: "transparent"
        flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Popup

        Loader {
            active: shellManager.quickSettingsVisible
            source: "quicksettings/QuickSettings.qml"
            anchors.fill: parent
        }
    }
}
