#!/usr/bin/env python3

'''Script to generate wxversion.h
'''

import os
import sys
import re
import time
import subprocess

FN = "wxversion.h"

def shellstr (cmd, *args):
    try:
        return subprocess.check_output ((cmd,) + args,
                                        universal_newlines = True,
                                        stderr = subprocess.STDOUT).rstrip ('\n')
    except subprocess.CalledProcessError:
        return ""
    
try:
    with open (FN, "rt") as f:
        old = f.read ()
except (OSError, IOError):
    old = ""

if sys.platform[:3] == "win":
    wxversion = None
    wx_re = re.compile (r"wxwidgets-([\.0-9]+)", re.I)
    for fn in os.listdir ("c:\\"):
        m = wx_re.match (fn)
        if m:
            wxversion = m.group (1)
            break
    if not wxversion:
        print ("Can't find wx version number")
        sys.exit (1)
else:
    wxversion = shellstr ("wx-config", "--version")

svnversion = shellstr ("svn", "info", "PtermApp.cpp")
if svnversion:
    m = re.search (r"Revision: (\d+)", svnversion)
    if m:
        svnversion = m.group (1)
        if shellstr ("svn", "status", "pterm_wx.cpp"):
            svnversion += "+"
    else:
        svnversion = ""
now = time.strftime ("%d %B %Y").lstrip ("0")

new = """#define WXVERSION "%s"
#define PTERMBUILDDATE "%s"
""" % (wxversion, now)

if svnversion:
    new += """#define PTERMSVNREV "%s"
""" % svnversion

if new == old:
    print (FN, "is up to date")
else:
    with open (FN, "wt") as f:
        f.write (new)
    print (FN, "updated:")
    print (new)

