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
        unsigned char record_byte;

        for (int i = 0; i < number_of_records_to_generate; ++i)
        {
            for (int j = 0; j < record_size; ++j)
            {
                record_byte = (unsigned char) 48u + 10u * ((double)rand()) / RAND_MAX;
                fprintf(file, "%c", record_byte);
            }

            //fprintf(file, "\n");
        }

        fclose(file);

        return 0;
    }

    return errno;
}

void assert_sorted(const char* filename, int number_of_records, int record_size)
{
    FILE* file = fopen(filename, "r");
    if (file != NULL)
    {
        unsigned char a, b;

        for (int i = 0; i < number_of_records / 2; ++i)
        {
            fseek(file, record_size * i * 2, SEEK_SET);
            fread(&a, 1, 1, file);
            fseek(file, record_size * (i * 2 + 1), SEEK_SET);
            fread(&b, 1, 1, file);

            if (b < a)
            {
                fprintf(stderr, "File not sorted!\n");
            }
        }

        fclose(file);
    }
}

void print_file(const char* filename, int number_of_records, int record_size)
{
    FILE* file = fopen(filename, "r");
    if (file != NULL)
    {
        unsigned char* a = malloc(record_size);

        for (int i = 0 ; i < number_of_records; ++i)
        {
            fread(a, record_size, 1, file);
            for (int j = 0; j < record_size; ++j)
            {
                printf("%c", a[j]);
            }
            printf("\n");
        }

        printf("\n");

        free(a);

        fclose(file);
    }
}

int sort_records_system(const char* filename, int number_of_records, int record_size)
{
    return 1;
}

int sort_records_cstdlib(const char* filename, int number_of_records, int record_size)
{
    print_file(filename, number_of_records, record_size);

    FILE* file = fopen(filename, "r+");
    if (file != NULL)
    {

        unsigned char min_key, data_ptr, cur_data;
        int min_key_pos;
        int records_processed = 0;
        //unsigned char* data_ptr = malloc(record_size);
        unsigned char cur_key;
        //unsigned char* cur_data = malloc(record_size);
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

            size_t read = fread(&data_ptr, 1, 1, file);
            eof = feof(file);

            if (read != 1u)
            {
                fprintf(stderr, "Error while reading file. Probably incorrect arguments passed to sorting function.");
                //free(data_ptr);
                //free(cur_data);
                fclose(file);
                return 1;
            }

            //min_key = *((unsigned char*) data_ptr);
            min_key = data_ptr;
            min_key_pos = records_processed;
            int pos = records_processed + 1;
            
            // look for a minimal element
            while (pos < number_of_records)
            {
                err = fseek(file, pos * record_size, SEEK_SET);
                size_t read = fread(&cur_data, 1, 1u, file);
                eof = feof(file);

                cur_key = cur_data;

                if (cur_key < min_key)
                {
                    min_key = cur_key;
                    min_key_pos = pos;
                }

                ++pos;
            }

            // swap
            if (min_key_pos != records_processed)
            {
                char* min_data = malloc(record_size);
                char* swap_data = malloc(record_size);

                err = fseek(file, min_key_pos * record_size, SEEK_SET);
                size_t read = fread(min_data, record_size, 1u, file);
                
                err = fseek(file, records_processed * record_size, SEEK_SET);
                fread(swap_data, record_size, 1u, file);

                err = fseek(file, records_processed * record_size, SEEK_SET);
                fwrite(min_data, record_size, 1u, file);

                err = fseek(file, min_key_pos * record_size, SEEK_SET);
                fwrite(swap_data, record_size, 1u, file);

                free(min_data);
                free(swap_data);
            }

            ++records_processed;

            clearerr(file);
        }

        //free(data_ptr);
        //free(cur_data);
        fclose(file);

        print_file(filename, number_of_records, record_size);
        assert_sorted(filename, number_of_records, record_size);

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
