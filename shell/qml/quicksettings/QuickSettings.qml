import QtQuick
import QtQuick.Layouts
import "../common"

LWPanel {
    width: 380
    height: 400
    color: Qt.rgba(0.16, 0.16, 0.16, 0.95)
    radius: 8

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        /* Quick action buttons grid */
        GridLayout {
            Layout.fillWidth: true
            columns: 3
            rowSpacing: 8
            columnSpacing: 8

            Repeater {
                model: [
                    { name: "Wi-Fi", icon: "network-wireless-symbolic", on: true },
                    { name: "Bluetooth", icon: "bluetooth-symbolic", on: false },
                    { name: "Airplane", icon: "airplane-mode-symbolic", on: false },
                    { name: "Night Light", icon: "night-light-symbolic", on: false },
                    { name: "Focus", icon: "user-busy-symbolic", on: false },
                    { name: "Nearby", icon: "send-to-symbolic", on: false }
                ]

                delegate: Rectangle {
                    Layout.fillWidth: true
                    height: 44
                    radius: 6
                    color: modelData.on ? "#0078D4" : Qt.rgba(1, 1, 1, 0.08)

                    Row {
                        anchors.centerIn: parent
                        spacing: 6
                        Image {
                            width: 16; height: 16
                            source: "image://theme/" + modelData.icon
                            sourceSize: Qt.size(16, 16)
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: modelData.name
                            color: "white"
                            font.pixelSize: 12
                            font.family: "Selawik"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }
        }

        /* Volume slider */
        RowLayout {
            Layout.fillWidth: true
            Image {
                width: 16; height: 16
                source: "image://theme/audio-volume-medium-symbolic"
                sourceSize: Qt.size(16, 16)
            }
            Rectangle {
                Layout.fillWidth: true
                height: 4
                radius: 2
                color: Qt.rgba(1, 1, 1, 0.15)
                Rectangle {
                    width: parent.width * 0.7
                    height: parent.height
                    radius: 2
                    color: "#0078D4"
                }
            }
        }

        /* Brightness slider */
        RowLayout {
            Layout.fillWidth: true
            Image {
                width: 16; height: 16
                source: "image://theme/display-brightness-symbolic"
                sourceSize: Qt.size(16, 16)
            }
            Rectangle {
                Layout.fillWidth: true
                height: 4
                radius: 2
                color: Qt.rgba(1, 1, 1, 0.15)
                Rectangle {
                    width: parent.width * 0.5
                    height: parent.height
                    radius: 2
                    color: "#0078D4"
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
