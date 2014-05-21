#! /bin/bash
if [ "$1" = "" ]; then
    USERNAME="paul"
else
    USERNAME="$1"
fi
ssh -L 0.0.0.0:5104:localhost:5004  -L 0.0.0.0:5105:localhost:5005  -L 0.0.0.0:5150:localhost:5050  -L 0.0.0.0:5106:localhost:5006  -L 0.0.0.0:5107:localhost:5007 -L 0.0.0.0:8105:localhost:8005 -L 0.0.0.0:8150:localhost:8050 -L 0.0.0.0:5204:localhost:5204  -L 0.0.0.0:5205:localhost:5205  -L 0.0.0.0:5206:localhost:5206  -L 0.0.0.0:5207:localhost:5207 -L 0.0.0.0:8205:localhost:8205  -L 0.0.0.0:5831:localhost:6021 -R 11851:localhost:6021 $USERNAME@cyberserv.org
