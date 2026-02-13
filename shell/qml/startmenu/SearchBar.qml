import QtQuick

Rectangle {
    height: 36
    radius: 18
    color: Qt.rgba(1, 1, 1, 0.08)
    border.color: Qt.rgba(1, 1, 1, 0.15)
    border.width: 1

    Row {
        anchors.fill: parent
        anchors.leftMargin: 12
        spacing: 8

        Text {
            text: "\uD83D\uDD0D"
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 14
        }

        TextInput {
            id: searchInput
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - 40
            color: "white"
            font.pixelSize: 13
            font.family: "Selawik"

            Text {
                visible: !searchInput.text
                text: "Type here to search"
                color: Qt.rgba(1, 1, 1, 0.4)
                font: searchInput.font
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
