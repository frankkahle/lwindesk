/*
 * lwindesk - compositor/src/xdg_shell.c - XDG shell surface handling (wlroots 0.17)
 */

#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <string.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/util/log.h>

#include "server.h"
#include "view.h"
#include "input.h"

static void xdg_toplevel_map(struct wl_listener *listener, void *data) {
    struct lw_view *view = wl_container_of(listener, view, map);
    wl_list_insert(&view->server->views, &view->link);
    view->mapped = true;

    /* Position shell components based on window title.
     * All shell windows share app_id "lwindesk-shell", so use title
     * to distinguish them. */
    const char *title = view->xdg_toplevel->title;

    if (title && strncmp(title, "lwindesk-", 9) == 0) {
        view->is_shell_window = true;

        struct wlr_box output_box;
        wlr_output_layout_get_box(view->server->output_layout, NULL,
                                   &output_box);

        if (strcmp(title, "lwindesk-startmenu") == 0) {
            /* Start menu: centered, above taskbar */
            int x = (output_box.width - 640) / 2;
            int y = output_box.height - 48 - 720 - 12;
            wlr_scene_node_set_position(&view->scene_tree->node, x, y);
            view->x = x;
            view->y = y;
            wlr_log(WLR_INFO, "Start menu at %d,%d", x, y);
        } else if (strcmp(title, "lwindesk-notifications") == 0) {
            /* Notification center: right side, above taskbar */
            int x = output_box.width - 380 - 12;
            int y = output_box.height - 48 - 600 - 12;
            wlr_scene_node_set_position(&view->scene_tree->node, x, y);
            view->x = x;
            view->y = y;
            wlr_log(WLR_INFO, "Notifications at %d,%d", x, y);
        } else if (strcmp(title, "lwindesk-quicksettings") == 0) {
            /* Quick settings: right side, above taskbar */
            int x = output_box.width - 360 - 12;
            int y = output_box.height - 48 - 400 - 12;
            wlr_scene_node_set_position(&view->scene_tree->node, x, y);
            view->x = x;
            view->y = y;
            wlr_log(WLR_INFO, "Quick settings at %d,%d", x, y);
        } else if (strcmp(title, "lwindesk-desktop") == 0) {
            /* Desktop click surface: fullscreen at origin, behind
             * all other windows */
            wlr_scene_node_set_position(&view->scene_tree->node, 0, 0);
            view->x = 0;
            view->y = 0;
            wlr_scene_node_lower_to_bottom(&view->scene_tree->node);
            wlr_log(WLR_INFO, "Desktop surface at 0,0 (lowered)");
        } else {
            /* Default: taskbar at bottom */
            int y = output_box.height - 48;
            wlr_scene_node_set_position(&view->scene_tree->node, 0, y);
            view->x = 0;
            view->y = y;
            wlr_log(WLR_INFO, "Shell taskbar at y=%d", y);
        }
    } else {
        /* Non-shell window: create server-side decorations and center */
        view->is_shell_window = false;
        lw_view_create_decorations(view);

        /* Center the window on the output */
        struct wlr_box output_box;
        wlr_output_layout_get_box(view->server->output_layout, NULL,
                                   &output_box);
        struct wlr_box geo;
        wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo);
        int win_w = geo.width > 0 ? geo.width : 640;
        int win_h = geo.height > 0 ? geo.height : 480;
        int total_h = win_h + (view->deco.has_decorations ? LW_TITLEBAR_HEIGHT : 0);
        int x = (output_box.width - win_w) / 2;
        int y = (output_box.height - 48 - total_h) / 2;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        wlr_scene_node_set_position(&view->scene_tree->node, x, y);
        view->x = x;
        view->y = y;
    }

    lw_view_focus(view);
}

static void xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
    struct lw_view *view = wl_container_of(listener, view, unmap);
    if (view->server->grabbed_view == view) {
        view->server->cursor_mode = LW_CURSOR_PASSTHROUGH;
        view->server->grabbed_view = NULL;
    }
    wl_list_remove(&view->link);
    view->mapped = false;
}

static void xdg_toplevel_commit(struct wl_listener *listener, void *data) {
    struct lw_view *view = wl_container_of(listener, view, commit);
    if (view->xdg_toplevel->base->initial_commit) {
        wlr_xdg_toplevel_set_size(view->xdg_toplevel, 0, 0);
    }

    /* Update decoration size when window geometry changes */
    if (view->deco.has_decorations) {
        lw_view_update_decorations(view);
    }
}

static void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
    struct lw_view *view = wl_container_of(listener, view, destroy);

    /* Clean up decorations before freeing to prevent use-after-free.
     * The xdg scene tree child is auto-destroyed by wlroots, but our
     * wrapper tree and decoration rects would remain as orphans with
     * dangling data pointers if not cleaned up here. */
    lw_view_destroy_decorations(view);

    wl_list_remove(&view->map.link);
    wl_list_remove(&view->unmap.link);
    wl_list_remove(&view->commit.link);
    wl_list_remove(&view->destroy.link);
    wl_list_remove(&view->request_move.link);
    wl_list_remove(&view->request_resize.link);
    wl_list_remove(&view->request_maximize.link);
    wl_list_remove(&view->request_minimize.link);
    wl_list_remove(&view->request_fullscreen.link);
    wl_list_remove(&view->set_title.link);

    free(view);
}

static void xdg_toplevel_request_move(struct wl_listener *listener,
                                        void *data) {
    struct lw_view *view = wl_container_of(listener, view, request_move);
    lw_view_begin_move(view);
}

static void xdg_toplevel_request_resize(struct wl_listener *listener,
                                          void *data) {
    struct wlr_xdg_toplevel_resize_event *event = data;
    struct lw_view *view = wl_container_of(listener, view, request_resize);
    lw_view_begin_resize(view, event->edges);
}

static void xdg_toplevel_request_maximize(struct wl_listener *listener,
                                            void *data) {
    struct lw_view *view = wl_container_of(listener, view, request_maximize);
    if (view->is_maximized) {
        lw_view_restore(view);
    } else {
        lw_view_snap(view, LW_SNAP_MAXIMIZE);
    }
}

static void xdg_toplevel_request_minimize(struct wl_listener *listener,
                                            void *data) {
    struct lw_view *view = wl_container_of(listener, view, request_minimize);
    lw_view_minimize(view);
}

static void xdg_toplevel_request_fullscreen(struct wl_listener *listener,
                                              void *data) {
    struct lw_view *view = wl_container_of(listener, view, request_fullscreen);
    /* TODO: implement fullscreen */
    (void)view;
}

static void xdg_toplevel_set_title(struct wl_listener *listener, void *data) {
    struct lw_view *view = wl_container_of(listener, view, set_title);

    /* If title changes and we have decorations, update them */
    if (view->deco.has_decorations) {
        lw_view_update_decorations(view);
    }
}

static void setup_toplevel(struct lw_server *server,
                            struct wlr_xdg_surface *xdg_surface) {
    struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;

    struct lw_view *view = calloc(1, sizeof(*view));
    view->server = server;
    view->xdg_toplevel = toplevel;
    view->scene_tree =
        wlr_scene_xdg_surface_create(&server->scene->tree, xdg_surface);
    view->scene_tree->node.data = view;
    xdg_surface->data = view->scene_tree;

    /* Listen to surface events (map/unmap/commit are on wlr_surface in 0.17) */
    view->map.notify = xdg_toplevel_map;
    wl_signal_add(&xdg_surface->surface->events.map, &view->map);
    view->unmap.notify = xdg_toplevel_unmap;
    wl_signal_add(&xdg_surface->surface->events.unmap, &view->unmap);
    view->commit.notify = xdg_toplevel_commit;
    wl_signal_add(&xdg_surface->surface->events.commit, &view->commit);
    view->destroy.notify = xdg_toplevel_destroy;
    wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

    view->request_move.notify = xdg_toplevel_request_move;
    wl_signal_add(&toplevel->events.request_move, &view->request_move);
    view->request_resize.notify = xdg_toplevel_request_resize;
    wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
    view->request_maximize.notify = xdg_toplevel_request_maximize;
    wl_signal_add(&toplevel->events.request_maximize,
                  &view->request_maximize);
    view->request_minimize.notify = xdg_toplevel_request_minimize;
    wl_signal_add(&toplevel->events.request_minimize,
                  &view->request_minimize);
    view->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
    wl_signal_add(&toplevel->events.request_fullscreen,
                  &view->request_fullscreen);
    view->set_title.notify = xdg_toplevel_set_title;
    wl_signal_add(&toplevel->events.set_title, &view->set_title);

    /* Assign to active workspace */
    view->workspace = server->active_workspace;

    wlr_log(WLR_INFO, "New toplevel: %s",
             toplevel->title ? toplevel->title : "(untitled)");
}

static void setup_popup(struct wlr_xdg_surface *xdg_surface) {
    struct wlr_xdg_surface *parent =
        wlr_xdg_surface_try_from_wlr_surface(xdg_surface->popup->parent);
    if (!parent) return;

    struct wlr_scene_tree *parent_tree = parent->data;
    xdg_surface->data =
        wlr_scene_xdg_surface_create(parent_tree, xdg_surface);
}

/*
 * wlroots 0.17 uses a single new_surface event.
 * We check the role and dispatch to toplevel or popup setup.
 */
void lw_xdg_new_surface(struct wl_listener *listener, void *data) {
    struct lw_server *server =
        wl_container_of(listener, server, new_xdg_surface);
    struct wlr_xdg_surface *xdg_surface = data;

    if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
        setup_toplevel(server, xdg_surface);
    } else if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
        setup_popup(xdg_surface);
    }
}

/*
 * XDG decoration protocol handler.
 * When a client announces it supports the decoration protocol, we tell
 * it to use server-side decorations so it won't draw its own title bar.
 */
struct lw_xdg_deco {
    struct wl_listener request_mode;
    struct wl_listener destroy;
};

static void xdg_decoration_handle_request_mode(struct wl_listener *listener,
                                                  void *data) {
    struct wlr_xdg_toplevel_decoration_v1 *deco = data;
    wlr_xdg_toplevel_decoration_v1_set_mode(deco,
        WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
}

static void xdg_decoration_handle_destroy(struct wl_listener *listener,
                                             void *data) {
    struct lw_xdg_deco *d = wl_container_of(listener, d, destroy);
    wl_list_remove(&d->request_mode.link);
    wl_list_remove(&d->destroy.link);
    free(d);
}

void lw_xdg_new_decoration(struct wl_listener *listener, void *data) {
    struct wlr_xdg_toplevel_decoration_v1 *deco = data;

    struct lw_xdg_deco *d = calloc(1, sizeof(*d));

    d->request_mode.notify = xdg_decoration_handle_request_mode;
    wl_signal_add(&deco->events.request_mode, &d->request_mode);

    d->destroy.notify = xdg_decoration_handle_destroy;
    wl_signal_add(&deco->events.destroy, &d->destroy);

    /* Immediately tell the client to use server-side decorations */
    wlr_xdg_toplevel_decoration_v1_set_mode(deco,
        WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
}
