#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"


char* get_precise_time(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return format_time(&spec);
}

char* format_time(struct timespec* spec)
{
    long ms = (long)(spec->tv_nsec / 1e6f) % 1000L;

    char* buffer = malloc(100 * sizeof(char));
    char temp[50];
    time_t seconds = (time_t) spec->tv_sec;
    struct tm* tms = localtime(&seconds);
    strftime(temp, sizeof(temp), "%H:%M:%S", tms);
    sprintf(buffer, "%s:%ld", temp, ms);

    return buffer;
}

void print_message(const char *message)
{
    char *time_stamp = get_precise_time();
    printf("%s %s\n", time_stamp, message);
    free(time_stamp);
}
