/*
 * lwindesk - A Windows 11-like Wayland Desktop Environment
 * Copyright (C) 2026 Frank Kahle
 *
 * compositor/include/input.h - Input device handling
 */

#ifndef LWINDESK_INPUT_H
#define LWINDESK_INPUT_H

#include "server.h"

struct lw_keyboard {
    struct wl_list link;                 /* lw_server.keyboards */
    struct lw_server *server;
    struct wlr_keyboard *wlr_keyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
};

/* Handle a new input device */
void lw_input_new(struct wl_listener *listener, void *data);

/* Handle cursor motion */
void lw_cursor_motion(struct wl_listener *listener, void *data);
void lw_cursor_motion_absolute(struct wl_listener *listener, void *data);
void lw_cursor_button(struct wl_listener *listener, void *data);
void lw_cursor_axis(struct wl_listener *listener, void *data);
void lw_cursor_frame(struct wl_listener *listener, void *data);

/* Process cursor movement (snap zone detection) */
void lw_process_cursor_motion(struct lw_server *server, uint32_t time);

/* Seat request handlers */
void lw_seat_request_cursor(struct wl_listener *listener, void *data);
void lw_seat_request_set_selection(struct wl_listener *listener, void *data);

/* Begin interactive move/resize */
void lw_view_begin_move(struct lw_view *view);
void lw_view_begin_resize(struct lw_view *view, uint32_t edges);

/* XDG shell handler */
void lw_xdg_new_surface(struct wl_listener *listener, void *data);

#endif /* LWINDESK_INPUT_H */
