#!/bin/bash

# first start dtcyber
ulimit -c unlimited
./dtcyber cybis580auto | tee cyberlog &
sleep 1

# now start one each of the UIs
./dd60 .06 &
./dtoper &
./pterm localhost 5005 &
