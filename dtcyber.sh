#!/bin/bash

# first start dtcyber
ulimit -c unlimited
./dtcyber cybis580auto | tee cyberlog &

# wait for the NIU port to appear (that's the last one of the UI ports
# to be initialized)
until netstat -tna | fgrep -q 127.0.0.1:5005; do
    sleep 1
    echo -n "."
done

# now start one each of the UIs
./dd60 .06 &
./dtoper &
./pterm localhost 5005 &
