import QtQuick

Rectangle {
    id: btn
    width: 44
    height: 44
    radius: 4
    color: mouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.1) :
           active ? Qt.rgba(1, 1, 1, 0.05) : "transparent"

    property string iconSource: ""
    property string tooltip: ""
    property bool active: false
    property bool isStartButton: false

    signal clicked()

    Image {
        anchors.centerIn: parent
        width: 22
        height: 22
        source: btn.iconSource
        sourceSize: Qt.size(22, 22)
    }

    /* Active indicator (bottom line) */
    Rectangle {
        visible: btn.active
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: 16
        height: 3
        radius: 1.5
        color: "#60CDFF"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: btn.clicked()
    }

    ToolTip {
        visible: mouseArea.containsMouse && btn.tooltip.length > 0
        text: btn.tooltip
        delay: 500
    }
}
