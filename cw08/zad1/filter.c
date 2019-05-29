#include "filter.h"
#include "pgm_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX(a, b) ((a > b) ? a : b)

float **read_filter_coefficients(const char *filepath, int *c)
{
    float **image = NULL;

    FILE *file = fopen(filepath, "r");
    if (file != NULL)
    {
        if (fscanf(file, "%d\n", c) != 1)
        {
            perror("fscanf");
            return image;
        }

        image = malloc(*c * sizeof(int *));
        for (int i = 0; i < *c; ++i)
        {
            image[i] = malloc(*c * sizeof(int));
        }

        for (int row = 0; row < *c; ++row)
        {
            for (int col = 0; col < *c; ++col)
            {
                float value;
                fscanf(file, "%f", &value);
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

int filter_xy(int **image, float **filter, int x, int y, int c)
{
    float sum = 0.f;
    for (int i = 0; i < c; ++i)
    {
        for (int j = 0; j < c; ++j)
        {
            sum += image[(int)MAX(1, x - ceil(c / 2) + i - 1)][(int)MAX(1, y - ceil(c / 2) + j - 1)] * filter[i][j];
        }
    }
    return (int)roundf(sum);
}

void filter_impl(int **image, float **filter, int **output, int width, int height, int c)
{
    for (int x = 0; x < height; ++x)
    {
        for (int y = 0; y < width; ++y)
        {
            output[x][y] = filter_xy(image, filter, x, y, c);
        }
    }
}

void filter_image(const char *input_image_path, const char *filter_path, const char *output_image_path, int num_threads, int is_block)
{
    // Read input image
    int width, height;
    int **image;
    if ((image = pgm_read(input_image_path, &width, &height)) == NULL)
    {
        fprintf(stderr, "Error when parsing file.\n");
        exit(EXIT_FAILURE);
    }

    // Read filter coefficients
    int c;
    float **filter;
    if ((filter = read_filter_coefficients(filter_path, &c)) == NULL)
    {
        fprintf(stderr, "Error when parsing filter coefficients.\n");
        exit(EXIT_FAILURE);
    }

    // Create output image
    int **output = malloc(height * sizeof(int));
    for (int i = 0; i < height; ++i)
    {
        output[i] = malloc(width * sizeof(int));
    }

    // Actual image filtering
    filter_impl(image, filter, output, width, height, c);

    // Write the output to file
    if (pgm_write(output_image_path, output, width, height) == 1)
    {
        fprintf(stderr, "Error when writing file.\n");
        exit(EXIT_FAILURE);
    }
}