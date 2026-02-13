import QtQuick
import QtQuick.Layouts
import "../common"

/* Windows 11 Task View - virtual desktop switcher */
LWPanel {
    width: parent.width * 0.8
    height: parent.height * 0.8
    color: Qt.rgba(0.1, 0.1, 0.1, 0.9)
    radius: 8
    anchors.centerIn: parent

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32

        /* Desktop thumbnails at top */
        Row {
            Layout.alignment: Qt.AlignHCenter
            spacing: 16

            Repeater {
                model: 3
                delegate: Rectangle {
                    width: 200
                    height: 120
                    radius: 8
                    color: index === shellManager.activeWorkspace ?
                        Qt.rgba(0, 0.47, 0.83, 0.3) :
                        Qt.rgba(1, 1, 1, 0.08)
                    border.color: index === shellManager.activeWorkspace ?
                        "#0078D4" : "transparent"
                    border.width: 2

                    Text {
                        anchors.centerIn: parent
                        text: "Desktop " + (index + 1)
                        color: "white"
                        font.pixelSize: 13
                        font.family: "Selawik"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: shellManager.switchWorkspace(index)
                    }
                }
            }

            /* New desktop button */
            Rectangle {
                width: 200
                height: 120
                radius: 8
                color: Qt.rgba(1, 1, 1, 0.05)
                border.color: Qt.rgba(1, 1, 1, 0.2)
                border.width: 1
                border.style: "dashed"

                Text {
                    anchors.centerIn: parent
                    text: "+ New desktop"
                    color: Qt.rgba(1, 1, 1, 0.5)
                    font.pixelSize: 13
                    font.family: "Selawik"
                }
            }
        }

        /* Window thumbnails area */
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Text {
                anchors.centerIn: parent
                text: "Window thumbnails will appear here"
                color: Qt.rgba(1, 1, 1, 0.2)
                font.pixelSize: 16
                font.family: "Selawik"
            }
        }
    }
}
