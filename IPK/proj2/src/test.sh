#!/bin/bash

PORT="12345"

rm -fr tmp .orig myfile server client
# Compile server and client
make &&
chmod +x {server,client}

if [[ $? -ne 0 ]]; then
    echo "Compilation failed, can't continue"
    exit 1
fi

# Create example data
mkdir tmp &&
echo "Synced content" > .orig &&
cp .orig tmp/myfile

if [[ $? -ne 0 ]]; then
    echo "Couldn't create example data"
    exit 1
fi

killall server
killall client
# Run server
echo "Server starting on port $PORT"
./server -p $PORT &>server.log &

# Run client and perform following operations:
# 1) Upload file 'myfile' to server
# 2) Remove this file from client's directory
# 3) Download the file again from server
cd tmp
ln -s ../client client
./client -h localhost -p $PORT -u myfile
rm myfile
./client -h localhost -p $PORT -d myfile

killall server
killall client

# Compare downloaded file with the original one
if [[ ! -e myfile ]]; then
    echo "Sync failed"
else
    diff myfile ../.orig

    if [[ $? -eq 0 ]]; then
        echo "Test has PASSED"
    else
        echo "Test has FAILED"
    fi

    cd ..
    # Print server log
    echo "Server log:"
    cat server.log
fi
