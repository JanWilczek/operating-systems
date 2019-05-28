#include "pgm_io.h"
#include <stdlib.h>
#include <stdio.h>


int** pgm_read(const char* filepath, int* width, int* height)
{
    int** image = NULL;

    FILE* file = fopen(filepath, "r");
    if (file != NULL)
    {   
        int M;
        if (fscanf(file, "P2\n%d %d\n%d\n", width, height, &M) != 3)
        {
            perror("fscanf");
            return image;
        }
        M = M + 2;  // for compiler warning removal

        image = malloc(*height * sizeof(int*));
        for (int i = 0; i < *height; ++i)
        {
            image[i] = malloc(*width * sizeof(int));
        }

        for (int row = 0; row < *height; ++row)
        {
            for (int col = 0; col < *width; ++col)
            {
                // int value = fgetc(file);
                int value;
                fscanf(file, "%d", &value);
                if (value == EOF)
                {
                    fprintf(stderr, "Invalid input image format.\n");
                    exit(EXIT_FAILURE);
                }

                image[row][col] = value;
            }
        }

        if (fclose(file) == EOF)
        {
            perror("fclose");
        }
    }

    return image;
}

int pgm_write(const char* filepath, int** image, int width, int height)
{
    FILE* file = fopen(filepath, "w");
    if (file != NULL)
    {
        fprintf(file, "P2\n");
        fprintf(file, "%d %d\n255\n", width, height);
        for (int row = 0; row < height; ++row)
        {
            for (int col = 0; col < width; ++col)
            {
                fprintf(file, "%d ", image[row][col]);
            }
            fprintf(file, "\n");
        }

        if (fclose(file) == EOF)
        {
            perror("fclose");
            return -1;
        }
    }
    else 
    {
        return -1;
    }

    return 0;
}
