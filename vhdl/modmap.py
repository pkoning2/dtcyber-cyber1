#!/usr/bin/env python

import re
import sys
import os

rows = [ ]

for rn in xrange (18):
    ch = chr (65 + rn)
    m = 0
    dm = "--"
    defrow = None
    r = [ ]
    for i in xrange (42):
        r.append ("--")
    while m < 42:
        if defrow:
            dm = defrow[m]
        mn = raw_input ("%c%02d [%s]: " % (ch, m + 1, dm)).upper ().strip ()
        if mn and mn[0] == "=":
            if len (mn) < 2:
                defrow = None
            else:
                defrow = rows[ord (mn[1]) - 65]
            continue
        if mn == "^":
            if m:
                m -= 1
            continue
        if mn == "-":
            mn = "--"
        elif not mn:
            if defrow:
                mn = defrow[m]
            else:
                mn = dm
        else:
            try:
                upto = int (mn)
                for i in xrange (m, upto):
                    if defrow:
                        r[i] = defrow[i]
                    else:
                        r[i] = dm
                    print r[i],
                print
                m = upto
                continue
            except ValueError:
                pass
            dm = mn
        r[m] = mn
        m += 1
    rows.append (r)
print rows
f = open ("chxx.map", "w")
print >> f, rows
