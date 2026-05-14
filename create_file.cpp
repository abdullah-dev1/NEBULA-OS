#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "kernel/ipc_manager.h"

int main() {
    send_resource_request("Create File", 15, 5);
    if (!wait_for_grant()) return 1;

    printf("\n--- NEBULA OS CREATE FILE ---\n");
    printf("  (Press Ctrl+Z to Minimize)\n\n");

    printf("  Enter filename to create: ");
    fflush(stdout);
    char filename[64];
    if (!fgets(filename, 64, stdin)) return 1;
    filename[strlen(filename)-1] = '\0'; 

    if (strlen(filename) == 0) {
        printf("  Error: Filename cannot be empty.\n");
    } else {
        char path[128];
        sprintf(path, "nebula_hdd/%s", filename);

        struct stat st;
        if (stat(path, &st) == 0) {
            printf("  Error: File already exists.\n");
        } else {
            FILE* f = fopen(path, "w");
            if (f) {
                fclose(f);
                printf("  File '%s' created successfully.\n", filename);
            } else {
                printf("  Error: Could not create file.\n");
            }
        }
    }

    printf("\n  Press Enter to exit...");
    fflush(stdout);
    getchar();

    send_termination_notice(getpid());
    return 0;
}
