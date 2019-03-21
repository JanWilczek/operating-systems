#include "file_operations.h"
#include "stop_watch.h"
#include <stdio.h>

void copy_speed_test()
{
    
}

int main(int argc, char* argv[])
{
    int record_sizes[] = {1, 4, 512, 1024, 4096, 8192};
    int nb_records[] = {4000, 11000};

    for (int size_id = 0; size_id < 6; ++size_id)
    {
        for (int nb_id = 0; nb_id < 2; ++nb_id)
        {
            const char filename[] = "data";
            const char filename2[] = "data2";
            stop_watch_t* stop_watch = malloc(sizeof(stop_watch_t));
            stop_watch->filename = "wyniki.txt";
            const char preamble_copy_template[] = "Copying %d records of size %d using %s functions";
            const char preamble_sort_template[] = "Sorting %d records of size %d using %s functions";
            size_t preamble_length = 70u;
            char* preamble_copy = malloc(preamble_length * sizeof(char));

            generate_random_records(filename, nb_records[nb_id], record_sizes[size_id]);

            // COPYING TIME MEASUREMENT
            // Using cstdlib
            snprintf(preamble_copy, preamble_length, preamble_copy_template, nb_records[nb_id], record_sizes[size_id], "lib");
            stop_watch->preamble = preamble_copy;
            start_time_measurement(stop_watch);
            copy_records(filename, filename2, nb_records[nb_id], record_sizes[size_id], "lib");
            stop_and_write(stop_watch, WRITE_KERNEL_TIME | WRITE_USER_TIME);

            free_stop_watch(stop_watch);
            free(preamble_copy);
        }
    }

    return 0;
}