TEMP_FLAGS=-g -Wall

all: date_printer

test: date_printer
	./date_printer

date_printer: date_printer.o
	gcc $(CFLAGS) $(TEMP_FLAGS) date_printer.o -o date_printer

date_printer.o: date_printer.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c date_printer.c
