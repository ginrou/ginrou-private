.SUFFIXES: .cpp

BASE = ..

CC=g++

All : matrix.o matdisp

matrix.o : matrix.c
	$(CC) -c $(CFLAGS) $(INCS) $(CPPFLAGS) $*.c

matdisp : matdisp.c matrix.o
	$(CC) -o matdisp matdisp.c matrix.o

clean:
	rm *~ *.o