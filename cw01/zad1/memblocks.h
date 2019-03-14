#pragma once
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct memblocks_table
{
    char** table_ptr;
    char* current_dir;
    char* searched_filename;
    size_t first_free_block_index;
    size_t size;
} memblocks_table_t;

extern memblocks_table_t* create_table(size_t size);
extern void free_table(memblocks_table_t* memblocks_ptr);
extern void set_search_dir(memblocks_table_t* memblocks_ptr, const char* dir_name);
extern void set_searched_filename(memblocks_table_t* memblocks_ptr, const char* filename);

/**
 * @param target_filename
 *      if NULL is provided a temporary file in the /tmp folder is created
 * */
extern size_t run_find(memblocks_table_t* memblocks_ptr, const char* target_filename);
extern void free_block_at_index(memblocks_table_t* memblocks_ptr, size_t block_index);


#ifdef __cplusplus
}
#endif

