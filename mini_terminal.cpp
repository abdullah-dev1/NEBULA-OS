#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "kernel/ipc_manager.h"

void trim_newline(char* s) {
    size_t len = strlen(s);
    if (len > 0 && s[len-1] == '\n') s[len-1] = '\0';
}

void cmd_ls() {
    DIR *d;
    struct dirent *dir;
    d = opendir("nebula_hdd");
    if (d) {
        printf("  [ Files in nebula_hdd/ ]\n");
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_name[0] == '.') continue;
            printf("    %s\n", dir->d_name);
        }
        closedir(d);
    } else {
        printf("  Error: Cannot open nebula_hdd/\n");
    }
}

void cmd_cat(const char* filename) {
    char path[128];
    sprintf(path, "nebula_hdd/%s", filename);
    FILE* f = fopen(path, "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) printf("  %s", line);
        fclose(f);
    } else {
        printf("  Error: Could not open %s\n", filename);
    }
}

int main() {
    send_resource_request("Mini Terminal", 45, 5);
    if (!wait_for_grant()) return 1;

    printf("\n--- NEBULA OS MINI TERMINAL ---\n");
    printf("  Available: ls, pwd, cat, mkdir, echo, clear, exit\n\n");

    char input[256];
    while (1) {
        printf("  nebula_hdd$ ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) break;
        trim_newline(input);

        if (strlen(input) == 0) continue;

        if (strcmp(input, "exit") == 0) break;
        else if (strcmp(input, "ls") == 0) cmd_ls();
        else if (strcmp(input, "pwd") == 0) printf("  /nebula_hdd\n");
        else if (strncmp(input, "cat ", 4) == 0) cmd_cat(input + 4);
        else if (strncmp(input, "mkdir ", 6) == 0) {
            char path[128];
            sprintf(path, "nebula_hdd/%s", input + 6);
            if (mkdir(path, 0700) == 0) printf("  Created directory %s\n", input + 6);
            else printf("  Error creating directory\n");
        }
        else if (strncmp(input, "echo ", 5) == 0) {
            char* text = input + 5;
            char* redirect = strchr(text, '>');
            if (redirect) {
                *redirect = '\0';
                char* filename = redirect + 1;
                while (*filename == ' ') filename++;
                char path[128];
                sprintf(path, "nebula_hdd/%s", filename);
                FILE* f = fopen(path, "w");
                if (f) {
                    fprintf(f, "%s\n", text);
                    fclose(f);
                    printf("  Written to %s\n", filename);
                }
            } else {
                printf("  %s\n", text);
            }
        }
        else if (strcmp(input, "clear") == 0) {
            for (int i = 0; i < 50; i++) printf("\n");
        }
        else {
            printf("  Unknown command: %s\n", input);
        }
    }

    send_termination_notice(getpid());
    return 0;
}
