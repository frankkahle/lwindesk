import QtQuick

Rectangle {
    id: btn
    width: label.width + 16
    height: 28
    radius: 4
    color: mouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"

    property alias text: label.text
    signal clicked()

    Text {
        id: label
        anchors.centerIn: parent
        color: Qt.rgba(1, 1, 1, 0.7)
        font.pixelSize: 12
        font.family: "Selawik"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: btn.clicked()
    }
}
