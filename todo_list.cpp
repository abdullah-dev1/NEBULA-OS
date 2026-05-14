#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "kernel/ipc_manager.h"

#define MAX_TODOS 50
#define TODO_SIZE 128

typedef struct {
    char text[TODO_SIZE];
    int done;
} Todo;

static Todo todos[MAX_TODOS];
static int todo_count = 0;
const char* TODO_FILE = "nebula_hdd/todo.txt";

void load_todos() {
    FILE* f = fopen(TODO_FILE, "r");
    if (!f) return;
    char line[TODO_SIZE + 8];
    while (fgets(line, sizeof(line), f) && todo_count < MAX_TODOS) {
        if (line[0] == '[') {
            todos[todo_count].done = (line[1] == 'X');
            strncpy(todos[todo_count].text, line + 4, TODO_SIZE - 1);
            todos[todo_count].text[strlen(todos[todo_count].text)-1] = '\0';
            todo_count++;
        }
    }
    fclose(f);
}

void save_todos() {
    FILE* f = fopen(TODO_FILE, "w");
    if (!f) return;
    for (int i = 0; i < todo_count; i++) {
        fprintf(f, "[%s] %s\n", todos[i].done ? "X" : " ", todos[i].text);
    }
    fclose(f);
}

void list_todos() {
    printf("\n--- TO-DO LIST ---\n");
    if (todo_count == 0) printf("  (List empty)\n");
    for (int i = 0; i < todo_count; i++) {
        printf("  %d. [%s] %s\n", i + 1, todos[i].done ? "X" : " ", todos[i].text);
    }
}

int main() {
    send_resource_request("To-Do List", 30, 8);
    if (!wait_for_grant()) return 1;

    load_todos();

    printf("\n--- NEBULA OS TO-DO LIST ---\n");
    printf("  (Press Ctrl+Z to Minimize)\n\n");

    char buf[16];
    while (1) {
        list_todos();
        printf("\n  [A]dd, [D]one, [R]emove, [Q]uit: ");
        fflush(stdout);
        if (!fgets(buf, 16, stdin)) break;
        char c = buf[0];
        if (c == 'q' || c == 'Q') break;
        if (c == 'a' || c == 'A') {
            printf("  Enter task: ");
            fflush(stdout);
            if (todo_count < MAX_TODOS) {
                fgets(todos[todo_count].text, TODO_SIZE, stdin);
                todos[todo_count].text[strlen(todos[todo_count].text)-1] = '\0';
                todos[todo_count].done = 0;
                todo_count++;
                save_todos();
            }
        } else if (c == 'd' || c == 'D') {
            printf("  Number to mark done: ");
            fflush(stdout);
            char nbuf[16];
            fgets(nbuf, 16, stdin);
            int idx = atoi(nbuf) - 1;
            if (idx >= 0 && idx < todo_count) {
                todos[idx].done = 1;
                save_todos();
            }
        } else if (c == 'r' || c == 'R') {
            printf("  Number to remove: ");
            fflush(stdout);
            char nbuf[16];
            fgets(nbuf, 16, stdin);
            int idx = atoi(nbuf) - 1;
            if (idx >= 0 && idx < todo_count) {
                for (int i = idx; i < todo_count - 1; i++) todos[i] = todos[i+1];
                todo_count--;
                save_todos();
            }
        }
    }

    send_termination_notice(getpid());
    return 0;
}
