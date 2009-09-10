#!/usr/bin/env python

import os
import re
import sys

f = open ("chassis2.wlist","r")
t = f.read ()
t = t.replace ("\r", "").split ("\n")
header = [ ]
while t[0].startswith ("#"):
    header.append (t[0])
    del t[0]

line = 0

mod = re.compile ("\014?([A-Z]*)[ \t]+(\d)([A-R])(\d+)$")
pin = re.compile ("\014?(.[^\t]?)(\t|$)")
pin1 = re.compile ("\014?1\t")

row = "A"
col = 0

try:
    while line < len (t):
        m1 = mod.match (t[line])
        m2 = pin1.match (t[line])
        col += 1
        if col == 43:
            col = 1
            row = chr (ord (row) + 1)
        if m1:
            if len (m1.group (1)) == 2 and \
                   m1.group (3) == row and int (m1.group (4)) == col:
                if m1.group (2) != "2":
                    t[line] = t[line][:m1.start (2)] + "2" + t[line][m1.end (2):]
                print t[line]
            else:
                mline = raw_input ("mod %s%d: %s: " % (row, col, t[line])).upper ()
                if mline == "D":
                    del t[line]
                    col -= 1
                    continue
                if mline:
                    if len (mline) <= 3:
                        mline += "\t2%s%d" % (row, col)
                    if mline.startswith (" "):
                        t[line] = "\014" + mline[1:]
                    else:
                        t.insert (line, "\014" + mline)
                m1 = mod.match (t[line])
                row = m1.group (3)
                col = int (m1.group (4))
        else:
            print t[line]
            mline = raw_input ("mod %s%d: " % (row, col)).upper ()
            if mline == "D":
                del t[line]
                col -= 1
                continue
            if len (mline) <= 3:
                mline += "\t2%s%d" % (row, col)
            if mline.startswith (" "):
                t[line] = "\014" + mline[1:]
            else:
                t.insert (line, "\014" + mline)
            m1 = mod.match (t[line])
            row = m1.group (3)
            col = int (m1.group (4))
        line += 1
        i = 0
        while i < 28:
            i += 1
            print t[line]
            m = pin.match (t[line])
            try:
                p1 = m.group (1)
                p2 = p1.replace ("I", "1")
                p2 = p2.replace ("O", "0")
                p2 = p2.replace ("\014", "")
                p = int (p2)
                if p != i:
                    raise KeyError
                elif p1 != p2:
                    t[line] = p2 + t[line][len (p1):]
            except:
                act = raw_input ("%d: %s" % (i, t[line])).upper ()
                if act == "R":
                    if m:
                        st = m.end (1)
                    elif t[line].startswith ("\t"):                
                        st = 1
                    else:
                        st = 0
                    t[line] = "%d\t%s" % (i, t[line][st:])
                elif act[0] == "D":
                    if len (act) > 1:
                        p = line + int (act[1:])
                        i -= 1
                    else:
                        p = line
                    del t[p]
                    line -= 1
                elif act.startswith ("-"):
                    n = int (act[1:])
                    i = n - 1
                    line -= 1
                elif act.startswith (" "):
                    t[line] = act[1:]
                else:
                    t.insert (line, act)
            line += 1
except:
    f = open ("chassis2.new", "w")
    f.write ("\n".join (header))
    f.write ("\n")
    f.write ("\n".join (t))
    f.write ("\n")
    f.close ()
    raise

f = open ("chassis2.new", "w")
f.write ("\n".join (header))
f.write ("\n")
f.write ("\n".join (t))
f.write ("\n")
f.close ()
