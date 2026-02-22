/*
 * lwindesk - compositor/src/ipc.c - IPC socket for shell communication
 *
 * Simple Unix domain socket server. The compositor listens, the shell
 * connects. Protocol is newline-delimited text commands:
 *   "toggle-start-menu\n"
 *   "show-desktop\n"
 *   "cycle-window\n"
 */

#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/util/log.h>

#include "ipc.h"
#include "server.h"

static int set_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) return -1;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static void ipc_client_disconnect(struct lw_ipc_client *client) {
	if (client->fd < 0) return;

	wlr_log(WLR_INFO, "IPC client disconnected (fd=%d)", client->fd);

	if (client->event_source) {
		wl_event_source_remove(client->event_source);
		client->event_source = NULL;
	}
	close(client->fd);
	client->fd = -1;

	/* Compact the clients array */
	struct lw_ipc *ipc = &client->server->ipc;
	int idx = client - ipc->clients;
	if (idx < ipc->client_count - 1) {
		memmove(&ipc->clients[idx], &ipc->clients[idx + 1],
			(ipc->client_count - 1 - idx) *
			sizeof(struct lw_ipc_client));
	}
	ipc->client_count--;
}

static int ipc_client_readable(int fd, uint32_t mask, void *data) {
	struct lw_ipc_client *client = data;

	if (mask & (WL_EVENT_HANGUP | WL_EVENT_ERROR)) {
		ipc_client_disconnect(client);
		return 0;
	}

	/* Read and discard data from client (we don't expect commands
	 * from the shell to the compositor via this channel yet) */
	char buf[512];
	ssize_t n = read(fd, buf, sizeof(buf));
	if (n <= 0) {
		ipc_client_disconnect(client);
		return 0;
	}

	return 0;
}

static int ipc_accept_client(int fd, uint32_t mask, void *data) {
	struct lw_server *server = data;
	struct lw_ipc *ipc = &server->ipc;

	int client_fd = accept(fd, NULL, NULL);
	if (client_fd < 0) {
		wlr_log(WLR_ERROR, "IPC accept failed: %s",
			strerror(errno));
		return 0;
	}

	if (ipc->client_count >= LW_IPC_MAX_CLIENTS) {
		wlr_log(WLR_ERROR, "IPC max clients reached, rejecting");
		close(client_fd);
		return 0;
	}

	if (set_nonblocking(client_fd) < 0) {
		wlr_log(WLR_ERROR, "IPC set_nonblocking failed");
		close(client_fd);
		return 0;
	}

	struct lw_ipc_client *client = &ipc->clients[ipc->client_count];
	client->fd = client_fd;
	client->server = server;

	struct wl_event_loop *loop =
		wl_display_get_event_loop(server->wl_display);
	client->event_source = wl_event_loop_add_fd(loop, client_fd,
		WL_EVENT_READABLE | WL_EVENT_HANGUP,
		ipc_client_readable, client);

	ipc->client_count++;
	wlr_log(WLR_INFO, "IPC client connected (fd=%d, total=%d)",
		client_fd, ipc->client_count);

	return 0;
}

int lw_ipc_init(struct lw_server *server) {
	struct lw_ipc *ipc = &server->ipc;
	memset(ipc, 0, sizeof(*ipc));
	ipc->listen_fd = -1;
	ipc->client_count = 0;

	/* Initialize all client slots */
	for (int i = 0; i < LW_IPC_MAX_CLIENTS; i++) {
		ipc->clients[i].fd = -1;
		ipc->clients[i].server = server;
	}

	/* Build socket path */
	const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
	if (!runtime_dir) {
		wlr_log(WLR_ERROR, "XDG_RUNTIME_DIR not set");
		return -1;
	}
	snprintf(ipc->socket_path, sizeof(ipc->socket_path),
		"%s/lwindesk-ipc", runtime_dir);

	/* Remove stale socket if it exists */
	unlink(ipc->socket_path);

	/* Create Unix domain socket */
	ipc->listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (ipc->listen_fd < 0) {
		wlr_log(WLR_ERROR, "IPC socket() failed: %s",
			strerror(errno));
		return -1;
	}

	if (set_nonblocking(ipc->listen_fd) < 0) {
		wlr_log(WLR_ERROR, "IPC set_nonblocking failed");
		close(ipc->listen_fd);
		ipc->listen_fd = -1;
		return -1;
	}

	struct sockaddr_un addr = {0};
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, ipc->socket_path,
		sizeof(addr.sun_path) - 1);

	if (bind(ipc->listen_fd, (struct sockaddr *)&addr,
		sizeof(addr)) < 0) {
		wlr_log(WLR_ERROR, "IPC bind(%s) failed: %s",
			ipc->socket_path, strerror(errno));
		close(ipc->listen_fd);
		ipc->listen_fd = -1;
		return -1;
	}

	if (listen(ipc->listen_fd, 4) < 0) {
		wlr_log(WLR_ERROR, "IPC listen() failed: %s",
			strerror(errno));
		close(ipc->listen_fd);
		ipc->listen_fd = -1;
		unlink(ipc->socket_path);
		return -1;
	}

	/* Integrate with Wayland event loop */
	struct wl_event_loop *loop =
		wl_display_get_event_loop(server->wl_display);
	ipc->listen_source = wl_event_loop_add_fd(loop, ipc->listen_fd,
		WL_EVENT_READABLE, ipc_accept_client, server);

	wlr_log(WLR_INFO, "IPC listening on %s", ipc->socket_path);
	return 0;
}

void lw_ipc_send(struct lw_server *server, const char *message) {
	struct lw_ipc *ipc = &server->ipc;
	if (ipc->client_count == 0) {
		wlr_log(WLR_DEBUG, "IPC send '%s' - no clients connected",
			message);
		return;
	}

	/* Build the newline-delimited message */
	char buf[256];
	int len = snprintf(buf, sizeof(buf), "%s\n", message);
	if (len < 0 || len >= (int)sizeof(buf)) return;

	wlr_log(WLR_DEBUG, "IPC sending '%s' to %d client(s)",
		message, ipc->client_count);

	/* Send to all connected clients. Iterate in reverse so
	 * disconnects during send don't skip entries. */
	for (int i = ipc->client_count - 1; i >= 0; i--) {
		struct lw_ipc_client *client = &ipc->clients[i];
		if (client->fd < 0) continue;

		ssize_t written = write(client->fd, buf, len);
		if (written < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				/* Client buffer full, skip this message */
				wlr_log(WLR_DEBUG,
					"IPC client %d buffer full", i);
			} else {
				/* Write error, disconnect client */
				ipc_client_disconnect(client);
			}
		}
	}
}

void lw_ipc_destroy(struct lw_server *server) {
	struct lw_ipc *ipc = &server->ipc;

	/* Disconnect all clients */
	for (int i = ipc->client_count - 1; i >= 0; i--) {
		ipc_client_disconnect(&ipc->clients[i]);
	}

	/* Close listener */
	if (ipc->listen_source) {
		wl_event_source_remove(ipc->listen_source);
		ipc->listen_source = NULL;
	}
	if (ipc->listen_fd >= 0) {
		close(ipc->listen_fd);
		ipc->listen_fd = -1;
	}

	/* Remove socket file */
	if (ipc->socket_path[0]) {
		unlink(ipc->socket_path);
	}

	wlr_log(WLR_INFO, "IPC destroyed");
}
