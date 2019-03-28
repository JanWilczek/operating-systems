#include "monitor.h"
#include <stdlib.h>
#include <stdio.h>

int MAX_LINE_SIZE = 256;

monitor_t *monitor_create()
{
    return malloc(sizeof(monitor_t));
}

int get_line_count(FILE *file)
{
    rewind(file);

    int eof = 0;
    int nb_lines = 0;
    const char *line_format = "%s %d";
    int unused;
    char *line = malloc(MAX_LINE_SIZE * sizeof(char));

    while (eof != EOF)
    {
        eof = fscanf(file, line_format, line, &unused);
        if (eof != EOF)
        {
            ++nb_lines;
        }
    }

    free(line);
    rewind(file);

    return nb_lines;
}

int monitor_parse_files(monitor_t *monitor, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file != NULL)
    {
        int nb_seconds = 0;
        const char *line_format = "%s %d";

        int line_count = get_line_count(file);

        printf("%d line(s)\n", line_count);

        rewind(file);

        monitor->file_count = line_count;
        monitor->files_to_monitor = malloc(monitor->file_count * sizeof(char*));
        monitor->monitor_interval = malloc(monitor->file_count * sizeof(int));
        for (int i = 0; i < line_count; ++i)
        {
            char *line = malloc(MAX_LINE_SIZE * sizeof(char));

            // Read files to monitor and seconds to monitor them for
            fscanf(file, line_format, line, &nb_seconds);

            monitor->files_to_monitor[i] = line;
            monitor->monitor_interval[i] = nb_seconds;
        }

        fclose(file);
        return 0;
    }

    return 1;
}

void monitor_set_monitor_time(monitor_t *monitor, int seconds)
{
    monitor->monitor_time_seconds = seconds;
}

void monitor_set_copy_mode(monitor_t *monitor, enum monitor_mode mode)
{
    monitor->mode = mode;
}

void monitor_start(monitor_t *monitor)
{
}

void monitor_free(monitor_t *monitor)
{
    for (int i = 0; i < monitor->file_count; ++i)
    {
        free(monitor->files_to_monitor[i]);
    }
    free(monitor->files_to_monitor);
    free(monitor->monitor_interval);
    free(monitor);
}
