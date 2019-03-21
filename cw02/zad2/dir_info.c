#include "dir_info.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ftw.h>
#include <stdint.h>

//=============================================================================
// ****************     HELPER FUNCTIONS     **********************************
//=============================================================================

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

    // tm struct conventions
    parsed_date.tm_mon -= 1;     // months are numbered 0-11
    parsed_date.tm_year -= 1900; // years are represented as years passed since 1900 (for example, 2019 corresponds to 119)

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

//=============================================================================
// ****************     OPENDIR, STAT ETC. IMPLEMENTATION     *****************
//=============================================================================

void get_file_type(__mode_t mode, char *buffer)
{
    if (S_ISLNK(mode))
    {
        strncpy(buffer, "slink", 8);
    }
    else if (S_ISREG(mode))
    {
        strncpy(buffer, "file", 8);
    }
    else if (S_ISDIR(mode))
    {
        strncpy(buffer, "dir", 8);
    }
    else if (S_ISCHR(mode))
    {
        strncpy(buffer, "char dev", 8);
    }
    else if (S_ISBLK(mode))
    {
        strncpy(buffer, "block dev", 8);
    }
    else if (S_ISFIFO(mode))
    {
        strncpy(buffer, "fifo", 8);
    }
    else if (S_ISSOCK(mode))
    {
        strncpy(buffer, "sock", 8);
    }
    else
    {
        strncpy(buffer, "unknown", 8);
    }
}

void print_dir_info(const char *path, char comparison_operator, char *date)
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
        char *file_type = malloc(8 * sizeof(char));

        const char header_format[] = "| %-*s | %-10s | %-14s | %-25s | %-25s |\n";
        const char fields_format[] = "| %-*s | %-10s | %-14ld | %-25s | %-25s |\n";
        printf(header_format, path_field_width, "ABSOLUTE PATH", "FILE TYPE", "SIZE IN BYTES", "LAST ACCESS DATE", "LAST MODIFICATION DATE");

        while (current_file != NULL)
        {
            snprintf(absolute_file_path, 4096u, "%s/%s", absolute_dir_path, current_file->d_name);
            lstat(absolute_file_path, &current_file_status);

            struct tm *time_info = localtime(&current_file_status.st_mtime);

            if (compare_dates(time_info, &compared_date, comparison_operator))
            {
                // Get absolute file path, file type and file's size in bytes
                get_file_type(current_file_status.st_mode, file_type);
                const long size_in_bytes = S_ISREG(current_file_status.st_mode) ? current_file_status.st_size : 0L;

                // Get access and modification dates removing the newline at the end of the string (that what's strtok() for)
                const char *access_time = strtok(ctime(&current_file_status.st_atime), "\n");
                const char *modification_time = strtok(ctime(&current_file_status.st_mtime), "\n");

                // Print the acquired information
                printf(fields_format, path_field_width, absolute_file_path, file_type, size_in_bytes, access_time, modification_time);
            }

            current_file = readdir(dirinfo);
        }

        free(file_type);
        free(absolute_file_path);
        closedir(dirinfo);
    }
    else
    {
        perror("Could not open the directory: ");
    }

    free(absolute_dir_path);
}

//=============================================================================
// ****************     NFTW IMPLEMENTATION     *******************************
//=============================================================================

void get_file_type_from_flags(int flag, __mode_t mode, char* buffer)
{
    if (flag == FTW_SL || flag == FTW_SLN)
    {
        strncpy(buffer, "slink", 8);
    }
    else if (flag == FTW_F)
    {
        strncpy(buffer, "file", 8);
    }
    else if (flag == FTW_D || flag == FTW_DP)
    {
        strncpy(buffer, "dir", 8);
    }
    // There are no FTW_ flags to check on the below types of files
    else if (S_ISCHR(mode))
    {
        strncpy(buffer, "char dev", 8);
    }
    else if (S_ISBLK(mode))
    {
        strncpy(buffer, "block dev", 8);
    }
    else if (S_ISFIFO(mode))
    {
        strncpy(buffer, "fifo", 8);
    }
    else if (S_ISSOCK(mode))
    {
        strncpy(buffer, "sock", 8);
    }
    else
    {
        strncpy(buffer, "unknown", 8);
    }
}

// Global parameters for nftw function
int nftw_path_field_width = 0;
struct tm nftw_compared_date;
char nftw_comparison_operator;

int for_each_dir_entry(const char *file_path, const struct stat *current_file_status, int typeflag, struct FTW *ftwbuf)
{
    if (ftwbuf->level > 1)
    {
        return 0;
    }

    struct tm* time_info = localtime(&current_file_status->st_mtime);
    if (!compare_dates(time_info, &nftw_compared_date, nftw_comparison_operator))
    {
        return 0;
    }

    char file_type[8];

    // Get absolute file path, file type and file's size in bytes
    get_file_type_from_flags(typeflag, current_file_status->st_mode, file_type);
    const long size_in_bytes = typeflag == FTW_F ? current_file_status->st_size : 0L;

    // Get access and modification dates removing the newline at the end of the string (that what's strtok() for)
    const char *access_time = strtok(ctime(&current_file_status->st_atime), "\n");
    const char *modification_time = strtok(ctime(&current_file_status->st_mtime), "\n");

    // Print the acquired information
    const char fields_format[] = "| %-*s | %-10s | %-14ld | %-25s | %-25s |\n";
    printf(fields_format, nftw_path_field_width, file_path, file_type, size_in_bytes, access_time, modification_time);

    return 0;
}

void print_dir_info_nftw(const char *path, char comparison_operator, char *date)
{
    // Assign comparison operator globally
    nftw_comparison_operator = comparison_operator;

    // Parse date
    nftw_compared_date = parse_date(date);

    // Get absolute path of the directory
    char *buffer = malloc(4096 * sizeof(char));
    char *absolute_dir_path = realpath(path, buffer);

    nftw_path_field_width = strlen(absolute_dir_path) + 30;
    const char header_format[] = "| %-*s | %-10s | %-14s | %-25s | %-25s |\n";
    printf(header_format, nftw_path_field_width, "ABSOLUTE PATH", "FILE TYPE", "SIZE IN BYTES", "LAST ACCESS DATE", "LAST MODIFICATION DATE");

    nftw(absolute_dir_path, &for_each_dir_entry, 10, FTW_PHYS); // FTW_PHYS is enabled to disable symbolic link following

    free(buffer);
}
