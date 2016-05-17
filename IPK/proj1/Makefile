CC=gcc
CFLAGS=-g -Wall -pedantic -std=c99# -DIPK_DEBUG
EXEC=webclient

all: $(EXEC)

webclient: webclient.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) -f $(EXEC)
