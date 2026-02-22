import QtQuick

/*
 * ContextMenuSeparator - thin horizontal line between menu groups.
 */
Item {
    width: parent ? parent.width : 200
    height: 9  // 4px padding top + 1px line + 4px padding bottom

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - 16
        height: 1
        color: "#404040"
    }
}
