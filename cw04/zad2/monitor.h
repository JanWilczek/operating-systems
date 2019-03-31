#pragma once
#include <sys/types.h>

enum monitor_mode { CONSTANT_STORE, COPY_ON_BACKUP };

typedef struct monitor {
    char** files_to_monitor;
    int* monitor_interval;
    pid_t* pids;
    int file_count;
} monitor_t;

monitor_t* monitor_create();
int monitor_parse_files(monitor_t* monitor, const char* filename);
void monitor_start(monitor_t* monitor);
void monitor_free(monitor_t* monitor);

