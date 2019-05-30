#include <stdlib.h>
#include <stdio.h>
#include "rollercoaster.h"


void print_usage(FILE* stream, const char* program_name)
{
    fprintf(stream, "Usage: %s  num_passengers num_carriages carriage_capacity number_of_rides\n", program_name);
}

int main(int argc, char* argv[])
{
    const char* program_name = argv[0];

    if (argc != 5)
    {
        print_usage(stderr, program_name);
        exit(EXIT_FAILURE);
    }

    int num_passengers = atoi(argv[1]);
    int num_carriages = atoi(argv[2]);
    int carriage_capacity = atoi(argv[3]);
    int number_of_rides = atoi(argv[4]);

    rollercoaster(num_passengers, num_carriages, carriage_capacity, number_of_rides);

    return EXIT_SUCCESS;
}
