import QtQuick
import QtQuick.Window
import LWinDesk

/*
 * Main shell window - contains the taskbar, start menu, and overlays.
 * This will be displayed as a layer-shell surface anchored to the screen.
 */
Window {
    id: root
    visible: true
    width: Screen.width
    height: Screen.height
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    /* Taskbar at the bottom */
    Loader {
        id: taskbarLoader
        source: "taskbar/Taskbar.qml"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }

    /* Start Menu (centered above taskbar) */
    Loader {
        id: startMenuLoader
        active: shellManager.startMenuVisible
        source: "startmenu/StartMenu.qml"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: taskbarLoader.top
        anchors.bottomMargin: 12
    }

    /* Notification Center (right side panel) */
    Loader {
        id: notifLoader
        active: shellManager.notificationCenterVisible
        source: "notifications/NotificationCenter.qml"
        anchors.right: parent.right
        anchors.bottom: taskbarLoader.top
        anchors.bottomMargin: 12
        anchors.rightMargin: 12
    }

    /* Quick Settings panel */
    Loader {
        id: quickSettingsLoader
        active: shellManager.quickSettingsVisible
        source: "quicksettings/QuickSettings.qml"
        anchors.right: parent.right
        anchors.bottom: taskbarLoader.top
        anchors.bottomMargin: 12
        anchors.rightMargin: 12
    }

    /* Snap zone overlay (shown during window drag) */
    Loader {
        id: snapLoader
        source: "snapoverlay/SnapOverlay.qml"
        anchors.fill: parent
    }

    /* Click outside panels to dismiss */
    MouseArea {
        anchors.fill: parent
        z: -1
        onClicked: {
            shellManager.startMenuVisible = false
            shellManager.notificationCenterVisible = false
            shellManager.quickSettingsVisible = false
        }
    }
}
