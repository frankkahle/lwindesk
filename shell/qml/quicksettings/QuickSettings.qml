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
                model: ListModel {
                    ListElement { name: "Wi-Fi"; icon: "network-wireless-symbolic"; on: true }
                    ListElement { name: "Bluetooth"; icon: "bluetooth-symbolic"; on: false }
                    ListElement { name: "Airplane"; icon: "airplane-mode-symbolic"; on: false }
                    ListElement { name: "Night Light"; icon: "night-light-symbolic"; on: false }
                    ListElement { name: "Focus"; icon: "user-busy-symbolic"; on: false }
                    ListElement { name: "Nearby"; icon: "send-to-symbolic"; on: false }
                }

                delegate: Rectangle {
                    Layout.fillWidth: true
                    height: 44
                    radius: 6
                    color: model.on ? "#0078D4" :
                           toggleMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.12) :
                           Qt.rgba(1, 1, 1, 0.08)

                    Row {
                        anchors.centerIn: parent
                        spacing: 6
                        Image {
                            width: 16; height: 16
                            source: "image://icon/" + model.icon + "?color=white"
                            sourceSize: Qt.size(16, 16)
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: model.name
                            color: "white"
                            font.pixelSize: 12
                            font.family: "Selawik"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        id: toggleMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: model.on = !model.on
                    }
                }
            }
        }

        /* Volume slider */
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Image {
                width: 16; height: 16
                source: "image://icon/audio-volume-medium-symbolic?color=white"
                sourceSize: Qt.size(16, 16)
            }
            Item {
                Layout.fillWidth: true
                height: 24
                Rectangle {
                    id: volTrack
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    height: 4
                    radius: 2
                    color: Qt.rgba(1, 1, 1, 0.15)
                    Rectangle {
                        id: volFill
                        width: parent.width * volSlider.value
                        height: parent.height
                        radius: 2
                        color: "#0078D4"
                    }
                }
                Rectangle {
                    id: volHandle
                    width: 14; height: 14; radius: 7
                    color: "white"
                    x: (parent.width - width) * volSlider.value
                    anchors.verticalCenter: parent.verticalCenter
                }
                MouseArea {
                    id: volSlider
                    anchors.fill: parent
                    property real value: 0.7
                    onPressed: function(mouse) { value = Math.max(0, Math.min(1, mouse.x / width)) }
                    onPositionChanged: function(mouse) {
                        if (pressed) value = Math.max(0, Math.min(1, mouse.x / width))
                    }
                }
            }
            Text {
                text: Math.round(volSlider.value * 100)
                color: Qt.rgba(1, 1, 1, 0.6)
                font.pixelSize: 11
                font.family: "Selawik"
            }
        }

        /* Brightness slider */
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Image {
                width: 16; height: 16
                source: "image://icon/display-brightness-symbolic?color=white"
                sourceSize: Qt.size(16, 16)
            }
            Item {
                Layout.fillWidth: true
                height: 24
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    height: 4
                    radius: 2
                    color: Qt.rgba(1, 1, 1, 0.15)
                    Rectangle {
                        width: parent.width * brightSlider.value
                        height: parent.height
                        radius: 2
                        color: "#0078D4"
                    }
                }
                Rectangle {
                    width: 14; height: 14; radius: 7
                    color: "white"
                    x: (parent.width - width) * brightSlider.value
                    anchors.verticalCenter: parent.verticalCenter
                }
                MouseArea {
                    id: brightSlider
                    anchors.fill: parent
                    property real value: 0.5
                    onPressed: function(mouse) { value = Math.max(0, Math.min(1, mouse.x / width)) }
                    onPositionChanged: function(mouse) {
                        if (pressed) value = Math.max(0, Math.min(1, mouse.x / width))
                    }
                }
            }
            Text {
                text: Math.round(brightSlider.value * 100)
                color: Qt.rgba(1, 1, 1, 0.6)
                font.pixelSize: 11
                font.family: "Selawik"
            }
        }

        Item { Layout.fillHeight: true }

        /* Bottom bar with battery and settings */
        RowLayout {
            Layout.fillWidth: true
            Row {
                spacing: 6
                Image {
                    width: 14; height: 14
                    source: "image://icon/battery-good-symbolic?color=white"
                    sourceSize: Qt.size(14, 14)
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: "72%"
                    color: Qt.rgba(1, 1, 1, 0.7)
                    font.pixelSize: 11
                    font.family: "Selawik"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                width: 28; height: 28; radius: 4
                color: settingsMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                Image {
                    anchors.centerIn: parent
                    width: 16; height: 16
                    source: "image://icon/emblem-system-symbolic?color=white"
                    sourceSize: Qt.size(16, 16)
                }
                MouseArea {
                    id: settingsMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        shellManager.launchApp("gnome-control-center")
                        shellManager.quickSettingsVisible = false
                    }
                }
            }
        }
    }
}
