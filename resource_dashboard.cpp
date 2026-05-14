#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "kernel/ipc_manager.h"

#define LINE_SIZE 256

void draw_bar(const char* label, int used, int total, int width) {
    float ratio = (total > 0) ? (float)used / (float)total : 0;
    if (ratio > 1.0f) ratio = 1.0f;
    int filled = (int)(ratio * width);
    printf("  %-4s [", label);
    for (int i = 0; i < width; i++) printf(i < filled ? "█" : "░");
    printf("] %d%% (%d/%d MB)\n", (int)(ratio * 100), used, total);
}

int main() {
    send_resource_request("Resource Dashboard", 40, 3);
    if (!wait_for_grant()) return 1;

    while (1) {
        system("clear");
        printf("╔═══════════════════════════════════════════════════════════╗\n");
        printf("║                  NEBULA OS RESOURCE MONITOR               ║\n");
        printf("╚═══════════════════════════════════════════════════════════╝\n");
        printf("  (Auto-refreshes every 2s | Press 'q' to quit)\n\n");

        int total_ram = 1024, free_ram = 1024;
        int total_hdd = 1024, free_hdd = 1024;
        int active_p = 0;

        FILE* f = fopen("system.log", "r");
        if (f) {
            char buf[LINE_SIZE];
            while (fgets(buf, LINE_SIZE, f)) {
                if (strstr(buf, "Resources: RAM=")) {
                    sscanf(strstr(buf, "RAM="), "RAM=%dMB, HDD=%dMB", &total_ram, &total_hdd);
                    free_ram = total_ram;
                    free_hdd = total_hdd;
                }
                if (strstr(buf, "Resources allocated: RAM=")) {
                    int r, h;
                    sscanf(strstr(buf, "RAM="), "RAM=%d, HDD=%d", &r, &h);
                    free_ram -= r; free_hdd -= h;
                }
                if (strstr(buf, "Resources reclaimed: RAM=")) {
                    int r, h;
                    sscanf(strstr(buf, "RAM="), "RAM=%d, HDD=%d", &r, &h);
                    free_ram += r; free_hdd += h;
                }
                if (strstr(buf, "Process created")) active_p++;
                if (strstr(buf, "Process terminated")) active_p--;
            }
            fclose(f);
        }

        if (free_ram < 0) free_ram = 0;
        if (free_hdd < 0) free_hdd = 0;
        if (active_p < 0) active_p = 0;

        draw_bar("RAM", total_ram - free_ram, total_ram, 30);
        draw_bar("HDD", total_hdd - free_hdd, total_hdd, 30);
        
        printf("\n  Active Processes: %d\n", active_p);
        printf("  System Status:    NORMAL\n");
        printf("\n  NebulaOS> ");
        fflush(stdout);

        fd_set fds;
        struct timeval tv = {2, 0};
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
            char cmd[16];
            if (fgets(cmd, 16, stdin) && (cmd[0] == 'q' || cmd[0] == 'Q')) break;
        }
    }

    send_termination_notice(getpid());
    return 0;
}
