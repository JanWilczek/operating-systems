#include "pgm_io.h"
#include <stdlib.h>
#include <stdio.h>


int** pgm_read(const char* filepath, int* width, int* height)
{
    int** image = NULL;

    FILE* file = fopen(filepath, "r");
    if (file != NULL)
    {   
        int width, height;
        if (fscanf(file, "P2\n%d %d\n", &width, &height) != 2)
        {
            perror("fscanf");
            return image;
        }

        image = malloc(height * sizeof(int*));
        for (int i = 0; i < height; ++i)
        {
            image[i] = malloc(width * sizeof(int));
        }

        for (int row = 0; row < height; ++row)
        {
            for (int col = 0; col < width; ++col)
            {
                int value = fgetc(file);
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
    return 0;
}
