#!/bin/bash
EXEC=pro
SRC=pro.cpp
NODES="$1"
CPU_COUNT=0
OPTS=""

if [[ -z $NODES ]]; then
    echo >&2 "Usage: $0 nodes"
fi

CPU_COUNT=$((2 * ${#NODES} - 2))

if [[ $HOSTNAME =~ "merlin" ]]; then
    OPTS="--prefix /usr/local/share/OpenMPI"
fi

mpic++ $OPTS -o $EXEC $SRC

mpirun -v $OPTS -np $CPU_COUNT $EXEC $NODES

rm -f $EXEC
