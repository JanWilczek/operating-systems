FLAGS=-g -Wall

all: trucker loader

loader: loader.o semaphore.o tape.o
	gcc $(CFLAGS) $(FLAGS) loader.o semaphore.o tape.o shared_queue.o -o loader

loader.o:semaphore.h tape.h loader.c
	gcc $(CFLAGS) $(FLAGS) -c loader.c

trucker: trucker.o semaphore.o tape.o
	gcc $(CFLAGS) $(FLAGS) trucker.o semaphore.o tape.o shared_queue.o -o trucker

trucker.o: semaphore.h tape.h trucker.c
	gcc $(CFLAGS) $(FLAGS) -c trucker.c

tape.o: tape.h semaphore.h shared_queue.h
	gcc $(CFLAGS) $(FLAGS) -c tape.c

semaphore.o: semaphore.h semaphore.c
	gcc $(CFLAGS) $(FLAGS) -c semaphore.c

shared_queue.o: shared_queue.h shared_queue.c
	gcc $(CFLAGS) $(FLAGS) -c shared_queue.c

clean:
	rm *.o trucker loader
