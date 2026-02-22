import QtQuick
import QtQuick.Layouts
import LWinDesk
import "../common"

/*
 * Windows 11-style Start Menu - centered, rounded corners,
 * search bar at top, pinned apps grid, recommended/recent section.
 * "All apps" toggles to a scrollable alphabetical list.
 */
LWPanel {
    id: startMenu
    width: 640
    height: 720
    color: Qt.rgba(0.16, 0.16, 0.16, 0.95)
    radius: 8

    property bool showAllApps: false

    StartMenuModel {
        id: appModel
    }

    Connections {
        target: shellManager
        function onSearchFocusRequestedChanged() {
            if (shellManager.searchFocusRequested) {
                focusTimer.start()
            }
        }
        function onSearchTextChanged() {
            searchBar.text = shellManager.searchText
        }
    }

    Timer {
        id: focusTimer
        interval: 150
        onTriggered: {
            searchBar.giveFocus()
            shellManager.clearSearchFocusRequest()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        /* Search bar */
        SearchBar {
            id: searchBar
            Layout.fillWidth: true
            onSearchChanged: function(text) {
                appModel.searchQuery = text
                if (text.length > 0)
                    startMenu.showAllApps = true
            }
        }

        /* Section header */
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: startMenu.showAllApps ? "All apps" : "Pinned"
                color: "white"
                font.pixelSize: 14
                font.family: "Selawik"
                font.bold: true
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                width: allAppsLabel.width + 16
                height: 28
                radius: 4
                color: allAppsMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"

                Text {
                    id: allAppsLabel
                    anchors.centerIn: parent
                    text: startMenu.showAllApps ? "\u2190 Back" : "All apps \u2192"
                    color: Qt.rgba(1, 1, 1, 0.7)
                    font.pixelSize: 12
                    font.family: "Selawik"
                }
                MouseArea {
                    id: allAppsMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: startMenu.showAllApps = !startMenu.showAllApps
                }
            }
        }

        /* ---- Pinned view (default) ---- */
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !startMenu.showAllApps

            ColumnLayout {
                anchors.fill: parent
                spacing: 16

                /* Pinned app grid */
                AppGrid {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 280
                }

                /* Divider */
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: Qt.rgba(1, 1, 1, 0.1)
                }

                /* "Recommended" section */
                Text {
                    text: "Recommended"
                    color: "white"
                    font.pixelSize: 14
                    font.family: "Selawik"
                    font.bold: true
                }

                /* Recent items placeholder */
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: 4
                    delegate: Rectangle {
                        width: ListView.view ? ListView.view.width : 0
                        height: 48
                        color: recMouseArea.containsMouse ?
                            Qt.rgba(1, 1, 1, 0.05) : "transparent"
                        radius: 4

                        Row {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 12
                            Rectangle {
                                width: 32; height: 32
                                radius: 4
                                color: Qt.rgba(1, 1, 1, 0.1)
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    anchors.centerIn: parent
                                    text: ["\uD83D\uDCC4", "\uD83D\uDCC1", "\u2699", "\uD83C\uDF10"][index]
                                    font.pixelSize: 14
                                }
                            }
                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    text: ["Recent document", "Downloads", "Settings", "Browser"][index]
                                    color: "white"
                                    font.pixelSize: 13
                                    font.family: "Selawik"
                                }
                                Text {
                                    text: "Recently opened"
                                    color: Qt.rgba(1, 1, 1, 0.5)
                                    font.pixelSize: 11
                                    font.family: "Selawik"
                                }
                            }
                        }
                        MouseArea {
                            id: recMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                        }
                    }
                }
            }
        }

        /* ---- All apps view (scrollable list) ---- */
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: startMenu.showAllApps
            clip: true
            model: appModel
            spacing: 2

            delegate: Rectangle {
                id: appDelegate
                required property int index
                required property string name
                required property string exec
                required property string iconName
                width: ListView.view ? ListView.view.width : 0
                height: 44
                radius: 4
                color: appMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : "transparent"

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    spacing: 12

                    /* App icon (with letter fallback) */
                    Item {
                        width: 32; height: 32
                        anchors.verticalCenter: parent.verticalCenter

                        Image {
                            id: appIconImg
                            anchors.fill: parent
                            sourceSize: Qt.size(32, 32)
                            source: appDelegate.iconName ? "image://icon/" + appDelegate.iconName : ""
                            smooth: true
                            visible: status === Image.Ready
                        }

                        /* Fallback: colored rectangle with first letter */
                        Rectangle {
                            anchors.fill: parent
                            radius: 6
                            visible: appIconImg.status !== Image.Ready
                            color: {
                                var colors = ["#0078D4", "#107C10", "#D83B01",
                                              "#8764B8", "#00B294", "#E81123",
                                              "#0099BC", "#767676"]
                                return colors[appDelegate.index % colors.length]
                            }
                            Text {
                                anchors.centerIn: parent
                                text: appDelegate.name ? appDelegate.name.charAt(0).toUpperCase() : "?"
                                color: "white"
                                font.pixelSize: 16
                                font.bold: true
                            }
                        }
                    }

                    Text {
                        text: appDelegate.name
                        color: "white"
                        font.pixelSize: 13
                        font.family: "Selawik"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                MouseArea {
                    id: appMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: shellManager.launchApp(appDelegate.exec)
                }
            }
        }

        /* Bottom bar: user profile + power */
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            /* User profile */
            Row {
                spacing: 8
                Rectangle {
                    width: 32; height: 32; radius: 16
                    color: "#0078D4"
                    Text {
                        anchors.centerIn: parent
                        text: "F"
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                    }
                }
                Text {
                    text: "Frank"
                    color: "white"
                    font.pixelSize: 13
                    font.family: "Selawik"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Item { Layout.fillWidth: true }

            /* Power button */
            Rectangle {
                width: 28; height: 28; radius: 4
                color: powerMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                Text {
                    anchors.centerIn: parent
                    text: "\u23FB"
                    color: Qt.rgba(1, 1, 1, 0.7)
                    font.pixelSize: 16
                }
                MouseArea {
                    id: powerMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: { /* TODO: power menu */ }
                }
            }
        }
    }
}
