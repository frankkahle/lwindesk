import QtQuick

/* Base component for all panels (taskbar, start menu, notifications, etc.) */
Rectangle {
    id: panel

    /* Default Windows 11 dark panel style */
    color: Qt.rgba(0.16, 0.16, 0.16, 0.95)
    radius: 8
    border.color: Qt.rgba(1, 1, 1, 0.08)
    border.width: 1

    /* Subtle shadow */
    layer.enabled: true
    layer.effect: null  /* TODO: add DropShadow when QtGraphicalEffects available */
}
