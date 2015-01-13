#!/usr/bin/env python

import re
import sys
import os
import wlist

def print_modmap (ch, f):
    """Print the module layout.
    """
    modmap = [ ]
    modname = [ ]
    empty = '<td bgcolor="#909090">'
    for r in xrange (18):
        modname.append ([ "" ] * 42)
    for m, i in ch.elements.iteritems ():
        mname = i.eltype.name
        if mname == "wire":
            continue
        r = ord (m[0]) - 97
        if r == 25:
            # Fake row Z for fake modules
            continue
        c = int (m[1:]) - 1
        modname[r][c] = mname
    for r in xrange (18):
        modmap.append ([ "<td><b>%s</b>" % chr (r + 65) ])
        curmod = ""
        repeats = 0
        m = 0
        while m < 43:
            if m < 42:
                mnam = modname[r][m]
            else:
                mnam = ""
            if m < 42 and mnam == curmod and curmod != "" \
                   and curmod != "mem":
                repeats += 1
            else:
                if curmod == "":
                    # Previous was empty
                    if repeats:
                        modmap[r].append (empty)
                else:
                    if curmod == "mem":
                        modmap[r].append ("<td colspan=8 rowspan=2"
                                          " align=center>%s" % curmod)
                    else:
                        modmap[r].append ("<td colspan=%d align=left>%s"
                                          % (repeats, curmod))
                if m < 42:
                    if mnam == "mem":
                        repeats = 0
                        m += 7
                    elif mnam == "" and curmod == "" and r > 0 and \
                             modname[r - 1][m] == "mem":
                        m += 7
                        repeats = 0
                    else:
                        repeats = 1
                    curmod = mnam
            m += 1
        modmap[r] = "".join (modmap[r])
    print >> f, """<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
    <html>
      <head>
        <title>Chassis %d module map</title>
      </head>

      <body>
        <h1>Chassis %d module map</h1>
        <table border=1 cellspacing=0 cellpadding=2>
        <tr align=center><th>
    """ % (ch.cnum, ch.cnum)
    for i in xrange (42):
        print >> f, "<th>%d" % (i + 1)
    print >> f, "</tr>\n<tr>"
    print >> f, "</tr>\n<tr>".join (modmap)
    print >> f, """
          </tr>
        </table>
      </body>
    </html>
    """
        
if __name__ == "__main__":
    args = sys.argv[1:]
    if len (args) < 1:
        print "usage: %s wirelist [ wirelist ... ]" % sys.argv[0]
        sys.exit (1)
    for f in args:
        print "processing", f
        wlist.process_file (f)
    for c in wlist.chassis_list.itervalues ():
        fn = "chassis%d.html" % c.cnum
        print "printing chassis", c.cnum, "module map to", fn
        with open (fn, "wt") as f:
            print_modmap (c, f)
            
