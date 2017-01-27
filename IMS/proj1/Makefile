CC=g++
CFLAGS=-std=c++11 #-g -Wall -Wextra -pedantic -D IMS_DEBUG
EXEC=main
PKG="06_xsumsa01.tar.gz"

all: $(EXEC)

$(EXEC): main.cpp
	$(CC) $(CFLAGS) -o $@ $^ -lsimlib -lm

run: all
	./$(EXEC)

pkg:
	cd docs && make -f Makefile && mv projekt.pdf ../doc.pdf
	tar pczvf $(PKG) main.cpp Makefile doc.pdf

clean:
	rm -f $(EXEC)
