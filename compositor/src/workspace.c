/*
 * lwindesk - compositor/src/workspace.c - Virtual desktop management
 */

#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <string.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>

#include "workspace.h"
#include "server.h"
#include "view.h"

struct lw_workspace *lw_workspace_create(struct lw_server *server,
                                          const char *name) {
    struct lw_workspace *ws = calloc(1, sizeof(*ws));
    ws->server = server;
    ws->index = server->workspace_count++;
    strncpy(ws->name, name, sizeof(ws->name) - 1);
    wl_list_init(&ws->views);
    ws->scene_tree = wlr_scene_tree_create(&server->scene->tree);

    wl_list_insert(&server->workspaces, &ws->link);

    wlr_log(WLR_INFO, "Created workspace %d: %s", ws->index, ws->name);
    return ws;
}

void lw_workspace_switch(struct lw_server *server, struct lw_workspace *ws) {
    if (server->active_workspace == ws) return;

    /* Hide current workspace */
    if (server->active_workspace) {
        wlr_scene_node_set_enabled(
            &server->active_workspace->scene_tree->node, false);
    }

    /* Show new workspace */
    wlr_scene_node_set_enabled(&ws->scene_tree->node, true);
    server->active_workspace = ws;

    wlr_log(WLR_INFO, "Switched to workspace %d: %s", ws->index, ws->name);
}

void lw_workspace_move_view(struct lw_view *view, struct lw_workspace *ws) {
    view->workspace = ws;
    wlr_scene_node_reparent(&view->scene_tree->node, ws->scene_tree);
}

struct lw_workspace *lw_workspace_get(struct lw_server *server, int index) {
    struct lw_workspace *ws;
    wl_list_for_each(ws, &server->workspaces, link) {
        if (ws->index == index) return ws;
    }
    return NULL;
}
