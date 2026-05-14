#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include "kernel/ipc_manager.h"

int main() {
    send_resource_request("Stopwatch", 15, 1);
    if (!wait_for_grant()) return 1;

    system("clear");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                ⏱️  NEBULA OS CHRONOMETER                 ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("  [1] Stopwatch  [2] Countdown  [Q] Quit\n");
    printf("  Choice: ");
    fflush(stdout);
    
    char choice[16];
    if (!fgets(choice, 16, stdin)) return 1;
    if (choice[0] == 'q' || choice[0] == 'Q') {
        send_termination_notice(getpid());
        return 0;
    }

    if (choice[0] == '2') {
        printf("  ⏲️  Enter seconds for countdown: ");
        fflush(stdout);
        char buf[16];
        fgets(buf, 16, stdin);
        int total = atoi(buf);
        time_t end = time(NULL) + total;
        
        printf("\n  ⏳ Starting countdown...\n");
        while (1) {
            int rem = end - time(NULL);
            if (rem <= 0) break;
            printf("\r  ⏰ Time Remaining: %02d:%02d   (Press 'q' to stop) ", rem/60, rem%60);
            fflush(stdout);

            fd_set fds;
            struct timeval tv = {1, 0};
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);
            if (select(STDIN_FILENO+1, &fds, NULL, NULL, &tv) > 0) {
                char c_buf[16];
                if (fgets(c_buf, 16, stdin) && (c_buf[0] == 'q' || c_buf[0] == 'Q')) {
                    printf("\n  [!] Countdown cancelled.\n");
                    break;
                }
            }
        }
        if (time(NULL) >= end) printf("\r  ✨ TIME'S UP! ✨         \n");
    } else {
        printf("\n  ▶️  Stopwatch started! \n");
        printf("  [Enter] Lap  [Q] Stop/Quit\n\n");
        time_t start = time(NULL);
        int laps = 0;
        while (1) {
            int elapsed = time(NULL) - start;
            printf("\r  ⏱️  Elapsed: %02d:%02d  ", elapsed/60, elapsed%60);
            fflush(stdout);

            fd_set fds;
            struct timeval tv = {1, 0};
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);
            if (select(STDIN_FILENO+1, &fds, NULL, NULL, &tv) > 0) {
                char buf[16];
                if (fgets(buf, 16, stdin)) {
                    if (buf[0] == 'q' || buf[0] == 'Q') break;
                    printf("\n  🏁 Lap %d: %02d:%02d\n", ++laps, elapsed/60, elapsed%60);
                }
            }
        }
    }

    printf("\n  Done! Press Enter to exit...");
    fflush(stdout);
    getchar();

    send_termination_notice(getpid());
    return 0;
}
