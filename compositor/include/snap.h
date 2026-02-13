/*
 * lwindesk - A Windows 11-like Wayland Desktop Environment
 * Copyright (C) 2026 Frank Kahle
 *
 * compositor/include/snap.h - Windows 11-style snap layout zones
 */

#ifndef LWINDESK_SNAP_H
#define LWINDESK_SNAP_H

#include "server.h"

/* Determine which snap zone the cursor is in based on position */
enum lw_snap_zone lw_snap_zone_at(struct lw_server *server,
                                    double cursor_x, double cursor_y);

/* Get the geometry for a snap zone on a given output */
struct wlr_box lw_snap_zone_geometry(struct wlr_output *output,
                                      enum lw_snap_zone zone);

/* Snap zone names (for IPC / shell communication) */
const char *lw_snap_zone_name(enum lw_snap_zone zone);

#endif /* LWINDESK_SNAP_H */
