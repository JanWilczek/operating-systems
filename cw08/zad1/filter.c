#include "filter.h"
#include "pgm_io.h"
#include <stdio.h>
#include <stdlib.h>

void filter_image(const char* input_image_path, const char* filter_path, const char* output_image_path, int num_threads, int is_block)
{
    int width, height;
    if (pgm_read(input_image_path, &width, &height) == NULL)
    {
        fprintf(stderr, "Error when parsing file.\n");
    }
}