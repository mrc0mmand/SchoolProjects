#!/bin/bash
EXEC=mss
SRC=mss.cpp
NUM_FILE="numbers"
NUM_COUNT=$1
CPU_COUNT=$2
OPTS=""

if [[ -z $1 || -z $2 ]]; then
    echo >&2 "Usage: $0 num_count cpu_count"
fi

if [[ $HOSTNAME =~ "merlin" ]]; then
    OPTS="--prefix /usr/local/share/OpenMPI"
fi

mpic++ $OPTS -o $EXEC $SRC

dd if=/dev/random bs=1 count=$NUM_COUNT of=$NUM_FILE &> dd.out
if [[ $? -ne 0 ]]; then
    echo >&2 "dd failed, check dd.out for more information"
    exit 1
fi

mpirun -v $OPTS -np $CPU_COUNT $EXEC $NUM_FILE

rm -f $EXEC $NUM_FILE dd.out
