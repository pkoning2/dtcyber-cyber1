#! /bin/bash
if [ "$1" = "" ]; then
    USERNAME="paul"
else
    USERNAME="$1"
fi
ssh -L 5104:localhost:5004  -L 5105:localhost:5005  -L 5150:localhost:5050  -L 5106:localhost:5006  -L 5107:localhost:5007 -L 8105:localhost:8005 -L 8150:localhost:8050 -L 5204:localhost:5204  -L 5205:localhost:5205  -L 5206:localhost:5206  -L 5207:localhost:5207 -L 8205:localhost:8205  -L 5831:localhost:6021 -R 11851:localhost:6021 $USERNAME@cyberserv.org
