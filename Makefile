CC = clang
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -DDEBUG

OBJECTS = error.o read.o eval.o write.o rescheme.o

.PHONY: clean cleaner

rescheme: rescheme.h $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

.c.o: rescheme.h

clean:
	rm -f *.o *~

cleaner:
	rm -f *.o *~ rescheme
