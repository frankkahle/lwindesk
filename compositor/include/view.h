/*
 * lwindesk - A Windows 11-like Wayland Desktop Environment
 * Copyright (C) 2026 Frank Kahle
 *
 * compositor/include/view.h - Window (view) management
 */

#ifndef LWINDESK_VIEW_H
#define LWINDESK_VIEW_H

#include "server.h"

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

#endif /* LWINDESK_VIEW_H */
