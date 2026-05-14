#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "kernel/kernel.h"
#include "kernel/logger.h"
#include "kernel/memory_manager.h"
#include "kernel/process_manager.h"
#include "kernel/scheduler.h"
#include "kernel/interrupt_handler.h"
#include "kernel/deadlock_detector.h"
#include "kernel/ipc_manager.h"
#include "kernel/storage_manager.h"

Process pcb_table[MAX_PROCESSES];
int pcb_count = 0;
pthread_mutex_t pcb_lock = PTHREAD_MUTEX_INITIALIZER;

KernelQueue queue_Q0;
KernelQueue queue_Q1;
KernelQueue queue_Q2;

int total_RAM = 0;
int free_RAM = 0;
int total_HDD = 0;
int free_HDD = 0;
int total_cores = 0;
int free_cores = 0;

pthread_mutex_t resource_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  queue_cond    = PTHREAD_COND_INITIALIZER;
pthread_mutex_t queue_mutex   = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  core_cv       = PTHREAD_COND_INITIALIZER;
pthread_mutex_t core_mutex    = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_lock      = PTHREAD_MUTEX_INITIALIZER;

int kernel_mode = 0;
int os_running = 1;

struct TaskInfo {
    const char* binary;
    const char* display_name;
};

static const TaskInfo task_list[] = {
    {"", ""},
    {"./tasks/calculator",    "Calculator"},
    {"./tasks/notepad",       "Notepad"},
    {"./tasks/clock",         "Clock"},
    {"./tasks/calendar",      "Calendar"},
    {"./tasks/create_file",   "Create File"},
    {"./tasks/delete_file",   "Delete File"},
    {"./tasks/move_file",     "Move File"},
    {"./tasks/copy_file",     "Copy File"},
    {"./tasks/file_info",     "File Info"},
    {"./tasks/task_manager",  "Task Manager"},
    {"./tasks/music_player",  "Music Player"},
    {"./tasks/minigame",      "Minigame"},
    {"./tasks/instruction_guide", "Instruction Guide"},
    {"./tasks/dice_roller",   "Dice Roller"},
    {"./tasks/rename_file",   "Rename File"},
    {"./tasks/text_search",   "Text Search"},
    {"./tasks/alarm",         "Alarm"},
    {"./tasks/stopwatch",     "Stopwatch"},
    {"./tasks/file_compression", "File Compression"},
    {"./tasks/log_viewer",    "Log Viewer"},
    {"./tasks/mini_terminal", "Mini Terminal"},
    {"./tasks/todo_list",     "To-Do List"},
    {"./tasks/resource_dashboard", "Resource Dashboard"},
};

struct BackgroundTask {
    pid_t pid;
    char name[64];
    int minimized;
};

static BackgroundTask bg_tasks[MAX_PROCESSES];
static int bg_task_count = 0;

void clear_screen() {
    system("clear");
}

void print_boot_line(const char* message) {
    printf("  [ OK ] %s\n", message);
}

void display_boot_screen(int ram_mb, int hdd_mb, int cores) {
    clear_screen();
    printf("  NEBULA OS BOOT SEQUENCE\n");
    print_boot_line("Initializing kernel...");
    printf("  [ OK ] RAM: %d MB\n", ram_mb);
    printf("  [ OK ] HDD: %d MB\n", hdd_mb);
    printf("  [ OK ] CPU Cores: %d\n", cores);
    print_boot_line("Starting Scheduler...");
    print_boot_line("Starting IPC Listener...");
}

void display_menu() {
    clear_screen();
    int active_count = 0;
    for (int i = 0; i < bg_task_count; i++) if (!bg_tasks[i].minimized) active_count++;

    printf("  NEBULA OS V3  |  RAM: %d/%d MB  |  Tasks: %d (%d active)\n",
           (total_RAM - free_RAM), total_RAM, bg_task_count, active_count);
    printf("  ------------------------------------------------------------\n");

    for (int i = 1; i <= 12; ++i) {
        printf("  [%2d] %-18s", i, task_list[i].display_name);
        if (i + 12 <= 23) printf(" [%2d] %s", i + 12, task_list[i + 12].display_name);
        printf("\n");
    }

    printf("  ------------------------------------------------------------\n");
    printf("  [M] Tasks  [F] Foreground  [K] Kernel  [L] Logs  [0] Shutdown\n");
    printf("  NebulaOS> ");
    fflush(stdout);
}

void start_daemon(const char* binary, const char* name) {
    pid_t child = fork();
    if (child == 0) {
        setpgid(0, 0);
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) { dup2(devnull, 1); dup2(devnull, 2); close(devnull); }
        execl(binary, binary, (char*)NULL);
        _exit(1);
    }
    create_pipe_for_process(child);

    pthread_t ipc_thread;
    int* pid_ptr = (int*)malloc(sizeof(int));
    *pid_ptr = child;
    pthread_create(&ipc_thread, NULL, handle_ipc_request_wrapper, pid_ptr);
    pthread_detach(ipc_thread);

    if (bg_task_count < MAX_PROCESSES) {
        bg_tasks[bg_task_count].pid = child;
        strncpy(bg_tasks[bg_task_count].name, name, 63);
        bg_tasks[bg_task_count].minimized = 0;
        bg_task_count++;
    }
}

void foreground_task(pid_t child_pid, const char* task_name, int is_new) {
    if (!is_new) send_resume(child_pid);
    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, child_pid);
    int status;
    waitpid(child_pid, &status, WUNTRACED);
    tcsetpgrp(STDIN_FILENO, getpid());
    signal(SIGTTOU, SIG_DFL);

    if (WIFSTOPPED(status)) {
        printf("\n  [ Task %s Minimized ]\n", task_name);
        send_minimize(child_pid);
        for (int i = 0; i < bg_task_count; i++) if (bg_tasks[i].pid == child_pid) bg_tasks[i].minimized = 1;
    } else {
        for (int i = 0; i < bg_task_count; i++) {
            if (bg_tasks[i].pid == child_pid) {
                for (int j = i; j < bg_task_count - 1; j++) bg_tasks[j] = bg_tasks[j+1];
                bg_task_count--;
                break;
            }
        }
        remove_pcb(child_pid);
        cleanup_pipe(child_pid);
    }
    printf("  [Press Enter]");
    fflush(stdout);
    getchar();
}

void launch_task(int task_number) {
    if (task_number < 1 || task_number > 23) return;
    const char* binary = task_list[task_number].binary;
    const char* task_name = task_list[task_number].display_name;

    pid_t child_pid = fork();
    if (child_pid == 0) {
        setpgid(0, 0);
        execl(binary, binary, (char*)NULL);
        _exit(1);
    }

    create_pipe_for_process(child_pid);

    pthread_t ipc_thread;
    int* pid_ptr = (int*)malloc(sizeof(int));
    *pid_ptr = child_pid;
    pthread_create(&ipc_thread, NULL, handle_ipc_request_wrapper, pid_ptr);
    pthread_detach(ipc_thread);

    if (bg_task_count < MAX_PROCESSES) {
        bg_tasks[bg_task_count].pid = child_pid;
        strncpy(bg_tasks[bg_task_count].name, task_name, 63);
        bg_tasks[bg_task_count].minimized = 0;
        bg_task_count++;
    }
    foreground_task(child_pid, task_name, 1);
}

void shutdown_os() {
    os_running = 0;
    printf("\n  NebulaOS shutting down...\n");
    for (int i = 0; i < bg_task_count; i++) {
        kill(bg_tasks[i].pid, SIGTERM);
        cleanup_pipe(bg_tasks[i].pid);
    }
    stop_scheduler();
    stop_deadlock_detector();
    usleep(500000);
    while (waitpid(-1, NULL, WNOHANG) > 0);
    printf("\n  Goodbye!\n\n");
}

void show_tasks() {
    printf("\n--- BACKGROUND TASKS ---\n");
    if (bg_task_count == 0) printf("  No tasks running.\n");
    for (int i = 0; i < bg_task_count; i++) {
        printf("  PID: %d | %-15s | %s\n", bg_tasks[i].pid, bg_tasks[i].name, 
               bg_tasks[i].minimized ? "Minimized" : "Active");
    }
    printf("------------------------\n");
}

void show_kernel_mode() {
    printf("\n--- KERNEL RESOURCE MONITOR ---\n");
    printf("  RAM: %d MB Free | HDD: %d MB Free | Cores: %d Free\n", (int)free_RAM, (int)free_HDD, (int)free_cores);
    printf("\n  PROCESS TABLE (%d processes):\n", pcb_count);
    printf("  PID   %-15s %-10s PRI QUEUE\n", "NAME", "STATE");
    pthread_mutex_lock(&pcb_lock);
    for (int i = 0; i < pcb_count; i++) {
        printf("  %d  %-15s %-10s %d   Q%d\n", pcb_table[i].pid, pcb_table[i].name, pcb_table[i].state, 
               pcb_table[i].priority, pcb_table[i].queue_level);
    }
    pthread_mutex_unlock(&pcb_lock);
    printf("-------------------------------\n");
}

void manage_foreground() {
    show_tasks();
    printf("  Enter PID to bring to foreground (or 0 to cancel): ");
    char buf[16];
    fgets(buf, 16, stdin);
    int target_pid = atoi(buf);
    if (target_pid <= 0) return;

    for (int i = 0; i < bg_task_count; i++) {
        if (bg_tasks[i].pid == target_pid) {
            bg_tasks[i].minimized = 0;
            foreground_task(target_pid, bg_tasks[i].name, 0);
            return;
        }
    }
    printf("  Error: PID not found.\n");
}

int main() {
    init_logger();
    printf("  NEBULA OS CONFIGURATION\n");
    int ram_gb, hdd_gb, cores;
    printf("  RAM (GB): "); scanf("%d", &ram_gb);
    printf("  HDD (GB): "); scanf("%d", &hdd_gb);
    printf("  Cores:    "); scanf("%d", &cores);
    getchar(); 

    init_resources(ram_gb * 1024, hdd_gb * 1024, cores);
    init_storage();
    register_signal_handlers();
    display_boot_screen(ram_gb * 1024, hdd_gb * 1024, cores);
    start_scheduler();
    start_deadlock_detector();
    start_ipc_listener();

    start_daemon("./tasks/clock", "Clock Daemon");
    start_daemon("./tasks/log_daemon", "Log Daemon");
    start_daemon("./tasks/sys_monitor", "System Monitor");

    while (os_running) {
        display_menu();
        char buffer[64];
        if (!fgets(buffer, 64, stdin)) break;

        char c = buffer[0];
        if (c == '0') { shutdown_os(); break; }
        else if (c == 'M' || c == 'm') {
            clear_screen();
            show_tasks();
            printf("  Press Enter...");
            fflush(stdout);
            getchar();
        }
        else if (c == 'F' || c == 'f') {
            clear_screen();
            manage_foreground();
        }
        else if (c == 'K' || c == 'k') {
            clear_screen();
            show_kernel_mode();
            printf("  Press Enter...");
            fflush(stdout);
            getchar();
        }
        else if (c == 'L' || c == 'l') {
            clear_screen();
            printf("--- SYSTEM LOGS ---\n");
            FILE* f = fopen(SYSTEM_LOG_PATH, "r");
            if (f) {
                char line[256];
                while (fgets(line, sizeof(line), f)) printf("  %s", line);
                fclose(f);
            }
            printf("-------------------\n  [C] Clear Logs  [ENTER] Back\n  NebulaOS> ");
            fflush(stdout);
            char l_buf[16];
            fgets(l_buf, 16, stdin);
            if (l_buf[0] == 'c' || l_buf[0] == 'C') {
                FILE* f2 = fopen(SYSTEM_LOG_PATH, "w");
                if (f2) fclose(f2);
                printf("  [System] Logs cleared.\n");
                sleep(1);
            }
        }
        else {
            int n = atoi(buffer);
            if (n > 0) {
                clear_screen();
                launch_task(n);
            }
        }
    }

    return 0;
}
