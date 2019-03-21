#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

// A flag needed to use the nftw function
#define _XOPEN_SOURCE 500 

extern void print_dir_info(const char* path, char comparison_operator, char* date);
extern void print_dir_info_nftw(const char* path, char comparison_operator, char* date);

#ifdef __cplusplus
    }
#endif