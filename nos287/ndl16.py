#!/usr/bin/python

"""Expand NDL16.tem for a given number of NAM terminals.
"""

import sys
import re

count = 1

range = re.compile (r"\[.+?\]", re.DOTALL)

def expand (m):
    s = m.group (0)
    s = s.strip ("[]")
    l = [ ]
    for i in xrange (count):
        num = "%02X" % (i + 1)
        l.append (s.replace ("#", num))
    return "\n".join (l)

def main (argv):
    global count
    count = int (argv[1])
    inf = open ("ndl16.tem", "r")
    text = inf.read ()
    inf.close ()
    outf = open ("ndl16", "w")
    text = range.sub (expand, text)
    outf.write (text)
    outf.close ()

if __name__ == "__main__":
    main (sys.argv)
    
