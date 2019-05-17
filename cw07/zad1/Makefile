FLAGS=-g -Wall

all: trucker loader loaders

test: trucker loader loaders
	./trucker 10 5 20

loaders: loader loaders.c
	gcc $(CFLAGS) $(TEMP_FLAGS) loaders.c -o loaders

loader: loader.o semaphore.o tape.o shared_queue.o time_stamp.o
	gcc $(CFLAGS) $(FLAGS) loader.o semaphore.o tape.o shared_queue.o time_stamp.o -o loader

loader.o: semaphore.h tape.h time_stamp.h loader.c
	gcc $(CFLAGS) $(FLAGS) -c loader.c

trucker: trucker.o semaphore.o tape.o shared_queue.o time_stamp.o
	gcc $(CFLAGS) $(FLAGS) trucker.o semaphore.o tape.o shared_queue.o time_stamp.o -o trucker

trucker.o: semaphore.h tape.h trucker.c time_stamp.h shared_resources.h
	gcc $(CFLAGS) $(FLAGS) -c trucker.c

time_stamp.o: time_stamp.h time_stamp.c
	gcc $(CFLAGS) $(FLAGS) -c time_stamp.c

tape.o: tape.c tape.h semaphore.h shared_queue.h shared_resources.h
	gcc $(CFLAGS) $(FLAGS) -c tape.c

semaphore.o: semaphore.h semaphore.c
	gcc $(CFLAGS) $(FLAGS) -c semaphore.c

shared_queue.o: shared_queue.h shared_queue.c shared_resources.h
	gcc $(CFLAGS) $(FLAGS) -c shared_queue.c

clean:
	rm *.o trucker loader loaders
