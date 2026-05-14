#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "kernel/ipc_manager.h"

#define MAX_FILES 100
#define NAME_SIZE 64

int main() {
    send_resource_request("Print File", 25, 10);
    if (!wait_for_grant()) return 1;

    printf("\n--- NEBULA OS PRINT FILE ---\n");
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

        printf("\n  Select file to print (number): ");
        fflush(stdout);
        char buf[16];
        fgets(buf, 16, stdin);
        int choice = atoi(buf);

        if (choice >= 1 && choice <= count) {
            printf("\n  Preparing to print: %s\n", files[choice-1]);
            for (int p = 0; p <= 100; p += 10) {
                printf("\r  Printing... [%d%%] ", p);
                fflush(stdout);
                usleep(100000);
            }
            printf("\n\n  --- Printed Output ---\n");
            char path[128];
            sprintf(path, "nebula_hdd/%s", files[choice-1]);
            FILE* f = fopen(path, "r");
            if (f) {
                char line[256];
                while (fgets(line, 256, f)) printf("  %s", line);
                fclose(f);
            }
            printf("\n  --- End of Print ---\n");
        }
    }

    printf("\n  Press Enter to exit...");
    fflush(stdout);
    getchar();

    send_termination_notice(getpid());
    return 0;
}
