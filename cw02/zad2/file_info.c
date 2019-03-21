#include "file_info.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

/** 
 * Parses date in a DD/MM/YYYY format and returns as a timespec struct.
 * */
struct tm parse_date(char *date)
{
    struct tm parsed_date;
    char *token;
    char *state;
    int *time_fields[] = {&parsed_date.tm_mday, &parsed_date.tm_mon, &parsed_date.tm_year};
    int i = 0;

    for (token = strtok_r(date, "/", &state);
         token != NULL;
         token = strtok_r(NULL, "/", &state), ++i)
    {
        *time_fields[i] = atoi(token);
    }

    parsed_date.tm_mon -= 1;
    parsed_date.tm_year -= 1900;

    return parsed_date;
}

int less(char comparison_operator)
{
    switch (comparison_operator)
    {
    case '<':
        return 1;
        
    case '>':
    case '=':
    default:
        return 0;
    }
}

int equal(char comparison_operator)
{
    switch (comparison_operator)
    {
    case '=':
        return 1;

    case '>':
    case '<':
    default:
        return 0;
    }
}

int greater(char comparison_operator)
{
    switch (comparison_operator)
    {
    case '>':
        return 1;

    case '=':
    case '<':
    default:
        return 0;
    }
}

int compare_dates(struct tm *file_date, struct tm *compare_date, char comparison_operator)
{
    if (file_date->tm_year < compare_date->tm_year)
    {
        return less(comparison_operator);
    }
    else if (file_date->tm_year == compare_date->tm_year)
    {
        if (file_date->tm_mon < compare_date->tm_mon)
        {
            return less(comparison_operator);
        }
        else if (file_date->tm_mon == compare_date->tm_mon)
        {
            if (file_date->tm_mday < compare_date->tm_mday)
            {
                return less(comparison_operator);
            }
            else if (file_date->tm_mday == compare_date->tm_mday)
            {
                return equal(comparison_operator);
            }
        }
    }
    return greater(comparison_operator);
}

const char *get_file_type(__mode_t mode)
{
    if (S_ISREG(mode))
    {
        return "file";
    }
    else if (S_ISDIR(mode))
    {
        return "dir";
    }
    else if (S_ISCHR(mode))
    {
        return "char dev";
    }
    else if (S_ISBLK(mode))
    {
        return "block dev";
    }
    else if (S_ISFIFO(mode))
    {
        return "fifo";
    }
    else if (S_ISLNK(mode))
    {
        return "slink";
    }
    else if (S_ISSOCK(mode))
    {
        return "sock";
    }
    else
    {
        return "unknown";
    }
}

void print_file_info(const char *path, char comparison_operator, char *date)
{
    // Parse date
    struct tm compared_date = parse_date(date);

    // Get absolute path of the directory
    char *buffer = malloc(4096 * sizeof(char));
    char *absolute_dir_path = realpath(path, buffer);

    if (absolute_dir_path == NULL)
    {
        free(buffer);
        perror("Error opening given directory: ");
    }

    // Iterate through directory
    DIR *dirinfo = opendir(path);
    if (dirinfo != NULL)
    {
        struct dirent *current_file = NULL;
        current_file = readdir(dirinfo);
        struct stat current_file_status;
        char *absolute_file_path = malloc(4096 * sizeof(char));
        int path_field_width = strlen(absolute_dir_path) + 30;

        const char header_format[] = "| %-*s | %-10s | %-14s | %-25s | %-25s |\n";
        const char fields_format[] = "| %-*s | %-10s | %-14ld | %-25s | %-25s |\n";
        printf(header_format, path_field_width, "ABSOLUTE PATH", "FILE TYPE", "SIZE IN BYTES", "LAST ACCESS DATE", "LAST MODIFICATION DATE");

        while (current_file != NULL)
        {
            lstat(current_file->d_name, &current_file_status);

            struct tm *time_info = localtime(&current_file_status.st_mtime);

            if (compare_dates(time_info, &compared_date, comparison_operator))
            {
                // Get absolute file path, file type and file's size in bytes
                snprintf(absolute_file_path, 4096u, "%s/%s", absolute_dir_path, current_file->d_name);
                const char *file_type = get_file_type(current_file_status.st_mode);
                const long size_in_bytes = S_ISREG(current_file_status.st_mode) ? current_file_status.st_size : 0L;

                // Get access and modification dates removing the newline at the end of the string (that what's strtok() for)
                const char *access_time = strtok(ctime(&current_file_status.st_atime), "\n");
                const char *modification_time = strtok(ctime(&current_file_status.st_mtime), "\n");

                // Print the acquired information
                printf(fields_format, path_field_width, absolute_file_path, file_type, size_in_bytes, access_time, modification_time);
            }

            current_file = readdir(dirinfo);
        }

        free(absolute_file_path);
        closedir(dirinfo);
    }
    else
    {
        perror("Could not open the directory: ");
    }

    free(absolute_dir_path);
}
