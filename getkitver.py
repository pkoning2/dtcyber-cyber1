#!/usr/bin/env python

import os
import sys

src = sys.argv[1]
f = open (src, "r")
for l in f:
    l = l.split ()
    if l[1] == "FILEVER":
        nums = l[2].split (',')
        nums2 = [ ]
        for n in nums:
            nums2.append (int (n))
        if len (nums2) == 3:
            print "%d.%d%d" % tuple (nums2)
        else:
            print "%d.%d%d-%d" % tuple (nums2)
        sys.exit (0)
