#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

int is_file(const char* filepath)
{
    struct stat file_info;
    if (stat(filepath, &file_info) == -1)
    {
        perror("stat");
        return 0;
    }

    if (S_ISREG(file_info.st_mode))
    {
        return 1;
    }
    return 0;
}
