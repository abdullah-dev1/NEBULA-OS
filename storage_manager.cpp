#include "storage_manager.h"
#include "kernel.h"
#include "logger.h"
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

void init_storage() {
    struct stat st;
    memset(&st, 0, sizeof(st));
    if (stat(NEBULA_HDD_PATH, &st) == -1) {
        mkdir(NEBULA_HDD_PATH, 0700);
        log_event("Storage: Created nebula_hdd/");
    } else {
        log_event("Storage: nebula_hdd/ exists");
    }
}

void list_hdd_files() {
    DIR *d;
    struct dirent *dir;
    d = opendir(NEBULA_HDD_PATH);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                printf("  %s\n", dir->d_name);
            }
        }
        closedir(d);
    }
}

int file_exists_in_hdd(const char* filename) {
    char path[128];
    sprintf(path, "%s/%s", NEBULA_HDD_PATH, filename);
    struct stat st;
    return (stat(path, &st) == 0);
}

long get_file_size(const char* filename) {
    char path[128];
    sprintf(path, "%s/%s", NEBULA_HDD_PATH, filename);
    struct stat st;
    if (stat(path, &st) == 0) return st.st_size;
    return -1;
}

void get_hdd_path(const char* filename, char* out) {
    sprintf(out, "%s/%s", NEBULA_HDD_PATH, filename);
}
