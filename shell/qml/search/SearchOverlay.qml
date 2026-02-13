import QtQuick
import "../common"

/* Full-screen search overlay - Windows 11 style */
LWPanel {
    width: 640
    height: 500
    color: Qt.rgba(0.16, 0.16, 0.16, 0.95)
    radius: 8

    /* TODO: Implement full search with app results, web results, file results */
    Text {
        anchors.centerIn: parent
        text: "Search"
        color: Qt.rgba(1, 1, 1, 0.3)
        font.pixelSize: 24
        font.family: "Selawik"
    }
}
