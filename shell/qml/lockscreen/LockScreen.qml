import QtQuick

/* Windows 11-style lock screen */
Rectangle {
    anchors.fill: parent
    color: "#001B2E"

    Column {
        anchors.centerIn: parent
        spacing: 16

        Text {
            text: shellManager.currentTime
            color: "white"
            font.pixelSize: 96
            font.family: "Selawik"
            font.weight: Font.Light
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: shellManager.currentDate
            color: "white"
            font.pixelSize: 24
            font.family: "Selawik"
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 64
        text: "Click or press a key to unlock"
        color: Qt.rgba(1, 1, 1, 0.6)
        font.pixelSize: 14
        font.family: "Selawik"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            /* TODO: transition to login/password prompt */
        }
    }
}
