#!/usr/bin/env python3

import os
import sys

hdr = """job(cm140000,t77770)
user(plato,plato)
"""

scrdir = os.path.dirname (sys.argv[0])
cfgdir = os.path.normpath (os.path.join (scrdir, "..", "nos287"))

configs = os.listdir (cfgdir)

attaches = list ()
texts = list ()

for c in configs:
    cfn = os.path.join (cfgdir, c)
    if "." in c or "~" in c or c == "ndl16" or c == "plprocs" \
           or os.path.isdir (cfn):
        continue
    with open (cfn) as t:
        texts.append (t.read ())
    attaches.append ("attach,%s/m=w,na.\n" % c)
    attaches.append ("copybr,input,%s.\n" % c)
    
hdr += "".join (attaches)
hdr += "~\n"
hdr += "~\n".join (texts)
with open ("configdecks", "wt") as f:
    print (hdr, file = f)
