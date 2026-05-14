#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <termios.h>
#include "kernel/ipc_manager.h"

static int clock_running = 1;

void handle_signal(int sig) {
    (void)sig;
    clock_running = 0;
}

void set_nonblocking_mode(struct termios* oldt) {
    struct termios newt;
    tcgetattr(STDIN_FILENO, oldt);
    newt = *oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void restore_terminal_mode(struct termios* oldt) {
    tcsetattr(STDIN_FILENO, TCSANOW, oldt);
}

int main() {
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    send_resource_request("Clock", 10, 1);
    if (!wait_for_grant()) return 1;

    struct termios oldt;
    set_nonblocking_mode(&oldt);

    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                   NEBULA OS SYSTEM CLOCK                  ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("  (Press 'q' to Quit | Press Ctrl+Z to Minimize)\n\n");

    while (clock_running) {
        time_t now = time(NULL);
        struct tm* t = localtime(&now);
        char time_str[64];
        strftime(time_str, 64, "%H:%M:%S | %A, %B %d, %Y", t);

        printf("\r  Current Time: %s    ", time_str);
        fflush(stdout);

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
        if (ret > 0) {
            char c;
            if (read(STDIN_FILENO, &c, 1) > 0) {
                if (c == 'q' || c == 'Q') {
                    clock_running = 0;
                }
            }
        }
    }

    restore_terminal_mode(&oldt);
    printf("\n  [Clock Stopped]\n");
    
    send_termination_notice(getpid());
    return 0;
}
