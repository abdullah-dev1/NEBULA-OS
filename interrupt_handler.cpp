#include "interrupt_handler.h"
#include "kernel.h"
#include "logger.h"
#include "process_manager.h"
#include "memory_manager.h"
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static void* unblock_after_delay(void* pid_ptr) {
    int pid = *(int*)pid_ptr;
    free(pid_ptr);
    sleep(2);
    pthread_mutex_lock(&pcb_lock);
    for (int i = 0; i < pcb_count; i++) {
        if (pcb_table[i].pid == pid && pcb_table[i].state[0] == 'B') {
            pcb_table[i].set_state("READY");
            char buf[64];
            sprintf(buf, "Interrupt resolved: PID=%d -> READY", pid);
            log_event(buf);
            break;
        }
    }
    pthread_mutex_unlock(&pcb_lock);
    pthread_cond_signal(&queue_cond);
    return 0;
}

static void sigchld_handler(int signo) {
    (void)signo;
    int status;
    pid_t child_pid;
    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
        char buf[64];
        sprintf(buf, "SIGCHLD: Child PID=%d terminated", child_pid);
        log_event(buf);
        remove_pcb(child_pid);
    }
}

void register_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, 0);
    log_event("Signal handlers registered");
}

void send_interrupt(int pid) {
    pthread_mutex_lock(&pcb_lock);
    for (int i = 0; i < pcb_count; i++) {
        if (pcb_table[i].pid == pid && pcb_table[i].state[0] == 'R' && pcb_table[i].state[1] == 'U') {
            pcb_table[i].set_state("BLOCKED");
            if (pcb_table[i].core_id >= 0) { pcb_table[i].core_id = -1; release_core(); }

            pthread_t t;
            int* p = (int*)malloc(sizeof(int));
            *p = pid;
            pthread_create(&t, 0, unblock_after_delay, p);
            pthread_detach(t);
            break;
        }
    }
    pthread_mutex_unlock(&pcb_lock);
    kill(pid, SIGUSR1);
}

void send_minimize(int pid) {
    pthread_mutex_lock(&pcb_lock);
    for (int i = 0; i < pcb_count; i++) {
        if (pcb_table[i].pid == pid) {
            pcb_table[i].set_state("MINIMIZED");
            if (pcb_table[i].core_id >= 0) { pcb_table[i].core_id = -1; release_core(); }
            break;
        }
    }
    pthread_mutex_unlock(&pcb_lock);
    kill(pid, SIGTSTP);
}

void send_resume(int pid) {
    pthread_mutex_lock(&pcb_lock);
    for (int i = 0; i < pcb_count; i++) {
        if (pcb_table[i].pid == pid) {
            pcb_table[i].set_state("RUNNING");
            break;
        }
    }
    pthread_mutex_unlock(&pcb_lock);
    kill(pid, SIGCONT);
}
