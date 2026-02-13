import QtQuick
import QtQuick.Layouts
import LWinDesk
import "../common"

/*
 * Windows 11-style taskbar - centered icons, system tray on right,
 * start button on left-center.
 */
LWPanel {
    id: taskbar
    height: 48
    color: Qt.rgba(0.12, 0.12, 0.12, 0.92)
    radius: 0

    RowLayout {
        anchors.fill: parent
        spacing: 2

        /* Left spacer */
        Item { Layout.fillWidth: true }

        /* Start button (Windows logo approximation) */
        TaskbarButton {
            iconText: "\u2756"
            tooltip: "Start"
            isStartButton: true
            onClicked: shellManager.toggleStartMenu()
        }

        /* Search button */
        TaskbarButton {
            iconText: "\uD83D\uDD0D"
            tooltip: "Search"
            onClicked: { /* TODO: open search overlay */ }
        }

        /* Virtual desktops button */
        TaskbarButton {
            iconText: "\u2B1A"
            tooltip: "Task View"
            onClicked: { /* TODO: open desktop switcher */ }
        }

        /* Right spacer */
        Item { Layout.fillWidth: true }

        /* System tray */
        SystemTray {
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }

        /* Separator */
        Rectangle {
            width: 1
            height: 24
            color: Qt.rgba(1, 1, 1, 0.15)
            Layout.alignment: Qt.AlignVCenter
        }

        /* Clock */
        ClockWidget {
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            onClicked: shellManager.notificationCenterVisible =
                !shellManager.notificationCenterVisible
        }

        /* Show desktop button (thin strip at far right) */
        Rectangle {
            width: 6
            Layout.fillHeight: true
            color: showDesktopMouse.containsMouse ?
                Qt.rgba(1, 1, 1, 0.2) : "transparent"

            MouseArea {
                id: showDesktopMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: shellManager.showDesktop()
            }
        }
    }
}
