CC = clang
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -DDEBUG

OBJECTS = read.o eval.o write.o buffer.o error.o rescheme.o

.PHONY: clean cleaner

rescheme: rescheme.h $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

%.o: %.c rescheme.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *~

cleaner: clean
	rm -f rescheme
