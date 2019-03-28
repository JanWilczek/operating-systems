#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include "monitor.h"

int main(int argc, char* argv[])
{
    const char* const short_opts = "f:m:c:";
    const struct option long_opts[] = {
        { "file",           1, NULL, 'f'},
        { "monitor-time",   1, NULL, 'm'},
        { "copy-mode",      1, NULL, 'c'},
        { NULL,             0, NULL, 0  }
    };

    int next_opt = 0;
    char* files_to_monitor = NULL;
    int monitor_time_seconds = 0;
    int copy_mode = -1;

    while (next_opt != -1)
    {
        next_opt = getopt_long(argc, argv, short_opts, long_opts, NULL);

        switch (next_opt) {
            case 'f':
                files_to_monitor = optarg;
                break;
            case 'm':
                monitor_time_seconds = atoi(optarg);
                break;
            case 'c':
                copy_mode = atoi(optarg);
                break;
            case '?':
                break;
            case -1:
                break;
            default:
                abort();
        }
    }

    if (files_to_monitor == NULL || monitor_time_seconds <= 0 || (copy_mode != 0 && copy_mode != 1))
    {
        fprintf(stderr, "Not all arguments passed.\n");
        return 1;
    }

    monitor_t* monitor = monitor_create();
    monitor_set_monitor_time(monitor, monitor_time_seconds);
    monitor_set_copy_mode(monitor, copy_mode);
    monitor_parse_files(monitor, files_to_monitor);

    monitor_start(monitor);

    monitor_free(monitor);

    return 0;
}