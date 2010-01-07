#!/usr/bin/env python

import os
import re
import sys

chnum = str (int (sys.argv[1]))
interactive = len (sys.argv) > 2

f = open ("chassis%s.wlist" % chnum,"r")
t = f.read ()
t = t.replace ("\r", "").split ("\n")
line = 0

modules = { }

_re_chslot = re.compile (r"(0?[1-9]|1[0-6])?([a-r])(0[1-9]|[5-9]|[1-3][0-9]?|4[0-2]?)$", re.I)
def normslot (slot):
    """Normalize a slot name.  Strip off leading chassis number,
    if present .  Make slot number 2 digits.
    """
    m = _re_chslot.match (slot)
    if not m:
        #print "Invalid module slot ID %s" % slot
        return "Z99"
    return "%s%02d" % (m.group (2), int (m.group (3)))

class Mod (object):
    def __init__ (self, name):
        self.name = name
        self.row = name[0]
        self.col = int (name[1:])
        p = [ ]
        for pn in xrange (28):
            p.append (Pin (self, pn))
        self.pins = p
        global modules
        modules[name] = self
        
    def pin (self, num):
        return self.pins[num - 1]
    
class Pin (object):
    def __init__ (self, parent, num):
        self.parent = parent
        self.num = num + 1
        self.linenum = 0
        self.errmsg = None
        self.connto = None
        self.conns = set ()

    def pinid (self):
        return (self.parent.row, self.parent.col, self.num)

    def setsrc (self, l):
        self.linenum = l

    def seterr (self, text):
        if self.errmsg:
            self.errmsg += ", " + text
        else:
            self.errmsg = text

    def connect (self, topin):
        self.connto = topin

    
_re_mod = re.compile ("([A-Z]*)[ \t]+(\d\d?)([A-R])(\d+)$")
_re_pin = re.compile ("^(\\d+)")
_re_wline = re.compile ("^(\\d+)(?:\t+(\\w+)\t+(\\w+)(:?\t(\\w+))?)?\s*$")
_re_checkline = re.compile ("^(\\d+)(?:\t+(GOOD|GRD|GND|[A-RW]\\d+)\t+(X|\\d+)(:?\t(\\d+))?)?$")

def interact (changepin, curpin, newin):
    which = ""
    ln = changepin.linenum
    if newin:
        newrow, newcol, newpin = newin
        newmod = "%s%02d" % (newrow, newcol)
        newmod = modules[newmod]
        newpin = newmod.pin (newpin)
        m3 = _re_wline.match (t[newpin.linenum])
    else:
        m3 = None
    if changepin != curpin:
        which = "other: "
        m2 = _re_wline.match (t[curpin.linenum])
        m = _re_wline.match (t[ln])
        newlen = (m and m.group (4)) or m2.group (4) or \
                 (m3 and m3.group (4)) or ""
    else:
        m = _re_wline.match (t[ln])
        newlen = (m and m.group (4)) or \
                 (m3 and m3.group (4)) or ""                 
    while True:
        repl = raw_input ("%s%s: " % (which, t[ln])).upper ()
        if repl:
            if repl == "R":
                if not newin:
                    print "No replacement available"
                    continue
                newrow, newcol, newpin = newin
                repl = "%d\t%s%02d\t%d%s" % (changepin.num, newrow, newcol, newpin, newlen)
                print repl
                n = _re_checkline.match (repl)
                if not n:
                    print "???invalid format"
                    continue
            elif not repl[0].isdigit ():
                repl = "%d\t%s" % (changepin.num, repl)
            repl = repl.strip ()
            n = _re_checkline.match (repl)
            if not n:
                print "invalid format", repl
                continue
        break
    t[ln] = repl
    # Now update the connection
    if changepin.connto:
        torow, tocol, topin = changepin.connto
        tomod = "%s%02d" % (torow, tocol)
        tomod = modules[tomod]
        topin = tomod.pin (topin)
    else:
        topin = None
    if not n.group (2) or n.group (2).startswith ("W") or \
           n.group (3) == "X":
        newpin = None
    else:
        try:
            newmod = modules[n.group (2)]
            newpin = newmod.pin (int (n.group (3)))
        except KeyError:
            newpin = None
    if topin:
        # We were connected to a pin.  Remove this pin from its
        # connections set
        topin.conns.discard (changepin.pinid ())
    if newpin:
        changepin.connto = newpin.pinid ()
        newpin.conns.add (changepin.pinid ())
    else:
        changepin.connto = None
    # Delete any error messages since we replaced the line
    changepin.errmsg = None
    

row = "A"
col = 0

try:
    while line < len (t):
        #print t[line]
        if t[line].startswith ("#"):
            line += 1
            continue
        m1 = _re_mod.match (t[line].upper ())
        if m1:
            modname = normslot (m1.group (3) + m1.group (4))
        else:
            print "no module header in", t[line]
            line += 1
            continue
        curmod = Mod (modname)
        line += 1
        i = 0
        while i < 28:
            i += 1
            curpin = curmod.pin (i)
            curpin.setsrc (line)
            if line >= len (t):
                print "Unexpected EOF in", modname, i
                sys.exit (1)
            #print t[line]
            pm = _re_pin.match (t[line])
            m = _re_wline.match (t[line].upper ())
            line += 1
            try:
                pnum = int (pm.group (1))
            except:
                curpin.seterr ("bad pin number")
                continue
            if pnum != i:
                curpin.seterr ("pin out of sequence")
                continue
            if not m:
                curpin.seterr ("invalid line")
                continue
            if m.group (2) is None:
                # Unused pin, carry on
                continue
            if m.group (3) == "X":
                # ground
                if m.group (2) == "GOOD" or \
                   m.group (2) == "GRD" or \
                   m.group (2) == "GND" or \
                   m.group (2) == modname:
                    # Grounded input, ignore
                    continue
            if m.group (4):
                try:
                    int (m.group (4))
                except:
                    curpin.seterr ("bad length")
            try:
                topin = int (m.group (3))
            except:
                curpin.seterr ("bad destination pin number")
            if "W" in m.group (2):
                # cable
                if not ((topin >= 90 and topin <= 99) or \
                   (topin >= 900 and topin <= 918)):
                    curpin.seterr ("invalid coax wire number")
                    continue
            else:
                # Regular slot to slot twisted pair wire
                if topin < 1 or topin > 28:
                    curpin.seterr ("invalid destination pin number")
                    continue
                tomod = normslot (m.group (2))
                torow = tomod[0]
                try:
                    tocol = int (tomod[1:])
                except:
                    tocol = 0
                if torow < 'A' or torow > 'R' or tocol < 1 or tocol > 42:
                    curpin.seterr ("invalid destination module id")
                    continue
                curpin.connect ((torow, tocol, topin))
    for modname in sorted (modules.keys ()):
        curmod = modules[modname]
        for curpin in curmod.pins:
            if curpin.errmsg is None and curpin.connto:
                torow, tocol, topin = curpin.connto
                tomod = "%s%02d" % (torow, tocol)
                try:
                    tomod = modules[tomod]
                except:
                    curpin.seterr ("destination module doesn't exist")
                    continue
                topin = tomod.pin (topin)
                topin.conns.add (curpin.pinid ())
    for modname in sorted (modules.keys ()):
        curmod = modules[modname]
        for curpin in curmod.pins:
            if curpin.errmsg:
                doreplace = False
                newin = None
                myset = set ((curpin.pinid (),))
                if curpin.connto:
                    torow, tocol, topin = curpin.connto
                    tomod = "%s%02d" % (torow, tocol)
                    tomod = modules[tomod]
                    topin = tomod.pin (topin)
                    toset = set ((curpin.connto,))
                else:
                    toset = set ()
                    tomod = topin = None
                if curpin.conns != toset or \
                       (topin and topin.conns != myset):
                    if curpin.conns != toset:
                        doreplace = interactive
                        changepin = curpin
                        newin = None
                        print "%s: %s: %s," % (modname, t[curpin.linenum], curpin.errmsg),
                        if curpin.connto:
                            print "connection mismatch, out %s%d.%d, in" % curpin.connto,
                        else:
                            print "connection mismatch, out None, in",
                        if not curpin.conns and topin:
                            print "None",
                            changepin = topin
                            newin = curpin.pinid ()
                        for inconn in sorted (list (curpin.conns)):
                            newin = inconn
                            print "%s%d.%d" % inconn,
                        print
                    else:
                        print "%s: %s:" % (modname, t[curpin.linenum]),
                        if topin.connto:
                            print "other connection mismatch, out %s%d.%d, in" % topin.connto,
                        else:
                            print "other connection mismatch, out None, in",
                        if not topin.conns:
                            print "None",
                        for inconn in sorted (list (topin.conns)):
                            print "%s%d.%d" % inconn,
                        print
                else:
                    doreplace = interactive
                    changepin = curpin
                    print "%s: %s: %s" % (modname, t[curpin.linenum], curpin.errmsg)
                if doreplace:
                    interact (changepin, curpin, newin)
    for modname in sorted (modules.keys ()):
        curmod = modules[modname]
        for curpin in curmod.pins:
            doreplace = False
            newin = None
            if curpin.connto:
                myset = set ((curpin.pinid (),))
                torow, tocol, topin = curpin.connto
                tomod = "%s%02d" % (torow, tocol)
                tomod = modules[tomod]
                topin = tomod.pin (topin)
                toset = set ((curpin.connto,))
                if curpin.conns != toset or topin.conns != myset:
                    if curpin.conns != toset:
                        doreplace = interactive
                        changepin = curpin
                        newin = None
                        print "%s: %s:" % (modname, t[curpin.linenum]),
                        if curpin.connto:
                            print "connection mismatch, out %s%d.%d, in" % curpin.connto,
                        else:
                            print "connection mismatch, out None, in",
                        if not curpin.conns:
                            print "None",
                            changepin = topin
                            newin = curpin.pinid ()
                        for inconn in sorted (list (curpin.conns)):
                            newin = inconn
                            print "%s%d.%d" % inconn,
                        print
                    else:
                        print "%s: %s:" % (modname, t[curpin.linenum]),
                        if topin.connto:
                            print "other connection mismatch, out %s%d.%d, in" % topin.connto,
                        else:
                            print "other connection mismatch, out None, in",
                        if not topin.conns:
                            print "None",
                        for inconn in sorted (list (topin.conns)):
                            print "%s%d.%d" % inconn,
                        print
            if doreplace:
                interact (changepin, curpin, newin)

except:
    f = open ("chassis%s.new" % chnum, "w")
    f.write ("\n".join (t))
    if t[-1]:
        f.write ("\n")
    f.close ()
    raise

f = open ("chassis%s.new" % chnum, "w")
f.write ("\n".join (t))
f.write ("\n")
f.close ()
