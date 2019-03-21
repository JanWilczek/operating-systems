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
struct tm parse_date(char* date)
{
    struct tm parsed_date;
    char* token;
    char* state;
    int* time_fields[] = { &parsed_date.tm_mday, &parsed_date.tm_mon, &parsed_date.tm_year };
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

int compare_dates(struct tm* file_date, struct tm* compare_date, char comparison_operator)
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

void print_file_info(const char* path, char comparison_operator, char* date)
{
    // Parse date
    struct tm compared_date = parse_date(date);

    // Iterate through directory
    DIR* dirinfo = opendir(path);
    if (dirinfo != NULL)
    {
        struct dirent* current_file = NULL;
        current_file = readdir(dirinfo);
        struct stat current_file_status;

        printf("ABSOLUTE PATH | FILE TYPE | SIZE IN BYTES | LAST ACCESS DATE | LAST MODIFICATION DATE\n");

        while (current_file != NULL)
        {
            lstat(current_file->d_name, &current_file_status);

            struct tm* time_info = localtime(&current_file_status.st_mtime);
            
            if (!compare_dates(time_info, &compared_date, comparison_operator))
            {
                continue;
            }

            printf("%s          ", current_file->d_name);

            if (S_ISREG(current_file_status.st_mode))
            {
                printf("file");
            }
            else if (S_ISDIR(current_file_status.st_mode))
            {
                printf("dir");
            }
            else if (S_ISCHR(current_file_status.st_mode))
            {
                printf("char dev");
            }
            else if (S_ISBLK(current_file_status.st_mode))
            {
                printf("block dev");
            }
            else if (S_ISFIFO(current_file_status.st_mode))
            {
                printf("fifo");
            }
            else if (S_ISLNK(current_file_status.st_mode))
            {
                printf("slink");
            }
            else if (S_ISSOCK(current_file_status.st_mode))
            {
                printf("sock");
            }
            else
            {
                printf("unknown");
            }
            printf("     ");
            
            if (S_ISREG(current_file_status.st_mode))
            {
                printf("%ld", current_file_status.st_size);
            }
            else
            {
                printf("    ");
            }
            
            printf("    ");
            printf("%s      ", ctime(&current_file_status.st_atime));
            printf("%s      ", ctime(&current_file_status.st_mtime));

            printf("\n");
            
            current_file = readdir(dirinfo);
        }

        closedir(dirinfo);
    }
    else
    {
        perror("Could not open the directory: ");
    }
}

