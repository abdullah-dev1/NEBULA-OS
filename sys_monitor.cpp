#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "kernel/ipc_manager.h"

static int daemon_running = 1;

void handle_signal(int sig) {
    (void)sig;
    daemon_running = 0;
}

void write_snapshot() {
    time_t now = time(0);
    struct tm* t = localtime(&now);
    char time_buf[20];
    strftime(time_buf, 20, "%Y-%m-%d %H:%M:%S", t);

    int active_count = 0;
    FILE *fp = popen("ls /tmp/nebula_pipe_* 2>/dev/null | wc -l", "r");
    if (fp) {
        char buf[32];
        if (fgets(buf, sizeof(buf), fp)) active_count = atoi(buf);
        pclose(fp);
    }

    FILE* f = fopen("system.log", "a");
    if (f) {
        fprintf(f, "[%s] [SysMonitor] Active tasks: %d | PID: %d\n", time_buf, active_count, getpid());
        fclose(f);
    }
}

int main() {
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    send_resource_request("System Monitor", 5, 1);
    if (!wait_for_grant()) return 1;

    while (daemon_running) {
        write_snapshot();
        for (int i = 0; i < 5 && daemon_running; i++) sleep(1);
    }

    send_termination_notice(getpid());
    return 0;
}
