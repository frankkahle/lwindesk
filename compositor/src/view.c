/*
 * lwindesk - compositor/src/view.c - Window (view) management
 */

#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "view.h"
#include "server.h"

void lw_view_focus(struct lw_view *view) {
    if (!view) return;

    struct lw_server *server = view->server;
    struct wlr_seat *seat = server->seat;
    struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
    struct wlr_surface *surface =
        view->xdg_toplevel->base->surface;

    if (prev_surface == surface) return;

    /* Deactivate previous */
    if (prev_surface) {
        struct wlr_xdg_toplevel *prev_toplevel =
            wlr_xdg_toplevel_try_from_wlr_surface(prev_surface);
        if (prev_toplevel) {
            wlr_xdg_toplevel_set_activated(prev_toplevel, false);
        }
    }

    /* Raise to top */
    wlr_scene_node_raise_to_top(&view->scene_tree->node);
    wl_list_remove(&view->link);
    wl_list_insert(&server->views, &view->link);

    /* Activate */
    wlr_xdg_toplevel_set_activated(view->xdg_toplevel, true);

    /* Send keyboard focus */
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
    if (keyboard) {
        wlr_seat_keyboard_notify_enter(seat, surface,
            keyboard->keycodes, keyboard->num_keycodes,
            &keyboard->modifiers);
    }
}

void lw_view_snap(struct lw_view *view, enum lw_snap_zone zone) {
    if (!view || zone == LW_SNAP_NONE) return;

    /* Save current geometry for restore */
    if (!view->is_snapped && !view->is_maximized) {
        struct wlr_box geo;
        wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo);
        view->saved_geometry.x = view->x;
        view->saved_geometry.y = view->y;
        view->saved_geometry.width = geo.width;
        view->saved_geometry.height = geo.height;
    }

    /* Get the output this view is on */
    struct wlr_output *output =
        wlr_output_layout_output_at(view->server->output_layout,
                                     view->x + 1, view->y + 1);
    if (!output) return;

    struct wlr_box output_box;
    wlr_output_layout_get_box(view->server->output_layout, output,
                               &output_box);

    /* Reserve 48px at bottom for taskbar */
    int taskbar_height = 48;
    int usable_h = output_box.height - taskbar_height;

    struct wlr_box target = {0};

    switch (zone) {
    case LW_SNAP_LEFT:
        target.x = output_box.x;
        target.y = output_box.y;
        target.width = output_box.width / 2;
        target.height = usable_h;
        break;
    case LW_SNAP_RIGHT:
        target.x = output_box.x + output_box.width / 2;
        target.y = output_box.y;
        target.width = output_box.width / 2;
        target.height = usable_h;
        break;
    case LW_SNAP_TOP_LEFT:
        target.x = output_box.x;
        target.y = output_box.y;
        target.width = output_box.width / 2;
        target.height = usable_h / 2;
        break;
    case LW_SNAP_TOP_RIGHT:
        target.x = output_box.x + output_box.width / 2;
        target.y = output_box.y;
        target.width = output_box.width / 2;
        target.height = usable_h / 2;
        break;
    case LW_SNAP_BOTTOM_LEFT:
        target.x = output_box.x;
        target.y = output_box.y + usable_h / 2;
        target.width = output_box.width / 2;
        target.height = usable_h / 2;
        break;
    case LW_SNAP_BOTTOM_RIGHT:
        target.x = output_box.x + output_box.width / 2;
        target.y = output_box.y + usable_h / 2;
        target.width = output_box.width / 2;
        target.height = usable_h / 2;
        break;
    case LW_SNAP_MAXIMIZE:
        target.x = output_box.x;
        target.y = output_box.y;
        target.width = output_box.width;
        target.height = usable_h;
        view->is_maximized = true;
        break;
    default:
        return;
    }

    wlr_xdg_toplevel_set_size(view->xdg_toplevel, target.width, target.height);
    wlr_scene_node_set_position(&view->scene_tree->node, target.x, target.y);
    view->x = target.x;
    view->y = target.y;
    view->is_snapped = true;
    view->snap_zone = zone;
}

void lw_view_restore(struct lw_view *view) {
    if (!view || (!view->is_snapped && !view->is_maximized)) return;

    wlr_xdg_toplevel_set_size(view->xdg_toplevel,
        view->saved_geometry.width, view->saved_geometry.height);
    wlr_scene_node_set_position(&view->scene_tree->node,
        view->saved_geometry.x, view->saved_geometry.y);

    view->x = view->saved_geometry.x;
    view->y = view->saved_geometry.y;
    view->is_snapped = false;
    view->is_maximized = false;
    view->snap_zone = LW_SNAP_NONE;
}

void lw_view_minimize(struct lw_view *view) {
    if (!view || view->is_minimized) return;
    wlr_scene_node_set_enabled(&view->scene_tree->node, false);
    view->is_minimized = true;
}

void lw_view_unminimize(struct lw_view *view) {
    if (!view || !view->is_minimized) return;
    wlr_scene_node_set_enabled(&view->scene_tree->node, true);
    view->is_minimized = false;
    lw_view_focus(view);
}

void lw_view_close(struct lw_view *view) {
    if (!view) return;
    wlr_xdg_toplevel_send_close(view->xdg_toplevel);
}

struct lw_view *lw_view_at(struct lw_server *server, double lx, double ly,
                            struct wlr_surface **surface,
                            double *sx, double *sy) {
    struct wlr_scene_node *node =
        wlr_scene_node_at(&server->scene->tree.node, lx, ly, sx, sy);
    if (!node || node->type != WLR_SCENE_NODE_BUFFER) {
        return NULL;
    }

    struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
    struct wlr_scene_surface *scene_surface =
        wlr_scene_surface_try_from_buffer(scene_buffer);
    if (!scene_surface) return NULL;

    *surface = scene_surface->surface;

    /* Walk up the scene tree to find the view */
    struct wlr_scene_tree *tree = node->parent;
    while (tree && !tree->node.data) {
        tree = tree->node.parent;
    }

    return tree ? tree->node.data : NULL;
}
