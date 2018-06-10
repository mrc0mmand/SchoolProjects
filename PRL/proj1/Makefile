CC=mpic++
CFLAGS=-Wall -pedantic

all: mss

mss: mss.cpp
	$(CC) $(CFLAGS) $^ -o $@

run: mss
	mpirun -np 5 mss

