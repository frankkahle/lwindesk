/*
 * lwindesk - compositor/src/view.c - Window (view) management
 *
 * Server-side decorations are rendered using cairo onto a pixel buffer
 * that is displayed via a wlr_scene_buffer node.  This gives us proper
 * Windows 11-style dark title bars with text and button symbols instead
 * of plain colored rectangles.
 */

#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <drm_fourcc.h>
#include <wlr/interfaces/wlr_buffer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "view.h"
#include "server.h"

/* Tag stored in the titlebar scene_buffer node->data so lw_view_at
 * and lw_deco_button_at can recognise it as a decoration. */
#define DECO_TAG_TITLEBAR  ((void *)(uintptr_t)1)

/* --- Custom pixel buffer implementation for wlr_buffer --- */

struct lw_pixel_buffer {
	struct wlr_buffer base;
	void *data;
	size_t stride;
};

static void pixel_buffer_destroy(struct wlr_buffer *wlr_buf) {
	struct lw_pixel_buffer *buf = wl_container_of(wlr_buf, buf, base);
	free(buf->data);
	free(buf);
}

static bool pixel_buffer_begin_data_ptr_access(struct wlr_buffer *wlr_buf,
		uint32_t flags, void **data, uint32_t *format, size_t *stride) {
	struct lw_pixel_buffer *buf = wl_container_of(wlr_buf, buf, base);
	*data = buf->data;
	*format = DRM_FORMAT_ARGB8888;
	*stride = buf->stride;
	return true;
}

static void pixel_buffer_end_data_ptr_access(struct wlr_buffer *wlr_buf) {
	/* no-op */
}

static const struct wlr_buffer_impl pixel_buffer_impl = {
	.destroy = pixel_buffer_destroy,
	.begin_data_ptr_access = pixel_buffer_begin_data_ptr_access,
	.end_data_ptr_access = pixel_buffer_end_data_ptr_access,
};

/* --- Cairo title bar rendering --- */

/*
 * Render a Windows 11 dark-theme title bar into a new wlr_buffer.
 *
 * Layout (right-to-left from the right edge):
 *   [0..width-138]  title text area  (#2b2b2b background)
 *   [width-138]     minimize button  (#2b2b2b, ─ symbol)
 *   [width-92]      maximize button  (#2b2b2b, □ symbol)
 *   [width-46]      close button     (#c42b1c, × symbol)
 *
 * Each button is 46px wide, the full titlebar is 32px tall.
 * Symbols and text are white.
 */
static struct wlr_buffer *render_titlebar(int width, int height,
		const char *title) {
	if (width <= 0 || height <= 0)
		return NULL;

	size_t stride = (size_t)width * 4;
	void *data = calloc(1, stride * height);
	if (!data)
		return NULL;

	cairo_surface_t *surface = cairo_image_surface_create_for_data(
		data, CAIRO_FORMAT_ARGB32, width, height, stride);
	cairo_t *cr = cairo_create(surface);

	/* Background (#2b2b2b) */
	cairo_set_source_rgb(cr, 0.169, 0.169, 0.169);
	cairo_paint(cr);

	/* Close button red background (#c42b1c) */
	cairo_set_source_rgb(cr, 0.769, 0.169, 0.110);
	cairo_rectangle(cr, width - LW_DECO_BUTTON_WIDTH, 0,
		LW_DECO_BUTTON_WIDTH, height);
	cairo_fill(cr);

	/* Button symbols — thin white strokes */
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_set_line_width(cr, 1.0);

	int cy = height / 2;

	/* Close button: × centered in rightmost 46px */
	{
		int cx = width - LW_DECO_BUTTON_WIDTH / 2;
		cairo_move_to(cr, cx - 5, cy - 5);
		cairo_line_to(cr, cx + 5, cy + 5);
		cairo_move_to(cr, cx + 5, cy - 5);
		cairo_line_to(cr, cx - 5, cy + 5);
		cairo_stroke(cr);
	}

	/* Maximize button: □ centered in second-from-right 46px */
	{
		int mx = width - LW_DECO_BUTTON_WIDTH - LW_DECO_BUTTON_WIDTH / 2;
		cairo_rectangle(cr, mx - 5, cy - 5, 10, 10);
		cairo_stroke(cr);
	}

	/* Minimize button: ─ centered in third-from-right 46px */
	{
		int nx = width - 2 * LW_DECO_BUTTON_WIDTH - LW_DECO_BUTTON_WIDTH / 2;
		cairo_move_to(cr, nx - 5, cy);
		cairo_line_to(cr, nx + 5, cy);
		cairo_stroke(cr);
	}

	/* Title text (white, left-aligned, ~12px Sans) */
	if (title && title[0]) {
		PangoLayout *layout = pango_cairo_create_layout(cr);
		PangoFontDescription *font =
			pango_font_description_from_string("Sans 11");
		pango_layout_set_font_description(layout, font);
		pango_layout_set_text(layout, title, -1);

		/* Clip text so it never overlaps the three buttons */
		int max_text_width = width - 3 * LW_DECO_BUTTON_WIDTH - 16;
		if (max_text_width > 0) {
			pango_layout_set_width(layout,
				max_text_width * PANGO_SCALE);
			pango_layout_set_ellipsize(layout,
				PANGO_ELLIPSIZE_END);
		}

		cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.9);

		int text_w, text_h;
		pango_layout_get_pixel_size(layout, &text_w, &text_h);
		cairo_move_to(cr, 12, (height - text_h) / 2);
		pango_cairo_show_layout(cr, layout);

		pango_font_description_free(font);
		g_object_unref(layout);
	}

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	/* Wrap the pixel data in a wlr_buffer */
	struct lw_pixel_buffer *buf = calloc(1, sizeof(*buf));
	if (!buf) {
		free(data);
		return NULL;
	}
	buf->data = data;
	buf->stride = stride;
	wlr_buffer_init(&buf->base, &pixel_buffer_impl, width, height);

	return &buf->base;
}

/* --- Server-side decoration (SSD) implementation --- */

/*
 * Create server-side decoration nodes for a non-shell view.
 *
 * Architecture:
 *   view->scene_tree (wrapper, node.data = view)
 *     +-- xdg_surface_tree (surface content, offset to y=TITLEBAR_HEIGHT)
 *     +-- titlebar_buffer  (cairo-rendered title bar at y=0)
 *
 * The wrapper tree is created by reparenting the existing xdg surface
 * tree into a new parent, then adding the rendered titlebar as a sibling.
 */
void lw_view_create_decorations(struct lw_view *view) {
	if (view->deco.has_decorations) return;

	struct lw_server *server = view->server;

	/*
	 * Create a wrapper scene tree. The current scene_tree (from
	 * wlr_scene_xdg_surface_create) will be reparented into it.
	 */
	struct wlr_scene_tree *wrapper =
		wlr_scene_tree_create(&server->scene->tree);
	if (!wrapper) {
		wlr_log(WLR_ERROR, "Failed to create decoration wrapper tree");
		return;
	}

	/* Copy current position from the xdg surface tree to the wrapper */
	wlr_scene_node_set_position(&wrapper->node,
		view->scene_tree->node.x, view->scene_tree->node.y);

	/* Reparent the xdg surface tree into the wrapper */
	struct wlr_scene_tree *xdg_tree = view->scene_tree;
	wlr_scene_node_reparent(&xdg_tree->node, wrapper);
	wlr_scene_node_set_position(&xdg_tree->node, 0, LW_TITLEBAR_HEIGHT);

	/* Move the view data pointer from old tree to wrapper */
	xdg_tree->node.data = NULL;
	wrapper->node.data = view;
	view->scene_tree = wrapper;

	/* Get the current surface width for sizing decorations */
	struct wlr_box geo;
	wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo);
	int width = geo.width > 0 ? geo.width : 640;

	/* Get the window title */
	const char *title = view->xdg_toplevel->title;

	/* Render the title bar */
	struct wlr_buffer *wlr_buf =
		render_titlebar(width, LW_TITLEBAR_HEIGHT, title);
	if (!wlr_buf) {
		wlr_log(WLR_ERROR, "Failed to render titlebar");
		return;
	}

	/* Create a scene buffer node and attach the rendered image */
	struct wlr_scene_buffer *scene_buf = wlr_scene_buffer_create(wrapper, NULL);
	if (!scene_buf) {
		wlr_buffer_drop(wlr_buf);
		wlr_log(WLR_ERROR, "Failed to create titlebar scene buffer");
		return;
	}
	wlr_scene_buffer_set_buffer(scene_buf, wlr_buf);
	wlr_scene_node_set_position(&scene_buf->node, 0, 0);

	/* Tag the node so hit-testing knows it is a decoration */
	scene_buf->node.data = DECO_TAG_TITLEBAR;

	/* Store references for later update/destroy */
	view->deco.titlebar_buffer = scene_buf;
	view->deco.titlebar_wlr_buffer = wlr_buf;
	view->deco.width = width;
	view->deco.cached_title = title ? strdup(title) : NULL;
	view->deco.has_decorations = true;

	/* The scene now holds a reference to the buffer.  We keep our own
	 * reference (from wlr_buffer_init) so we can drop it in destroy. */

	wlr_log(WLR_INFO, "Created SSD for view (width=%d, title=%s)",
		width, title ? title : "(none)");
}

/*
 * Destroy decoration scene nodes and clear the wrapper data pointer.
 * Must be called before freeing the view to prevent use-after-free
 * when the cursor hits orphaned decoration nodes.
 */
void lw_view_destroy_decorations(struct lw_view *view) {
	if (!view->deco.has_decorations) return;

	/* Destroy the scene buffer node (removes from scene graph) */
	if (view->deco.titlebar_buffer) {
		wlr_scene_node_destroy(&view->deco.titlebar_buffer->node);
		view->deco.titlebar_buffer = NULL;
	}

	/* Drop our reference to the wlr_buffer (frees pixel data) */
	if (view->deco.titlebar_wlr_buffer) {
		wlr_buffer_drop(view->deco.titlebar_wlr_buffer);
		view->deco.titlebar_wlr_buffer = NULL;
	}

	free(view->deco.cached_title);
	view->deco.cached_title = NULL;
	view->deco.width = 0;
	view->deco.has_decorations = false;

	/* Clear wrapper data so it won't be found by view_at / deco_button_at */
	if (view->scene_tree)
		view->scene_tree->node.data = NULL;
}

/*
 * Update decoration to match the current window width and title.
 * Called on every surface commit and when the title changes.
 * If the width (or title) changed, we re-render the entire titlebar.
 */
void lw_view_update_decorations(struct lw_view *view) {
	if (!view->deco.has_decorations) return;

	struct wlr_box geo;
	wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo);
	int width = geo.width;
	if (width <= 0) return;

	const char *title = view->xdg_toplevel->title;

	/* Skip re-render if nothing changed */
	const char *cached = view->deco.cached_title;
	bool title_changed = (title && cached && strcmp(title, cached) != 0) ||
	                      (title && !cached) || (!title && cached);
	if (width == view->deco.width && !title_changed) {
		return;
	}

	/* Render a new titlebar buffer */
	struct wlr_buffer *new_buf =
		render_titlebar(width, LW_TITLEBAR_HEIGHT, title);
	if (!new_buf) return;

	/* Swap the buffer on the existing scene node */
	wlr_scene_buffer_set_buffer(view->deco.titlebar_buffer, new_buf);

	/* Drop the old buffer */
	if (view->deco.titlebar_wlr_buffer) {
		wlr_buffer_drop(view->deco.titlebar_wlr_buffer);
	}
	view->deco.titlebar_wlr_buffer = new_buf;
	view->deco.width = width;

	/* Cache the title for change detection */
	free(view->deco.cached_title);
	view->deco.cached_title = title ? strdup(title) : NULL;
}

/*
 * Check if the cursor is over a decoration button using coordinate-based
 * hit testing.  We find whatever scene node is under the cursor; if it
 * is our tagged titlebar buffer, we figure out the button from the X
 * coordinate within the titlebar.
 *
 * Returns the button type and sets *out_view to the owning view.
 */
enum lw_deco_button lw_deco_button_at(struct lw_server *server,
                                        double lx, double ly,
                                        struct lw_view **out_view) {
	double sx, sy;
	struct wlr_scene_node *node =
		wlr_scene_node_at(&server->scene->tree.node, lx, ly, &sx, &sy);

	if (!node) return LW_DECO_NONE;

	/* Check if this node is our tagged titlebar buffer */
	if (node->type == WLR_SCENE_NODE_BUFFER &&
			node->data == DECO_TAG_TITLEBAR) {
		/* Walk up to find the view */
		struct wlr_scene_tree *tree = node->parent;
		while (tree && !tree->node.data) {
			tree = tree->node.parent;
		}
		struct lw_view *view = tree ? tree->node.data : NULL;
		if (!view || !view->deco.has_decorations) {
			return LW_DECO_NONE;
		}

		if (out_view) {
			*out_view = view;
		}

		/*
		 * sx is the local X coordinate within the buffer.
		 * The titlebar is view->deco.width pixels wide.
		 * Buttons are laid out from the right edge:
		 *   Close:    [width-46 .. width]
		 *   Maximize: [width-92 .. width-46]
		 *   Minimize: [width-138 .. width-92]
		 */
		int w = view->deco.width;
		int ix = (int)sx;

		if (ix >= w - LW_DECO_BUTTON_WIDTH) {
			return LW_DECO_CLOSE;
		} else if (ix >= w - 2 * LW_DECO_BUTTON_WIDTH) {
			return LW_DECO_MAXIMIZE;
		} else if (ix >= w - 3 * LW_DECO_BUTTON_WIDTH) {
			return LW_DECO_MINIMIZE;
		}

		/* Anywhere else on the titlebar */
		return LW_DECO_TITLEBAR;
	}

	return LW_DECO_NONE;
}

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

    /* If the view has decorations, the title bar consumes space from the
     * snap target. The scene_tree position includes the title bar, and
     * the surface is offset by LW_TITLEBAR_HEIGHT inside it. */
    int surface_height = target.height;
    if (view->deco.has_decorations) {
        surface_height -= LW_TITLEBAR_HEIGHT;
        if (surface_height < 1) surface_height = 1;
    }

    wlr_xdg_toplevel_set_size(view->xdg_toplevel,
        target.width, surface_height);
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
    if (!node) return NULL;

    /*
     * If we hit a RECT node (e.g. desktop background gradient strips),
     * walk up to find a view.  The surface pointer is NULL since these
     * are not Wayland surfaces.
     */
    if (node->type == WLR_SCENE_NODE_RECT) {
        *surface = NULL;
        struct wlr_scene_tree *tree = node->parent;
        while (tree && !tree->node.data) {
            tree = tree->node.parent;
        }
        return tree ? tree->node.data : NULL;
    }

    if (node->type != WLR_SCENE_NODE_BUFFER) {
        return NULL;
    }

    /*
     * If this buffer node is our titlebar decoration (tagged with
     * DECO_TAG_TITLEBAR), treat it like a decoration hit: return
     * the view but set surface to NULL.
     */
    if (node->data == DECO_TAG_TITLEBAR) {
        *surface = NULL;
        struct wlr_scene_tree *tree = node->parent;
        while (tree && !tree->node.data) {
            tree = tree->node.parent;
        }
        return tree ? tree->node.data : NULL;
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
