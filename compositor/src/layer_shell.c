/*
 * lwindesk - compositor/src/layer_shell.c - Layer shell protocol (for shell panels)
 *
 * TODO: Implement wlr-layer-shell-unstable-v1 for taskbar and overlays.
 * The shell (Qt/QML) will use layer shell to anchor the taskbar to the bottom
 * of the screen and render overlays (start menu, notifications) above windows.
 */

#define _POSIX_C_SOURCE 200112L
#include "server.h"

/* Placeholder - layer shell implementation will be added when the
 * shell components need to register as layer surfaces */
