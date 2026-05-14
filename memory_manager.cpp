#include "memory_manager.h"
#include "kernel.h"
#include "logger.h"
#include <pthread.h>
#include <stdio.h>

void init_resources(int ram_mb, int hdd_mb, int cores) {
    total_RAM = ram_mb;
    free_RAM = ram_mb;
    total_HDD = hdd_mb;
    free_HDD = hdd_mb;
    total_cores = cores;
    free_cores = cores;

    char buf[128];
    sprintf(buf, "Resources: RAM=%dMB, HDD=%dMB, Cores=%d", ram_mb, hdd_mb, cores);
    log_event(buf);
}

bool allocate_resources(int ram, int hdd) {
    pthread_mutex_lock(&resource_lock);
    int success = 0;
    if (free_RAM >= ram && free_HDD >= hdd) {
        free_RAM -= ram;
        free_HDD -= hdd;
        success = 1;
    }
    pthread_mutex_unlock(&resource_lock);

    if (success) {
        char buf[64];
        sprintf(buf, "Resources allocated: RAM=%d, HDD=%d", ram, hdd);
        log_event(buf);
    }
    return success;
}

void reclaim_resources(int ram, int hdd) {
    pthread_mutex_lock(&resource_lock);
    free_RAM += ram;
    free_HDD += hdd;
    pthread_mutex_unlock(&resource_lock);

    char buf[64];
    sprintf(buf, "Resources reclaimed: RAM=%d, HDD=%d", ram, hdd);
    log_event(buf);
}

void acquire_core() {
    pthread_mutex_lock(&core_mutex);
    while (free_cores <= 0) {
        pthread_cond_wait(&core_cv, &core_mutex);
    }
    free_cores--;
    pthread_mutex_unlock(&core_mutex);
}

void release_core() {
    pthread_mutex_lock(&core_mutex);
    free_cores++;
    pthread_cond_signal(&core_cv);
    pthread_mutex_unlock(&core_mutex);
}

int get_free_ram() { return free_RAM; }
int get_free_hdd() { return free_HDD; }
int get_free_cores() { return free_cores; }
