TEMP_FLAGS=-g -Wall

all: command_interpreter

test: command_interpreter
	./command_interpreter commands

command_interpreter: command_interpreter.o main.o
	gcc $(CFLAGS) $(TEMP_FLAGS) main.o command_interpreter.o -o command_interpreter

main.o: main.c command_interpreter.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

command_interpreter.o: command_interpreter.c command_interpreter.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c command_interpreter.c

clean:
	rm *.o command_interpreter
