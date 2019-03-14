#include "stop_watch.h"
#include "memblocks.h"
#include <stdio.h>
#include <sys/times.h>

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

    if (argc < 2)
    {
        fprintf(stderr, "Usage: ./speed_test output_filename");
        return 1;
    }

    memblocks_table_t* table = create_table(30u);
    stop_watch_t* stop_watch = malloc(sizeof(stop_watch_t));
    
    stop_watch->filename = argv[1];

    const char* dirs[] = { "/home", "/usr/lib", "/usr"};
    const char* files[] = {"main.c", "'*.so'", "'*.bin'"};
    char* preambles[] = {"Small dataset find and allocation", "Middle-sized dataset find and allocation", 
    "Big dataset find and allocation", "Small dataset block deletion", 
    "Middle-sized dataset block deletion", "Big dataset block deletion"};

    for (int i = 0; i < 3; ++i)
    {
        set_search_dir(table, dirs[i]);
        set_searched_filename(table, files[i]);

        // TIME MEASUREMENT
        // 1. Find + block allocation
        stop_watch->preamble = preambles[i];
        start_time_measurement(stop_watch);
        size_t index = run_find(table, NULL);
        stop_and_write(stop_watch);

        // 2. Block deletion
        stop_watch->preamble = preambles[i + 3];
        start_time_measurement(stop_watch);
        free_block_at_index(table, index);
        stop_and_write(stop_watch);
    }

    // 3. Find + allocation mixed with block deletion
    stop_watch->preamble = "Allocation with deletion";
    start_time_measurement(stop_watch);
    for (int i = 0; i < 10; ++i)
    {
        size_t index = run_find(table, NULL);
        free_block_at_index(table, index);
    }
    stop_and_write(stop_watch);

    free_table(table);
    free_stop_watch(stop_watch);

    struct tms* program_end_time = malloc(sizeof(struct tms));
    times(program_end_time);

    printf("Kernel mode program time: %ld\nUser mode program time: %ld\n",
        program_end_time->tms_stime - program_time->tms_stime,
        program_end_time->tms_utime - program_time->tms_utime);

    free(program_time);
    free(program_end_time);

    return 0;
}
