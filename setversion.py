#!/usr/bin/env python3

import sys
import re
import time
import os

# Filename, commas, crlf.  Special hack: crlf == 0 (not False) means
# do only dates and only the first match.
files = ( ( "pterm-info.plist", False, False ),
          ( "Pterm.pmdoc/01pterm.xml", False, False ),
          ( "pterm.spec", False, False ),
          ( "pterm.iss", False, True ),
          ( "pterm.rc", True, True ),
          ( "pterm-license.rtf", True, 0 ),
          ( "ptermversion.h", True, False ) )

# Negative lookahead for trailing \ here, otherwise it matches filespecs
# with version numbers in them in pterm.iss
dot_re = re.compile (r"\d+\.\d+\.\d+(?!\\)")
comma_re = re.compile (r"\d+,\d+,\d+")
copy_re = re.compile (r"((?:copyright|\(c\)).+?\d+ *- *)20\d\d", re.I)

def setyear (m):
    return m.group (1) + year

def procfile (fn, commas, crlf):
    with open (fn, "rt", encoding = "latin1") as f:
        t = f.read ()
    if crlf is 0:
        t = copy_re.sub (setyear, t, 1)
    else:
        t = dot_re.sub (dotver, t)
        t = comma_re.sub (commaver, t)
        t = copy_re.sub (setyear, t)
    os.rename (fn, fn + "~")
    if crlf:
        newline = "\r\n"
    else:
        newline = "\n"
    with open (fn, "wt", newline = newline, encoding = "latin1") as f:
        f.write (t)

def procfiles (args):
    global dotver, commaver, year
    vers = tuple (args[0].split ("."))
    if len (args) > 1:
        # For testing
        year = args[1]
    else:
        year = str (time.localtime ().tm_year)
    dotver = "%s.%s.%s" % vers
    commaver = "%s,%s,%s" % vers
    
    for fn, commas, crlf in files:
        procfile (fn, commas, crlf)
        print ("processed", fn)

if __name__ == "__main__":
    procfiles (sys.argv[1:])
    
