#pragma once
#include "shared_resources.h"

#ifdef __cplusplus
    extern "C" {
#endif

extern void tape_init(int K, int M);
/* Returns 0 on success and -1 if the load is to0 big on tape and -2 if it's too big to ever be put on tape */
extern int tape_put_package(int N);
extern struct queue_entry* tape_get_package(void);
extern void tape_close(void);

#ifdef __cplusplus
    }
#endif
