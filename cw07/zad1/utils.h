#pragma once
#include <time.h>

#ifdef __cplusplus
    extern "C" {
#endif

char* get_precise_time(void);
char* format_time(struct timespec* tm);
void print_message(const char* message, const char* preamble);

#ifdef __cplusplus
    }
#endif
