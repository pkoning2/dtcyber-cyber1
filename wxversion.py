#!/usr/bin/env python

'''Script to generate wxversion.h

	@echo "#define WXVERSION \"$(WXVERSION)\"" > wxversion.h.tmp
	@date +"#define PTERMBUILDDATE \"%e %B %Y\"" >> wxversion.h.tmp
ifneq ("$(PTERMSVNREV)", "")
	@echo "#define PTERMSVNREV \"$(PTERMSVNREV)\"" >> wxversion.h.tmp
endif
	@if [ ! -r wxversion.h ]; then mv -f wxversion.h.tmp wxversion.h; else \
	  if cmp -s wxversion.h.tmp wxversion.h; then rm -f wxversion.h.tmp; \
	    else mv -f wxversion.h.tmp wxversion.h; fi; fi
'''

import os
import re
import time
import subprocess

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
except OSError:
    old = ""

wxversion = shellstr ("wx-config", "--version")
svnversion = shellstr ("svn", "info", "pterm_wx.cpp")
if svnversion:
    m = re.search (r"Revision: (\d+)", svnversion)
    if m:
        svnversion = m.group (1)
    else:
        svnversion = ""
now = time.strftime ("%e %B %Y")

new = """#define WXVERSION "%s"
#define PTERMBUILDDATE "%s"
""" % (wxversion, now)

if svnversion:
    new += """#define PTERMSVNREV "%s"
""" % svnversion

with open (FN + ".tmp", "wt") as f:
    f.write (new)



