import QtQuick
import QtQuick.Layouts
import LWinDesk
import "../common"

/*
 * Windows 11-style Start Menu - centered, rounded corners,
 * search bar at top, pinned apps grid, recommended/recent section.
 */
LWPanel {
    id: startMenu
    width: 640
    height: 720
    color: Qt.rgba(0.16, 0.16, 0.16, 0.95)
    radius: 8

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        /* Search bar */
        SearchBar {
            Layout.fillWidth: true
        }

        /* "Pinned" section header */
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "Pinned"
                color: "white"
                font.pixelSize: 14
                font.family: "Selawik"
                font.bold: true
            }
            Item { Layout.fillWidth: true }
            LWButton {
                text: "All apps >"
                onClicked: { /* TODO: show full app list */ }
            }
        }

        /* Pinned app grid */
        AppGrid {
            Layout.fillWidth: true
            Layout.preferredHeight: 280
        }

        /* Divider */
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Qt.rgba(1, 1, 1, 0.1)
        }

        /* "Recommended" section */
        Text {
            text: "Recommended"
            color: "white"
            font.pixelSize: 14
            font.family: "Selawik"
            font.bold: true
        }

        /* Recent items list */
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: 6
            delegate: Rectangle {
                width: parent ? parent.width : 0
                height: 48
                color: recMouseArea.containsMouse ?
                    Qt.rgba(1, 1, 1, 0.05) : "transparent"
                radius: 4

                Row {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 12
                    Rectangle {
                        width: 32; height: 32
                        radius: 4
                        color: Qt.rgba(1, 1, 1, 0.1)
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        Text {
                            text: "Recent Item " + (index + 1)
                            color: "white"
                            font.pixelSize: 13
                            font.family: "Selawik"
                        }
                        Text {
                            text: "Recently opened"
                            color: Qt.rgba(1, 1, 1, 0.5)
                            font.pixelSize: 11
                            font.family: "Selawik"
                        }
                    }
                }
                MouseArea {
                    id: recMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                }
            }
        }

        /* Bottom bar: user profile + power */
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            /* User profile */
            Row {
                spacing: 8
                Rectangle {
                    width: 32; height: 32; radius: 16
                    color: "#0078D4"
                    Text {
                        anchors.centerIn: parent
                        text: "F"
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                    }
                }
                Text {
                    text: "Frank"
                    color: "white"
                    font.pixelSize: 13
                    font.family: "Selawik"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Item { Layout.fillWidth: true }

            /* Power button */
            LWButton {
                text: "\u23FB"
                onClicked: { /* TODO: power menu */ }
            }
        }
    }
}
