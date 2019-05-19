#pragma once
#include <time.h>
#include "shared_resources.h"

#ifdef __cplusplus
    extern "C" {
#endif

extern void queue_init(int size, int max_units);
extern void put_to_queue(struct queue_entry* element);
extern int get_from_queue(struct queue_entry * element);
extern int queue_capacity(void);
extern int queue_size(void);
extern int queue_max_units(void);
extern int queue_units_sum(void);
extern void queue_close(void);

#ifdef __cplusplus
    }
#endif
