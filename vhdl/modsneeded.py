#!/usr/bin/env python

import os
import re
import sys

chnum = str (int (sys.argv[1]))

modslot = re.compile ("([a-z]{2,5})\t[0-9]{1,2}([a-r][0-9]{1,2})", re.I)

f = open ("chassis%s.wlist" % chnum, "rt")
mods = list ()
for l in f:
    m = modslot.match (l)
    if m:
        mods.append (m.groups ())
f.close ()

checked = set ()

for m, slot in mods:
    if m in checked:
        continue
    checked.add(m)
    try:
        os.stat("%s.vhd" % m.lower ())
        #print "Module %s (%s) exists" % (m, slot)
    except:
        print "Module %s (%s)" % (m, slot)
