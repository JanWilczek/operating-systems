FLAGS=-g -Wall

all: trucker loader loaders

test: trucker loader loaders
	./trucker 10 5 20

loaders: loader loaders.c
	gcc $(CFLAGS) $(TEMP_FLAGS) loaders.c -o loaders

loader: loader.o semaphore.o tape.o shared_queue.o utils.o
	gcc $(CFLAGS) $(FLAGS) loader.o semaphore.o tape.o shared_queue.o utils.o -o loader

loader.o: semaphore.h tape.h utils.h loader.c
	gcc $(CFLAGS) $(FLAGS) -c loader.c

trucker: trucker.o semaphore.o tape.o shared_queue.o utils.o
	gcc $(CFLAGS) $(FLAGS) trucker.o semaphore.o tape.o shared_queue.o utils.o -o trucker

trucker.o: semaphore.h tape.h trucker.c utils.h shared_resources.h
	gcc $(CFLAGS) $(FLAGS) -c trucker.c

utils.o: utils.h utils.c
	gcc $(CFLAGS) $(FLAGS) -c utils.c

tape.o: tape.c tape.h semaphore.h shared_queue.h shared_resources.h
	gcc $(CFLAGS) $(FLAGS) -c tape.c

semaphore.o: semaphore.h semaphore.c
	gcc $(CFLAGS) $(FLAGS) -c semaphore.c

shared_queue.o: shared_queue.h shared_queue.c shared_resources.h
	gcc $(CFLAGS) $(FLAGS) -c shared_queue.c

clean:
	rm *.o trucker loader loaders
