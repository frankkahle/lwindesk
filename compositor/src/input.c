/*
 * lwindesk - compositor/src/input.c - Input device handling
 */

#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

#include "input.h"
#include "server.h"
#include "view.h"
#include "snap.h"

/* Handle compositor keybindings (Super+key shortcuts) */
static bool handle_keybinding(struct lw_server *server, xkb_keysym_t sym,
                               uint32_t modifiers) {
    bool super = modifiers & WLR_MODIFIER_LOGO;
    bool alt = modifiers & WLR_MODIFIER_ALT;

    if (super) {
        switch (sym) {
        case XKB_KEY_d:
        case XKB_KEY_D:
            /* Super+D: Show desktop (minimize all) */
            {
                struct lw_view *view;
                wl_list_for_each(view, &server->views, link) {
                    if (view->mapped && !view->is_minimized) {
                        lw_view_minimize(view);
                    }
                }
            }
            return true;

        case XKB_KEY_Left:
            /* Super+Left: Snap left */
            if (!wl_list_empty(&server->views)) {
                struct lw_view *top =
                    wl_container_of(server->views.next, top, link);
                lw_view_snap(top, LW_SNAP_LEFT);
            }
            return true;

        case XKB_KEY_Right:
            /* Super+Right: Snap right */
            if (!wl_list_empty(&server->views)) {
                struct lw_view *top =
                    wl_container_of(server->views.next, top, link);
                lw_view_snap(top, LW_SNAP_RIGHT);
            }
            return true;

        case XKB_KEY_Up:
            /* Super+Up: Maximize */
            if (!wl_list_empty(&server->views)) {
                struct lw_view *top =
                    wl_container_of(server->views.next, top, link);
                lw_view_snap(top, LW_SNAP_MAXIMIZE);
            }
            return true;

        case XKB_KEY_Down:
            /* Super+Down: Restore */
            if (!wl_list_empty(&server->views)) {
                struct lw_view *top =
                    wl_container_of(server->views.next, top, link);
                lw_view_restore(top);
            }
            return true;

        case XKB_KEY_1: case XKB_KEY_2: case XKB_KEY_3:
        case XKB_KEY_4: case XKB_KEY_5: case XKB_KEY_6:
        case XKB_KEY_7: case XKB_KEY_8: case XKB_KEY_9:
            /* Super+N: Switch to workspace N */
            {
                int idx = sym - XKB_KEY_1;
                struct lw_workspace *ws =
                    lw_workspace_get(server, idx);
                if (ws) lw_workspace_switch(server, ws);
            }
            return true;

        case XKB_KEY_q:
        case XKB_KEY_Q:
            /* Super+Q: Close focused window */
            if (!wl_list_empty(&server->views)) {
                struct lw_view *top =
                    wl_container_of(server->views.next, top, link);
                lw_view_close(top);
            }
            return true;

        default:
            break;
        }
    }

    if (alt && sym == XKB_KEY_F4) {
        /* Alt+F4: Close focused window */
        if (!wl_list_empty(&server->views)) {
            struct lw_view *top =
                wl_container_of(server->views.next, top, link);
            lw_view_close(top);
        }
        return true;
    }

    return false;
}

static void keyboard_handle_modifiers(struct wl_listener *listener,
                                        void *data) {
    struct lw_keyboard *keyboard =
        wl_container_of(listener, keyboard, modifiers);
    wlr_seat_set_keyboard(keyboard->server->seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(keyboard->server->seat,
        &keyboard->wlr_keyboard->modifiers);
}

static void keyboard_handle_key(struct wl_listener *listener, void *data) {
    struct lw_keyboard *keyboard = wl_container_of(listener, keyboard, key);
    struct wlr_keyboard_key_event *event = data;
    struct lw_server *server = keyboard->server;

    uint32_t keycode = event->keycode + 8;
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(
        keyboard->wlr_keyboard->xkb_state, keycode, &syms);

    bool handled = false;
    uint32_t modifiers =
        wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);

    if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        for (int i = 0; i < nsyms; i++) {
            handled = handle_keybinding(server, syms[i], modifiers);
            if (handled) break;
        }
    }

    if (!handled) {
        wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);
        wlr_seat_keyboard_notify_key(server->seat, event->time_msec,
            event->keycode, event->state);
    }
}

static void keyboard_handle_destroy(struct wl_listener *listener, void *data) {
    struct lw_keyboard *keyboard =
        wl_container_of(listener, keyboard, destroy);
    wl_list_remove(&keyboard->modifiers.link);
    wl_list_remove(&keyboard->key.link);
    wl_list_remove(&keyboard->destroy.link);
    wl_list_remove(&keyboard->link);
    free(keyboard);
}

static void new_keyboard(struct lw_server *server,
                           struct wlr_input_device *device) {
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

    struct lw_keyboard *keyboard = calloc(1, sizeof(*keyboard));
    keyboard->server = server;
    keyboard->wlr_keyboard = wlr_keyboard;

    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL,
        XKB_KEYMAP_COMPILE_NO_FLAGS);
    wlr_keyboard_set_keymap(wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

    keyboard->modifiers.notify = keyboard_handle_modifiers;
    wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);
    keyboard->key.notify = keyboard_handle_key;
    wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);
    keyboard->destroy.notify = keyboard_handle_destroy;
    wl_signal_add(&device->events.destroy, &keyboard->destroy);

    wlr_seat_set_keyboard(server->seat, wlr_keyboard);
    wl_list_insert(&server->keyboards, &keyboard->link);
}

static void new_pointer(struct lw_server *server,
                          struct wlr_input_device *device) {
    wlr_cursor_attach_input_device(server->cursor, device);
}

void lw_input_new(struct wl_listener *listener, void *data) {
    struct lw_server *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *device = data;

    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        new_keyboard(server, device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        new_pointer(server, device);
        break;
    default:
        break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(server->seat, caps);
}

void lw_process_cursor_motion(struct lw_server *server, uint32_t time) {
    if (server->cursor_mode == LW_CURSOR_MOVE) {
        struct lw_view *view = server->grabbed_view;
        view->x = server->cursor->x - server->grab_x;
        view->y = server->cursor->y - server->grab_y;
        wlr_scene_node_set_position(&view->scene_tree->node,
                                     view->x, view->y);

        /* Detect snap zones while dragging */
        server->pending_snap = lw_snap_zone_at(server,
            server->cursor->x, server->cursor->y);
        return;
    }

    if (server->cursor_mode == LW_CURSOR_RESIZE) {
        /* TODO: interactive resize */
        return;
    }

    /* Passthrough: find view under cursor */
    double sx, sy;
    struct wlr_surface *surface = NULL;
    struct lw_view *view = lw_view_at(server,
        server->cursor->x, server->cursor->y, &surface, &sx, &sy);

    if (!view) {
        wlr_cursor_set_xcursor(server->cursor, server->cursor_mgr, "default");
    }

    if (surface) {
        wlr_seat_pointer_notify_enter(server->seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(server->seat, time, sx, sy);
    } else {
        wlr_seat_pointer_clear_focus(server->seat);
    }
}

void lw_cursor_motion(struct wl_listener *listener, void *data) {
    struct lw_server *server =
        wl_container_of(listener, server, cursor_motion);
    struct wlr_pointer_motion_event *event = data;
    wlr_cursor_move(server->cursor, &event->pointer->base,
                     event->delta_x, event->delta_y);
    lw_process_cursor_motion(server, event->time_msec);
}

void lw_cursor_motion_absolute(struct wl_listener *listener, void *data) {
    struct lw_server *server =
        wl_container_of(listener, server, cursor_motion_absolute);
    struct wlr_pointer_motion_absolute_event *event = data;
    wlr_cursor_warp_absolute(server->cursor, &event->pointer->base,
                              event->x, event->y);
    lw_process_cursor_motion(server, event->time_msec);
}

void lw_cursor_button(struct wl_listener *listener, void *data) {
    struct lw_server *server =
        wl_container_of(listener, server, cursor_button);
    struct wlr_pointer_button_event *event = data;

    wlr_seat_pointer_notify_button(server->seat,
        event->time_msec, event->button, event->state);

    if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
        /* On release during move, apply snap if pending */
        if (server->cursor_mode == LW_CURSOR_MOVE &&
            server->pending_snap != LW_SNAP_NONE) {
            lw_view_snap(server->grabbed_view, server->pending_snap);
            server->pending_snap = LW_SNAP_NONE;
        }
        server->cursor_mode = LW_CURSOR_PASSTHROUGH;
        server->grabbed_view = NULL;
        return;
    }

    /* Focus the clicked view */
    double sx, sy;
    struct wlr_surface *surface = NULL;
    struct lw_view *view = lw_view_at(server,
        server->cursor->x, server->cursor->y, &surface, &sx, &sy);
    if (view) {
        lw_view_focus(view);
    }
}

void lw_cursor_axis(struct wl_listener *listener, void *data) {
    struct lw_server *server =
        wl_container_of(listener, server, cursor_axis);
    struct wlr_pointer_axis_event *event = data;
    wlr_seat_pointer_notify_axis(server->seat, event->time_msec,
        event->orientation, event->delta, event->delta_discrete,
        event->source, event->relative_direction);
}

void lw_cursor_frame(struct wl_listener *listener, void *data) {
    struct lw_server *server =
        wl_container_of(listener, server, cursor_frame);
    wlr_seat_pointer_notify_frame(server->seat);
}

void lw_seat_request_cursor(struct wl_listener *listener, void *data) {
    struct lw_server *server =
        wl_container_of(listener, server, request_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;
    struct wlr_seat_client *focused =
        server->seat->pointer_state.focused_client;
    if (focused == event->seat_client) {
        wlr_cursor_set_surface(server->cursor, event->surface,
            event->hotspot_x, event->hotspot_y);
    }
}

void lw_seat_request_set_selection(struct wl_listener *listener, void *data) {
    struct lw_server *server =
        wl_container_of(listener, server, request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(server->seat, event->source, event->serial);
}

void lw_view_begin_move(struct lw_view *view) {
    struct lw_server *server = view->server;
    if (server->seat->pointer_state.focused_surface !=
        view->xdg_toplevel->base->surface) {
        return;
    }
    server->grabbed_view = view;
    server->cursor_mode = LW_CURSOR_MOVE;
    server->grab_x = server->cursor->x - view->x;
    server->grab_y = server->cursor->y - view->y;
}

void lw_view_begin_resize(struct lw_view *view, uint32_t edges) {
    struct lw_server *server = view->server;
    if (server->seat->pointer_state.focused_surface !=
        view->xdg_toplevel->base->surface) {
        return;
    }
    server->grabbed_view = view;
    server->cursor_mode = LW_CURSOR_RESIZE;
    server->resize_edges = edges;

    struct wlr_box geo;
    wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo);
    server->grab_geobox = geo;
    server->grab_geobox.x = view->x;
    server->grab_geobox.y = view->y;
    server->grab_x = server->cursor->x;
    server->grab_y = server->cursor->y;
}
