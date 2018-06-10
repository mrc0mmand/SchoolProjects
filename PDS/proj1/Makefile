CC=g++
CFLAGS=-Wall -pedantic -g -std=c++11 -lpcap

all: pds-dhcpstarve pds-dhcprogue

pds-dhcpstarve: dhcpstarve.cpp dhcp.c
	$(CC) $(CFLAGS) $^ -o $@

pds-dhcprogue: dhcprogue.cpp dhcp.c
	$(CC) $(CFLAGS) $^ -o $@


clean:
	rm -fv pds-dhcpstarve pds-dhcprogue
