import QtQuick
import QtQuick.Layouts

Row {
    spacing: 2
    height: parent.height

    /* System tray icons using Unicode symbols */
    Repeater {
        model: [
            { icon: "\uD83D\uDD0A", tip: "Volume" },
            { icon: "\u2B06", tip: "Network" },
            { icon: "\uD83D\uDD0B", tip: "Battery" }
        ]
        delegate: Rectangle {
            width: 28
            height: 28
            radius: 4
            anchors.verticalCenter: parent.verticalCenter
            color: trayMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"

            Text {
                anchors.centerIn: parent
                text: modelData.icon
                font.pixelSize: 14
                color: Qt.rgba(1, 1, 1, 0.85)
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
