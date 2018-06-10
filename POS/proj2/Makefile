CC=gcc
CFLAGS=-Wall -g -pedantic -lpthread
EXEC=shell

all: shell

shell: shell.c
	$(CC) $(CFLAGS) $^ -o $(EXEC)

clean:
	rm -fv $(EXEC)
