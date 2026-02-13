/*
 * lwindesk - A Windows 11-like Wayland Desktop Environment
 * Copyright (C) 2026 Frank Kahle
 *
 * compositor/include/workspace.h - Virtual desktop (workspace) management
 */

#ifndef LWINDESK_WORKSPACE_H
#define LWINDESK_WORKSPACE_H

#include "server.h"

struct lw_workspace {
    struct wl_list link;                 /* lw_server.workspaces */
    struct lw_server *server;
    int index;
    char name[64];
    struct wl_list views;                /* views assigned to this workspace */
    struct wlr_scene_tree *scene_tree;   /* scene tree for this workspace */
};

/* Create a new workspace */
struct lw_workspace *lw_workspace_create(struct lw_server *server,
                                          const char *name);

/* Switch to a workspace (show its views, hide others) */
void lw_workspace_switch(struct lw_server *server, struct lw_workspace *ws);

/* Move a view to a workspace */
void lw_workspace_move_view(struct lw_view *view, struct lw_workspace *ws);

/* Get workspace by index */
struct lw_workspace *lw_workspace_get(struct lw_server *server, int index);

#endif /* LWINDESK_WORKSPACE_H */
