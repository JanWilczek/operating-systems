#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memblocks.h"

int invalid_number_of_arguments()
{
    printf("Invalid number of arguments for this operaion!");
    return 1;
}

int main(int argc, char* argv[])
{
    memblocks_table_t* table = NULL;

    for (int i = 1; i < argc; ++i)
    {
        printf("Executing %s\n", argv[i]);

        if (strcmp(argv[i], "create_table") == 0)
        {
            if (i + 1 >= argc)
            {
                return invalid_number_of_arguments();
            }
            size_t size = (size_t) atoi(argv[++i]);
            printf("You want to create a table of size %ld!\n", size);
            
            table = create_table(size);
        }
        else if (strcmp(argv[i], "search_directory") == 0)
        {
            if (i + 3 >= argc)
            {
                return invalid_number_of_arguments();
            }
            char* dir = argv[++i];
            char* file = argv[++i];
            char* file_tmp = argv[++i];
            printf("You want to search a directory %s for file %s and store its results in %s!\n", dir, file, file_tmp);

            set_search_dir(table, dir);
            set_searched_filename(table, file);
            size_t index = run_find(table, file_tmp);

            printf("Results stored in file %s and at index %ld of the table.\n", file_tmp, index);
        }
        else if (strcmp(argv[i], "remove_block_index") == 0)
        {
            if (i + 1 >= argc)
            {
                return invalid_number_of_arguments();
            }
            size_t index = (size_t) atoi(argv[++i]);
            printf("You want to remove a block at index %ld!\n", index);

            free_block_at_index(table, index);
        }
        else
        {
            printf("Invalid argument!\n");
        }
    }

    free_table(table);

    return 0;
}
