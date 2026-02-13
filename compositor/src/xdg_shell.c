/*
 * lwindesk - compositor/src/xdg_shell.c - XDG shell toplevel/popup handling
 */

#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "server.h"
#include "view.h"
#include "input.h"

static void xdg_toplevel_map(struct wl_listener *listener, void *data) {
    struct lw_view *view = wl_container_of(listener, view, map);
    wl_list_insert(&view->server->views, &view->link);
    view->mapped = true;
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
}

static void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
    struct lw_view *view = wl_container_of(listener, view, destroy);

    wl_list_remove(&view->map.link);
    wl_list_remove(&view->unmap.link);
    wl_list_remove(&view->commit.link);
    wl_list_remove(&view->destroy.link);
    wl_list_remove(&view->request_move.link);
    wl_list_remove(&view->request_resize.link);
    wl_list_remove(&view->request_maximize.link);
    wl_list_remove(&view->request_minimize.link);
    wl_list_remove(&view->request_fullscreen.link);

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

void lw_xdg_new_toplevel(struct wl_listener *listener, void *data) {
    struct lw_server *server =
        wl_container_of(listener, server, new_xdg_toplevel);
    struct wlr_xdg_toplevel *toplevel = data;

    struct lw_view *view = calloc(1, sizeof(*view));
    view->server = server;
    view->xdg_toplevel = toplevel;
    view->scene_tree =
        wlr_scene_xdg_surface_create(&server->scene->tree,
                                       toplevel->base);
    view->scene_tree->node.data = view;

    /* Listen to toplevel events */
    view->map.notify = xdg_toplevel_map;
    wl_signal_add(&toplevel->base->surface->events.map, &view->map);
    view->unmap.notify = xdg_toplevel_unmap;
    wl_signal_add(&toplevel->base->surface->events.unmap, &view->unmap);
    view->commit.notify = xdg_toplevel_commit;
    wl_signal_add(&toplevel->base->surface->events.commit, &view->commit);
    view->destroy.notify = xdg_toplevel_destroy;
    wl_signal_add(&toplevel->events.destroy, &view->destroy);

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

    /* Assign to active workspace */
    view->workspace = server->active_workspace;

    wlr_log(WLR_INFO, "New toplevel: %s",
             toplevel->title ? toplevel->title : "(untitled)");
}

void lw_xdg_new_popup(struct wl_listener *listener, void *data) {
    struct wlr_xdg_popup *popup = data;
    struct wlr_xdg_surface *parent =
        wlr_xdg_surface_try_from_wlr_surface(popup->parent);
    if (!parent) return;

    struct wlr_scene_tree *parent_tree = parent->data;
    popup->base->data =
        wlr_scene_xdg_surface_create(parent_tree, popup->base);
}
