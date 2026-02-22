/*
 * lwindesk - A Windows 11-like Wayland Desktop Environment
 * Copyright (C) 2026 Frank Kahle
 *
 * compositor/src/server.c - Core server initialization and lifecycle
 */

#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
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
#include <wlr/util/log.h>

#include "server.h"
#include "output.h"
#include "input.h"
#include "ipc.h"
#include "view.h"
#include "workspace.h"

int lw_server_init(struct lw_server *server) {
    server->wl_display = wl_display_create();
    if (!server->wl_display) {
        wlr_log(WLR_ERROR, "Failed to create Wayland display");
        return -1;
    }

    /* Create backend (auto-detects DRM/libinput or Wayland/X11 nested) */
    server->backend = wlr_backend_autocreate(server->wl_display, NULL);
    if (!server->backend) {
        wlr_log(WLR_ERROR, "Failed to create wlroots backend");
        return -1;
    }

    /* Create renderer and allocator */
    server->renderer = wlr_renderer_autocreate(server->backend);
    if (!server->renderer) {
        wlr_log(WLR_ERROR, "Failed to create renderer");
        return -1;
    }
    wlr_renderer_init_wl_display(server->renderer, server->wl_display);

    server->allocator = wlr_allocator_autocreate(server->backend,
                                                   server->renderer);
    if (!server->allocator) {
        wlr_log(WLR_ERROR, "Failed to create allocator");
        return -1;
    }

    /* Create scene graph for efficient rendering */
    server->scene = wlr_scene_create();
    server->output_layout = wlr_output_layout_create();
    server->scene_layout = wlr_scene_attach_output_layout(server->scene,
                                                            server->output_layout);

    /* Desktop background — smooth gradient from dark navy → blue → dark purple */
    {
        /* Gradient color stops: position (0-1), R, G, B */
        struct { float pos, r, g, b; } stops[] = {
            {0.00f, 0.00f, 0.04f, 0.12f},
            {0.35f, 0.00f, 0.35f, 0.70f},
            {0.50f, 0.00f, 0.47f, 0.83f},
            {0.65f, 0.00f, 0.35f, 0.70f},
            {1.00f, 0.08f, 0.05f, 0.22f},
        };
        int nstops = sizeof(stops) / sizeof(stops[0]);
        int nstrips = 128;
        int total_h = 4096;
        int strip_h = total_h / nstrips;
        for (int i = 0; i < nstrips; i++) {
            float t = (float)i / (float)(nstrips - 1);
            /* Find the two stops to interpolate between */
            int s = 0;
            for (int j = 0; j < nstops - 1; j++) {
                if (t >= stops[j].pos) s = j;
            }
            float local_t = (t - stops[s].pos) /
                            (stops[s + 1].pos - stops[s].pos);
            if (local_t < 0.0f) local_t = 0.0f;
            if (local_t > 1.0f) local_t = 1.0f;
            float color[4] = {
                stops[s].r + (stops[s+1].r - stops[s].r) * local_t,
                stops[s].g + (stops[s+1].g - stops[s].g) * local_t,
                stops[s].b + (stops[s+1].b - stops[s].b) * local_t,
                1.0f
            };
            struct wlr_scene_rect *rect =
                wlr_scene_rect_create(&server->scene->tree, 8192, strip_h + 1, color);
            wlr_scene_node_set_position(&rect->node, 0, i * strip_h);
        }
    }

    /* Create Wayland globals */
    wlr_compositor_create(server->wl_display, 5, server->renderer);
    wlr_subcompositor_create(server->wl_display);
    wlr_data_device_manager_create(server->wl_display);

    /* XDG shell for application windows */
    server->xdg_shell = wlr_xdg_shell_create(server->wl_display, 3);
    server->new_xdg_surface.notify = lw_xdg_new_surface;
    wl_signal_add(&server->xdg_shell->events.new_surface,
                  &server->new_xdg_surface);

    /* XDG decoration manager — tells clients to use server-side decorations */
    server->xdg_decoration_mgr =
        wlr_xdg_decoration_manager_v1_create(server->wl_display);
    server->new_xdg_decoration.notify = lw_xdg_new_decoration;
    wl_signal_add(&server->xdg_decoration_mgr->events.new_toplevel_decoration,
                  &server->new_xdg_decoration);

    /* Initialize view list */
    wl_list_init(&server->views);

    /* Output handling */
    wl_list_init(&server->outputs);
    server->new_output.notify = lw_output_new;
    wl_signal_add(&server->backend->events.new_output, &server->new_output);

    /* Cursor */
    server->cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server->cursor, server->output_layout);
    server->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);

    server->cursor_motion.notify = lw_cursor_motion;
    wl_signal_add(&server->cursor->events.motion, &server->cursor_motion);
    server->cursor_motion_absolute.notify = lw_cursor_motion_absolute;
    wl_signal_add(&server->cursor->events.motion_absolute,
                  &server->cursor_motion_absolute);
    server->cursor_button.notify = lw_cursor_button;
    wl_signal_add(&server->cursor->events.button, &server->cursor_button);
    server->cursor_axis.notify = lw_cursor_axis;
    wl_signal_add(&server->cursor->events.axis, &server->cursor_axis);
    server->cursor_frame.notify = lw_cursor_frame;
    wl_signal_add(&server->cursor->events.frame, &server->cursor_frame);

    /* Seat (input) */
    wl_list_init(&server->keyboards);
    server->seat = wlr_seat_create(server->wl_display, "seat0");
    server->new_input.notify = lw_input_new;
    wl_signal_add(&server->backend->events.new_input, &server->new_input);
    server->request_cursor.notify = lw_seat_request_cursor;
    wl_signal_add(&server->seat->events.request_set_cursor,
                  &server->request_cursor);
    server->request_set_selection.notify = lw_seat_request_set_selection;
    wl_signal_add(&server->seat->events.request_set_selection,
                  &server->request_set_selection);

    /* Initialize workspaces - create first desktop */
    wl_list_init(&server->workspaces);
    server->workspace_count = 0;
    struct lw_workspace *ws = lw_workspace_create(server, "Desktop 1");
    server->active_workspace = ws;

    /* Add Wayland socket */
    server->socket = wl_display_add_socket_auto(server->wl_display);
    if (!server->socket) {
        wlr_log(WLR_ERROR, "Failed to create Wayland socket");
        return -1;
    }

    wlr_log(WLR_INFO, "Wayland compositor listening on %s", server->socket);
    setenv("WAYLAND_DISPLAY", server->socket, true);

    /* Initialize IPC socket for shell communication */
    if (lw_ipc_init(server) != 0) {
        wlr_log(WLR_ERROR, "Failed to initialize IPC (non-fatal)");
        /* IPC failure is non-fatal; shell just won't get shortcut events */
    }

    /* Initialize keyboard shortcut state */
    server->super_pressed = false;
    server->super_used_in_combo = false;

    return 0;
}

int lw_server_run(struct lw_server *server) {
    /* Start the backend (outputs, input devices) */
    if (!wlr_backend_start(server->backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        return -1;
    }

    wlr_log(WLR_INFO, "lwindesk compositor running");
    wl_display_run(server->wl_display);

    return 0;
}

void lw_server_destroy(struct lw_server *server) {
    wlr_log(WLR_INFO, "Shutting down compositor");
    lw_ipc_destroy(server);
    wl_display_destroy_clients(server->wl_display);
    wlr_scene_node_destroy(&server->scene->tree.node);
    wlr_xcursor_manager_destroy(server->cursor_mgr);
    wlr_cursor_destroy(server->cursor);
    wlr_allocator_destroy(server->allocator);
    wlr_renderer_destroy(server->renderer);
    wlr_backend_destroy(server->backend);
    wl_display_destroy(server->wl_display);
}
