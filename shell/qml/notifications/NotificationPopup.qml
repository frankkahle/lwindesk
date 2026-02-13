import QtQuick

Rectangle {
    width: parent ? parent.width : 350
    height: 72
    radius: 6
    color: notifMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(1, 1, 1, 0.04)

    Row {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Rectangle {
            width: 36; height: 36; radius: 4
            color: Qt.rgba(1, 1, 1, 0.1)
            anchors.verticalCenter: parent.verticalCenter
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2
            width: parent.width - 58

            Text {
                text: model.appName || "Application"
                color: Qt.rgba(1, 1, 1, 0.6)
                font.pixelSize: 11
                font.family: "Selawik"
            }
            Text {
                text: model.summary || "Notification"
                color: "white"
                font.pixelSize: 13
                font.family: "Selawik"
                font.bold: true
                elide: Text.ElideRight
                width: parent.width
            }
            Text {
                text: model.body || ""
                color: Qt.rgba(1, 1, 1, 0.7)
                font.pixelSize: 12
                font.family: "Selawik"
                elide: Text.ElideRight
                width: parent.width
            }
        }
    }

    MouseArea {
        id: notifMouse
        anchors.fill: parent
        hoverEnabled: true
    }
}
