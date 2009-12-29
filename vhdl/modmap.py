#!/usr/bin/env python

import re
import sys
import os

rows = [ ]

for rn in xrange (18):
    ch = chr (65 + rn)
    m = 1
    dm = "--"
    r = [ ]
    for i in xrange (42):
        r.append ("--")
    while m <= 42:
        mn = raw_input ("%c%02d [%s]: " % (ch, m, dm)).upper ().strip ()
        if mn[0] == "=":
            r = rows[ord (mn[1]) - 65)
            break
        if mn == "^":
            m -= 1
            continue
        if mn == "-":
            mn = "--"
        elif not mn:
            mn = dm
        else:
            dm = mn
        r[m - 1] = mn
        m += 1
    rows.append (r)
print rows
f = open ("chxx.map", "w")
print >> f, rows
