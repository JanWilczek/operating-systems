#include "memblocks.h"

int main(int argc, char* argv[])
{
    memblocks_table_t* table_ptr = create_table(5u);

    set_search_dir(table_ptr, "/home/jawi/Documents/School");
    set_searched_filename(table_ptr, "main.c");
    size_t index = run_find(table_ptr, NULL);
    free_block_at_index(table_ptr, index);

    free_table(table_ptr);

    return 0;
}
