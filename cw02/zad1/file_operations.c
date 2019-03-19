#include "file_operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int generate_random_records(const char* filename, int number_of_records_to_generate, int record_size)
{
    FILE* file = fopen(filename, "w");
    if (file != NULL)
    {
        unsigned short int record_byte;

        for (int i = 0; i < number_of_records_to_generate; ++i)
        {
            for (int j = 0; j < record_size; ++j)
            {
                record_byte = (unsigned short int) rand();
                fprintf(file, "%d", record_byte);
            }

            fprintf(file, "\n");
        }

        fclose(file);

        return 0;
    }

    return errno;
}

int sort_records_system(const char* filename, int number_of_records, int record_size)
{
    return 1;
}

int sort_records_cstdlib(const char* filename, int number_of_records, int record_size)
{
    FILE* file = fopen(filename, "r+");
    if (file != NULL)
    {
        unsigned char min_key = 255u;
        int min_key_pos;
        int records_processed = 0;
        //unsigned char* data_ptr = malloc(record_size);
        unsigned char data_ptr;
        int cur_key;
        //unsigned char* cur_data = malloc(record_size);
        unsigned char cur_data;
        int eof = 0;

        while (records_processed < number_of_records - 1)
        {
            int err = fseek(file, records_processed * record_size, SEEK_SET);

            if (err != 0)
            {
                fclose(file);
                //free(data_ptr);
                //free(cur_data);
                perror("Sorting error: ");
                return 1;
            }

            size_t read = fread(data_ptr, record_size, 1, file);
            eof = feof(file);

            if (read != 1u)
            {
                fprintf(stderr, "Error while reading file. Probably incorrect arguments passed to sorting function.");
                free(data_ptr);
                free(cur_data);
                fclose(file);
                return 1;
            }

            min_key = *((unsigned char*) data_ptr);
            min_key_pos = records_processed;
            int pos = records_processed;

            while (!eof)
            {
                size_t read = fread(cur_data, record_size, 1u, file);
                eof = feof(file);

                cur_key = *cur_data;
                ++pos;

                if (cur_key < min_key)
                {
                    min_key = cur_key;
                    min_key_pos = pos;
                }
            }

            // swap
            if (min_key_pos != records_processed)
            {
                err = fseek(file, min_key_pos * record_size, SEEK_SET);
                size_t read = fread(cur_data, record_size, 1u, file);
                
                err = fseek(file, min_key_pos * record_size, SEEK_SET);
                fwrite(data_ptr, record_size, 1u, file);
                
                err = fseek(file, records_processed * record_size, SEEK_SET);
                fwrite(cur_data, record_size, 1u, file);
            }

            ++records_processed;

            clearerr(file);
        }

        free(data_ptr);
        free(cur_data);
        fclose(file);

        return 0;
    }

    return errno;
}

int sort_records(const char* filename, int number_of_records, int record_size, const char* library_name)
{
    if (strcmp(library_name, "sys") == 0)
    {
        return sort_records_system(filename, number_of_records, record_size);
    }
    else if (strcmp(library_name, "lib") == 0)
    {
        return sort_records_cstdlib(filename, number_of_records, record_size);
    }
    else
    {
        return 1; // Invalid library name
    }
}
