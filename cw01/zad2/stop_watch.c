#include "stop_watch.h"
#include <stdio.h>

void start_time_measurement(stop_watch_t* sw)
{
    sw->start = malloc(sizeof(struct tms));
    sw->stop = malloc(sizeof(struct tms));

    sw->realtime_start = times(sw->start);
}

void stop(stop_watch_t* sw)
{
    sw->realtime_stop = times(sw->stop);
}

void write(stop_watch_t* sw)
{
    FILE* file = fopen(sw->filename, "a");
    if (file != NULL)
    {
        clock_t realtime = sw->realtime_stop - sw->realtime_start;
        clock_t kernel_mode_time = sw->stop->tms_stime - sw->start->tms_stime;
        clock_t user_mode_time = sw->stop->tms_utime - sw->start->tms_utime;

        fprintf(file, "%s - real time: %ld\n", sw->preamble, realtime);
        fprintf(file, "%s - kernel mode time: %ld\n", sw->preamble, kernel_mode_time);
        fprintf(file, "%s - user mode time: %ld\n", sw->preamble, user_mode_time);

        fclose(file);
    }
}

void stop_and_write(stop_watch_t* sw)
{
    stop(sw);
    write(sw);
}

void free_stop_watch(stop_watch_t* sw)
{
    free(sw->start);
    free(sw->stop);
    free(sw);
}
