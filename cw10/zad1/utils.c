#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

void usend(int fd, char* buff, int bytes_in_buff)
{
    write(fd, bytes_in_buff, sizeof(bytes_in_buff));    // Send the size of the message.

    int bytes_send;
    while (bytes_in_buff > 0)
    {
        bytes_sent = write(fd, buff, bytes_in_buff);
        if (bytes_sent < 0)
        {
            perror("write"); // or whatever
            break;
        }
        buff += bytes_sent;
        bytes_in_buff -= bytes_sent;
    }
}

int is_file(const char *filepath)
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
