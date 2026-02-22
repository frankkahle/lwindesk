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
#include "ipc.h"
#include "server.h"
#include "view.h"
#include "snap.h"
#include "workspace.h"

#include <linux/input-event-codes.h>
#include <string.h>

/* Check if a view is a shell window (title starting with "lwindesk-") */
static bool is_shell_view(struct lw_view *view) {
    const char *title = view->xdg_toplevel->title;
    return title && strncmp(title, "lwindesk-", 9) == 0;
}

/* Alt+Tab: cycle to next non-shell window.
 * Move the current focused view to the back of the list, then
 * focus the next mapped, non-shell, non-minimized view. */
static void cycle_window(struct lw_server *server) {
    if (wl_list_empty(&server->views)) return;

    /* Find the next suitable view to focus. We start from the
     * second entry in the list (the one behind the current top). */
    struct lw_view *current = wl_container_of(
        server->views.next, current, link);

    struct lw_view *next = NULL;
    struct lw_view *view;
    bool past_current = false;

    wl_list_for_each(view, &server->views, link) {
        if (view == current) {
            past_current = true;
            continue;
        }
        if (past_current && view->mapped && !view->is_minimized &&
            !is_shell_view(view)) {
            next = view;
            break;
        }
    }

    /* If we didn't find one after current, wrap around */
    if (!next) {
        wl_list_for_each(view, &server->views, link) {
            if (view == current) break;
            if (view->mapped && !view->is_minimized &&
                !is_shell_view(view)) {
                next = view;
                break;
            }
        }
    }

    if (next && next != current) {
        lw_view_focus(next);
    }
}

/* Handle compositor keybindings (Super+key shortcuts) */
static bool handle_keybinding(struct lw_server *server, xkb_keysym_t sym,
                               uint32_t modifiers) {
    bool super = modifiers & WLR_MODIFIER_LOGO;
    bool alt = modifiers & WLR_MODIFIER_ALT;

    if (super) {
        /* Any key pressed with Super means Super is used in a combo,
         * so we should not fire toggle-start-menu on release */
        server->super_used_in_combo = true;

        switch (sym) {
        case XKB_KEY_d:
        case XKB_KEY_D:
            /* Super+D: Show desktop (minimize all) + notify shell */
            {
                struct lw_view *view;
                wl_list_for_each(view, &server->views, link) {
                    if (view->mapped && !view->is_minimized) {
                        lw_view_minimize(view);
                    }
                }
                lw_ipc_send(server, "show-desktop");
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

    if (alt) {
        if (sym == XKB_KEY_Tab) {
            /* Alt+Tab: Cycle windows */
            cycle_window(server);
            lw_ipc_send(server, "cycle-window");
            return true;
        }

        if (sym == XKB_KEY_F4) {
            /* Alt+F4: Close focused window */
            if (!wl_list_empty(&server->views)) {
                struct lw_view *top =
                    wl_container_of(server->views.next, top, link);
                lw_view_close(top);
            }
            return true;
        }
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

static bool is_super_key(uint32_t keycode) {
    /* Linux input event codes for left/right meta (Super) keys */
    return keycode == KEY_LEFTMETA || keycode == KEY_RIGHTMETA;
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

    /* Track Super key state for tap-to-toggle-start-menu.
     * We use the raw evdev keycode (event->keycode) which does NOT
     * have the +8 XKB offset. */
    if (is_super_key(event->keycode)) {
        if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
            server->super_pressed = true;
            server->super_used_in_combo = false;
            /* Don't pass the bare Super press to clients */
            handled = true;
        } else if (event->state == WL_KEYBOARD_KEY_STATE_RELEASED) {
            if (server->super_pressed && !server->super_used_in_combo) {
                /* Super was tapped alone: toggle start menu */
                lw_ipc_send(server, "toggle-start-menu");
            }
            server->super_pressed = false;
            server->super_used_in_combo = false;
            /* Don't pass the bare Super release to clients */
            handled = true;
        }
    } else {
        /* Any non-Super key while Super is held marks it as a combo */
        if (server->super_pressed &&
            event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
            server->super_used_in_combo = true;
        }
    }

    if (!handled && event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
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

    /* Check if cursor is over a decoration element */
    struct lw_view *deco_view = NULL;
    enum lw_deco_button btn = lw_deco_button_at(server,
        server->cursor->x, server->cursor->y, &deco_view);
    if (btn != LW_DECO_NONE) {
        /* Over a decoration: set appropriate cursor and clear surface focus */
        wlr_cursor_set_xcursor(server->cursor, server->cursor_mgr, "default");
        wlr_seat_pointer_clear_focus(server->seat);
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

    if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
        /* On release during move, apply snap if pending */
        if (server->cursor_mode == LW_CURSOR_MOVE &&
            server->pending_snap != LW_SNAP_NONE) {
            lw_view_snap(server->grabbed_view, server->pending_snap);
            server->pending_snap = LW_SNAP_NONE;
        }
        server->cursor_mode = LW_CURSOR_PASSTHROUGH;
        server->grabbed_view = NULL;

        wlr_seat_pointer_notify_button(server->seat,
            event->time_msec, event->button, event->state);
        return;
    }

    /* Check if the click is on a decoration button */
    struct lw_view *deco_view = NULL;
    enum lw_deco_button btn = lw_deco_button_at(server,
        server->cursor->x, server->cursor->y, &deco_view);

    if (btn != LW_DECO_NONE && deco_view) {
        /* Focus the view that owns the decoration */
        lw_view_focus(deco_view);

        switch (btn) {
        case LW_DECO_CLOSE:
            lw_view_close(deco_view);
            return;
        case LW_DECO_MAXIMIZE:
            if (deco_view->is_maximized) {
                lw_view_restore(deco_view);
            } else {
                lw_view_snap(deco_view, LW_SNAP_MAXIMIZE);
            }
            return;
        case LW_DECO_MINIMIZE:
            lw_view_minimize(deco_view);
            return;
        case LW_DECO_TITLEBAR:
            /* Clicking the title bar initiates a window move */
            lw_view_begin_move_from_titlebar(deco_view);
            return;
        default:
            break;
        }
    }

    wlr_seat_pointer_notify_button(server->seat,
        event->time_msec, event->button, event->state);

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
        event->source);
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

void lw_view_begin_move_from_titlebar(struct lw_view *view) {
    struct lw_server *server = view->server;
    /* Initiated by compositor (title bar click), no focus check needed */
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
