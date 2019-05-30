TEMP_FLAGS=-g -Wall

all: rollercoaster

test: rollercoaster
	./rollercoaster 20 5 2 4

rollercoaster: main.o rollercoaster.o
	gcc $(CFLAGS) $(TEMP_FLAGS) -o rollercoaster main.o rollercoaster.o -pthread

main.o: main.c rollercoaster.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

rollercoaster.o: rollercoaster.h rollercoaster.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c rollercoaster.c -pthread

clean:
	rm *.o rollercoaster

