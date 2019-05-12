#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

extern void tape_init(int M, int K);
extern void tape_put_package(int N);
extern int tape_get_package();

#ifdef __cplusplus
    }
#endif
