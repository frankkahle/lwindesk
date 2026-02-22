import QtQuick

/*
 * DesktopContextMenu - Windows 11-style right-click context menu for the
 * desktop background.  Uses a plain Rectangle instead of Qt Quick Controls
 * Popup/Menu to avoid Wayland positioning issues on Qt 6.4.
 */
Item {
    id: ctxRoot

    /* The menu is shown at (menuX, menuY) relative to the parent. */
    property real menuX: 0
    property real menuY: 0
    property bool menuVisible: false

    /* Dismiss the menu when the user clicks anywhere outside. */
    signal dismissed()

    /* ---- background dismiss area ---- */
    MouseArea {
        anchors.fill: parent
        visible: ctxRoot.menuVisible
        onClicked: {
            ctxRoot.menuVisible = false;
            ctxRoot.dismissed();
        }
        /* Allow right-click through so the menu can be re-opened at a
           different position without an extra click. */
        acceptedButtons: Qt.LeftButton | Qt.RightButton
    }

    /* ---- the menu rectangle ---- */
    Rectangle {
        id: menuRect
        visible: ctxRoot.menuVisible
        x: ctxRoot.menuX
        y: ctxRoot.menuY
        width: 200
        height: menuColumn.implicitHeight + 16  // 8px top + 8px bottom padding
        radius: 8
        color: "#2c2c2c"
        border.color: "#404040"
        border.width: 1

        /* Subtle drop shadow via a second rect behind */
        Rectangle {
            z: -1
            anchors.fill: parent
            anchors.margins: -1
            radius: parent.radius + 1
            color: "#80000000"
        }

        Column {
            id: menuColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.bottomMargin: 8

            /* ---------- View ---------- */
            ContextMenuItem {
                text: "View"
                hasSubmenu: true
                onClicked: { /* submenu later */ }
            }

            /* ---------- Sort by ---------- */
            ContextMenuItem {
                text: "Sort by"
                hasSubmenu: true
                onClicked: { /* submenu later */ }
            }

            /* ---------- separator ---------- */
            ContextMenuSeparator {}

            /* ---------- New  (with submenu) ---------- */
            ContextMenuItem {
                id: newItem
                text: "New"
                hasSubmenu: true
                property bool subOpen: false
                onClicked: {
                    subOpen = !subOpen;
                }
            }

            /* Inline sub-menu items shown when "New" is expanded */
            ContextMenuItem {
                visible: newItem.subOpen
                text: "    Folder"
                onClicked: {
                    shellManager.launchApp("mkdir -p ~/Desktop/NewFolder");
                    ctxRoot.menuVisible = false;
                    ctxRoot.dismissed();
                }
            }
            ContextMenuItem {
                visible: newItem.subOpen
                text: "    Text Document"
                onClicked: {
                    shellManager.launchApp("touch ~/Desktop/NewDocument.txt");
                    ctxRoot.menuVisible = false;
                    ctxRoot.dismissed();
                }
            }

            /* ---------- separator ---------- */
            ContextMenuSeparator {}

            /* ---------- Open in Terminal ---------- */
            ContextMenuItem {
                text: "Open in Terminal"
                onClicked: {
                    shellManager.openTerminal();
                    ctxRoot.menuVisible = false;
                    ctxRoot.dismissed();
                }
            }

            /* ---------- Display settings ---------- */
            ContextMenuItem {
                text: "Display settings"
                onClicked: {
                    shellManager.launchApp("gnome-control-center display");
                    ctxRoot.menuVisible = false;
                    ctxRoot.dismissed();
                }
            }

            /* ---------- Personalize ---------- */
            ContextMenuItem {
                text: "Personalize"
                onClicked: {
                    shellManager.launchApp("gnome-control-center background");
                    ctxRoot.menuVisible = false;
                    ctxRoot.dismissed();
                }
            }

            /* ---------- separator ---------- */
            ContextMenuSeparator {}

            /* ---------- Refresh ---------- */
            ContextMenuItem {
                text: "Refresh"
                onClicked: {
                    /* Force a visual refresh of the desktop */
                    ctxRoot.menuVisible = false;
                    ctxRoot.dismissed();
                }
            }
        }
    }

    /* Clamp menu position so it doesn't overflow the window edges. */
    onMenuXChanged: {
        if (menuRect.visible) clampPosition();
    }
    onMenuYChanged: {
        if (menuRect.visible) clampPosition();
    }
    onMenuVisibleChanged: {
        if (menuVisible) clampPosition();
    }

    function clampPosition() {
        if (menuX + menuRect.width > ctxRoot.width)
            menuX = ctxRoot.width - menuRect.width;
        if (menuY + menuRect.height > ctxRoot.height)
            menuY = ctxRoot.height - menuRect.height;
        if (menuX < 0) menuX = 0;
        if (menuY < 0) menuY = 0;
    }
}
