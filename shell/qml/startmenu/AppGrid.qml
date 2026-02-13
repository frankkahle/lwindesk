import QtQuick
import LWinDesk

GridView {
    id: appGrid
    cellWidth: 96
    cellHeight: 96
    clip: true

    model: StartMenuModel {}

    delegate: Rectangle {
        width: 88
        height: 88
        radius: 4
        color: gridMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : "transparent"

        Column {
            anchors.centerIn: parent
            spacing: 6

            Image {
                width: 32; height: 32
                source: model.iconName ?
                    "image://theme/" + model.iconName : ""
                sourceSize: Qt.size(32, 32)
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: model.name
                color: "white"
                font.pixelSize: 11
                font.family: "Selawik"
                width: 80
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                maximumLineCount: 2
                wrapMode: Text.Wrap
            }
        }

        MouseArea {
            id: gridMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: shellManager.launchApp(model.exec)
        }
    }
}
