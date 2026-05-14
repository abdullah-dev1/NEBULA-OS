#include "ipc_manager.h"
#include "kernel.h"
#include "logger.h"
#include "memory_manager.h"
#include "process_manager.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

void create_pipe_for_process(int pid) {
    char path[64];
    sprintf(path, "/tmp/nebula_pipe_%d", pid);
    unlink(path);
    if (mkfifo(path, 0666) != 0) {

    }
}

void cleanup_pipe(int pid) {
    char path[64];
    sprintf(path, "/tmp/nebula_pipe_%d", pid);
    unlink(path);
}

void handle_ipc_request(int child_pid) {
    char path[64];
    sprintf(path, "/tmp/nebula_pipe_%d", child_pid);
    int fd = open(path, O_RDONLY);
    if (fd < 0) return;

    char buf[512] = {0};
    if (read(fd, buf, sizeof(buf) - 1) > 0) {
        if (strncmp(buf, "REQ:", 4) == 0) {

            char name[64];
            int ram, hdd;
            char* p1 = buf + 4;
            char* p2 = strchr(p1, ':');
            if (p2) {
                *p2 = '\0';
                strncpy(name, p1, 63);
                char* p3 = p2 + 1;
                char* p4 = strchr(p3, ':');
                if (p4) {
                    *p4 = '\0';
                    ram = atoi(p3);
                    hdd = atoi(p4 + 1);

                    int granted = allocate_resources(ram, hdd);
                    char resp[64];
                    if (granted) {
                        sprintf(resp, "GRANT:%d", child_pid);
                        int q = get_queue_level(name);
                        int p = (q == 0) ? DEFAULT_PRIORITY_Q0 : (q == 1 ? DEFAULT_PRIORITY_Q1 : DEFAULT_PRIORITY_Q2);
                        create_pcb(child_pid, name, p, q, ram, hdd);
                    } else {
                        strcpy(resp, "DENY");
                    }

                    int wfd = open(path, O_WRONLY);
                    if (wfd >= 0) {
                        write(wfd, resp, strlen(resp));
                        close(wfd);
                    }
                }
            }
        } else if (strncmp(buf, "TERM:", 5) == 0) {
            remove_pcb(atoi(buf + 5));
            cleanup_pipe(child_pid);
        }
    }
    close(fd);
}

void* handle_ipc_request_wrapper(void* pid_ptr) {
    if (!pid_ptr) return 0;
    int pid = *(int*)pid_ptr;
    handle_ipc_request(pid);
    free(pid_ptr);
    return 0;
}

static void* ipc_listener_loop(void* arg) {
    (void)arg;
    while (os_running) {
        int pids[MAX_PROCESSES];
        int count = 0;
        pthread_mutex_lock(&pcb_lock);
        for (int i = 0; i < pcb_count; i++) pids[count++] = pcb_table[i].pid;
        pthread_mutex_unlock(&pcb_lock);

        for (int i = 0; i < count; i++) {
            char path[64];
            sprintf(path, "/tmp/nebula_pipe_%d", pids[i]);
            int fd = open(path, O_RDONLY | O_NONBLOCK);
            if (fd >= 0) {
                char buf[256] = {0};
                int bytes = read(fd, buf, sizeof(buf) - 1);
                if (bytes > 0) {
                    if (strncmp(buf, "TERM:", 5) == 0) {
                        remove_pcb(pids[i]);
                        cleanup_pipe(pids[i]);
                    }
                }
                close(fd);
            }
        }
        usleep(100000);
    }
    return 0;
}

void start_ipc_listener() {
    pthread_t t;
    pthread_create(&t, 0, ipc_listener_loop, 0);
    pthread_detach(t);
    log_event("Low-level IPC Listener started.");
}
