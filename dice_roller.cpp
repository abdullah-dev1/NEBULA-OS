#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "kernel/ipc_manager.h"

void draw_die(int value) {
    const char* dice[] = {
        " ╔═══════╗ \n ║       ║ \n ║   ●   ║ \n ║       ║ \n ╚═══════╝ ",
        " ╔═══════╗ \n ║ ●     ║ \n ║       ║ \n ║     ● ║ \n ╚═══════╝ ",
        " ╔═══════╗ \n ║ ●     ║ \n ║   ●   ║ \n ║     ● ║ \n ╚═══════╝ ",
        " ╔═══════╗ \n ║ ●   ● ║ \n ║       ║ \n ║ ●   ● ║ \n ╚═══════╝ ",
        " ╔═══════╗ \n ║ ●   ● ║ \n ║   ●   ║ \n ║ ●   ● ║ \n ╚═══════╝ ",
        " ╔═══════╗ \n ║ ●   ● ║ \n ║ ●   ● ║ \n ║ ●   ● ║ \n ╚═══════╝ "
    };
    if (value >= 1 && value <= 6) {
        printf("%s\n", dice[value-1]);
    }
}

int main() {
    send_resource_request("Dice Roller", 20, 1);
    if (!wait_for_grant()) return 1;

    srand(time(NULL) ^ getpid());
    system("clear");

    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                🎲 NEBULA OS: COMIC DICE 🎲                ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("  (Press Ctrl+Z to Minimize)\n\n");

    while (1) {
        printf("  How many dice to roll? 🎲 (1-6, 0 to quit): ");
        fflush(stdout);
        char buf[16];
        if (!fgets(buf, 16, stdin)) break;
        int count = atoi(buf);
        if (count == 0) break;
        if (count < 0 || count > 6) {
            printf("  🛑 Whoops! Stick to 1-6 dice, please! 🚫\n");
            continue;
        }

        printf("\n  ✨ ROLLING THE DICE... ✨\n");
        int total = 0;
        for (int i = 0; i < count; i++) {
            usleep(200000); // Add a little dramatic delay
            int roll = (rand() % 6) + 1;
            printf("  Die #%d:\n", i+1);
            draw_die(roll);
            total += roll;
        }
        printf("\n  🏆 TOTAL SCORE: %d 🏆\n\n", total);
        printf("  ──────────────────────────────────────────────────────────\n");
    }

    printf("\n  👋 Thanks for playing! Stay lucky! 🍀\n");
    send_termination_notice(getpid());
    return 0;
}
