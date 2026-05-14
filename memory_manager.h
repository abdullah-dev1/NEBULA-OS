#ifndef NEBULA_MEMORY_MANAGER_H
#define NEBULA_MEMORY_MANAGER_H

void init_resources(int ram_mb, int hdd_mb, int cores);

bool allocate_resources(int ram, int hdd);

void reclaim_resources(int ram, int hdd);

void acquire_core();

void release_core();

int get_free_ram();

int get_free_hdd();

int get_free_cores();

#endif
