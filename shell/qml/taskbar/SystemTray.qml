import QtQuick
import QtQuick.Layouts

Row {
    spacing: 2
    height: parent.height

    /* System tray icons using freedesktop icon theme */
    Repeater {
        model: [
            { icon: "audio-volume-medium-symbolic", tip: "Volume" },
            { icon: "network-wireless-symbolic", tip: "Network" },
            { icon: "battery-good-symbolic", tip: "Battery" }
        ]
        delegate: Rectangle {
            width: 28
            height: 28
            radius: 4
            anchors.verticalCenter: parent.verticalCenter
            color: trayMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"

            Image {
                anchors.centerIn: parent
                width: 16
                height: 16
                sourceSize: Qt.size(16, 16)
                source: "image://icon/" + modelData.icon + "?color=white"
                smooth: true
            }

            MouseArea {
                id: trayMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: shellManager.quickSettingsVisible =
                    !shellManager.quickSettingsVisible
            }
        }
    }
}
