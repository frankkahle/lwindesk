import QtQuick
import QtQuick.Layouts
import LWinDesk
import "../common"

LWPanel {
    width: 380
    height: 600
    color: Qt.rgba(0.16, 0.16, 0.16, 0.95)
    radius: 8

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "Notifications"
                color: "white"
                font.pixelSize: 16
                font.family: "Selawik"
                font.bold: true
            }
            Item { Layout.fillWidth: true }
            LWButton {
                text: "Clear all"
                onClicked: notifManager.clearAll()
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: NotificationManager { id: notifManager }
            delegate: NotificationPopup {}

            Text {
                visible: parent.count === 0
                anchors.centerIn: parent
                text: "No new notifications"
                color: Qt.rgba(1, 1, 1, 0.4)
                font.pixelSize: 13
                font.family: "Selawik"
            }
        }

        /* Calendar placeholder */
        Rectangle {
            Layout.fillWidth: true
            height: 200
            radius: 8
            color: Qt.rgba(1, 1, 1, 0.05)

            Text {
                anchors.centerIn: parent
                text: "Calendar"
                color: Qt.rgba(1, 1, 1, 0.3)
                font.pixelSize: 14
                font.family: "Selawik"
            }
        }
    }
}
