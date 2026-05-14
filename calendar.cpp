#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "kernel/ipc_manager.h"

int is_leap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int get_days_in_month(int m, int y) {
    int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && is_leap(y)) return 29;
    return days[m];
}

int get_start_day(int m, int y) {
    struct tm t;
    memset(&t, 0, sizeof(t));
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = 1;
    mktime(&t);
    return t.tm_wday;
}

void display_calendar(int m, int y) {
    const char* months[] = {"", "January", "February", "March", "April", "May", "June", 
                            "July", "August", "September", "October", "November", "December"};
    
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                   NEBULA OS SYSTEM CALENDAR               ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("  [ %s %d ]\n\n", months[m], y);
    printf("  Su  Mo  Tu  We  Th  Fr  Sa\n");
    printf("  ──────────────────────────\n  ");

    int start = get_start_day(m, y);
    int total = get_days_in_month(m, y);

    for (int i = 0; i < start; i++) printf("    ");
    for (int d = 1; d <= total; d++) {
        printf("%2d  ", d);
        if ((start + d) % 7 == 0) printf("\n  ");
    }
    printf("\n  ──────────────────────────\n");
}

int main() {
    send_resource_request("Calendar", 25, 2);
    if (!wait_for_grant()) return 1;

    time_t now = time(NULL);
    struct tm* cur = localtime(&now);
    int m = cur->tm_mon + 1;
    int y = cur->tm_year + 1900;

    while (1) {
        system("clear");
        display_calendar(m, y);
        printf("\n  [N] Next Month  [P] Prev Month  [Q] Quit\n");
        printf("  Calendar> ");
        fflush(stdout);
        
        char buf[16];
        if (!fgets(buf, 16, stdin)) break;
        char c = buf[0];
        if (c == 'q' || c == 'Q') break;
        if (c == 'n' || c == 'N') {
            m++; if (m > 12) { m = 1; y++; }
        } else if (c == 'p' || c == 'P') {
            m--; if (m < 1) { m = 12; y--; }
        }
    }

    send_termination_notice(getpid());
    return 0;
}
