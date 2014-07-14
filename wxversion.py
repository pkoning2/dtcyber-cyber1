#!/usr/bin/env python

'''Script to generate wxversion.h
'''

import os
import sys
import re
import time
import subprocess

try:
    subprocess.check_output
except AttributeError:
    # That function was added in Python 2.7.  This is a copy of that code,
    # use it if our Python is older than that.
    def check_output(*popenargs, **kwargs):
        if 'stdout' in kwargs:
            raise ValueError('stdout argument not allowed, it will be overridden.')
        process = subprocess.Popen(stdout=subprocess.PIPE, *popenargs, **kwargs)
        output, unused_err = process.communicate()
        retcode = process.poll()
        if retcode:
            cmd = kwargs.get("args")
            if cmd is None:
                cmd = popenargs[0]
            raise subprocessCalledProcessError(retcode, cmd, output=output)
        return output
    subprocess.check_output = check_output


FN = "wxversion.h"

def shellstr (cmd, *args):
    try:
        return subprocess.check_output ((cmd,) + args,
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
        print "Can't find wx version number"
        sys.exit (1)
else:
    wxversion = shellstr ("wx-config", "--version")

svnversion = shellstr ("svn", "info", "pterm_wx.cpp")
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
    print FN, "is up to date"
else:
    with open (FN, "wt") as f:
        f.write (new)
    print FN, "updated"
