#ifndef NEBULA_PROCESS_MANAGER_H
#define NEBULA_PROCESS_MANAGER_H

#include "kernel.h"

void create_pcb(int pid, const char* name, int priority, int queue_level, int ram, int hdd);
void remove_pcb(int pid);
void update_state(int pid, const char* new_state);
int  get_queue_level(const char* task_name);
int  get_all_pcbs_copy(Process* dest);
int  set_process_priority(int pid, int priority);

#endif
