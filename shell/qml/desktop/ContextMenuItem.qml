import QtQuick

/*
 * ContextMenuItem - a single row in the desktop context menu.
 * Windows 11 styling: Selawik font, subtle hover, 32px height.
 */
Rectangle {
    id: itemRoot
    width: parent ? parent.width : 200
    height: visible ? 32 : 0
    color: itemMouse.containsMouse ? "#404040" : "transparent"
    radius: 4

    /* Left/right padding inside the menu body */
    anchors.leftMargin: 4
    anchors.rightMargin: 4

    property alias text: label.text
    property bool hasSubmenu: false

    signal clicked()

    Text {
        id: label
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 12
        font.family: "Selawik"
        font.pixelSize: 13
        color: "#e0e0e0"
    }

    /* Submenu arrow indicator */
    Text {
        visible: itemRoot.hasSubmenu
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 12
        text: "\u25B8"  // right-pointing small triangle
        font.pixelSize: 11
        color: "#a0a0a0"
    }

    MouseArea {
        id: itemMouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: itemRoot.clicked()
    }
}
