CC		  = gcc
CFLAGS  = -pedantic -Wall -g
LD		  = gcc
LDFLAGS = 

mush: mush.o parseline.o
	$(LD) $(LDFLAGS) mush.o parseline.o -o osh

mush.o: mush.c
	$(CC) $(CFLAGS) -c mush.c

parseline.o: parseline.c parseline.h
	$(CC) $(CFLAGS) -c parseline.c

clean:
	rm *.o mush
