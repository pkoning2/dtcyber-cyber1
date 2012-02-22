#!/bin/bash

# first start dtcyber
HOSTNAME=$(hostname)
HOST2=${HOSTNAME%.*}
HOST2=${HOST2##*.}

OS=$(uname)


if [ "${HOSTNAME}" = "monster" ]; then 
	HOSTNAME="cyber1"
elif [ "${HOSTNAME}" = "pkoning-laptop" ]; then 
	HOSTNAME="pkoning"
elif [ "${HOSTNAME}" = "paul-konings-macbook-pro.local" ]; then 
	HOSTNAME="pkoning"
elif [ "${HOST2}" = "equallogic" ]; then 
	HOSTNAME="pkoning"
fi
export HOSTNAME
ulimit -c unlimited
if [ "$1" = "" ]; then
    cp sys/871/cy871.ecs.initial sys/871/cy871.ecs
    ./dtcyber cybis871auto | tee cyberlog &
else
#    ./dtcyber $1  | tee cyberlog &
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

if [ "${OS}" = "Darwin" ]; then
    open -a dd60 --args .1 5007
    open -a dtoper --args 5006
    # open -a Pterm --args localhost 5005
else
    ./dd60 .1 5007 &
    ./dtoper 5006 &
    # ./pterm localhost 5005 &
fi

echo ui started
