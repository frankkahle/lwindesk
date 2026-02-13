import QtQuick

Rectangle {
    id: clockWidget
    width: clockColumn.width + 24
    height: parent.height
    color: mouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
    radius: 4

    signal clicked()

    Column {
        id: clockColumn
        anchors.centerIn: parent
        spacing: 0

        Text {
            text: shellManager.currentTime
            color: "white"
            font.pixelSize: 12
            font.family: "Selawik"
            horizontalAlignment: Text.AlignRight
            anchors.right: parent.right
        }

        Text {
            text: shellManager.currentDate
            color: "white"
            font.pixelSize: 11
            font.family: "Selawik"
            horizontalAlignment: Text.AlignRight
            anchors.right: parent.right
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: clockWidget.clicked()
    }
}
