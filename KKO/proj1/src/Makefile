CC=g++
CFLAGS=-Wall -g -pedantic -lm
EXEC=gif2bmp

all: main.c gif2bmp.o
	$(CC) $(CFLAGS) $^ -o $(EXEC)

gif2bmp.o: gif2bmp.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -fv *.o $(EXEC)
