import QtQuick
import QtQuick.Controls

Rectangle {
    id: btn
    width: 44
    height: 44
    radius: 4
    color: mouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.15) :
           active ? Qt.rgba(1, 1, 1, 0.08) : "transparent"

    property string iconSource: ""
    property string iconText: ""
    property string tooltip: ""
    property bool active: false
    property bool isStartButton: false

    signal clicked()

    /* Icon image (used when iconSource is set) */
    Image {
        id: iconImage
        anchors.centerIn: parent
        width: btn.isStartButton ? 24 : 20
        height: btn.isStartButton ? 24 : 20
        sourceSize: Qt.size(width, height)
        source: btn.iconSource
        visible: btn.iconSource.length > 0
        smooth: true
    }

    /* Fallback text icon (used when iconSource is empty) */
    Text {
        anchors.centerIn: parent
        text: btn.iconText
        font.pixelSize: isStartButton ? 20 : 16
        font.family: "Segoe UI Symbol"
        color: "white"
        visible: btn.iconSource.length === 0 && btn.iconText.length > 0
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
        cursorShape: Qt.PointingHandCursor
        onClicked: btn.clicked()
    }

    ToolTip {
        visible: mouseArea.containsMouse && btn.tooltip.length > 0
        text: btn.tooltip
        delay: 500
    }
}
