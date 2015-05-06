CC=gcc
CPPC=g++
CFLAGS=-std=c99 -O2
CPPFLAGS=-std=c++11 -O2
EXEC=tail tail2 wordcount-static wordcount-dynamic
LIBHTABLE_STATIC_BUILD=libhtable_static
LIBHTABLE_DYNAMIC_BUILD=libhtable_dynamic

all: ${EXEC}

debug: CFLAGS+=-g -pedantic -Wall -Wextra
debug: CPPFLAGS+=-g -pedantic -Wall -Wextra
debug: ${EXEC}

tail: tail.c
	${CC} ${CFLAGS} $^ -o $@

tail2: tail2.cc
	${CPPC} ${CPPFLAGS} $^ -o $@

io.o: io.c io.h
	${CC} ${CFLAGS} -c io.c

wordcount-static: wordcount.c htable.h io.o libhtable.a 
	${CC} -static ${CFLAGS} $^ -o $@

libhtable.a: libhtable-static
	ar -rcsv $@ ${LIBHTABLE_STATIC_BUILD}/*.o

libhtable-static: htable-*.c
	${CC} ${CFLAGS} -c $^
	mkdir -p ${LIBHTABLE_STATIC_BUILD}
	mv htable-*.o ${LIBHTABLE_STATIC_BUILD}/

wordcount-dynamic: wordcount.c htable.h io.o libhtable.so
	${CC} ${CFLAGS} $^ -o $@

libhtable.so: libhtable-dynamic
	gcc -shared ${CFLAGS} -Wl,-soname,$@ -o $@ ${LIBHTABLE_DYNAMIC_BUILD}/*.o

libhtable-dynamic: htable-*.c
	gcc -c ${CFLAGS} -fPIC $^
	mkdir -p ${LIBHTABLE_DYNAMIC_BUILD}
	mv htable-*.o ${LIBHTABLE_DYNAMIC_BUILD}/ 

clean:
	${RM} *.o *~ *.exe *.a *.so ${LIBHTABLE_STATIC_BUILD}/*.o ${LIBHTABLE_STATIC_BUILD}/*.o ${EXEC}