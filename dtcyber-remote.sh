#!/bin/bash

# first start dtcyber
ulimit -c unlimited
./dtcyber cybis580auto 2>&1 > cyberlog &
