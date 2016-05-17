CC=c++
# I used -static-libstdc++, because eva just doesn't like my client
# when he's linked dynamically with libstdc++
CFLAGS=-std=c++11 -Wall -Wextra -pedantic -g #-static-libstdc++
EXEC=server client

all: $(EXEC)

server: server.cpp
	$(CC) $(CFLAGS) -o $@ $<

client: client.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(EXEC)
