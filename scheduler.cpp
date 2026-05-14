#include "scheduler.h"
#include "kernel.h"
#include "logger.h"
#include "memory_manager.h"
#include "process_manager.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

static void dispatch_process(Process *process) {
    if (!process) return;

    int assigned_core = total_cores - free_cores;
    if (assigned_core < 0) assigned_core = 0;
    if (assigned_core >= total_cores) assigned_core = total_cores - 1;

    process->set_state("RUNNING");
    process->core_id = assigned_core;

    char buf[128];
    sprintf(buf, "Context switch: PID=%d (%s) -> Core %d", process->pid, process->name, process->core_id);
    log_event(buf);
}

static void* scheduler_loop(void* arg) {
    (void)arg;
    while (os_running) {
        int dispatched = 0;
        pthread_mutex_lock(&pcb_lock);

        if (queue_Q0.size() > 0) {

            for (int i = 0; i < queue_Q0.size() - 1; i++) {
                for (int j = 0; j < queue_Q0.size() - i - 1; j++) {
                    if (queue_Q0.get(j)->priority > queue_Q0.get(j+1)->priority) {

                    }
                }
            }
            for (int i = 0; i < queue_Q0.size(); i++) {
                Process* p = queue_Q0.get(i);
                if (p->state[0] == 'R' && p->state[1] == 'E') { 
                    dispatch_process(p); dispatched = 1; break;
                }
            }
        }

        if (!dispatched && queue_Q1.size() > 0) {
            for (int i = 0; i < queue_Q1.size(); i++) {
                Process* p = queue_Q1.get(i);
                if (p->state[0] == 'R' && p->state[1] == 'E') {
                    dispatch_process(p);

                    queue_Q1.pop();
                    queue_Q1.push(p);
                    dispatched = 1;
                    break;
                }
            }
        }

        if (!dispatched && queue_Q2.size() > 0) {
            for (int i = 0; i < queue_Q2.size(); i++) {
                Process* p = queue_Q2.get(i);
                if (p->state[0] == 'R' && p->state[1] == 'E') {
                    dispatch_process(p); dispatched = 1; break;
                }
            }
        }
        pthread_mutex_unlock(&pcb_lock);

        if (!dispatched) {
            pthread_mutex_lock(&queue_mutex);
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 1;
            pthread_cond_timedwait(&queue_cond, &queue_mutex, &ts);
            pthread_mutex_unlock(&queue_mutex);
        } else {
            usleep(RR_QUANTUM_MS * 1000);
        }
    }
    return 0;
}

static void* aging_loop(void* arg) {
    (void)arg;
    while (os_running) {
        sleep(1);
        pthread_mutex_lock(&pcb_lock);
        int changed = 0;
        for (int i = 0; i < pcb_count; i++) {
            if (pcb_table[i].state[0] == 'R' && pcb_table[i].queue_level != QUEUE_SYSTEM) {
                pcb_table[i].wait_time++;
                if (pcb_table[i].wait_time % AGING_INTERVAL == 0) {
                    if (pcb_table[i].priority > 0) pcb_table[i].priority--;
                    if (pcb_table[i].priority <= Q0_PROMOTE_THRESH && pcb_table[i].queue_level == QUEUE_BACKGROUND) {
                        pcb_table[i].queue_level = QUEUE_INTERACTIVE; changed = 1;
                    } else if (pcb_table[i].priority == 0 && pcb_table[i].queue_level == QUEUE_INTERACTIVE) {
                        pcb_table[i].queue_level = QUEUE_SYSTEM; changed = 1;
                    }
                }
            }
        }
        if (changed) {
            queue_Q0.remove(-1); queue_Q1.remove(-1); queue_Q2.remove(-1); 

            while(queue_Q0.pop()); 
            while(queue_Q1.pop()); 
            while(queue_Q2.pop());
            for (int i = 0; i < pcb_count; i++) {
                if (pcb_table[i].queue_level == QUEUE_SYSTEM) queue_Q0.push(&pcb_table[i]);
                else if (pcb_table[i].queue_level == QUEUE_INTERACTIVE) queue_Q1.push(&pcb_table[i]);
                else queue_Q2.push(&pcb_table[i]);
            }
        }
        pthread_mutex_unlock(&pcb_lock);
    }
    return 0;
}

void start_scheduler() {
    pthread_t t1, t2;
    pthread_create(&t1, 0, scheduler_loop, 0);
    pthread_create(&t2, 0, aging_loop, 0);
    pthread_detach(t1);
    pthread_detach(t2);
    log_event("Low-level Scheduler started.");
}

void stop_scheduler() {
    pthread_cond_broadcast(&queue_cond);
    log_event("Scheduler stop requested.");
}
