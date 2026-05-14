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
    send_resource_request("Move File", 20, 50);
    if (!wait_for_grant()) return 1;

    printf("\n--- NEBULA OS MOVE FILE ---\n");
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

        printf("\n  Select file to move (number): ");
        fflush(stdout);
        char buf[16];
        fgets(buf, 16, stdin);
        int choice = atoi(buf);

        if (choice >= 1 && choice <= count) {
            printf("  Enter destination subfolder: ");
            fflush(stdout);
            char dest_folder[64];
            if (fgets(dest_folder, 64, stdin)) {
                dest_folder[strlen(dest_folder)-1] = '\0';
                char dest_path[256], source_path[256], target_path[256];
                snprintf(dest_path, sizeof(dest_path), "nebula_hdd/%s", dest_folder);
                mkdir(dest_path, 0755);
                snprintf(source_path, sizeof(source_path), "nebula_hdd/%s", files[choice-1]);
                snprintf(target_path, sizeof(target_path), "%s/%s", dest_path, files[choice-1]);
                if (rename(source_path, target_path) == 0) printf("  File moved.\n");
                else printf("  Error moving file.\n");
            }
        }
    }

    printf("\n  Press Enter to exit...");
    fflush(stdout);
    getchar();

    send_termination_notice(getpid());
    return 0;
}
