#include "memblocks.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

void err_sys(const char* err_msg)
{
    fprintf(stderr, "%s\n", err_msg);
    exit(1);
}

void error_ocurred(memblocks_table_t* memblocks_ptr, const char* err_msg)
{
    free_table(memblocks_ptr);
    err_sys(err_msg);
}

memblocks_table_t* create_table(size_t size)
{
    memblocks_table_t *table_ptr;
    table_ptr = malloc(sizeof(memblocks_table_t));
    table_ptr->table_ptr = calloc(size, sizeof(char*));

    table_ptr->current_dir = NULL;
    table_ptr->searched_filename = NULL;
    table_ptr->first_free_block_index = 0u;
    table_ptr->size = size;

    return table_ptr;
}

void free_table(memblocks_table_t *memblocks_ptr)
{
    free(memblocks_ptr->table_ptr);

    if (memblocks_ptr->current_dir != NULL)
    {
        free(memblocks_ptr->current_dir);
    }

    if (memblocks_ptr->searched_filename != NULL)
    {
        free(memblocks_ptr->searched_filename);
    }

    // Should I free the allocated blocks?
    for (size_t i = 0u; i < memblocks_ptr->size; ++i)
    {
        if (memblocks_ptr->table_ptr[i] != NULL)
        {
            free(memblocks_ptr->table_ptr[i]);
        }
    }

    free(memblocks_ptr);
}

void set_search_dir(memblocks_table_t *memblocks_ptr, const char *dir_name)
{
    memblocks_ptr->current_dir = calloc(strlen(dir_name), sizeof(char));
    strcpy(memblocks_ptr->current_dir, dir_name);
}

void set_searched_filename(memblocks_table_t *memblocks_ptr, const char *filename)
{
    memblocks_ptr->searched_filename = calloc(strlen(filename), sizeof(char));
    strcpy(memblocks_ptr->searched_filename, filename);
}

/**
    @return the size of the given file in bytes.
*/
size_t get_file_size(FILE* file)
{
    long initial_position = ftell(file);

    fseek(file, 0L, SEEK_END);
    long size = ftell(file);

    fseek(file, initial_position, SEEK_SET);
    
    return (size_t)size;
}

void run_find_command(memblocks_table_t* memblocks_ptr, const char* tmp_filename)
{
    char *command = malloc((40 + strlen(memblocks_ptr->current_dir) + strlen(memblocks_ptr->searched_filename)) * sizeof(char));
    sprintf(command, "find %s -name %s > %s 2>&1", memblocks_ptr->current_dir, memblocks_ptr->searched_filename, tmp_filename);
    
    // This line can be used for testing - it prints out find's results on the command line.
    // sprintf(command, "find %s -name %s", memblocks_ptr->current_dir, memblocks_ptr->searched_filename);
    
    system(command);
    free(command);
}

char* read_file(const char* filename)
{
    FILE* file = fopen(filename, "r");

    if (file != NULL)
    {
        size_t file_size = get_file_size(file);

        char* block_for_result = malloc(file_size);

        rewind(file);
        fread(block_for_result, sizeof(char), file_size / sizeof(char), file);
        fclose(file);

        return block_for_result;
    }

    return NULL;
}

int create_temporary_file(char* tmp_filename)
{
    int file_descriptor;
    struct stat sbuf;

    if ((file_descriptor = mkstemp(tmp_filename)) < 0)
    {
        return -1;
    }
    close(file_descriptor);

    if (stat(tmp_filename, &sbuf) < 0)
    {
        return -1;
    }

    return 0;
}

size_t run_find(memblocks_table_t* memblocks_ptr, const char* target_filename)
{
    // Check if directory or file is not NULL
    if (memblocks_ptr->current_dir == NULL ||
        memblocks_ptr->searched_filename == NULL)
    { 
        error_ocurred(memblocks_ptr, "Directory or file to search not set.");
    }

    // Create filename for the temporary file
    int file_has_been_created = 0;

    if (target_filename == NULL)
    {
        char tmp_filename[] = "/tmp/dirXXXXXX";
        int error = create_temporary_file(tmp_filename);
        if (error != 0)
        {
            error_ocurred(memblocks_ptr, "Could not create temporary file.");
        }

        target_filename = tmp_filename;
        file_has_been_created = 1;
    }

    // Run the find command
    run_find_command(memblocks_ptr, target_filename);

    // Read the results from file
    char* result_block = read_file(target_filename);
    if (file_has_been_created)
    {
        unlink(target_filename);
    }

    if (result_block == NULL)
    {
        error_ocurred(memblocks_ptr, "Could not read from file.");
    }

    // Check if table can accomodate more blocks
    if (memblocks_ptr->first_free_block_index >= memblocks_ptr->size)
    {
        free(result_block);
        error_ocurred(memblocks_ptr, "Exceeded allowed table size.");
    }

    // Write the block in the table
    memblocks_ptr->table_ptr[memblocks_ptr->first_free_block_index] = result_block;

    // Return block index
    return memblocks_ptr->first_free_block_index++;
}

void free_block_at_index(memblocks_table_t* memblocks_ptr, size_t block_index)
{
    assert(block_index < memblocks_ptr->size);

    if (block_index >= memblocks_ptr->first_free_block_index)
    {
        error_ocurred(memblocks_ptr, "The given block has not been allocated! Exiting.");
    }
    
    free(memblocks_ptr->table_ptr[block_index]);
    memblocks_ptr->table_ptr[block_index] = NULL;
}

