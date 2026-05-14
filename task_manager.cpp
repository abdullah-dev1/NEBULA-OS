#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include "kernel/ipc_manager.h"

#define MAX_LOG_DISPLAY 15
#define LINE_SIZE 256

void display_task_manager() {
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                   NEBULA OS TASK MANAGER                  ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    
    FILE* f = fopen("system.log", "r");
    if (!f) {
        printf("  [Error] Cannot read system.log\n");
        return;
    }

    char lines[100][LINE_SIZE];
    int count = 0;
    char buf[LINE_SIZE];
    while (fgets(buf, LINE_SIZE, f)) {
        if (strstr(buf, "Process") || strstr(buf, "Resources") || strstr(buf, "Context")) {
            strncpy(lines[count % 100], buf, LINE_SIZE - 1);
            count++;
        }
    }
    fclose(f);

    printf("  Recent System Events:\n");
    printf("  ──────────────────────────────────────────────────────────\n");
    int start = (count > MAX_LOG_DISPLAY) ? (count - MAX_LOG_DISPLAY) : 0;
    for (int i = start; i < count; i++) {
        printf("  %s", lines[i % 100]);
    }
    printf("  ──────────────────────────────────────────────────────────\n");
}

int main() {
    send_resource_request("Task Manager", 50, 5);
    if (!wait_for_grant()) return 1;

    while (1) {
        system("clear");
        display_task_manager();
        printf("\n  [K] Kill Process  [P] Change Priority  [Q] Quit\n");
        printf("  TaskManager> ");
        fflush(stdout);

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        struct timeval tv = {2, 0};

        int ret = select(STDIN_FILENO + 1, &fds, 0, 0, &tv);
        if (ret > 0) {
            char cmd[64];
            if (!fgets(cmd, 64, stdin)) break;
            char c = cmd[0];
            if (c == 'q' || c == 'Q') break;
            else if (c == 'k' || c == 'K') {
                printf("  Enter PID to kill: ");
                char pid_buf[16];
                if (fgets(pid_buf, 16, stdin)) {
                    int pid = atoi(pid_buf);
                    if (pid > 0) {
                        kill(pid, SIGTERM);
                        printf("  [System] SIGTERM sent to PID %d\n", pid);
                        sleep(1);
                    }
                }
            }
            else if (c == 'p' || c == 'P') {
                printf("  Enter PID: ");
                char pid_buf[16];
                if (fgets(pid_buf, 16, stdin)) {
                    int pid = atoi(pid_buf);
                    printf("  Enter new priority (0-10): ");
                    char pri_buf[16];
                    if (fgets(pri_buf, 16, stdin)) {
                        int pri = atoi(pri_buf);
                        printf("  [System] Priority change to %d requested for PID %d\n", pri, pid);
                        sleep(1);
                    }
                }
            }
        }
    }

    send_termination_notice(getpid());
    return 0;
}
