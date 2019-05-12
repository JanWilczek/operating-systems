FLAGS=-g -Wall

all: trucker

trucker: main.o semaphore.o shared_queue.o
	gcc $(CFLAGS) $(FLAGS) main.o semaphore.o shared_queue.o -o trucker

main.o: semaphore.h shared_queue.h main.c
	gcc $(CFLAGS) $(FLAGS) -c main.c

semaphore.o: semaphore.h semaphore.c
	gcc $(CFLAGS) $(FLAGS) -c semaphore.c

shared_queue.o: shared_queue.h shared_queue.c
	gcc $(CFLAGS) $(FLAGS) -c shared_queue.c

clean:
	rm *.o trucker
