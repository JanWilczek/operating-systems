#include "rollercoaster.h"
// #include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

struct passenger
{
    pthread_t id;
};

struct carriage
{
    pthread_t id;
    pthread_mutex_t enter_mutex;
    int *passengers;
    int number_of_passengers;
};

struct thread_args
{
    struct passenger **passengers;
    int num_passengers;
    struct carriage **carriages;
    int num_carriages;

    int *rides_left; // can be changed only by the first carriage
    pthread_mutex_t* rides_left_mutex;

    int carriage_capacity;
    int index;      // contains passenger index for passenger thread and carriage index for carriage thread

    int *can_enter; // boolean
    pthread_cond_t *can_enter_cv;
    pthread_mutex_t *can_enter_mutex;

    int *current_carriage; // current carriage to enter if can_enter is true

    int* start_pressed;
    pthread_cond_t* start_pressed_cv;
    pthread_mutex_t* start_pressed_mutex;
};

struct carriage *get_current_carriage(struct thread_args *a)
{
    return &(*a->carriages)[*a->current_carriage];
}

/** 
 * Thread-safe.
 * */
int get_number_of_rides(struct thread_args* a)
{
    int result;
    pthread_mutex_lock(a->rides_left_mutex);
    result = *a->rides_left;
    pthread_mutex_unlock(a->rides_left_mutex);
    return result;
}

void press_start(struct thread_args* a)
{
    pthread_mutex_lock(a->start_pressed_mutex);
    *a->start_pressed = 1;
    pthread_cond_broadcast(a->start_pressed_cv);
    pthread_mutex_unlock(a->start_pressed_mutex);
}

void *passenger_thread(void *args)
{
    struct thread_args *a = (struct thread_args *)args;

    while (get_number_of_rides(a) > 0)  // is it ok?
    {
        // 1. Wait for carriage to be available (conditional variable).
        pthread_mutex_lock(a->can_enter_mutex);
        while (!(*a->can_enter))
        {
            pthread_cond_wait(a->can_enter_cv, a->can_enter_mutex);
        }
        pthread_mutex_unlock(a->can_enter_mutex);

        // 2. Enter the carriage and write out current number of passengers in the carriage.
        struct carriage *current = get_current_carriage(a);
        pthread_mutex_lock(&current->enter_mutex);
        current->passengers[current->number_of_passengers++] = a->index; // place the passenger in the carriage
        
        printf("Passenger %d entered carriage %d which currently has %d/%d people.\n",
               a->index, *a->current_carriage, current->number_of_passengers, a->carriage_capacity);

        if (current->number_of_passengers >= a->carriage_capacity)
        {
            pthread_mutex_lock(a->can_enter_mutex);
            *a->can_enter = 0;
            pthread_cond_broadcast(a->can_enter_cv);
            pthread_mutex_unlock(a->can_enter_mutex);

            // pthread_mutex_lock(a->can_enter_mutex);
            // *a->current_carriage++;

            // 3. Press start.
            press_start(a);
            printf("Passenger %d in carriage %d pressed \'start\'.\n", a->index, *a->current_carriage);
            // *a->can_enter = 1;
            // pthread_cond_broadcast(a->can_enter_cv);
            // pthread_mutex_unlock(a->can_enter_mutex);

        }

        pthread_mutex_unlock(&current->enter_mutex);
        // pthread_mutex_unlock(a->can_enter_mutex);

        // 4. Leave the carriage and write out current number of passengers in the carriage.
    }

    // 5. End thread
    printf("Passenger %d's thread is ending.\n", a->index);

    free(a);
    return 0;
}

void *carriage_thread(void *args)
{
    struct thread_args *a = (struct thread_args *)args;

    while(get_number_of_rides(a) > 0)
    {
        // Wait until this carriage is current
        pthread_mutex_lock(a->can_enter_mutex);
        while (a->current_carriage != a->index)
        {
            pthread_cond_wait(a->can_enter_cv, a->can_enter_mutex);
        }

        // 1. Open the door.
        printf("Carriage %d opens the door.\n", a->index);
        *a->can_enter = 1;
        pthread_cond_broadcast(a->can_enter_cv);
        pthread_mutex_unlock(a->can_enter_mutex);

        // 2. Close the door.
        pthread_mutex_lock(a->start_pressed_mutex);
        while (!(*a->start_pressed))
        {
            pthread_cond_wait(a->start_pressed_cv, a->start_pressed_mutex);
        }
        *a->start_pressed = 0;
        pthread_mutex_unlock(a->start_pressed);
        printf("Carriage %d closese the door.\n", a->index);

        if (a->index < a->num_carriages - 1)
        {
            pthread_mutex_lock(a->can_enter_mutex);
            *a->current_carriage++;
            
            // Wait for the carriages to switch
            sleep(1);

            pthread_cond_broadcast(a->can_enter_cv);
            pthread_mutex_unlock(a->can_enter_mutex);
        }
        else
        {
            // 3. Start the ride.
            printf("The ride started.\n");
            sleep(a->ride_duration);

            // 4. End the ride.
        }

    }

    // 5. End thread.

    free(a);
    return 0;
}

void rollercoaster(int num_passengers, int num_carriages, int carriage_capacity, int number_of_rides)
{
    struct passenger *passengers = malloc(num_passengers * sizeof(struct passenger));
    struct carriage *carriages = malloc(num_carriages * sizeof(struct carriage));

    pthread_mutex_t can_enter_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t can_enter_cv = PTHREAD_COND_INITIALIZER;
    int can_enter = 0;

    int current_carriage = -1;
    int start_pressed = 0;

    // Prepare arguments
    struct thread_args *general_arguments = malloc(sizeof(struct thread_args));
    general_arguments->passengers = &passengers;
    general_arguments->num_passengers = num_passengers;
    general_arguments->num_carriages = num_carriages;
    general_arguments->carriages = &carriages;
    general_arguments->rides_left = &number_of_rides;
    pthread_mutex_init(general_arguments->rides_left_mutex, NULL);
    general_arguments->carriage_capacity = carriage_capacity;
    general_arguments->index = -1;
    general_arguments->can_enter = &can_enter;
    general_arguments->can_enter_cv = &can_enter_cv;
    general_arguments->can_enter_mutex = &can_enter_mutex;
    general_arguments->current_carriage = &current_carriage;
    general_arguments->start_pressed = & start_pressed;
    pthread_mutex_init(general_arguments->start_pressed_mutex, NULL);
    pthread_cond_init(general_arguments->start_pressed_cv, NULL);

    // Start passenger threads
    for (int i = 0; i < num_passengers; ++i)
    {
        // Prepare thread-specific arguments
        struct thread_args *args = malloc(sizeof(struct thread_args));
        if (args != memcpy(args, general_arguments, sizeof(struct thread_args)))
        {
            fprintf(stderr, "Memory failure.\n");
            exit(EXIT_FAILURE);
        }
        args->index = i;

        // Start the thread
        pthread_t passenger_id;
        int err;
        if ((err = pthread_create(&passenger_id, NULL, passenger_thread, args)) != 0)
        {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
            continue;
        }
        passengers[i].id = passenger_id;
    }

    // Start carriage threads
    for (int i = 0; i < num_carriages; ++i)
    {
        // Prepare thread-specific arguments
        struct thread_args *args = malloc(sizeof(struct thread_args));
        if (args != memcpy(args, general_arguments, sizeof(struct thread_args)))
        {
            fprintf(stderr, "Memory failure.\n");
            exit(EXIT_FAILURE);
        }
        args->index = i;

        // Start the thread
        pthread_t carriage_id;
        int err;
        if ((err = pthread_create(&carriage_id, NULL, carriage_thread, args)) != 0)
        {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
            continue;
        }

        // Initialize carriage struct fields
        carriages[i].id = carriage_id;
        pthread_mutex_init(&carriages[i].enter_mutex, NULL);
        carriages[i].passengers = malloc(carriage_capacity);
        carriages[i].number_of_passengers = 0;
    }

    // Start the fun
    *general_arguments->current_carriage = 0;
    pthread_mutex_lock(&can_enter_mutex);
    can_enter = 1;
    pthread_cond_broadcast(&can_enter_cv);
    pthread_mutex_unlock(&can_enter_mutex);

    for (int i = 0; i < num_passengers; ++i)
    {
        pthread_join(passengers[i].id, NULL);
    }

    pthread_cond_destroy(&can_enter_cv);
    pthread_mutex_destroy(&can_enter_mutex);
    pthread_mutex_destroy(general_arguments->rides_left_mutex);

    free(general_arguments);
    free(passengers);
    free(carriages);
}