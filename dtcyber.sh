#!/bin/bash

# first start dtcyber
./dtcyber cybis580auto &
sleep 1

# now start one each of the UIs
./dd60 .1 &
./dtoper &
./pterm localhost 5005 &
