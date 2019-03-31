TEMP_FLAGS=-g -Wall

all: monitor tester

test: monitor
	./monitor -f list -m 60

tester_test: tester
	./tester -f testfile -i 1 -x 10 -b 40

tester: tester.o random_append.o
	gcc $(CFAGS) $(TEMP_FLAGS) random_append.o tester.o -o tester

tester.o: tester.c random_append.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c tester.c

random_append.o: random_append.h random_append.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c random_append.c

monitor: main.o monitor.o
	gcc $(CFLAGS) $(TEMP_FLAGS) main.o monitor.o -o monitor

main.o: main.c monitor.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

monitor.o: monitor.c monitor.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c monitor.c

clean:
	rm -R *.o monitor tester archive/