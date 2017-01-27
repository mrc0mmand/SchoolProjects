CC=g++
CFLAGS=-lssl -lcrypto
CFLAGS+=-std=c++11 -g -pedantic -Wall -Wextra
EXEC = imapcl
TAR = xsumsa01.tar

all: ${EXEC}

imapcl: main.cpp imap.cpp tls.cpp utils.cpp
	g++ $^ ${CFLAGS} -o $@

clean:
	rm -f ${EXEC} ${TAR}

tar:
	cd docs/; make
	cp docs/projekt.pdf manual.pdf
	tar pcvf ${TAR} Makefile README main.cpp imap.[ch]pp tls.[ch]pp \
					 utils.[ch]pp manual.pdf
