#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include "kernel/ipc_manager.h"

int main() {
    send_resource_request("Text Search", 25, 0);
    if (!wait_for_grant()) return 1;

    printf("\n--- NEBULA OS TEXT SEARCH ---\n");
    printf("  (Press Ctrl+Z to Minimize)\n\n");

    printf("  Enter keyword to search: ");
    fflush(stdout);
    char keyword[64];
    if (!fgets(keyword, 64, stdin)) return 1;
    keyword[strlen(keyword)-1] = '\0';

    if (strlen(keyword) == 0) {
        printf("  Error: Keyword cannot be empty.\n");
    } else {
        DIR *dir = opendir("nebula_hdd");
        if (dir) {
            struct dirent *entry;
            int total = 0;
            while ((entry = readdir(dir)) != NULL) {
                if (strstr(entry->d_name, ".txt")) {
                    char path[256];
                    snprintf(path, sizeof(path), "nebula_hdd/%s", entry->d_name);
                    FILE* f = fopen(path, "r");
                    if (f) {
                        char line[256];
                        int line_num = 0;
                        int shown_header = 0;
                        while (fgets(line, 256, f)) {
                            line_num++;
                            if (strcasestr(line, keyword)) {
                                if (!shown_header) {
                                    printf("\n  -- %s --\n", entry->d_name);
                                    shown_header = 1;
                                }
                                printf("    Line %d: %s", line_num, line);
                                total++;
                            }
                        }
                        fclose(f);
                    }
                }
            }
            closedir(dir);
            printf("\n  Total matches: %d\n", total);
        }
    }

    printf("\n  Press Enter to exit...");
    fflush(stdout);
    getchar();

    send_termination_notice(getpid());
    return 0;
}
