/*
 * lwindesk - A Windows 11-like Wayland Desktop Environment
 * Copyright (C) 2026 Frank Kahle
 *
 * compositor/include/output.h - Output (monitor) management
 */

#ifndef LWINDESK_OUTPUT_H
#define LWINDESK_OUTPUT_H

#include "server.h"

struct lw_output {
    struct wl_list link;                 /* lw_server.outputs */
    struct lw_server *server;
    struct wlr_output *wlr_output;
    struct wlr_scene_output *scene_output;

    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;
};

/* Handle a new output being connected */
void lw_output_new(struct wl_listener *listener, void *data);

#endif /* LWINDESK_OUTPUT_H */
