#include "pgm_io.h"
#include <stdio.h>

int** pgm_read(const char* filepath, int* width, int* height)
{
    int** image = NULL;

    FILE* file = fopen(filepath, "r");
    if (file != NULL)
    {   
        int width, height;
        if (fscanf(file, "P2\n%d %d\n", width, height) != 2)
        {
            perror("fscanf");
            return image;
        }

        image = malloc(height * sizeof(int*));
        for (int i = 0; i < height; ++i)
        {
            image[i] = malloc(width * sizeof(int));
        }

        int row = 0;
        int col = 0;
        int value;
        while ((value = fgetc(file)) != EOF)
        {
            image[row][col] = value;
            ++col;
            if (col >= width)
            {
                col = 0;
                ++row;
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

}
