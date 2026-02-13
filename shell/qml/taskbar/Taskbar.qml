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
    color: Qt.rgba(0.15, 0.15, 0.15, 0.85)
    radius: 0

    RowLayout {
        anchors.fill: parent
        spacing: 0

        /* Left spacer */
        Item { Layout.fillWidth: true }

        /* Start button */
        TaskbarButton {
            iconSource: "image://theme/start-here-symbolic"
            tooltip: "Start"
            isStartButton: true
            onClicked: shellManager.toggleStartMenu()
        }

        /* Search button */
        TaskbarButton {
            iconSource: "image://theme/edit-find-symbolic"
            tooltip: "Search"
            onClicked: { /* TODO: open search overlay */ }
        }

        /* Virtual desktops button */
        TaskbarButton {
            iconSource: "image://theme/view-grid-symbolic"
            tooltip: "Task View"
            onClicked: { /* TODO: open desktop switcher */ }
        }

        /* Pinned and running app buttons (from taskbar model) */
        Repeater {
            model: TaskbarModel {}
            delegate: TaskbarButton {
                iconSource: "image://theme/" + model.iconName
                tooltip: model.title
                active: model.active
                onClicked: taskbarModel.activateWindow(index)
            }
        }

        /* Right spacer */
        Item { Layout.fillWidth: true }

        /* System tray */
        SystemTray {
            Layout.alignment: Qt.AlignRight
        }

        /* Clock */
        ClockWidget {
            Layout.alignment: Qt.AlignRight
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
