CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
EXEC=h2o

all: ${EXEC}

h2o: h2o.c
	${CC} ${CFLAGS} $^ -o $@

clean:
	${RM} *.o *.exe ${EXEC}