#include "rollercoaster.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#define RIDE_TIME_US 100000

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

    int can_exit; // boolean
    pthread_cond_t can_exit_cv;
    pthread_mutex_t can_exit_mutex;
};

struct thread_args
{
    struct passenger **passengers;
    int num_passengers;
    struct carriage **carriages;
    int num_carriages;

    int *rides_left; // can be changed only by the first carriage
    pthread_mutex_t *rides_left_mutex;

    int carriage_capacity;
    int index; // contains passenger index for passenger thread and carriage index for carriage thread

    int *can_enter; // boolean
    pthread_cond_t *can_enter_cv;
    pthread_mutex_t *can_enter_mutex;

    int *current_carriage; // current carriage to enter if can_enter is true
    pthread_cond_t *current_carriage_cv;
    pthread_mutex_t *current_carriage_mutex;

    int *carriage_full; // boolean
    pthread_cond_t* carriage_full_cv;
    pthread_mutex_t* carriage_full_mutex;

    int *start_pressed;
    pthread_cond_t *start_pressed_cv;
    pthread_mutex_t *start_pressed_mutex;

    int* passenger_to_press_button;
};

/** 
 * Unsynchronized
 * */
struct carriage *get_carriage(struct thread_args *a, int carriage_index)
{
    return &(*a->carriages)[carriage_index];
}

/**
 * Synchronized
 * */
struct carriage *get_current_carriage(struct thread_args *a)
{
    pthread_mutex_lock(a->current_carriage_mutex);
    struct carriage* result = get_carriage(a, *a->current_carriage);
    pthread_mutex_unlock(a->current_carriage_mutex);
    return result;
}

/** 
 * Thread-safe.
 * */
int get_number_of_rides(struct thread_args *a)
{
    int result;
    pthread_mutex_lock(a->rides_left_mutex);
    result = *a->rides_left;
    pthread_mutex_unlock(a->rides_left_mutex);
    return result;
}

void press_start(struct thread_args *a)
{
    char buffer[400];

    pthread_mutex_lock(a->start_pressed_mutex);

    *a->start_pressed = 1;
    sprintf(buffer, "Passenger %d in carriage %d pressed \'start\'.", a->index, *a->current_carriage);
    print_message(buffer);

    pthread_cond_broadcast(a->start_pressed_cv);
    pthread_mutex_unlock(a->start_pressed_mutex);
}

/**
 * Leaves with the can_enter_mutex locked when can_enter flag is true.
 * */
void wait_for_can_enter(struct thread_args *a)
{
    pthread_mutex_lock(a->can_enter_mutex);
    while (!(*a->can_enter))
    {
        pthread_cond_wait(a->can_enter_cv, a->can_enter_mutex);
    }
}

void set_can_enter(struct thread_args *a, int value)
{
    pthread_mutex_lock(a->can_enter_mutex);
    *a->can_enter = value;
    pthread_cond_broadcast(a->can_enter_cv);
    pthread_mutex_unlock(a->can_enter_mutex);
}

void set_can_exit(struct carriage *this_carriage, int value)
{
    pthread_mutex_lock(&this_carriage->can_exit_mutex);
    this_carriage->can_exit = value;
    pthread_cond_broadcast(&this_carriage->can_exit_cv);
    pthread_mutex_unlock(&this_carriage->can_exit_mutex);
}

void increase_carriage(struct thread_args *a)
{
    pthread_mutex_lock(a->current_carriage_mutex);
    
    // Wait for the carriages to switch
    usleep(100);
    *a->current_carriage = (*a->current_carriage + 1) % a->num_carriages;

    pthread_cond_broadcast(a->current_carriage_cv);
    pthread_mutex_unlock(a->current_carriage_mutex);
}

/**
 * Does holds any mutexes afterwards.
 * */
void wait_for_carriage_full(struct thread_args* a)
{
    pthread_mutex_lock(a->carriage_full_mutex);
    while(!(*a->carriage_full))
    {
        pthread_cond_wait(a->carriage_full_cv, a->carriage_full_mutex);
    }
    *a->carriage_full = 0;
    pthread_mutex_unlock(a->carriage_full_mutex);
}

/**
 * Returns random integer from range [a. b].
 * */
int rand_int(int a, int b)
{
    return (int) (b - a) * (((float) rand()) / RAND_MAX) + a;
}

void *passenger_thread(void *args)
{
    struct thread_args *a = (struct thread_args *)args;
    char buffer[400];

    while (get_number_of_rides(a) > 0)
    {
        // 1. Wait for carriage to be available (conditional variable).
        wait_for_can_enter(a);

        // No point in getting in if there are no more rides
        if (get_number_of_rides(a) == 0)
        {
            pthread_mutex_unlock(a->can_enter_mutex);
            break;
        }

        // 2. Enter the carriage and write out current number of passengers in the carriage.
        struct carriage *current = get_current_carriage(a);
        pthread_mutex_lock(&current->enter_mutex);
        current->passengers[current->number_of_passengers++] = a->index; // place the passenger in the carriage
        int carriage_entered = *a->current_carriage;

        sprintf(buffer, "Passenger %d entered carriage %d which currently has %d/%d people.",
                a->index, carriage_entered, current->number_of_passengers, a->carriage_capacity);
        print_message(buffer);

        if (current->number_of_passengers > a->carriage_capacity)
        {
            // FATAL ERROR
            fprintf(stderr, "Too many passengers on carriage %d.\n", carriage_entered);
            exit(EXIT_FAILURE);
        }
        else if (current->number_of_passengers == a->carriage_capacity)
        {
            *a->can_enter = 0;  // synchronized, because this thread holds can_enter_mutex

            pthread_mutex_lock(a->carriage_full_mutex);
            *a->carriage_full = 1;
            pthread_cond_broadcast(a->carriage_full_cv);
            pthread_mutex_unlock(a->carriage_full_mutex);
        }

        pthread_mutex_unlock(&current->enter_mutex);
        pthread_mutex_unlock(a->can_enter_mutex);

        // 4. Leave the carriage and write out current number of passengers in the carriage.
        pthread_mutex_lock(&current->can_exit_mutex);
        while (!(current->can_exit))
        {
            // Check if should press button
            if (*a->passenger_to_press_button == a->index)
            {
                // Restore initial state
                *a->passenger_to_press_button = -1;

                // 3. Press start.
                press_start(a);
            }
            pthread_cond_wait(&current->can_exit_cv, &current->can_exit_mutex);
        }
        current->number_of_passengers--;
        sprintf(buffer, "Passenger %d left carriage %d which currently has %d/%d people.",
                a->index, carriage_entered, current->number_of_passengers, a->carriage_capacity);
        print_message(buffer);
        pthread_cond_broadcast(&current->can_exit_cv);
        pthread_mutex_unlock(&current->can_exit_mutex);
    }

    // 5. End thread
    sprintf(buffer, "Passenger %d's thread is ending.", a->index);
    print_message(buffer);

    free(a);
    return 0;
}

void *carriage_thread(void *args)
{
    struct thread_args *a = (struct thread_args *)args;
    char buffer[400];

    struct carriage *this_carriage = get_carriage(a, a->index);

    while (get_number_of_rides(a) >= 0)
    {
        // Wait until this carriage is current
        pthread_mutex_lock(a->current_carriage_mutex);
        while (*a->current_carriage != a->index)
        {
            pthread_cond_wait(a->current_carriage_cv, a->current_carriage_mutex);
        }
        pthread_mutex_unlock(a->current_carriage_mutex);

        // 1. Open the door.
        sprintf(buffer, "Carriage %d opens the door.", a->index);
        print_message(buffer);

        // Allow exiting of passengers
        set_can_exit(this_carriage, 1);

        // Wait for emptying the carriage
        pthread_mutex_lock(&this_carriage->can_exit_mutex);
        while (this_carriage->number_of_passengers > 0)
        {
            pthread_cond_wait(&this_carriage->can_exit_cv, &this_carriage->can_exit_mutex);
        }
        this_carriage->can_exit = 0;
        for (int i = 0; i < a->carriage_capacity; ++i)
        {
            this_carriage->passengers[i] = -1;
        }
        pthread_mutex_unlock(&this_carriage->can_exit_mutex);

        // If it is not the last ride let new people in
        if (get_number_of_rides(a) > 0)
        {
            // "Unpress" the start button
            pthread_mutex_lock(a->start_pressed_mutex);
            *a->start_pressed = 0;  
            pthread_mutex_unlock(a->start_pressed_mutex);

            set_can_enter(a, 1);

            wait_for_carriage_full(a);

            // Pick random passenger to press the button - does not have to be synchronized because carriage writes, passenger reads
            pthread_mutex_lock(&this_carriage->can_exit_mutex);
            *a->passenger_to_press_button = this_carriage->passengers[rand_int(0, a->carriage_capacity - 1)];
            pthread_cond_broadcast(&this_carriage->can_exit_cv);
            pthread_mutex_unlock(&this_carriage->can_exit_mutex);            

            // 2. Close the door.
            pthread_mutex_lock(a->start_pressed_mutex);
            while (!(*a->start_pressed))
            {
                pthread_cond_wait(a->start_pressed_cv, a->start_pressed_mutex);
            }

            sprintf(buffer, "Carriage %d closes the door.", a->index);
            print_message(buffer);
            pthread_mutex_unlock(a->start_pressed_mutex);
        }

        // Check if it's the last carriage
        if (a->index < a->num_carriages - 1)
        {
            increase_carriage(a);

            if (get_number_of_rides(a) == 0)
            {
                break;
            }
        }
        else if (get_number_of_rides(a) > 0) // ride only of there are rides left
        {
            // 3. Start the ride.
            print_message("The ride started.");
            usleep(RIDE_TIME_US);

            // 4. End the ride.
            print_message("The ride ended.");

            pthread_mutex_lock(a->rides_left_mutex);
            --(*a->rides_left);
            pthread_mutex_unlock(a->rides_left_mutex);

            increase_carriage(a);   // will set current carriage to 0
        }
        else    // last carriage, no rides left
        {
            assert(a->index == a->num_carriages - 1);
            assert(get_number_of_rides(a) == 0);
            break;
        }
    }

    // 5. End thread.
    sprintf(buffer, "The carriage %d's thread has ended.", a->index);
    print_message(buffer);

    free(a);
    return 0;
}

void rollercoaster(int num_passengers, int num_carriages, int carriage_capacity, int number_of_rides)
{
    srand(time(NULL));

    struct passenger *passengers = malloc(num_passengers * sizeof(struct passenger));
    struct carriage *carriages = malloc(num_carriages * sizeof(struct carriage));

    pthread_mutex_t rides_left_mutex = PTHREAD_MUTEX_INITIALIZER;
    
    int can_enter = 0;
    pthread_mutex_t can_enter_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t can_enter_cv = PTHREAD_COND_INITIALIZER;
    
    int start_pressed = 0;
    pthread_mutex_t start_pressed_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t start_pressed_cv = PTHREAD_COND_INITIALIZER;
    
    int carriage_full = 0;
    pthread_mutex_t carriage_full_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t carriage_full_cv = PTHREAD_COND_INITIALIZER;

    int current_carriage = -1;
    pthread_mutex_t current_carriage_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t current_carriage_cv = PTHREAD_COND_INITIALIZER;

    int passenger_to_press_button = -1;

    // Prepare arguments
    struct thread_args *general_arguments = malloc(sizeof(struct thread_args));
    general_arguments->passengers = &passengers;
    general_arguments->num_passengers = num_passengers;
    general_arguments->num_carriages = num_carriages;
    general_arguments->carriages = &carriages;
    general_arguments->rides_left = &number_of_rides;
    general_arguments->rides_left_mutex = &rides_left_mutex;
    general_arguments->carriage_capacity = carriage_capacity;
    general_arguments->index = -1;
    general_arguments->can_enter = &can_enter;
    general_arguments->can_enter_cv = &can_enter_cv;
    general_arguments->can_enter_mutex = &can_enter_mutex;
    general_arguments->carriage_full = &carriage_full;
    general_arguments->carriage_full_mutex = &carriage_full_mutex;
    general_arguments->carriage_full_cv = &carriage_full_cv;
    general_arguments->current_carriage = &current_carriage;
    general_arguments->current_carriage_mutex = &current_carriage_mutex;
    general_arguments->current_carriage_cv = &current_carriage_cv;
    general_arguments->start_pressed = &start_pressed;
    general_arguments->start_pressed_mutex = &start_pressed_mutex;
    general_arguments->start_pressed_cv = &start_pressed_cv;
    general_arguments->passenger_to_press_button = &passenger_to_press_button;

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
        carriages[i].can_exit = 0;
        pthread_mutex_init(&carriages[i].enter_mutex, NULL);
        pthread_mutex_init(&carriages[i].can_exit_mutex, NULL);
        pthread_cond_init(&carriages[i].can_exit_cv, NULL);
        carriages[i].passengers = malloc(carriage_capacity);
        carriages[i].number_of_passengers = 0;
    }

    // Start the fun
    pthread_mutex_lock(&current_carriage_mutex);
    *general_arguments->current_carriage = 0;
    // can_enter = 1;
    pthread_cond_broadcast(&current_carriage_cv);
    pthread_mutex_unlock(&current_carriage_mutex);

    // Wait for all threads to complete
    for (int i = 0; i < num_carriages; ++i)
    {
        pthread_join(carriages[i].id, NULL);
    }

    // Notify passengers that all carriages stopped
    set_can_enter(general_arguments, 1);
    for (int i = 0; i < num_passengers; ++i)
    {
        pthread_join(passengers[i].id, NULL);
    }

    // Cleanup
    for (int i = 0; i < num_carriages; ++i)
    {
        pthread_mutex_destroy(&carriages[i].enter_mutex);
        pthread_cond_destroy(&carriages[i].can_exit_cv);
        pthread_mutex_destroy(&carriages[i].can_exit_mutex);
    }

    pthread_cond_destroy(&start_pressed_cv);
    pthread_mutex_destroy(&start_pressed_mutex);
    pthread_cond_destroy(&carriage_full_cv);
    pthread_mutex_destroy(&carriage_full_mutex);
    pthread_mutex_destroy(&current_carriage_mutex);
    pthread_cond_destroy(&current_carriage_cv);
    pthread_cond_destroy(&can_enter_cv);
    pthread_mutex_destroy(&can_enter_mutex);
    pthread_mutex_destroy(&rides_left_mutex);

    free(general_arguments);
    free(passengers);
    free(carriages);
}
