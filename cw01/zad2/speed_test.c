#include "stop_watch.h"
#include <stdio.h>
#include <sys/times.h>
#include "memblocks.h"
#ifdef DYNAMIC_LOAD
#include <dlfcn.h>
#endif

/**
 * Scenarios:
 *  Small: find ~ -name main.c
 *  Middle: find /usr/lib -name '*.so'
 *  Big: find /usr -name '*.bin'
 * */
int main(int argc, char* argv[])
{
    struct tms* program_time = malloc(sizeof(struct tms));
    times(program_time);

#ifdef DYNAMIC_LOAD
    void* memblock_library_handle = dlopen("libmemblocks.so", RTLD_LAZY);

    memblocks_table_t* (*memblocks_create_table)(size_t);
    memblocks_create_table = (memblocks_table_t* (*)(size_t)) dlsym(memblock_library_handle, "create_table");

    void (*memblocks_free_table)(memblocks_table_t*);
    memblocks_free_table = (void (*)(memblocks_table_t*)) dlsym(memblock_library_handle, "free_table");

    void (*memblocks_set_search_dir)(memblocks_table_t*, const char*);
    memblocks_set_search_dir = (void (*)(memblocks_table_t*, const char*)) dlsym(memblock_library_handle, "set_search_dir");

    void (*memblocks_set_searched_filename)(memblocks_table_t*, const char*);
    memblocks_set_searched_filename = (void (*)(memblocks_table_t*, const char*)) dlsym(memblock_library_handle, "set_searched_filename");

    size_t (*memblocks_run_find)(memblocks_table_t*, const char*);
    memblocks_run_find = (size_t (*)(memblocks_table_t*, const char*)) dlsym(memblock_library_handle, "run_find");

    void (*memblocks_free_block_at_index)(memblocks_table_t*, size_t);
    memblocks_free_block_at_index = (void (*)(memblocks_table_t*, size_t)) dlsym(memblock_library_handle, "free_block_at_index");
#endif

    if (argc < 2)
    {
        fprintf(stderr, "Usage: ./speed_test output_filename [session_commentary]\n");
        return 1;
    }

    if (argc == 3)
    {
        FILE* file = fopen(argv[1], "a");
        if (file != NULL)
        {
            fprintf(file, "\n%s\n", argv[2]);
            fflush(file);
            fclose(file);
        }
    }

    memblocks_table_t* table;
    size_t initial_size = 30u;
#ifdef DYNAMIC_LOAD
    table = (*memblocks_create_table)(initial_size);
#else
    table = create_table(initial_size);
#endif

    stop_watch_t* stop_watch = malloc(sizeof(stop_watch_t));
    
    stop_watch->filename = argv[1];

    const char* dirs[] = { "/home", "/usr/lib", "/usr"};
    const char* files[] = {"main.c", "'*.so'", "'*.bin'"};
    char* preambles[] = {"Small dataset find and allocation", "Middle-sized dataset find and allocation", 
    "Big dataset find and allocation", "Small dataset block deletion", 
    "Middle-sized dataset block deletion", "Big dataset block deletion"};

    for (int i = 0; i < 3; ++i)
    {
#ifdef DYNAMIC_LOAD
        (*memblocks_set_search_dir)(table, dirs[i]);
        (*memblocks_set_searched_filename)(table, files[i]);
#else
        set_search_dir(table, dirs[i]);
        set_searched_filename(table, files[i]);
#endif

        // TIME MEASUREMENT
        // 1. Find + block allocation
        stop_watch->preamble = preambles[i];
        start_time_measurement(stop_watch);
#ifdef DYNAMIC_LOAD
        size_t index = (*memblocks_run_find)(table, NULL);
#else
        size_t index = run_find(table, NULL);
#endif
        stop_and_write(stop_watch);

        // 2. Block deletion
        stop_watch->preamble = preambles[i + 3];
        start_time_measurement(stop_watch);
#ifdef DYNAMIC_LOAD
        (*memblocks_free_block_at_index)(table, index);
#else
        free_block_at_index(table, index);
#endif
        stop_and_write(stop_watch);
    }

    // 3. Find + allocation mixed with block deletion
    stop_watch->preamble = "Allocation with deletion";
    start_time_measurement(stop_watch);
    for (int i = 0; i < 10; ++i)
    {
#ifdef DYNAMIC_LOAD
        size_t index = (*memblocks_run_find)(table, NULL);
        (*memblocks_free_block_at_index)(table, index);
#else
        size_t index = run_find(table, NULL);
        free_block_at_index(table, index);
#endif
    }
    stop_and_write(stop_watch);

#ifdef DYNAMIC_LOAD
    (*memblocks_free_table)(table);
#else
    free_table(table);
#endif

    free_stop_watch(stop_watch);

    struct tms* program_end_time = malloc(sizeof(struct tms));
    times(program_end_time);

    printf("Kernel mode program time: %ld\nUser mode program time: %ld\n",
        program_end_time->tms_stime - program_time->tms_stime,
        program_end_time->tms_utime - program_time->tms_utime);

    free(program_time);
    free(program_end_time);

#ifdef DYNAMIC_LOAD
    dlclose(memblock_library_handle);
#endif

    return 0;
}
