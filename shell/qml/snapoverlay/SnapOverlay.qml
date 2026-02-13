import QtQuick

/* Snap zone visual overlay - shows zone highlights while dragging windows */
Item {
    id: snapOverlay
    visible: false  /* Made visible by compositor IPC during window drag */

    /* Semi-transparent zone previews */
    Repeater {
        model: [
            { x: 0, y: 0, w: 0.5, h: 1.0, label: "Left" },
            { x: 0.5, y: 0, w: 0.5, h: 1.0, label: "Right" },
            { x: 0, y: 0, w: 0.5, h: 0.5, label: "Top Left" },
            { x: 0.5, y: 0, w: 0.5, h: 0.5, label: "Top Right" },
            { x: 0, y: 0.5, w: 0.5, h: 0.5, label: "Bottom Left" },
            { x: 0.5, y: 0.5, w: 0.5, h: 0.5, label: "Bottom Right" }
        ]

        delegate: Rectangle {
            x: modelData.x * snapOverlay.width
            y: modelData.y * snapOverlay.height
            width: modelData.w * snapOverlay.width
            height: modelData.h * snapOverlay.height
            color: "transparent"
            border.color: Qt.rgba(0, 0.47, 0.83, 0.5)
            border.width: 2
            radius: 8
            visible: snapOverlay.visible

            Rectangle {
                anchors.fill: parent
                anchors.margins: 4
                radius: 6
                color: Qt.rgba(0, 0.47, 0.83, 0.15)
            }
        }
    }
}
