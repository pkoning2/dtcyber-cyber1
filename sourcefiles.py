#!/usr/bin/env python3

"""Given a list of .d files, output the list of source and header files
it references, not including any system headers.
"""

import re
import os
import sys

skipfile_re = re.compile (r"/usr/local/include|/usr/include|/usr/lib|/usr/local/lib|\.o:?$|\.d:?")

def main ():
    srcs = set ()
    curwd = os.path.abspath (".") + "/"
    for a in sys.argv[1:]:
        with open (a, "rt") as f:
            l = f.read ()
        l = l.replace ("\\\n", "")
        l = l.replace (curwd, "")
        for fn in l.split ():
            if skipfile_re.search (fn):
                continue
            srcs.add (fn)
    for fn in sorted (srcs):
        print (fn)

if __name__ == "__main__":
    main ()
    
