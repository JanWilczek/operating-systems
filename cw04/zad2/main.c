#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include "monitor.h"
#include <stdio.h>

void print_usage(FILE* stream, const char* program_name)
{
    fprintf(stream, "Usage: %s { option argument }\n", program_name);
    fprintf(stream, "   -h  --help          Display this usage information.\n"
                    "   -f  --file          Path to the file with the lists of files to monitor\n"
                    "                       in format: \"filename seconds_count\" in each line\n"
                    "                       where filename is the path to the file to monitor and\n"
                    "                       seconds_count is the update interval of the monitoring.\n"
                    );
}

int main(int argc, char* argv[])
{
    const char* const short_opts = "hf:";
    const struct option long_opts[] = {
        { "help",           0, NULL, 'h'},
        { "file",           1, NULL, 'f'},
        { NULL,             0, NULL, 0  }
    };

    int next_opt = 0;
    char* files_to_monitor = NULL;

    while (next_opt != -1)
    {
        next_opt = getopt_long(argc, argv, short_opts, long_opts, NULL);

        switch (next_opt) {
            case 'h':
                print_usage(stdout, argv[0]);
                return 0;
            case 'f':
                files_to_monitor = optarg;
                break;
            case -1:
                break;
            case '?':
                print_usage(stderr, argv[0]);
                return 1;
            default:
                abort();
        }
    }

    if (files_to_monitor == NULL)
    {
        fprintf(stderr, "Not all arguments passed.\n");
        return 1;
    }

    monitor_t* monitor = monitor_create();
    monitor_parse_files(monitor, files_to_monitor);

    monitor_start(monitor);

    monitor_free(monitor);

    return 0;
}