#include "filter.h"
#include "pgm_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>


#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

int ticks_to_microseconds(clock_t start, clock_t end)
{
    return (int) ((float) (end - start)) / ((float) CLOCKS_PER_SEC / 1e6);
}

float **read_filter_coefficients(const char *filepath, int *c)
{
    float **filter = NULL;

    FILE *file = fopen(filepath, "r");
    if (file != NULL)
    {
        if (fscanf(file, "%d\n", c) != 1)
        {
            perror("fscanf");
            return filter;
        }

        filter = malloc(*c * sizeof(int *));
        for (int i = 0; i < *c; ++i)
        {
            filter[i] = malloc(*c * sizeof(int));
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

                filter[row][col] = value;
            }
        }

        if (fclose(file) == EOF)
        {
            perror("fclose");
        }
    }

    return filter;
}

int filter_xy(int **image, float **filter, int x, int y, int width, int height, int c)
{
    float sum = 0.f;
    for (int i = 0; i < c; ++i)
    {
        for (int j = 0; j < c; ++j)
        {
            int imrow = (int)MIN(height - 1, MAX(0, x - (int) ceil(((float)c) / 2) + i - 1));
            int imcol = (int)MIN(width - 1, MAX(0, y - (int) ceil(((float)c) / 2) + j - 1));
            sum += image[imrow][imcol] * filter[i][j];
        }
    }
    return (int)roundf(sum);
}

void filter_impl_synchronous(int **image, float **filter, int **output, int width, int height, int c)
{
    for (int x = 0; x < height; ++x)
    {
        for (int y = 0; y < width; ++y)
        {
            output[x][y] = filter_xy(image, filter, x, y, width, height, c);
        }
    }
}

struct from_to{
    int** image;
    float** filter;
    int** output;
    int width;
    int height;
    int c;
    int from;
    int to;
};

void* filter_from_to(void* args)
{
    clock_t start = clock();

    struct from_to* arguments = (struct from_to*) args;
    for (int x = 0; x < arguments->height; ++x)
    {
        for (int y = arguments->from; y <= arguments->to; ++y)
        {
            arguments->output[x][y] = filter_xy(arguments->image, arguments->filter, x, y, arguments->width, arguments->height, arguments->c);
        }
    }
    free(arguments);

    clock_t end = clock();
    int time_spent_us = ticks_to_microseconds(start, end);

    int* retval = malloc(sizeof(int));
    *retval = time_spent_us;

    pthread_exit(retval);
}

void filter_impl_multithreaded_block(int** image, float** filter, int** output, int width, int height, int c, int num_threads)
{
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));

    for (int thread = 1; thread <= num_threads; ++thread)
    {
        int from = (thread - 1) * ceil(width / num_threads);
        int to = thread * ceil(width / num_threads);
        // filter_from_to(image, filter, output, width, height, c, from, to);

        struct from_to* args = malloc(sizeof(struct from_to));
        args->image = image;
        args->filter = filter;
        args->output = output;
        args->width = width;
        args->height = height;
        args->c = c;
        args->from = from;
        args->to = to;

        pthread_t thread_id;
        int err;
        if ((err = pthread_create(&thread_id, NULL, filter_from_to, args)) != 0)
        {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
        }        
        threads[thread - 1] = thread_id;
    }

    for (int i = 0; i < num_threads; ++i)
    {
        int err;
        int* thread_time_us;
        if ((err = pthread_join(threads[i], (void **) &thread_time_us)) != 0)
        {
            fprintf(stderr, "pthread_join: %s\n", strerror(err));
        }
        else
        {
            printf("Thread %ld filtered its columns in %d us.\n", threads[i], *thread_time_us);
            free(thread_time_us);
        }
    }

    free(threads);
}

struct every {
    int** image;
    float** filter;
    int** output;
    int width;
    int height;
    int c;
    int thread_num;
    int num_threads;
};

void* filter_every(void* args)
{
    clock_t start = clock();

    struct every* arguments = (struct every*) args;

    for (int x = 0; x < arguments->height; ++x)
    {
        for (int y = arguments->thread_num - 1; y < arguments->width; y += arguments->num_threads)
        {
            arguments->output[x][y] = filter_xy(arguments->image, arguments->filter, x, y, arguments->width, arguments->height, arguments->c);
        }
    }
    free(arguments);

    clock_t end = clock();
    int time_spent_us = ticks_to_microseconds(start, end);

    int* retval = malloc(sizeof(int));
    *retval = time_spent_us;

    pthread_exit(retval);
}

void filter_impl_multithreaded_interleaved(int** image, float** filter, int** output, int width, int height, int c, int num_threads)
{
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));

    for (int thread = 1; thread <= num_threads; ++thread)
    {
        struct every* args = malloc(sizeof(struct every));
        args->image = image;
        args->filter = filter;
        args->output = output;
        args->width = width;
        args->height = height;
        args->c = c;
        args->thread_num = thread;
        args->num_threads = num_threads;

        // filter_every(image, filter, output, width, height, c, thread, num_threads);
        pthread_t thread_id;
        int err;
        if ((err = pthread_create(&thread_id, NULL, filter_every, args)) != 0)
        {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
        }
        threads[thread - 1] = thread_id;
    }

    for (int i = 0; i < num_threads; ++i)
    {
        int err;
        int* thread_time_us;
        if ((err = pthread_join(threads[i], (void **) &thread_time_us)) != 0)
        {
            fprintf(stderr, "pthread_join: %s\n", strerror(err));
        }
        else
        {
            printf("Thread %ld filtered its columns in %d us.\n", threads[i], *thread_time_us);
            free(thread_time_us);
        }
    }

    free(threads);
}

void filter_impl_multithreaded(int** image, float** filter, int** output, int width, int height, int c, int num_threads, int is_block)
{
    clock_t start = clock();

    if (is_block)
    {
        filter_impl_multithreaded_block(image, filter, output, width, height, c, num_threads);
    }
    else
    {
        filter_impl_multithreaded_interleaved(image, filter, output, width, height, c, num_threads);
    }
 
    clock_t end = clock();
    int total_time_us = ticks_to_microseconds(start, end);
    printf("Time elapsed: %d us, image resolution: %dx%d, filter size: %d, number of threads used: %d, method: %s.\n", total_time_us, width, height, c, num_threads, is_block ? "block" : "interleaved");
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
    int **output = malloc(height * sizeof(int*));
    for (int i = 0; i < height; ++i)
    {
        output[i] = malloc(width * sizeof(int));
    }

    // Actual image filtering
    filter_impl_multithreaded(image, filter, output, width, height, c, num_threads, is_block);

    // Write the output to file
    if (pgm_write(output_image_path, output, width, height) == 1)
    {
        fprintf(stderr, "Error when writing file.\n");
        exit(EXIT_FAILURE);
    }
}
