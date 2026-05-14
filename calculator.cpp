#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "kernel/ipc_manager.h"

int main() {
    send_resource_request("Calculator", 30, 2);
    if (!wait_for_grant()) return 1;

    printf("\n--- NEBULA OS CALCULATOR ---\n");
    printf("  (Press Ctrl+Z to Minimize)\n\n");
    printf("  Operations: +, -, *, /, %%\n");
    printf("  Type 'q' to quit.\n\n");

    char buf[128];
    while (1) {
        printf("  > ");
        fflush(stdout);
        if (!fgets(buf, 128, stdin)) break;
        if (buf[0] == 'q' || buf[0] == 'Q') break;

        double n1, n2;
        char op;
        if (sscanf(buf, "%lf %c %lf", &n1, &op, &n2) == 3) {
            double res = 0;
            int valid = 1;
            if (op == '+') res = n1 + n2;
            else if (op == '-') res = n1 - n2;
            else if (op == '*') res = n1 * n2;
            else if (op == '/') {
                if (n2 != 0) res = n1 / n2;
                else { printf("  Error: Div by zero\n"); valid = 0; }
            }
            else if (op == '%') {
                if (n2 != 0) res = fmod(n1, n2);
                else { printf("  Error: Mod by zero\n"); valid = 0; }
            }
            else { printf("  Error: Invalid operator\n"); valid = 0; }

            if (valid) printf("  Result: %.2f\n", res);
        } else {
            printf("  Error: Use format 'num op num'\n");
        }
    }

    send_termination_notice(getpid());
    return 0;
}
