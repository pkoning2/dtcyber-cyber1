#!/usr/bin/env python

"""Process CDC 6600 wiring list

Copyright (C) 2008, 2009 Paul Koning
"""

import re
import sys
import traceback
import getopt
import cmodule

chassis_list = [ None ] * 17
curch = None
curcnum = None
ground = "ground"
real_length = 60     # simulate wire delay for wires this long, None to disable

_re_wstrip = re.compile ("(^[\014\\s]+|\\s*#.*$)", re.M)
_re_wmod = re.compile ("(\\w+)(\\(.+?\\))?\t(\\w+)\n+((?:\\d+(?:\t+.+)?\n+)+)((\\w+)\n+((?:\\d+(?:\t+.+)?\n+)+))?")
_re_wline = re.compile ("^(\\d+)\t+(\\w+)\t+(\\w+)")
_re_chslot = re.compile (r"([1-9]|1[0-6])?([a-r](:?0[1-9]|[5-9]|[1-3][0-9]?|4[0-2]?))$", re.I)
_re_cable = re.compile (r"(\d+)?w(\d+)$")

class Chassis (cmodule.cmod):
    """An instance of a 6000 chassis.
    """
    def __init__ (self, num):
        self.cnum = num
        cmodule.cmod.__init__ (self, "chassis%d" % num)
        self.connectors = { }
        self.cables = { }
        
    header = """-------------------------------------------------------------------------------
--
-- CDC 6600 model -- chassis %d
--
-- Converted from wire lists by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-------------------------------------------------------------------------------
"""

    def printheader (self):
        return self.header % self.cnum

    def finish (self):
        """Check for internal consistency
        """
        for w in sorted (self.signals):
            if not (w.source and w.dest):
                print "half-connected wire", w

    def normcable (self, name):
        """Normalize a cable name.  Make cable number 2 digits, and
        prefix with chassis number.  If chassis number was already present,
        verify that it's correct.
        """
        m = _re_cable (name)
        if not m:
            error ("Invalid cable name %s" % name)
            return None
        if m.group (1):
            if int (m.group (1)) != self.cnum:
                error ("Cable number in %s doesn't match chassis number %d" % (name, self.cnum))
                return
        return "%dw%02d" % (self.cnum, int (m.group (2)))

    def findcable (self, name):
        name = self.normname (name)
        if not name:
            return None
        try:
            return self.cables[name]
        except KeyError:
            return None
        
    def checkconnector (self, slot):
        m = _re_chslot.match (slot)
        if not m:
            error ("Invalid module slot ID %s" % slot)
            return None
        if m.group (1):
            if int (m.group (1)) != self.cnum:
                error ("Module slot ID %s chassis number is not %d" % (slot, self.cnum))
                return None
        return m.group (2)
        
    def addconnector (self, slot, modinst):
        """Add a connector for the named slot.  Check that the slot name
        is valid and that it's not a duplicate entry.
        """
        cslot = self.checkconnector (slot)
        if cslot:
            if cslot in self.connectors:
                error ("Slot %s already defined" % slot)
            else:
                self.connectors[cslot] = Connector (cslot, modinst)

    def addmodule (self, slot, modname):
        """Add a module and its connector.  Returns the module instance
        """
        if not self.checkconnector (slot):
            return
        try:
            mtype = cmodule.elements[modname]
        except KeyError:
            print "reading", modname
            mtype = cmodule.readmodule (modname)
        inst = self.elements[slot] = cmodule.ElementInstance (slot, modname)
        self.addconnector (slot, inst)
        return inst
    
    def processwlist (self, wl):
        for sl in _re_wmod.finditer (wl):
            inst = self.addmodule (sl.group (3), sl.group (1))
            #if sl.group (2):
            #    inst.generics = sl.group (2)
            if sl.group (5):
                self.addconnector (sl.group (6), inst)

class Connector (object):
    """A connector for a module, in a slot
    """
    def __init__ (self, slot, inst):
        self.name = slot
        self.modinst = inst
        
def error (text):
    print text

_re_fncnum = re.compile (r"chassis(\d+)\.")
def readwlist (fn):
    """Read the wire list from the named file
    """
    cnum = int (_re_fncnum.match (fn).group (1))
    f = open (fn, "r")
    wl = _re_wstrip.sub ("", f.read ()).lower ()
    f.close ()
    ch = Chassis (cnum)
    ch.processwlist (wl)
    return ch

class Wire (cmodule.Signal):
    """A wire (twisted pair) between two connector pins in the same chassis.
    """
    def __init__ (self, end1, end2):
        name = makename (end1, end2)
        cmodule.Signal.__init__ (self, name)
        self.dest = None
        self.ptype = "std_logic"
        
class Cablewire (cmodule.Signal):
    """A strand of a cable terminating at some pin in this chassis.
    It may be coax or twisted pair, internal (if coax) or external.
    """
    def __init__ (self, chassis, cable, wirenum, dir):
        cable = chassis.findcable (cablename)
        name = cable.makecname (wirenum, dir)
        cmodule.signal.__init__ (self, name)
        self.cable = cable
        self.dir = dir

class Coaxwire (Cablewire):
    """A strand of coax terminating at some pin in this chassis.
    It may be internal or external.
    """
    def __init__ (self, chassis, coaxname, wirenum, dir):
        Cablewire.__init__ (self, chassis, coaxname, wirenum, dir)
        self.ptype = "coaxsig"

class Tpwire (Cablewire):
    """A strand of twised-pair cable terminating at some pin in this chassis.
    These appear only in connections to the deadstart panel.
    """
    def __init__ (self, chassis, cablename, wirenum):
        Cablewire.__init__ (self, chassis, coaxname, wirenum, "in")
        self.ptype = "std_logic"
        
class Cable (cmodule.hitem):
    """A cable (coax or twisted pair)
    """
    def __init__ (self, name):
        cmodule.hitem.__init__ (self, name)
        
class Coax (Cable):
    """A coax cable.  This is modeled as separate input and output cables,
    unless the cable is internal.
    """
    def __init__ (self, name, internal = False):
        Cable.__init__ (self, name)
        self.internal = internal
        self.used = { "in" : False, "out" : False }

    def makecname (self, wnum, dir):
        try:
            wnum = int (wnum)
            # Note that coaxsigs (a coax cable) is indexed 1-based
            if wnum >= 90 and wnum <= 99:
                wnum -= 89
            elif wnum >= 900 and wnum <= 908:
                wnum -= 889
            else:
                return "Invalid wire number %d" % wnum
            self.used[dir] = True
            if self.internal:
                return "c_%s[%d]" % (self.name, wnum)
            else:
                return "c_%s_%s[%d]" % (self.name, dir, wnum)
        except:
            return "Invalid wire number %s" % wnum

class Tpcable (Cable):
    """A twisted pair cable.  This is modeled as just one cable because
    we only have these as inputs (from the deadstart panel)
    """
    def __init__ (self, name, internal = False):
        Cable.__init__ (self, name)
        self.used = False

    def makecname (self, wnum, dir):
        if dir != "in":
            return "Invalid direction for %s" % wnum
        try:
            # TP cable is indexed 0-based
            x, y = divmod (int (wnum), 100)
            wnum = y * 2 + x - 3
            self.used = True
            return "c_%s[%d]" % (self.name, wnum)
        except:
            return "Invalid wire number %s" % wnum
        
