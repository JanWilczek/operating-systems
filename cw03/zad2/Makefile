TEMP_FLAGS=-g -Wall

all: monitor

test: monitor
	./monitor -f list -m 30 -c 0

monitor: main.o monitor.o
	gcc $(CFLAGS) $(TEMP_FLAGS) main.o monitor.o -o monitor

main.o: main.c monitor.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

monitor.o: monitor.c monitor.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c monitor.c

clean:
	rm *.o monitor