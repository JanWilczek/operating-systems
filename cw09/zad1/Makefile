TEMP_FLAGS=-g -Wall

all: rollercoaster

rollercoaster: main.o
	gcc $(CFLAGS) $(TEMP_FLAGS) -o rollercoaster main.o

main.o: main.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

clean:
	rm *.o rollercoaster

