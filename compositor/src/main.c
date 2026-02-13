/*
 * lwindesk - A Windows 11-like Wayland Desktop Environment
 * Copyright (C) 2026 Frank Kahle
 *
 * compositor/src/main.c - Compositor entry point
 */

#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wlr/util/log.h>

#include "server.h"

static void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s [-s startup-command] [-d]\n", prog);
    fprintf(stderr, "  -s  Command to run at startup (e.g., lwindesk-shell)\n");
    fprintf(stderr, "  -d  Enable debug logging\n");
}

int main(int argc, char *argv[]) {
    char *startup_cmd = NULL;
    enum wlr_log_importance log_level = WLR_INFO;
    int opt;

    while ((opt = getopt(argc, argv, "s:dh")) != -1) {
        switch (opt) {
        case 's':
            startup_cmd = optarg;
            break;
        case 'd':
            log_level = WLR_DEBUG;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    wlr_log_init(log_level, NULL);
    wlr_log(WLR_INFO, "lwindesk compositor v0.1.0 starting");

    struct lw_server server = {0};

    if (lw_server_init(&server) != 0) {
        wlr_log(WLR_ERROR, "Failed to initialize server");
        return 1;
    }

    /* Launch startup command (the shell) if provided */
    if (startup_cmd) {
        wlr_log(WLR_INFO, "Launching startup command: %s", startup_cmd);
        if (fork() == 0) {
            execl("/bin/sh", "/bin/sh", "-c", startup_cmd, NULL);
            perror("execl");
            _exit(1);
        }
    }

    /* Run the compositor event loop */
    int ret = lw_server_run(&server);

    /* Cleanup */
    lw_server_destroy(&server);

    return ret;
}
