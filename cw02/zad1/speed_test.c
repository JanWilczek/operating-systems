#include "file_operations.h"
#include "stop_watch.h"
#include <stdio.h>

char* prepare_preamble(const char* preamble_format, int nb_records, int record_size, const char* lib_to_use)
{
    size_t preamble_length = 70u;
    char* preamble = malloc(preamble_length * sizeof(char));
    snprintf(preamble, preamble_length, preamble_format, nb_records, record_size, lib_to_use);
    return preamble;
}

void copy_speed_test(stop_watch_t* stop_watch, const char* filename, const char* filename2, int nb_records, int record_size, const char* lib_to_use)
{
    // Preamble preparation
    const char preamble_copy_format[] = "Copying %d records of size %d using %s functions";
    char* preamble_copy = prepare_preamble(preamble_copy_format, nb_records, record_size, lib_to_use);
    stop_watch->preamble = preamble_copy;

    // Actual measurement
    start_time_measurement(stop_watch);
    copy_records(filename, filename2, nb_records, record_size, lib_to_use);
    stop_and_write(stop_watch, WRITE_KERNEL_TIME | WRITE_USER_TIME);    

    // Resource deallocation
    free(preamble_copy);
}

void sort_speed_test(stop_watch_t* stop_watch, const char* filename, int nb_records, int record_size, const char* lib_to_use)
{
    // Preamble preparation
    const char preamble_sort_format[] = "Sorting %d records of size %d using %s functions";
    char* preamble_sort = prepare_preamble(preamble_sort_format, nb_records, record_size, lib_to_use);
    stop_watch->preamble = preamble_sort;

    // Actual measurement
    start_time_measurement(stop_watch);
    sort_records(filename, nb_records, record_size, lib_to_use);
    stop_and_write(stop_watch, WRITE_KERNEL_TIME | WRITE_USER_TIME);    

    // Resource deallocation
    free(preamble_sort);
}

void append_commentary(const char* filename)
{
    FILE* file = fopen(filename, "a");
    if (file != NULL)
    {
        const char message[] = "\n\nMożna zauważyć, że kopiowanie przy użyciu biblioteki standardowej C jest minimalnie\n"
                                "szybsze od systemowego w prawie każdym przypadku (oprócz 4000 rekordów o rozmiarze\n"
                                "8192), natomiast sortowanie zawsze jest zdecydowanie szybsze przy użyciu funkcji systemowych.\n"
                                "Wynika to z faktu, że funkcje biblioteki standardowej są opakowaniami funkcji systemowych, co\n"
                                "sprawia, że zawsze bezpośrednie użycie funkcji systemowych powinno dać lepsze wyniki, kosztem\n"
                                "przenoszalności kodu.";

        fwrite(message, sizeof(message) - 1, 1, file);
        fclose(file);
    }
}

int main(int argc, char* argv[])
{
    int record_sizes[] = {1, 4, 512, 1024, 4096, 8192};
    int nb_records[] = {4000, 10000};
    char output_filename[] = "wyniki.txt";
    remove(output_filename);

    for (int size_id = 0; size_id < 6; ++size_id)
    {
        for (int nb_id = 0; nb_id < 2; ++nb_id)
        {
            const char filename[] = "data";
            const char filename2[] = "data2";
            stop_watch_t* stop_watch = malloc(sizeof(stop_watch_t));
            stop_watch->filename = output_filename;

            generate_random_records(filename, nb_records[nb_id], record_sizes[size_id]);

            // COPYING TIME MEASUREMENT
            // Using cstdlib functions
            copy_speed_test(stop_watch, filename, filename2, nb_records[nb_id], record_sizes[size_id], "lib");

            // Using system functions
            copy_speed_test(stop_watch, filename, filename2, nb_records[nb_id], record_sizes[size_id], "sys");

            // SORTING TIME MEASUREMENT
            // Using cstdlib functions
            sort_speed_test(stop_watch, filename, nb_records[nb_id], record_sizes[size_id], "lib");

            // Using system functions
            sort_speed_test(stop_watch, filename2, nb_records[nb_id], record_sizes[size_id], "sys");

            // Resource deallocation and cleanup
            free_stop_watch(stop_watch);
            remove(filename);
            remove(filename2);
        }
    }

    append_commentary(output_filename);

    return 0;
}