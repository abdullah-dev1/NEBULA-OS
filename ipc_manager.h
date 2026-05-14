#ifndef NEBULA_IPC_MANAGER_H
#define NEBULA_IPC_MANAGER_H

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void start_ipc_listener();
void handle_ipc_request(int child_pid);
void* handle_ipc_request_wrapper(void* pid_ptr);
void create_pipe_for_process(int pid);
void cleanup_pipe(int pid);

void send_minimize(int pid);
void send_resume(int pid);

static inline void get_pipe_path(int pid, char* out) 
{
    sprintf(out, "/tmp/nebula_pipe_%d", pid);
}

static inline void send_resource_request(const char* task_name, int ram_mb, int hdd_mb) 
{
    int pid = getpid();
    char pipe_path[64];
    get_pipe_path(pid, pipe_path);
    mkfifo(pipe_path, 0666);

    char request[256];
    sprintf(request, "REQ:%s:%d:%d", task_name, ram_mb, hdd_mb);

    int fd = open(pipe_path, O_WRONLY);
    if (fd >= 0) {
        write(fd, request, strlen(request));
        close(fd);
    }
}

static inline int wait_for_grant()
{
    int pid = getpid();
    char pipe_path[64];
    get_pipe_path(pid, pipe_path);

    int fd = open(pipe_path, O_RDONLY);
    if (fd < 0) return 0;

    char buffer[256] = {0};
    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytes > 0 && strstr(buffer, "GRANT")) return 1;
    return 0;
}

static inline void send_termination_notice(int pid)
{
    char pipe_path[64];
    get_pipe_path(pid, pipe_path);
    mkfifo(pipe_path, 0666);

    char message[64];
    sprintf(message, "TERM:%d", pid);

    int fd = open(pipe_path, O_WRONLY | O_NONBLOCK);
    if (fd >= 0) {
        write(fd, message, strlen(message));
        close(fd);
    }
    unlink(pipe_path);
}

#endif
