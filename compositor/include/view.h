/*
 * lwindesk - A Windows 11-like Wayland Desktop Environment
 * Copyright (C) 2026 Frank Kahle
 *
 * compositor/include/view.h - Window (view) management
 */

#ifndef LWINDESK_VIEW_H
#define LWINDESK_VIEW_H

#include "server.h"

/* Decoration button identifiers (stored in node->data) */
enum lw_deco_button {
	LW_DECO_NONE = 0,
	LW_DECO_TITLEBAR,
	LW_DECO_CLOSE,
	LW_DECO_MAXIMIZE,
	LW_DECO_MINIMIZE,
};

/* Title bar height in pixels */
#define LW_TITLEBAR_HEIGHT 32
/* Decoration button width */
#define LW_DECO_BUTTON_WIDTH 46

/* Server-side decoration nodes */
struct lw_decoration {
	struct wlr_scene_buffer *titlebar_buffer;
	struct wlr_buffer *titlebar_wlr_buffer;
	int width;
	char *cached_title;
	bool has_decorations;
};

/* A view represents a single toplevel window */
struct lw_view {
    struct wl_list link;                 /* lw_server.views */
    struct lw_server *server;
    struct wlr_xdg_toplevel *xdg_toplevel;
    struct wlr_scene_tree *scene_tree;

    /* XDG toplevel listeners */
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener commit;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
    struct wl_listener request_minimize;
    struct wl_listener request_fullscreen;
    struct wl_listener set_title;

    /* Server-side decorations */
    struct lw_decoration deco;

    /* Geometry before snap/maximize (for restore) */
    struct wlr_box saved_geometry;
    bool is_snapped;
    bool is_maximized;
    bool is_minimized;
    enum lw_snap_zone snap_zone;

    /* Workspace assignment */
    struct lw_workspace *workspace;

    /* Window state */
    int x, y;
    bool mapped;
    bool is_shell_window;
};

/* Create a new view for a toplevel surface */
struct lw_view *lw_view_create(struct lw_server *server,
                                struct wlr_xdg_toplevel *toplevel);

/* Focus this view (raise + keyboard focus) */
void lw_view_focus(struct lw_view *view);

/* Snap a view to a zone */
void lw_view_snap(struct lw_view *view, enum lw_snap_zone zone);

/* Restore a view from snapped/maximized state */
void lw_view_restore(struct lw_view *view);

/* Minimize a view (hide from scene) */
void lw_view_minimize(struct lw_view *view);

/* Unminimize a view (show in scene) */
void lw_view_unminimize(struct lw_view *view);

/* Close a view */
void lw_view_close(struct lw_view *view);

/* Get the view at a given layout coordinate */
struct lw_view *lw_view_at(struct lw_server *server, double lx, double ly,
                            struct wlr_surface **surface,
                            double *sx, double *sy);

/* Create server-side decorations for a view */
void lw_view_create_decorations(struct lw_view *view);

/* Destroy decoration nodes (must call before freeing view) */
void lw_view_destroy_decorations(struct lw_view *view);

/* Update decoration size to match window width */
void lw_view_update_decorations(struct lw_view *view);

/* Check if a scene node is a decoration button, return the button type */
enum lw_deco_button lw_deco_button_at(struct lw_server *server,
                                        double lx, double ly,
                                        struct lw_view **out_view);

#endif /* LWINDESK_VIEW_H */
