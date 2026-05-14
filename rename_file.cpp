#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "kernel/ipc_manager.h"

#define MAX_FILES 100
#define NAME_SIZE 64

int main() {
    send_resource_request("Rename File", 15, 2);
    if (!wait_for_grant()) return 1;

    printf("\n--- NEBULA OS RENAME FILE ---\n");
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

        printf("\n  Select file to rename (number): ");
        fflush(stdout);
        char buf[16];
        fgets(buf, 16, stdin);
        int choice = atoi(buf);

        if (choice >= 1 && choice <= count) {
            printf("  Enter new name: ");
            fflush(stdout);
            char new_name[64];
            if (fgets(new_name, 64, stdin)) {
                new_name[strlen(new_name)-1] = '\0';
                char old_path[128], new_path[128];
                sprintf(old_path, "nebula_hdd/%s", files[choice-1]);
                sprintf(new_path, "nebula_hdd/%s", new_name);

                struct stat st;
                if (stat(new_path, &st) == 0) printf("  Error: New name already exists.\n");
                else if (rename(old_path, new_path) == 0) printf("  File renamed.\n");
                else printf("  Error renaming file.\n");
            }
        }
    }

    printf("\n  Press Enter to exit...");
    fflush(stdout);
    getchar();

    send_termination_notice(getpid());
    return 0;
}
