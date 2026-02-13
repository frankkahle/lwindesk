/*
 * lwindesk - A Windows 11-like Wayland Desktop Environment
 * Copyright (C) 2026 Frank Kahle
 *
 * compositor/include/server.h - Core compositor server state
 */

#ifndef LWINDESK_SERVER_H
#define LWINDESK_SERVER_H

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>

/* Forward declarations */
struct lw_view;
struct lw_workspace;

/* Snap zones for Windows 11-style snap layouts */
enum lw_snap_zone {
    LW_SNAP_NONE = 0,
    LW_SNAP_LEFT,
    LW_SNAP_RIGHT,
    LW_SNAP_TOP_LEFT,
    LW_SNAP_TOP_RIGHT,
    LW_SNAP_BOTTOM_LEFT,
    LW_SNAP_BOTTOM_RIGHT,
    LW_SNAP_MAXIMIZE,
};

/* Cursor mode for interactive move/resize */
enum lw_cursor_mode {
    LW_CURSOR_PASSTHROUGH,
    LW_CURSOR_MOVE,
    LW_CURSOR_RESIZE,
};

/* Main server state */
struct lw_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_scene *scene;
    struct wlr_scene_output_layout *scene_layout;

    struct wlr_xdg_shell *xdg_shell;
    struct wl_listener new_xdg_surface;

    struct wlr_output_layout *output_layout;
    struct wl_list outputs;              /* lw_output.link */
    struct wl_listener new_output;

    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursor_mgr;
    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;
    struct wl_listener cursor_button;
    struct wl_listener cursor_axis;
    struct wl_listener cursor_frame;

    struct wlr_seat *seat;
    struct wl_listener new_input;
    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;
    struct wl_list keyboards;            /* lw_keyboard.link */

    /* Views (windows) */
    struct wl_list views;                /* lw_view.link */

    /* Virtual desktops (workspaces) */
    struct wl_list workspaces;           /* lw_workspace.link */
    struct lw_workspace *active_workspace;
    int workspace_count;

    /* Interactive move/resize state */
    enum lw_cursor_mode cursor_mode;
    struct lw_view *grabbed_view;
    double grab_x, grab_y;
    struct wlr_box grab_geobox;
    uint32_t resize_edges;

    /* Snap state */
    enum lw_snap_zone pending_snap;

    /* Wayland socket name for clients */
    const char *socket;
};

/* Initialize the compositor server */
int lw_server_init(struct lw_server *server);

/* Start the backend and event loop */
int lw_server_run(struct lw_server *server);

/* Clean up resources */
void lw_server_destroy(struct lw_server *server);

#endif /* LWINDESK_SERVER_H */
