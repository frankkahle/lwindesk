import QtQuick
import "../common"

/* Windows 11-style Widgets panel (left side slide-in) */
LWPanel {
    width: 420
    height: parent.height - 60
    color: Qt.rgba(0.96, 0.96, 0.96, 0.95)
    radius: 8

    /* TODO: Weather, news, calendar widgets */
    Text {
        anchors.centerIn: parent
        text: "Widgets"
        color: Qt.rgba(0, 0, 0, 0.3)
        font.pixelSize: 24
        font.family: "Selawik"
    }
}
