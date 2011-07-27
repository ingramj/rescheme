CC = clang
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -DDEBUG

OBJECTS = error.o read.o eval.o write.o rescheme.o

.PHONY: clean cleaner

rescheme: rescheme.h $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

%.o: %.c rescheme.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *~

cleaner: clean
	rm -f rescheme
