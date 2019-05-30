#include "rollercoaster.h"
#include <pthread.h>

struct passenger{
    pthread_t id;
};

struct carriage{
    pthread_t id;
    int index;
};

struct passenger_thread_args{

};

void* passenger_thread(void* args)
{
    struct passenger_thread_args* arguments = (struct passenger_thread_args*) args;

}

void rollercoaster(int num_passengers, int num_carriages, int carriage_capacity, int number_of_rides)
{
    struct passenger* passengers = malloc(num_passengers * sizeof(struct passenger));
    struct carriage* carriages = malloc(num_carriages * sizeof(struct carriage));

    for (int i = 0; i < num_passengers; ++i)
    {
        start_passenger_thread(passengers, num_passengers, i);
    }

    for (int i = 0; i < num_carriages; ++i)
    {
        start_carriage_thread(carriages, num_carriages, i);
    }

    for (int i = 0; i < num_passengers; ++i)
    {
        pthread_join(passengers[i].id, NULL);
    }
}