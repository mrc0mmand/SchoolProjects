CC=ghc
EXEC=subs-cipher

all: subs-cipher.hs symboldb.hs
	$(CC) --make $^ -o $(EXEC)

clean:
	rm -fv *.hi *.o $(EXEC)
