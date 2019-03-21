#include "file_operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

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
        }

        fclose(file);

        return 0;
    }

    return errno;
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

//==================================================
// ****        FILE SORTING         ****************
//==================================================

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

int sort_records_system(const char* filename, int number_of_records, int record_size)
{
    //printf("Records before sorting:\n");
    //print_file(filename, number_of_records, record_size);

    int file = open(filename, O_RDWR);
    if (file != -1)
    {

        unsigned char min_key, data_ptr, cur_key;
        int min_key_pos;
        int records_processed = 0;

        while (records_processed < number_of_records - 1)
        {
            int new_offset = lseek(file, records_processed * record_size, SEEK_SET);

            if (new_offset < 0)
            {
                close(file);
                perror("Sorting error: ");
                return 1;
            }

            size_t read_chars = read(file, &data_ptr, 1);

            if (read_chars != 1u)
            {
                fprintf(stderr, "Error while reading file. Probably incorrect arguments passed to sorting function.");
                close(file);
                return 1;
            }

            min_key = data_ptr;
            min_key_pos = records_processed;
            int pos = records_processed + 1;
            
            // look for a minimal element
            while (pos < number_of_records)
            {
                new_offset = lseek(file, pos * record_size, SEEK_SET);
                read_chars = read(file, &cur_key, 1u);

                if (cur_key < min_key)
                {
                    min_key = cur_key;
                    min_key_pos = pos;
                }

                ++pos;
            }

            // swap record with minimal key with the current record
            if (min_key_pos != records_processed)
            {
                char* min_data = malloc(record_size);
                char* swap_data = malloc(record_size);

                new_offset = lseek(file, min_key_pos * record_size, SEEK_SET);
                read_chars = read(file, min_data, record_size);
                
                new_offset = lseek(file, records_processed * record_size, SEEK_SET);
                read(file, swap_data, record_size);

                new_offset = lseek(file, records_processed * record_size, SEEK_SET);
                write(file, min_data, record_size);

                new_offset = lseek(file, min_key_pos * record_size, SEEK_SET);
                write(file, swap_data, record_size);

                free(min_data);
                free(swap_data);
            }

            ++records_processed;
        }

        close(file);

        //printf("Records after sorting:\n");
        //print_file(filename, number_of_records, record_size);
        //assert_sorted(filename, number_of_records, record_size);

        return 0;
    }

    return errno;
}

int sort_records_cstdlib(const char* filename, int number_of_records, int record_size)
{
    //printf("Records before sorting:\n");
    //print_file(filename, number_of_records, record_size);

    FILE* file = fopen(filename, "r+");
    if (file != NULL)
    {

        unsigned char min_key, data_ptr, cur_key;
        int min_key_pos;
        int records_processed = 0;
        int eof = 0;

        while (records_processed < number_of_records - 1 && !eof)
        {
            int err = fseek(file, records_processed * record_size, SEEK_SET);

            if (err != 0)
            {
                fclose(file);
                perror("Sorting error: ");
                return 1;
            }

            size_t read = fread(&data_ptr, 1, 1, file);
            eof = feof(file);

            if (read != 1u)
            {
                fprintf(stderr, "Error while reading file. Probably incorrect arguments passed to sorting function.");
                fclose(file);
                return 1;
            }

            min_key = data_ptr;
            min_key_pos = records_processed;
            int pos = records_processed + 1;
            
            // look for a minimal element
            while (pos < number_of_records && !eof)
            {
                err = fseek(file, pos * record_size, SEEK_SET);
                read = fread(&cur_key, 1, 1u, file);
                eof = feof(file);

                if (cur_key < min_key)
                {
                    min_key = cur_key;
                    min_key_pos = pos;
                }

                ++pos;
            }

            // swap record with minimal key with the current record
            if (min_key_pos != records_processed)
            {
                char* min_data = malloc(record_size);
                char* swap_data = malloc(record_size);

                err = fseek(file, min_key_pos * record_size, SEEK_SET);
                read = fread(min_data, record_size, 1u, file);
                
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

        fclose(file);

        //printf("Records after sorting:\n");
        //print_file(filename, number_of_records, record_size);
        //assert_sorted(filename, number_of_records, record_size);

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
    else // not "sys" defaulting to "lib"
    {
        return sort_records_cstdlib(filename, number_of_records, record_size);
    }
}

//==================================================
// ****        FILE COPYING         ****************
//==================================================

int copy_records_system(const char* source_filename, const char* target_filename, int number_of_records, int record_size)
{
    int source_fd = open(source_filename, O_RDONLY);
    int target_fd = open(target_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

    if (source_fd != -1 && target_fd != -1)
    {
        unsigned char* data = malloc(record_size);

        for (int i = 0; i < number_of_records; ++i)
        {
            read(source_fd, data, record_size);
            write(target_fd, data, record_size);
        }

        free(data);
        close(target_fd);
        close(source_fd);

        return 0;
    }

    return errno;
}

int copy_records_cstdlib(const char* source_filename, const char* target_filename, int number_of_records, int record_size)
{
    FILE* source = fopen(source_filename, "r");
    FILE* target = fopen(target_filename, "w");
    if (source != NULL && target != NULL)
    {
        unsigned char* data = malloc(record_size);

        for (int i = 0; i < number_of_records; ++i)
        {
            fread(data, record_size, 1, source);
            fwrite(data, record_size, 1, target);
        }

        free(data);
        fclose(target);
        fclose(source);

        return 0;
    }

    return errno;
}

int copy_records(const char* source_filename, const char* target_filename, int number_of_records, int record_size, const char* library_name)
{
    if (strcmp(library_name, "sys") == 0)
    {
        return copy_records_system(source_filename, target_filename, number_of_records, record_size);
    }
    else // not "sys" defaulting to "lib"
    {
        return copy_records_cstdlib(source_filename, target_filename, number_of_records, record_size);
    }
}
