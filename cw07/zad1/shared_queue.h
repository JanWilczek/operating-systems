#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

extern void queue_init(int size);
extern void put_to_queue(int element);
extern int get_from_queue();
extern int queue_capacity();
extern int queue_size();

#ifdef __cplusplus
    }
#endif
