TEMP_FLAGS=-g -Wall

all: rollercoaster

test: rollercoaster
	./rollercoaster 5 2 2 3

rollercoaster: main.o rollercoaster.o utils.o
	gcc $(CFLAGS) $(TEMP_FLAGS) -o rollercoaster main.o rollercoaster.o utils.o -pthread

main.o: main.c rollercoaster.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

rollercoaster.o: rollercoaster.h rollercoaster.c utils.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c rollercoaster.c -pthread

utils.o: utils.h utils.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c utils.c


clean:
	rm *.o rollercoaster

