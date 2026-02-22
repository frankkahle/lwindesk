import QtQuick
import LWinDesk

GridView {
    id: appGrid
    cellWidth: 96
    cellHeight: 96
    clip: true

    model: StartMenuModel {}

    delegate: Rectangle {
        id: gridDelegate
        required property int index
        required property string name
        required property string exec
        required property string iconName
        width: 88
        height: 88
        radius: 4
        color: gridMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : "transparent"

        Column {
            anchors.centerIn: parent
            spacing: 6

            /* App icon (with letter fallback) */
            Item {
                width: 36; height: 36
                anchors.horizontalCenter: parent.horizontalCenter

                Image {
                    id: gridIconImg
                    anchors.fill: parent
                    sourceSize: Qt.size(36, 36)
                    source: gridDelegate.iconName ? "image://icon/" + gridDelegate.iconName : ""
                    smooth: true
                    visible: status === Image.Ready
                }

                /* Fallback: colored rectangle with first letter */
                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    visible: gridIconImg.status !== Image.Ready
                    color: {
                        var colors = ["#0078D4", "#107C10", "#D83B01",
                                      "#8764B8", "#00B294", "#E81123",
                                      "#0099BC", "#767676"]
                        return colors[gridDelegate.index % colors.length]
                    }
                    Text {
                        anchors.centerIn: parent
                        text: gridDelegate.name ? gridDelegate.name.charAt(0).toUpperCase() : "?"
                        color: "white"
                        font.pixelSize: 18
                        font.bold: true
                    }
                }
            }

            Text {
                text: gridDelegate.name
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
            onClicked: shellManager.launchApp(gridDelegate.exec)
        }
    }
}
