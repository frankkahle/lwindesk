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

            /* App icon placeholder with first letter */
            Rectangle {
                width: 36; height: 36
                radius: 8
                color: {
                    var colors = ["#0078D4", "#107C10", "#D83B01",
                                  "#8764B8", "#00B294", "#E81123",
                                  "#0099BC", "#767676"]
                    return colors[index % colors.length]
                }
                anchors.horizontalCenter: parent.horizontalCenter

                Text {
                    anchors.centerIn: parent
                    text: model.name ? model.name.charAt(0).toUpperCase() : "?"
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                }
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
            cursorShape: Qt.PointingHandCursor
            onClicked: shellManager.launchApp(model.exec)
        }
    }
}
