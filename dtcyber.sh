#!/bin/bash

# first start dtcyber
ulimit -c unlimited
if [ "$1" = "" ]; then
    ./dtcyber cybis580auto | tee cyberlog &
else
    ./dtcyber $1 &
fi

echo dtcyber started

# wait for the operator port to appear
until netstat -tna | fgrep -q 127.0.0.1:5006; do
    sleep 1
    echo -n "."
done

echo done waiting

# now start one each of the UIs
./dd60 .06 &
./dtoper &

echo ui started

# ./pterm localhost 5005 &
