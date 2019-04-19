TEMP_FLAGS=-g -Wall

all: master slave

# Starts master and 3 slave processes, all in background
test: master slave
	./master fifo_queue &
	./slave fifo_queue 5 &
	./slave fifo_queue 3 &
	./slave fifo_queue 7 &

master: master.o
	gcc $(CFLAGS) $(TEMP_FLAGS) master.o -o master

slave: slave.o
	gcc $(CFLAGS) $(TEMP_FLAGS) slave.o -o slave

master.o: master.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c master.c

slave.o: slave.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c slave.c

clean:
	rm *.o master slave
