#include "deadlock_detector.h"
#include "kernel.h"
#include "logger.h"
#include "process_manager.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define GRAPH_SIZE (MAX_PROCESSES + 3)

static int adj[GRAPH_SIZE][GRAPH_SIZE];
static int visited[GRAPH_SIZE];
static int stack[GRAPH_SIZE];
static int cycle[GRAPH_SIZE];
static int cycle_len = 0;

static int dfs(int u, int n) {
    visited[u] = 1;
    stack[u] = 1;
    for (int v = 0; v < n; v++) {
        if (adj[u][v]) {
            if (!visited[v]) {
                if (dfs(v, n)) { cycle[cycle_len++] = u; return 1; }
            } else if (stack[v]) {
                cycle[cycle_len++] = v; cycle[cycle_len++] = u; return 1;
            }
        }
    }
    stack[u] = 0;
    return 0;
}

static void run_detection() {
    for (int i = 0; i < GRAPH_SIZE; i++) {
        for (int j = 0; j < GRAPH_SIZE; j++) adj[i][j] = 0;
        visited[i] = 0; stack[i] = 0;
    }
    cycle_len = 0;

    pthread_mutex_lock(&pcb_lock);
    for (int i = 0; i < pcb_count; i++) {
        int u = i;
        if (pcb_table[i].state[0] == 'B') { 
            if (pcb_table[i].ram_alloc > 0) adj[u][MAX_PROCESSES] = 1;
            if (pcb_table[i].hdd_alloc > 0) adj[u][MAX_PROCESSES+1] = 1;
            if (pcb_table[i].core_id < 0) adj[u][MAX_PROCESSES+2] = 1;
        }
        if (pcb_table[i].state[0] == 'R' && pcb_table[i].state[1] == 'U') { 
            if (pcb_table[i].ram_alloc > 0) adj[MAX_PROCESSES][u] = 1;
            if (pcb_table[i].hdd_alloc > 0) adj[MAX_PROCESSES+1][u] = 1;
            if (pcb_table[i].core_id >= 0) adj[MAX_PROCESSES+2][u] = 1;
        }
    }
    pthread_mutex_unlock(&pcb_lock);

    for (int i = 0; i < GRAPH_SIZE; i++) {
        if (!visited[i]) {
            if (dfs(i, GRAPH_SIZE)) {
                log_event("Deadlock detected!");
                break;
            }
        }
    }
}

static void* detector_loop(void* arg) {
    (void)arg;
    while (os_running) {
        sleep(5);
        if (!os_running) break;
        run_detection();
    }
    return 0;
}

void start_deadlock_detector() {
    pthread_t t;
    pthread_create(&t, 0, detector_loop, 0);
    pthread_detach(t);
    log_event("Deadlock Detector started.");
}

void stop_deadlock_detector() {
    log_event("Deadlock detector stop requested.");
}
