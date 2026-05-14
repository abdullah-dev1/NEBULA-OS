#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include "kernel/ipc_manager.h"

#define MAX_SONGS 20

void list_songs(char songs[MAX_SONGS][256], int* count) {
    DIR *d;
    struct dirent *dir;
    d = opendir("music");
    *count = 0;
    if (d) {
        while ((dir = readdir(d)) != NULL && *count < MAX_SONGS) {
            if (strstr(dir->d_name, ".mp3")) {
                strncpy(songs[*count], dir->d_name, 255);
                (*count)++;
            }
        }
        closedir(d);
    }
}

int main() {
    send_resource_request("Music Player", 40, 10);
    if (!wait_for_grant()) return 1;

    char songs[MAX_SONGS][256];
    int song_count = 0;

    while (1) {
        system("clear");
        printf("╔═══════════════════════════════════════════════════════════╗\n");
        printf("║                   NEBULA OS MUSIC PLAYER                  ║\n");
        printf("╚═══════════════════════════════════════════════════════════╝\n");

        list_songs(songs, &song_count);
        if (song_count == 0) {
            printf("  [!] No .mp3 files found in music/ folder.\n");
            printf("  Press Enter to exit...");
            fflush(stdout);
            getchar();
            break;
        }

        printf("  Available Songs:\n");
        for (int i = 0; i < song_count; i++) {
            printf("  [%2d] %s\n", i + 1, songs[i]);
        }
        printf("\n  [Q] Quit OS Dashboard\n");
        printf("\n  Select a song to play (1-%d): ", song_count);
        fflush(stdout);

        char buf[64];
        if (!fgets(buf, 64, stdin)) break;
        if (buf[0] == 'q' || buf[0] == 'Q') break;

        int choice = atoi(buf);
        if (choice < 1 || choice > song_count) continue;

        char path[512];
        sprintf(path, "music/%s", songs[choice - 1]);

        system("clear");
        printf("╔═══════════════════════════════════════════════════════════╗\n");
        printf("║                   NEBULA OS MUSIC PLAYER                  ║\n");
        printf("╚═══════════════════════════════════════════════════════════╝\n");
        printf("\n  ♪ NOW PLAYING: %s\n", songs[choice - 1]);
        printf("  ──────────────────────────────────────────────────────────\n");
        printf("  (Playing via mpg123... Press 'q' to stop this song)\n\n");

        pid_t player_pid = fork();
        if (player_pid == 0) {
            // Child: run mpg123
            // Redirect stderr to null to keep UI clean
            int devnull = open("/dev/null", O_WRONLY);
            dup2(devnull, 2);
            close(devnull);
            execlp("mpg123", "mpg123", "-q", path, (char*)NULL);
            _exit(1);
        }

        // Parent: monitor for 'q'
        int playing = 1;
        while (playing) {
            int status;
            if (waitpid(player_pid, &status, WNOHANG) != 0) {
                // Player finished
                playing = 0;
                break;
            }

            // Check for 'q'
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);
            struct timeval tv = {0, 500000}; // 0.5s check
            if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
                char c;
                if (read(STDIN_FILENO, &c, 1) > 0) {
                    if (c == 'q' || c == 'Q') {
                        kill(player_pid, SIGTERM);
                        waitpid(player_pid, &status, 0);
                        playing = 0;
                    }
                }
            }
        }
        printf("\n  ♪ Song stopped.\n");
        sleep(1);
    }

    send_termination_notice(getpid());
    return 0;
}
