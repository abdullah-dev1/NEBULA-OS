#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "kernel/ipc_manager.h"

#define MAX_LOG_LINES 500
#define LINE_SIZE 256
#define PAGE_SIZE 15

int main() {
    send_resource_request("Log Viewer", 20, 0);
    if (!wait_for_grant()) {
        printf("Resource denied.\n");
        return 1;
    }

    char filter[64] = {0};
    int page = 0;

    while (1) {
        printf("\n--- SYSTEM LOG VIEWER ---\n");
        if (filter[0]) printf("  Filter: [%s]\n", filter);

        FILE* f = fopen("system.log", "r");
        char lines[MAX_LOG_LINES][LINE_SIZE];
        int count = 0;

        if (f) {
            char buf[LINE_SIZE];
            while (fgets(buf, LINE_SIZE, f) && count < MAX_LOG_LINES) {
                if (!filter[0] || strstr(buf, filter)) {
                    strncpy(lines[count], buf, LINE_SIZE - 1);
                    lines[count][strlen(buf)-1] = '\0'; 
                    count++;
                }
            }
            fclose(f);
        }

        int total_pages = (count + PAGE_SIZE - 1) / PAGE_SIZE;
        if (total_pages == 0) total_pages = 1;
        if (page >= total_pages) page = total_pages - 1;
        if (page < 0) page = 0;

        int start = page * PAGE_SIZE;
        int end = start + PAGE_SIZE;
        if (end > count) end = count;

        for (int i = start; i < end; i++) {
            printf("  %s\n", lines[i]);
        }

        printf("\n  Page %d/%d | Total: %d\n", page + 1, total_pages, count);
        printf("  [F]ilter [N]ext [P]rev [Q]uit: ");
        fflush(stdout);

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        struct timeval tv = {3, 0}; 

        int ret = select(STDIN_FILENO + 1, &fds, 0, 0, &tv);
        if (ret > 0) {
            char cmd[64];
            if (!fgets(cmd, 64, stdin)) break;
            char c = cmd[0];
            if (c == 'q' || c == 'Q') break;
            else if (c == 'n' || c == 'N') page++;
            else if (c == 'p' || c == 'P') page--;
            else if (c == 'f' || c == 'F') {
                printf("  Enter filter keyword: ");
                fgets(filter, 64, stdin);
                filter[strlen(filter)-1] = '\0';
                page = 0;
            }
        }
    }

    send_termination_notice(getpid());
    return 0;
}
