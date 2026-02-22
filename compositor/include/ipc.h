/*
 * lwindesk - A Windows 11-like Wayland Desktop Environment
 * Copyright (C) 2026 Frank Kahle
 *
 * compositor/include/ipc.h - IPC socket for shell communication
 */

#ifndef LWINDESK_IPC_H
#define LWINDESK_IPC_H

#include "server.h"

/* Initialize IPC socket at $XDG_RUNTIME_DIR/lwindesk-ipc */
int lw_ipc_init(struct lw_server *server);

/* Send a newline-delimited message to all connected IPC clients */
void lw_ipc_send(struct lw_server *server, const char *message);

/* Clean up IPC resources */
void lw_ipc_destroy(struct lw_server *server);

#endif /* LWINDESK_IPC_H */
