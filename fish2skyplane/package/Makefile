CC = gcc
CFLAGS = -Wall -O3
INCLUDES =
LFLAGS =
LIBS = -ljpeg -lm

OBJS = fish2skyplane.o bitmaplib.o

all: fish2skyplane

fish2skyplane: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o fish2skyplane $(OBJS) $(LFLAGS) $(LIBS)

fish2skyplane.o: fish2skyplane.c fish2skyplane.h
	$(CC) $(INCLUDES) $(CFLAGS) -c fish2skyplane.c
 
bitmaplib.o: bitmaplib.c bitmaplib.h
	$(CC) $(INCLUDES) $(CFLAGS) -c bitmaplib.c

clean:
	rm -rf core fish2skyplane $(OBJS)

