#include "rollercoaster.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

struct passenger{
    pthread_t id;
};

struct carriage{
    pthread_t id;
    int index;
};

struct thread_args{
    struct passenger** passengers;
    struct carriage** carriages;
    int* rides_left;
    int carriage_capacity;
    int index;  // contains passenger index for passenger thread and carriage index for carriage thread
};

void* passenger_thread(void* args)
{
    struct thread_args* arguments = (struct thread_args*) args;

    // 1. Wait for carriage to be available (conditional variable).

    // 2. Enter the carriage and write out current number of passengers in the carriage.

    // 3. Press start.

    // 4. Leave the carriage and write out current number of passengers in the carriage.

    // 5. End thread

    free(arguments);
    return 0;
}

void* carriage_thread(void* args)
{
    struct thread_args* arguments = (struct thread_args*) args;

    // 1. Close the door.

    // 2. Start the ride.

    // 3. End the ride.

    // 4. Open the door.

    // 5. End thread.

    free(arguments);
    return 0;
}

void rollercoaster(int num_passengers, int num_carriages, int carriage_capacity, int number_of_rides)
{
    struct passenger* passengers = malloc(num_passengers * sizeof(struct passenger));
    struct carriage* carriages = malloc(num_carriages * sizeof(struct carriage));

    for (int i = 0; i < num_passengers; ++i)
    {
        // start_passenger_thread(passengers, num_passengers, i);
        struct thread_args* args = malloc(sizeof(struct thread_args));
        pthread_t passenger_id;
        int err;
        if ((err = pthread_create(&passenger_id, NULL, passenger_thread, args)) != 0)
        {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
        }
    }

    for (int i = 0; i < num_carriages; ++i)
    {
        // start_carriage_thread(carriages, num_carriages, i);
    }

    for (int i = 0; i < num_passengers; ++i)
    {
        pthread_join(passengers[i].id, NULL);
    }

    free(passengers);
    free(carriages);
}