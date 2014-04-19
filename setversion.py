#!/usr/bin/env python3

import sys
import re
import time

YEAR = str (time.localtime ().tm_year)

# Filename, commas, crlf
files = ( ( "pterm-info.plist", False, False ),
          ( "pterm.spec", False, False ),
          ( "pterm.iss", False, True ),
          ( "pterm.rc", True, True ),
          ( "ptermversion.h", True, False ) )

dot_re = re.compile (r"\d+\.\d+\.\d+")
comma_re = re.compile (r"\d+,\d+,\d+")
copy_re = re.compile (r"((?:copyright|\(c\)).+?\d+-)20\d\d", re.I)

def setyear (m):
    return m.group (1) + YEAR

def procfile (fn, commas, crlf):
    with open (fn, "rt") as f:
        t = f.read ()
    t = dot_re.sub (dotver, t)
    t = comma_re.sub (commaver, t)
    t = copy_re.sub (setyear, t)
    os.rename (fn, fn + "~")
    if crlf:
        newline = "\r\n"
    else:
        newline = "\n"
    with open (fn, "wt", newline = newline) as f:
        f.write (t)

def procfiles (args):
    global dotver, commaver
    vers = args[0].split (".")
    dotver = "%s.%s.%s" % vers
    commaver = "%s,%s,%s" % vers
    
    for fn, commas, crlf in files:
        procfile (fn, commas, crlf)
        print ("processed", fn)

if __name__ == "__main__":
    procfiles (sys.argv[1:])
    
