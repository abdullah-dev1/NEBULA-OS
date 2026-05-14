#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "kernel/ipc_manager.h"

int main() {
    send_resource_request("Minigame", 35, 3);
    if (!wait_for_grant()) return 1;

    srand(time(NULL) ^ getpid());
    system("clear");

    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                🚀 NEBULA OS: SPACE GUESSER 🛸             ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("  (Press 'q' to quit | Press Ctrl+Z to minimize)\n\n");

    int playing = 1;
    while (playing) {
        int secret = (rand() % 100) + 1;
        int guesses = 7;
        int won = 0;

        printf("  👾 [NEW GAME] I'm thinking of a number (1-100).\n");
        printf("  🎯 You have %d attempts. Can you beat the alien? 👽\n\n", guesses);

        for (int i = 1; i <= guesses; i++) {
            printf("  [Attempt #%d] 📝 Your guess: ", i);
            fflush(stdout);
            
            char buf[16];
            if (!fgets(buf, 16, stdin)) { playing = 0; break; }
            
            if (buf[0] == 'q' || buf[0] == 'Q') {
                playing = 0;
                break;
            }

            int g = atoi(buf);
            if (g <= 0) {
                printf("  ⚠  Whoops! That's not a valid number! 🛑\n");
                i--; continue;
            }

            if (g == secret) {
                printf("\n  ✨ BOOM! ✨ The number was %d. You're a Space Legend! 👑\n", secret);
                won = 1;
                break;
            } else if (g < secret) {
                printf("  📈 HIGHER! (Too low...) ☁\n");
            } else {
                printf("  📉 LOWER! (Too high...) 🔥\n");
            }
        }

        if (!playing) break;
        if (!won) printf("\n  💥 KABOOM! Game over. The secret was %d. 🛸 Escaped!\n", secret);

        printf("\n  🔄 Play another round? (y/n): ");
        fflush(stdout);
        char buf[16];
        if (!fgets(buf, 16, stdin) || (buf[0] != 'y' && buf[0] != 'Y')) playing = 0;
        if (playing) system("clear");
    }

    printf("\n  👋 See you next time, Space Cowboy! 🤠\n");
    send_termination_notice(getpid());
    return 0;
}
