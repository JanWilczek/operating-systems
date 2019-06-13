#include "utils.h"

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int writen(int fd, char* buff, int bytes_in_buff)
{
    write(fd, &bytes_in_buff, sizeof(bytes_in_buff));    // Send the size of the message.

    int bytes_sent;
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

    return bytes_in_buff;
}

/*
 * Reads n bytes from sd into where p points to.
 *
 * returns 0 on succes or -1 on error.
 *
 * Note: 
 * The function's name is inspired by and dedicated to "W. Richard Stevens" (RIP).
 */
int readn(int sd, void * p, size_t n)
{
  size_t bytes_to_read = n;
  size_t bytes_read = 0;

  while (bytes_to_read > bytes_read)
  {
    ssize_t result = read(sd, p + bytes_read, bytes_to_read);
    if (-1 == result)
    {
      if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
      {
        continue;
      }

#     ifdef DEBUG     
      {
        int errno_save = errno;
        perror("read() failed");
        errno = errno_save;
      }
#     endif

      break;
    }
    else if(0 == result)
    {
#     ifdef DEBUG
      {     
        int errno_save = errno;
        fprintf(stderr, "%s: Connection closed by peer.", __FUNCTION__);
        errno = errno_save;
      }
#     endif

      break;
    }

    bytes_to_read -= result;
    bytes_read += result;
  }

  return (bytes_read < bytes_to_read) ?-1 :0; 
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
