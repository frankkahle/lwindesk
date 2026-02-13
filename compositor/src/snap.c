/*
 * lwindesk - compositor/src/snap.c - Windows 11-style snap zone detection
 */

#define _POSIX_C_SOURCE 200112L
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>

#include "snap.h"
#include "server.h"

/* Edge threshold in pixels for snap zone detection */
#define SNAP_EDGE_THRESHOLD 16
#define SNAP_CORNER_THRESHOLD 48

enum lw_snap_zone lw_snap_zone_at(struct lw_server *server,
                                    double cursor_x, double cursor_y) {
    struct wlr_output *output =
        wlr_output_layout_output_at(server->output_layout,
                                     cursor_x, cursor_y);
    if (!output) return LW_SNAP_NONE;

    struct wlr_box box;
    wlr_output_layout_get_box(server->output_layout, output, &box);

    double rx = cursor_x - box.x;
    double ry = cursor_y - box.y;

    bool at_left = rx < SNAP_EDGE_THRESHOLD;
    bool at_right = rx > box.width - SNAP_EDGE_THRESHOLD;
    bool at_top = ry < SNAP_EDGE_THRESHOLD;
    bool near_top_corner = ry < SNAP_CORNER_THRESHOLD;
    bool near_bottom_corner = ry > box.height - SNAP_CORNER_THRESHOLD;

    /* Top edge = maximize */
    if (at_top && !at_left && !at_right) {
        return LW_SNAP_MAXIMIZE;
    }

    /* Left edge with corners */
    if (at_left) {
        if (near_top_corner) return LW_SNAP_TOP_LEFT;
        if (near_bottom_corner) return LW_SNAP_BOTTOM_LEFT;
        return LW_SNAP_LEFT;
    }

    /* Right edge with corners */
    if (at_right) {
        if (near_top_corner) return LW_SNAP_TOP_RIGHT;
        if (near_bottom_corner) return LW_SNAP_BOTTOM_RIGHT;
        return LW_SNAP_RIGHT;
    }

    return LW_SNAP_NONE;
}

struct wlr_box lw_snap_zone_geometry(struct wlr_output *output,
                                      enum lw_snap_zone zone) {
    struct wlr_box box = {0};
    /* This would be filled using output layout - used by shell overlay */
    (void)output;
    (void)zone;
    return box;
}

const char *lw_snap_zone_name(enum lw_snap_zone zone) {
    switch (zone) {
    case LW_SNAP_NONE:         return "none";
    case LW_SNAP_LEFT:         return "left";
    case LW_SNAP_RIGHT:        return "right";
    case LW_SNAP_TOP_LEFT:     return "top-left";
    case LW_SNAP_TOP_RIGHT:    return "top-right";
    case LW_SNAP_BOTTOM_LEFT:  return "bottom-left";
    case LW_SNAP_BOTTOM_RIGHT: return "bottom-right";
    case LW_SNAP_MAXIMIZE:     return "maximize";
    }
    return "unknown";
}
