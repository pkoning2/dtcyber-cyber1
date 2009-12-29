#!/usr/bin/env python

import os
import re
import sys

chnum = str (int (sys.argv[1]))

try:
    f = open ("chassis%s.map" % chnum, "r")
    chmap = eval (f.read ())
    f.close ()
except:
    print "no chassis map"

f = open ("chassis%s.wlist" % chnum,"r")
t = f.read ()
t = t.replace ("\r", "").split ("\n")
header = [ ]
print t[0].split()
print t[0].startswith ("#")
while t[0].startswith ("#"):
    header.append (t[0])
    print t[0], len (header)
    del t[0]

line = 0

mod = re.compile ("([A-Z]*)[ \t]+(\d\d?)([A-R])(\d+)$")
pin = re.compile ("(.[^\t]?)(\t|$)")
pin1 = re.compile ("1(\t.+)?$")

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
        if row == "S":
            break
        defmod = chmap[ord (row) - 65][col - 1]
        # print "default: %c%02d %s" % (row, col, defmod)
        if defmod == "--":
            continue
        if m1:
            if len (m1.group (1)) == 2 and m1.group (1) == defmod and \
                   m1.group (3) == row and int (m1.group (4)) == col:
                if m1.group (2) != chnum:
                    t[line] = t[line][:m1.start (2)] + chnum + t[line][m1.end (2):]
                print t[line]
            else:
                mline = raw_input ("mod %s%d: %s [%s]: " % (row, col, t[line], defmod)).upper ()
                if mline == "D":
                    del t[line]
                    col -= 1
                    continue
                if not mline:
                    mline = " " + defmod
                if mline:
                    if len (mline) <= 3:
                        mline += "\t%s%s%d" % (chnum, row, col)
                    if mline.startswith (" "):
                        t[line] = mline[1:]
                    else:
                        t.insert (line, mline)
                m1 = mod.match (t[line])
                row = m1.group (3)
                col = int (m1.group (4))
                if not mline.startswith (" "):
                    line += 1
        else:
            print t[line]
            #print m2,
            #if m2: print m2.groups (),
            #print
            mline = raw_input ("mod %s%d [%s]: " % (row, col, defmod)).upper ()
            if mline == "D":
                del t[line]
                col -= 1
                continue
            if not mline:
                if m2:
                    mline = defmod
                else:
                    mline = " " + defmod
            if len (mline) <= 3:
                mline += "\t%s%s%d" % (chnum, row, col)
            if mline.startswith (" "):
                t[line] = mline[1:]
            else:
                t.insert (line, mline)
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
                p2 = p2.replace ("S", "5")
                p = int (p2)
                if p != i:
                    raise KeyError
                elif p1 != p2:
                    t[line] = p2 + t[line][len (p1):]
                if m.group (2) == "\t":
                    p = m.end (2) + 1
                    p1 = t[line][p:]
                    p2 = p1.replace ("I", "1")
                    p2 = p2.replace ("O", "0")
                    p2 = p2.replace ("S", "5")
                    if p1 != p2:
                        t[line] = t[line][:p] + p2                    
            except:
                while True:
                    act = raw_input ("%d: %s" % (i, t[line])).upper ()
                    if act:
                        break
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
    f = open ("chassis%s.new" % chnum, "w")
    if header:
        f.write ("\n".join (header))
        f.write ("\n")
    f.write ("\n".join (t))
    f.write ("\n")
    f.close ()
    raise

f = open ("chassis%s.new" % chnum, "w")
if header:
    f.write ("\n".join (header))
    f.write ("\n")
f.write ("\n".join (t))
f.write ("\n")
f.close ()
