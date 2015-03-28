CC=gcc
CFLAGS=-std=c99 -O2
EXEC=prvocisla prvocisla-inline steg-decode
ifdef WITH_M32
CFLAGS += -m32
endif

all: ${EXEC}

steg-decode: steg-decode.c error.o ppm.o eratosthenes.o
	${CC} ${CFLAGS} -lm -o steg-decode steg-decode.c error.o ppm.o eratosthenes.o

prvocisla: prvocisla.o eratosthenes.o error.o
	${CC} ${CFLAGS} -lm -o prvocisla prvocisla.o eratosthenes.o error.o

prvocisla-inline: prvocisla-inline.o eratosthenes-inline.o
	${CC} ${CFLAGS} -lm -DUSE_INLINE -o prvocisla-inline prvocisla-inline.o eratosthenes-inline.o error.o

prvocisla.o: prvocisla.c
	${CC} ${CFLAGS} -c prvocisla.c

eratosthenes.o: eratosthenes.c eratosthenes.h bit-array.h
	${CC} ${CFLAGS} -lm -c eratosthenes.c

prvocisla-inline.o: prvocisla.c
	${CC} ${CFLAGS} -DUSE_INLINE -c prvocisla.c -o prvocisla-inline.o

eratosthenes-inline.o: eratosthenes.c eratosthenes.h bit-array.h error.o
	${CC} ${CFLAGS} -DUSE_INLINE -c eratosthenes.c -o eratosthenes-inline.o

error.o: error.c error.h
	${CC} ${CFLAGS} -c error.c

ppm.o: ppm.c ppm.h
	${CC} ${CFLAGS} -c ppm.c

clean:
	${RM} *.o *~ *.exe ${EXEC}