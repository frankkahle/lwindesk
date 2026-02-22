import QtQuick

Rectangle {
    height: 36
    radius: 18
    color: searchInput.activeFocus ? Qt.rgba(1, 1, 1, 0.12) :
           Qt.rgba(1, 1, 1, 0.08)
    border.color: searchInput.activeFocus ? "#60CDFF" :
                  Qt.rgba(1, 1, 1, 0.15)
    border.width: 1

    property alias text: searchInput.text
    signal searchChanged(string text)

    function clear() { searchInput.text = "" }
    function giveFocus() { searchInput.forceActiveFocus() }

    MouseArea {
        anchors.fill: parent
        onClicked: searchInput.forceActiveFocus()
    }

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
            clip: true
            selectByMouse: true
            onTextChanged: searchChanged(text)

            Text {
                visible: !searchInput.text && !searchInput.activeFocus
                text: "Type here to search"
                color: Qt.rgba(1, 1, 1, 0.4)
                font: searchInput.font
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
