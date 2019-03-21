#include "stop_watch.h"
#include <stdio.h>
#include <unistd.h>

void start_time_measurement(stop_watch_t* sw)
{
   // sw->start = malloc(sizeof(struct tms));
    //sw->stop = malloc(sizeof(struct tms));

    //sw->realtime_start = times(sw->start);
    sw->realtime_start = times(&sw->start);
}

void stop(stop_watch_t* sw)
{
    //sw->realtime_stop = times(sw->stop);
    sw->realtime_stop = times(&sw->stop);
}

void write_times(stop_watch_t* sw, int flags)
{
    FILE* file = fopen(sw->filename, "a");
    if (file != NULL)
    {
        if (flags & WRITE_REALTIME)
        {
            int realtime = get_realtime_in_ms(sw);
            fprintf(file, "%s - real time: %d ms\n", sw->preamble, realtime);
        }

        if (flags & WRITE_KERNEL_TIME)
        {
            int kernel_mode_time = get_kernel_time_in_ms(sw);
            fprintf(file, "%s - kernel mode time: %d ms\n", sw->preamble, kernel_mode_time);
        }
        //clock_t kernel_mode_time = sw->stop->tms_stime - sw->start->tms_stime;
        //clock_t user_mode_time = sw->stop->tms_utime - sw->start->tms_utime;

        if (flags & WRITE_USER_TIME)
        {
            int user_mode_time = get_user_time_in_ms(sw);
            fprintf(file, "%s - user mode time: %d ms\n", sw->preamble, user_mode_time);
        }

        fclose(file);
    }
}

void stop_and_write(stop_watch_t* sw, int flags)
{
    stop(sw);
    write_times(sw, flags);
}

void free_stop_watch(stop_watch_t* sw)
{
    //free(sw->start);
    //free(sw->stop);
    free(sw);
}

int get_realtime_in_ms(stop_watch_t* sw)
{
    const int clock_ticks_per_second = sysconf(_SC_CLK_TCK);
    const int ticks_passed = sw->realtime_stop - sw->realtime_start;
    return (int) 1000.f * ticks_passed / ((float) clock_ticks_per_second);
}

int get_user_time_in_ms(stop_watch_t* sw)
{
    const int clock_ticks_per_second = sysconf(_SC_CLK_TCK);
    const int ticks_passed = sw->stop.tms_utime - sw->start.tms_utime;
    return (int) 1000.f * ticks_passed / ((float) clock_ticks_per_second);
}

int get_kernel_time_in_ms(stop_watch_t* sw)
{
    const int clock_ticks_per_second = sysconf(_SC_CLK_TCK);
    const int ticks_passed = sw->stop.tms_stime - sw->start.tms_stime;
    return (int) 1000.f * ticks_passed / ((float) clock_ticks_per_second);
}
