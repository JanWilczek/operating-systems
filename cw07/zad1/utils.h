#pragma once
#include <time.h>

#ifdef __cplusplus
    extern "C" {
#endif

char* get_precise_time(void);
char* format_time(struct timespec* tm);

#ifdef __cplusplus
    }
#endif
