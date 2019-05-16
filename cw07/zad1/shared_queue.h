#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

extern void queue_init(int size);
extern void put_to_queue(int element);
extern int get_from_queue(void);
extern int queue_capacity(void);
extern void queue_close(void);

#ifdef __cplusplus
    }
#endif
