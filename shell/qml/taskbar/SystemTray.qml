import QtQuick
import QtQuick.Layouts

Row {
    spacing: 4
    height: parent.height

    /* Placeholder tray icons */
    Repeater {
        model: ["network-wireless-symbolic",
                "audio-volume-medium-symbolic",
                "battery-good-symbolic"]
        delegate: Rectangle {
            width: 24
            height: 24
            anchors.verticalCenter: parent.verticalCenter
            color: "transparent"

            Image {
                anchors.centerIn: parent
                width: 16
                height: 16
                source: "image://theme/" + modelData
                sourceSize: Qt.size(16, 16)
                opacity: 0.8
            }
        }
    }
}
