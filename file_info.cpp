#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "kernel/ipc_manager.h"

#define MAX_FILES 100
#define NAME_SIZE 64

int main() {
    send_resource_request("File Info", 10, 0);
    if (!wait_for_grant()) return 1;

    printf("\n--- NEBULA OS FILE INFO ---\n");
    printf("  (Press Ctrl+Z to Minimize)\n\n");

    char files[MAX_FILES][NAME_SIZE];
    int count = 0;

    DIR *dir = opendir("nebula_hdd");
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL && count < MAX_FILES) {
            if (entry->d_name[0] == '.') continue;
            strncpy(files[count], entry->d_name, NAME_SIZE - 1);
            count++;
        }
        closedir(dir);
    }

    if (count == 0) {
        printf("  No files found in nebula_hdd/\n");
    } else {
        printf("  Files in nebula_hdd/:\n");
        for (int i = 0; i < count; i++) printf("  [%d] %s\n", i + 1, files[i]);

        printf("\n  Select file (1-%d, 0 for all): ", count);
        fflush(stdout);
        char buf[16];
        fgets(buf, 16, stdin);
        int choice = atoi(buf);

        int start = (choice > 0) ? choice - 1 : 0;
        int end = (choice > 0) ? choice : count;

        for (int i = start; i < end; i++) {
            char path[128];
            sprintf(path, "nebula_hdd/%s", files[i]);
            struct stat st;
            if (stat(path, &st) == 0) {
                printf("\n  -- %s --\n", files[i]);
                printf("  Size:    %ld bytes\n", st.st_size);
                char time_buf[64];
                strftime(time_buf, 64, "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));
                printf("  Modified: %s\n", time_buf);
            }
        }
    }

    printf("\n  Press Enter to exit...");
    fflush(stdout);
    getchar();

    send_termination_notice(getpid());
    return 0;
}
