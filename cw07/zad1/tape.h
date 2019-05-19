#pragma once
#include "shared_resources.h"

#ifdef __cplusplus
    extern "C" {
#endif

extern void tape_init(int K, int M);
extern void tape_put_package(int N);
extern struct queue_entry* tape_get_package(void);
extern void tape_close(void);

#ifdef __cplusplus
    }
#endif
