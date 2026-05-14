#include <stdio.h>
#include <unistd.h>
#include "kernel/ipc_manager.h"

int main() {
    send_resource_request("Instruction Guide", 15, 0);
    if (!wait_for_grant()) return 1;

    printf("\n--- NEBULA OS INSTRUCTION GUIDE ---\n");
    printf("  (Press Ctrl+Z to Minimize)\n\n");

    printf("  TASK LIST:\n");
    printf("  1. Calculator     2. Notepad       3. Clock         4. Calendar\n");
    printf("  5. Create File    6. Delete File   7. Move File     8. Copy File\n");
    printf("  9. File Info     10. Task Manager 11. Music Player 12. Minigame\n");
    printf("  13. Print File   14. Guide        15. Dice Roller  16. Rename File\n");
    printf("  17. Text Search  18. Alarm        19. Stopwatch    20. Compression\n");
    printf("  21. Log Viewer   22. Terminal     23. To-Do List   24. Dashboard\n\n");

    printf("  OS CONCEPTS:\n");
    printf("  - Process Management (PCB, fork, exec)\n");
    printf("  - Multilevel Queue Scheduling (Priority/RR/FCFS)\n");
    printf("  - Memory Management (RAM/HDD allocation)\n");
    printf("  - IPC via Named Pipes (FIFO)\n");
    printf("  - Synchronization (Mutex, Semaphore, CondVar)\n");
    printf("  - Signal Handling (SIGUSR1, SIGUSR2, SIGCHLD)\n");
    printf("  - Deadlock Detection (RAG + DFS)\n\n");

    printf("  Press Enter to exit...");
    fflush(stdout);
    getchar();

    send_termination_notice(getpid());
    return 0;
}
