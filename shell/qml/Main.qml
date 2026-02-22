import QtQuick
import QtQuick.Window
import LWinDesk

/*
 * Main shell window - a 48px taskbar anchored to the bottom of the screen.
 * Start menu and panels open as child windows above the taskbar.
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
    title: "lwindesk-taskbar"

    /* Taskbar fills the window */
    Loader {
        id: taskbarLoader
        source: "taskbar/Taskbar.qml"
        anchors.fill: parent
    }

    /* Start Menu window */
    Window {
        id: startMenuWindow
        visible: shellManager.startMenuVisible
        width: 640
        height: 720
        color: "transparent"
        flags: Qt.FramelessWindowHint
        title: "lwindesk-startmenu"

        Loader {
            id: startMenuLoader
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
        color: "transparent"
        flags: Qt.FramelessWindowHint
        title: "lwindesk-notifications"

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
        color: "transparent"
        flags: Qt.FramelessWindowHint
        title: "lwindesk-quicksettings"

        Loader {
            active: shellManager.quickSettingsVisible
            source: "quicksettings/QuickSettings.qml"
            anchors.fill: parent
        }
    }

    /* Desktop click surface - fullscreen transparent window behind all other
     * windows but above the wallpaper.  Catches right-click to show the
     * desktop context menu. */
    Window {
        id: desktopWindow
        visible: true
        width: Screen.width > 0 ? Screen.width : 1920
        height: (Screen.height > 0 ? Screen.height : 1080) - 48  // above taskbar
        color: "transparent"
        flags: Qt.FramelessWindowHint
        title: "lwindesk-desktop"

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton | Qt.LeftButton

            onClicked: function(mouse) {
                if (!desktopMenuLoader.item) return;
                if (mouse.button === Qt.RightButton) {
                    desktopMenuLoader.item.menuX = mouse.x;
                    desktopMenuLoader.item.menuY = mouse.y;
                    desktopMenuLoader.item.menuVisible = true;
                } else {
                    desktopMenuLoader.item.menuVisible = false;
                }
            }
        }

        Loader {
            id: desktopMenuLoader
            anchors.fill: parent
            source: "desktop/DesktopContextMenu.qml"
        }
    }
}
