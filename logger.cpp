#include "logger.h"
#include "kernel.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h>

void log_event(const char* message) {
    pthread_mutex_lock(&log_lock);

    time_t now = time(0);
    struct tm* t = localtime(&now);
    char time_buf[16];
    strftime(time_buf, 16, "%H:%M:%S", t);

    FILE* f = fopen(SYSTEM_LOG_PATH, "a");
    if (f) {
        fprintf(f, "[%s]  %s\n", time_buf, message);
        fclose(f);
    }

    pthread_mutex_unlock(&log_lock);
}

void init_logger() {
    FILE* f = fopen(SYSTEM_LOG_PATH, "w");
    if (f) fclose(f);
}
