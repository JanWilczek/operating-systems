#pragma once
#include <sys/types.h>


int is_file(const char* filepath);
int writen(int fd, char* buff, int bytes_in_buff);
int readn(int sd, void * p, size_t n);
