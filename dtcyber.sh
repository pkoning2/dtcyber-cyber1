#!/bin/bash

# This script can be invoked as dtcyber-remote.sh, in which case
# it does not start the UI apps.
REMOTE=${0##*-}
SCRDIR=${0%/*}

cd ${SCRDIR}

# first start dtcyber
HOSTNAME=$(hostname)
HOST1=${HOSTNAME%%.*}
HOST2=${HOSTNAME%.*}
HOST2=${HOST2##*.}

OS=$(uname)

if [ "${REMOTE}" = "remote.sh" ]; then
    TEE=">"
else
    TEE="| tee "
fi

if [ "${HOST1}" = "monster" ]; then 
    HOSTNAME="cyber1"
elif [ "$HOST1" = "bullwinkle" ]; then
    HOSTNAME="cyber1"
elif [ "${HOST1}" = "pkoning-laptop" ]; then 
    HOSTNAME="pkoning"
elif [ "${HOST1}" = "paul-konings-macbook-pro" ]; then 
    HOSTNAME="pkoning"
elif [ "${HOST2}" = "equallogic" ]; then 
    HOSTNAME="pkoning"
elif [ "${HOST1}" = "plato4" ]; then 
    HOSTNAME="pkoning"
fi
export HOSTNAME
ulimit -c unlimited

# If blackbox is running, make it go away
if [ -s "blackbox.pid" ]; then
    kill `cat blackbox.pid`
    rm -f blackbox.pid
    sleep 3
    echo "Blackbox stopped"
fi

rm -f cyberlog
if [ "$1" = "" ]; then
    cp sys/871/cy871.ecs.initial sys/871/cy871.ecs
    eval "./dtcyber cybis871auto 2>&1 ${TEE} cyberlog &"
else
    eval "./dtcyber $1 2>&1  ${TEE} cyberlog &"
fi

echo dtcyber started

# wait for each of the ports to appear
until netstat -tna | egrep -q 127.0.0.1.5005; do
    sleep 1
    echo -n "."
done
until netstat -tna | egrep -q 127.0.0.1.5006; do
    sleep 1
    echo -n "."
done
until netstat -tna | egrep -q 127.0.0.1.5007; do
    sleep 1
    echo -n "."
done

sleep 1

echo ""

if [ "${REMOTE}" = "remote.sh" ]; then
    echo "dtcyber is running"
else
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
fi
