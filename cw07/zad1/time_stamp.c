#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "time_stamp.h"


char* get_precise_time(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    long us = spec.tv_nsec / 1e3f;
    long ms = spec.tv_nsec / 1e6f;

    char* buffer = malloc(100 * sizeof(char));
    char temp[50];
    time_t seconds = (time_t) spec.tv_sec;
    struct tm* tms = localtime(&seconds);
    strftime(temp, sizeof(temp), "%H:%M:%S", tms);
    sprintf(buffer, "%s:%ld:%ld", temp, ms, us);

    return buffer;
}
