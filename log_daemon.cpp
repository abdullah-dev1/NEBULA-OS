#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include "kernel/ipc_manager.h"

static int daemon_running = 1;
static const long MAX_LOG_SIZE = 1024 * 1024;

void handle_signal(int sig) {
    (void)sig;
    daemon_running = 0;
}

void rotate_log(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return;

    char lines[100][256];
    int count = 0;
    char buf[256];
    while (fgets(buf, 256, f)) {
        strncpy(lines[count % 100], buf, 255);
        count++;
    }
    fclose(f);

    f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "[Log Daemon] Log rotated\n");
    int start = (count > 100) ? (count % 100) : 0;
    int total = (count > 100) ? 100 : count;
    for (int i = 0; i < total; i++) {
        fprintf(f, "%s", lines[(start + i) % 100]);
    }
    fclose(f);
}

int main() {
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    send_resource_request("Log Daemon", 5, 1);
    if (!wait_for_grant()) return 1;

    while (daemon_running) {
        struct stat st;
        if (stat("system.log", &st) == 0 && st.st_size > MAX_LOG_SIZE) {
            rotate_log("system.log");
        }
        for (int i = 0; i < 10 && daemon_running; i++) sleep(1);
    }

    send_termination_notice(getpid());
    return 0;
}
