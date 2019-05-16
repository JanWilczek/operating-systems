#pragma once
#include <time.h>
#include "shared_resources.h"

#ifdef __cplusplus
    extern "C" {
#endif

extern void queue_init(int size);
extern void put_to_queue(struct queue_entry* element);
extern int get_from_queue(struct queue_entry * element);
extern int queue_capacity(void);
extern void queue_close(void);

#ifdef __cplusplus
    }
#endif
