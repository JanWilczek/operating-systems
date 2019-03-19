#pragma once
#ifdef __cplusplus
extern "C" {
#endif

extern int generate_random_records(const char* filename, int number_of_records_to_generate, int record_size);

/**
 * @param library_name
 *      may be "sys" or "lib": the former will use system file manipulation functions the latter
 *      functions from the standard C library.
 * */
extern int sort_records(const char* filename, int number_of_records, int record_size, const char* library_name);

#ifdef __cplusplus
}
#endif