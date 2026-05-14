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
    send_resource_request("File Compression", 30, 20);
    if (!wait_for_grant()) return 1;

    printf("\n--- NEBULA OS FILE COMPRESSION ---\n");
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

        printf("\n  Select file to compress (number): ");
        fflush(stdout);
        char buf[16];
        fgets(buf, 16, stdin);
        int choice = atoi(buf);

        if (choice >= 1 && choice <= count) {
            char source_path[256], dest_path[256], dest_name[128];
            snprintf(source_path, sizeof(source_path), "nebula_hdd/%s", files[choice-1]);
            snprintf(dest_name, sizeof(dest_name), "%s.neb", files[choice-1]);
            snprintf(dest_path, sizeof(dest_path), "nebula_hdd/%s", dest_name);

            struct stat st;
            long size = 0;
            if (stat(source_path, &st) == 0) size = st.st_size;

            printf("\n  Compressing: %s (%ld bytes)\n", files[choice-1], size);
            for (int p = 0; p <= 100; p += 10) {
                printf("\r  Progress: [%d%%] ", p);
                fflush(stdout);
                usleep(100000);
            }

            FILE* src = fopen(source_path, "rb");
            FILE* dst = fopen(dest_path, "wb");
            if (src && dst) {
                fprintf(dst, "[NEBULA_COMPRESSED]\n");
                char buffer[1024];
                size_t n = fread(buffer, 1, 1024, src);
                if (n > 0) fwrite(buffer, 1, n / 2 + 1, dst); 
                printf("\n\n  ✓ Compression complete: %s\n", dest_name);
                printf("  New size: ~%ld bytes\n", size / 2);
            } else {
                printf("\n  Error: Could not process file.\n");
            }
            if (src) fclose(src);
            if (dst) fclose(dst);
        }
    }

    printf("\n  Press Enter to exit...");
    fflush(stdout);
    getchar();

    send_termination_notice(getpid());
    return 0;
}
