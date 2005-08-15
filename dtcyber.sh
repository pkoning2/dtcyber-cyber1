#!/bin/bash

# first start dtcyber
ulimit -c unlimited
if [ "$1" = "" ]; then
    ./dtcyber cybis871auto | tee cyberlog &
else
    ./dtcyber $1 &
fi

echo dtcyber started

# wait for each of the ports to appear
until netstat -tna | grep -q 127.0.0.1.5005; do
    sleep 1
    echo -n "."
done
until netstat -tna | grep -q 127.0.0.1.5006; do
    sleep 1
    echo -n "."
done
until netstat -tna | grep -q 127.0.0.1.5007; do
    sleep 1
    echo -n "."
done

sleep 1

echo ""

# now start one each of the UIs
./dd60 .06 &
./dtoper &
# ./pterm localhost 5005 &

echo ui started
