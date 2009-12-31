#!/usr/bin/env python

import os
import re
import sys

chnum = str (int (sys.argv[1]))

f = open ("chassis%s.map" % chnum, "r")
chmap = eval (f.read ())
f.close ()

checked = set ()

for r in xrange (18):
    rc = chr (65 + r)
    for c in xrange (42):
        m = chmap[r][c]
        if m in checked:
            continue
        checked.add(m)
        try:
            os.stat("%s.vhd" % m.lower ())
            #print "Module %s (%c%d) exists" % (m, rc, c + 1)
        except:
            print "Module %s (%c%d)" % (m, rc, c + 1)
