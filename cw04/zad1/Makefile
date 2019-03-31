TEMP_FLAGS=-g -Wall

all: date_printer date_printer_with_child_process

date_printer_with_child_process: date_printer_with_child_process.o
	gcc $(CFLAGS) $(TEMP_FLAGS) date_printer_with_child_process.o -o date_printer_with_child_process

date_printer_with_child_process.o: date_printer_with_child_process.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c date_printer_with_child_process.c

test: date_printer
	./date_printer

date_printer: date_printer.o
	gcc $(CFLAGS) $(TEMP_FLAGS) date_printer.o -o date_printer

date_printer.o: date_printer.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c date_printer.c
