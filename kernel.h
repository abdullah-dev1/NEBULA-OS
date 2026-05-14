#ifndef NEBULA_KERNEL_H
#define NEBULA_KERNEL_H

#include <pthread.h>
#include <time.h>

#define MAX_PROCESSES 256
#define MAX_NAME 64
#define MAX_STATE 32

#define QUEUE_SYSTEM      0
#define QUEUE_INTERACTIVE 1
#define QUEUE_BACKGROUND  2

class Process {
public:
    int     pid;
    char    name[MAX_NAME];
    char    state[MAX_STATE];
    int     priority;
    int     queue_level;
    int     ram_alloc;
    int     hdd_alloc;
    time_t  arrive_time;
    int     wait_time;
    int     turnaround;
    int     core_id;

    Process() {
        pid = -1;
        priority = 10;
        queue_level = 2;
        ram_alloc = 0;
        hdd_alloc = 0;
        wait_time = 0;
        turnaround = 0;
        core_id = -1;
        name[0] = '\0';
        state[0] = '\0';
    }

    void set_name(const char* n) {
        int i = 0;
        while (n[i] != '\0' && i < MAX_NAME - 1) {
            name[i] = n[i];
            i++;
        }
        name[i] = '\0';
    }

    void set_state(const char* s) {
        int i = 0;
        while (s[i] != '\0' && i < MAX_STATE - 1) {
            state[i] = s[i];
            i++;
        }
        state[i] = '\0';
    }
};

class KernelQueue {
private:
    Process* data[MAX_PROCESSES];
    int front_idx;
    int rear_idx;
    int count;

public:
    KernelQueue() : front_idx(0), rear_idx(0), count(0) {}

    bool push(Process* p) {
        if (count >= MAX_PROCESSES) return false;
        data[rear_idx] = p;
        rear_idx = (rear_idx + 1) % MAX_PROCESSES;
        count++;
        return true;
    }

    Process* pop() {
        if (count == 0) return 0;
        Process* p = data[front_idx];
        front_idx = (front_idx + 1) % MAX_PROCESSES;
        count--;
        return p;
    }

    int size() { return count; }

    Process* get(int index) {
        if (index < 0 || index >= count) return 0;
        return data[(front_idx + index) % MAX_PROCESSES];
    }

    void remove(int pid) {
        int cur_count = count;
        Process* temp[MAX_PROCESSES];
        int t_idx = 0;
        for (int i = 0; i < cur_count; i++) {
            Process* p = pop();
            if (p->pid != pid) temp[t_idx++] = p;
        }
        for (int i = 0; i < t_idx; i++) push(temp[i]);
    }
};

extern Process pcb_table[MAX_PROCESSES];
extern int pcb_count;
extern pthread_mutex_t pcb_lock;

extern KernelQueue queue_Q0;
extern KernelQueue queue_Q1;
extern KernelQueue queue_Q2;

extern int total_RAM;
extern int free_RAM;
extern int total_HDD;
extern int free_HDD;
extern int total_cores;
extern int free_cores;

extern pthread_mutex_t resource_lock;
extern pthread_cond_t  queue_cond;
extern pthread_mutex_t queue_mutex;
extern pthread_cond_t  core_cv;
extern pthread_mutex_t core_mutex;
extern pthread_mutex_t log_lock;

extern int os_running;
extern int kernel_mode;

#define DEFAULT_PRIORITY_Q0 0
#define DEFAULT_PRIORITY_Q1 3
#define DEFAULT_PRIORITY_Q2 6
#define AGING_INTERVAL     5
#define Q0_PROMOTE_THRESH  1
#define RR_QUANTUM_MS      200

#define NEBULA_HDD_PATH  "nebula_hdd"
#define SYSTEM_LOG_PATH  "system.log"

#endif 
