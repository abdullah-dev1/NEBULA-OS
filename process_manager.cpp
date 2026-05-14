#include "process_manager.h"
#include "kernel.h"
#include "logger.h"
#include "memory_manager.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

int get_queue_level(const char* task_name) {
    if (strcmp(task_name, "Clock") == 0 || strcmp(task_name, "Log Daemon") == 0) return QUEUE_SYSTEM;

    const char* interactive[] = {
        "Calculator", "Notepad", "Calendar", "Minigame", "Dice Roller", 
        "Task Manager", "Mini Terminal", "To-Do List", "Resource Dashboard", 
        "Log Viewer", "Instruction Guide"
    };
    for (int i = 0; i < 11; i++) {
        if (strcmp(task_name, interactive[i]) == 0) return QUEUE_INTERACTIVE;
    }
    return QUEUE_BACKGROUND;
}

void create_pcb(int pid, const char* name, int priority, int queue_level, int ram, int hdd) {
    pthread_mutex_lock(&pcb_lock);
    if (pcb_count < MAX_PROCESSES) {
        Process* p = &pcb_table[pcb_count++];
        p->pid = pid;
        p->set_name(name);
        p->set_state("READY");
        p->priority = priority;
        p->queue_level = queue_level;
        p->ram_alloc = ram;
        p->hdd_alloc = hdd;
        p->arrive_time = time(0);
        p->wait_time = 0;
        p->turnaround = 0;
        p->core_id = -1;

        if (queue_level == QUEUE_SYSTEM) queue_Q0.push(p);
        else if (queue_level == QUEUE_INTERACTIVE) queue_Q1.push(p);
        else queue_Q2.push(p);
    }
    pthread_mutex_unlock(&pcb_lock);

    pthread_mutex_lock(&queue_mutex);
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    char buf[128];
    sprintf(buf, "Process created: PID=%d (%s)", pid, name);
    log_event(buf);
}

void remove_pcb(int pid) {
    int ram = 0, hdd = 0;
    int found = 0;
    pthread_mutex_lock(&pcb_lock);
    for (int i = 0; i < pcb_count; i++) {
        if (pcb_table[i].pid == pid) {
            ram = pcb_table[i].ram_alloc;
            hdd = pcb_table[i].hdd_alloc;
            found = 1;

            for (int j = i; j < pcb_count - 1; j++) pcb_table[j] = pcb_table[j+1];
            pcb_count--;

            while(queue_Q0.pop()); 
            while(queue_Q1.pop()); 
            while(queue_Q2.pop());
            for (int k = 0; k < pcb_count; k++) {
                if (pcb_table[k].queue_level == QUEUE_SYSTEM) queue_Q0.push(&pcb_table[k]);
                else if (pcb_table[k].queue_level == QUEUE_INTERACTIVE) queue_Q1.push(&pcb_table[k]);
                else queue_Q2.push(&pcb_table[k]);
            }
            break;
        }
    }
    pthread_mutex_unlock(&pcb_lock);

    if (found) {
        reclaim_resources(ram, hdd);
        char buf[64];
        sprintf(buf, "Process terminated: PID=%d", pid);
        log_event(buf);
    }
}

void update_state(int pid, const char* new_state) {
    pthread_mutex_lock(&pcb_lock);
    for (int i = 0; i < pcb_count; i++) {
        if (pcb_table[i].pid == pid) {
            char buf[128];
            sprintf(buf, "State transition: PID=%d %s -> %s", pid, pcb_table[i].state, new_state);
            log_event(buf);
            pcb_table[i].set_state(new_state);
            break;
        }
    }
    pthread_mutex_unlock(&pcb_lock);
}

int get_all_pcbs_copy(Process* dest) {
    pthread_mutex_lock(&pcb_lock);
    for (int i = 0; i < pcb_count; i++) dest[i] = pcb_table[i];
    int count = pcb_count;
    pthread_mutex_unlock(&pcb_lock);
    return count;
}

int set_process_priority(int pid, int priority) {
    int success = 0;
    pthread_mutex_lock(&pcb_lock);
    for (int i = 0; i < pcb_count; i++) {
        if (pcb_table[i].pid == pid) {
            if (priority < 0) priority = 0;
            if (priority > 10) priority = 10;
            pcb_table[i].priority = priority;
            success = 1;
            break;
        }
    }
    pthread_mutex_unlock(&pcb_lock);
    return success;
}
