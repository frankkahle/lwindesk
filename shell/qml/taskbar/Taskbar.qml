import QtQuick
import QtQuick.Layouts
import LWinDesk
import "../common"

/*
 * Windows 11-style taskbar - centered icons with inline search bar,
 * system tray on right, start button on left-center.
 */
LWPanel {
    id: taskbar
    height: 48
    color: Qt.rgba(0.12, 0.12, 0.12, 0.92)
    radius: 0

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 4
        spacing: 2

        /* Left spacer */
        Item { Layout.fillWidth: true }

        /* Start button */
        TaskbarButton {
            iconSource: "image://icon/view-app-grid-symbolic?color=white"
            tooltip: "Start"
            isStartButton: true
            onClicked: shellManager.toggleStartMenu()
        }

        /* Inline search bar (Windows 11 style) */
        Rectangle {
            id: searchBox
            Layout.preferredWidth: 220
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignVCenter
            radius: 16
            color: searchMouse.containsMouse || searchField.activeFocus ?
                Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(1, 1, 1, 0.06)
            border.color: searchField.activeFocus ? "#60CDFF" :
                          Qt.rgba(1, 1, 1, 0.1)
            border.width: 1

            Row {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 6

                Text {
                    text: "\uD83D\uDD0D"
                    font.pixelSize: 12
                    anchors.verticalCenter: parent.verticalCenter
                    color: Qt.rgba(1, 1, 1, 0.6)
                }

                TextInput {
                    id: searchField
                    width: parent.width - 30
                    anchors.verticalCenter: parent.verticalCenter
                    color: "white"
                    font.pixelSize: 12
                    font.family: "Selawik"
                    clip: true
                    selectByMouse: true

                    onTextChanged: {
                        shellManager.searchText = text
                        if (text.length > 0) {
                            shellManager.openSearch()
                        }
                    }

                    Text {
                        visible: !searchField.text && !searchField.activeFocus
                        text: "Search"
                        color: Qt.rgba(1, 1, 1, 0.35)
                        font: searchField.font
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            MouseArea {
                id: searchMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.IBeamCursor
                onClicked: {
                    searchField.forceActiveFocus()
                    shellManager.openSearch()
                }
                /* Let text input handle actual text editing */
                propagateComposedEvents: true
            }
        }

        /* Terminal button */
        TaskbarButton {
            iconSource: "image://icon/utilities-terminal-symbolic?color=white"
            tooltip: "Terminal"
            onClicked: shellManager.openTerminal()
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

    /* Sync: when start menu closes, clear the taskbar search field */
    Connections {
        target: shellManager
        function onStartMenuVisibleChanged() {
            if (!shellManager.startMenuVisible) {
                searchField.text = ""
                searchField.focus = false
            }
        }
    }
}
