#include "filter.h"
#include "pgm_io.h"
#include <stdio.h>
#include <stdlib.h>

void filter_image(const char* input_image_path, const char* filter_path, const char* output_image_path, int num_threads, int is_block)
{
    int width, height;
    int** image;
    if ((image = pgm_read(input_image_path, &width, &height)) == NULL)
    {
        fprintf(stderr, "Error when parsing file.\n");
        exit(EXIT_FAILURE);
    }

    if (pgm_write(output_image_path, image, width, height) == 1)
    {
        fprintf(stderr, "Error when writing file.\n");
        exit(EXIT_FAILURE);
    }
}