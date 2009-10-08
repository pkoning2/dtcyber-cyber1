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
curslot = None
real_length = 60     # simulate wire delay for wires this long, None to disable

_re_wstrip = re.compile ("(^[\014\\s]+|\\s*#.*$)", re.M)
_re_wmod = re.compile ("([a-z]+)(\\(.+?\\))?\t(\\w+)\n+((?:\\d+(?:\t+.+)?\n+)+)((\\w+)\n+((?:\\d+(?:\t+.+)?\n+)+))?")
_re_wline = re.compile ("^(\\d+)(?:$|\t+(\\w+)\t+(\\w+)(:?\t(\\d+))?)?", re.M)
_re_chslot = re.compile (r"(0?[1-9]|1[0-6])?([a-r])(0[1-9]|[5-9]|[1-3][0-9]?|4[0-2]?)$", re.I)
_re_cable = re.compile (r"(\d+)?w(\d+)$")
_re_cables = re.compile (r"^(\d+w\d+)\s+(\d+w\d+)", re.M)

class Chassis (cmodule.cmod):
    """An instance of a 6000 chassis.
    """
    def __init__ (self, num):
        cmodule.cmod.__init__ (self, "chassis%d" % num)
        self.cnum = num
        self.connectors = { }
        
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

    def printassigns (self, sigdict, comp = None):
        return ("", dict ())
    
    def finish (self):
        """Check for internal consistency
        """
        for w in sorted (self.signals):
            w = self.signals[w]
            if not (w.source and w.destcount == 1) and \
                   not isinstance (w, Cable) and \
                   w.ptype != "analog":
                if not w.source:
                    print w, "has no source"
                if not w.destcount:
                    print w, "has no destination"
                elif w.destcount > 1:
                    print w, "has multiple destinations"
        for w in self.signals.itervalues ():
            if isinstance (w, Cable):
                for dir in ("in", "out"):
                    if dir in w.dirs:
                        self.addpin (("%s_%s" % (w, dir),), dir, w.ptype)
            elif w.ptype == "analog":
                self.addpin ((w,), "out", w.ptype)
                
    def normslot (self, slot):
        """Normalize a slot name.  Strip off leading chassis number,
        if present (must be correct if present).  Make slot number 2 digits.
        """
        m = _re_chslot.match (slot)
        if not m:
            error ("Invalid module slot ID %s" % slot)
            return None
        if m.group (1):
            if int (m.group (1)) != self.cnum:
                error ("Module slot ID %s chassis number is not %d" % (slot, self.cnum))
                return None
        return "%s%02d" % (m.group (2), int (m.group (3)))
        
    def normcable (self, name):
        """Normalize a cable name.  Make cable number 2 digits, and
        prefix with chassis number.  If chassis number was already present,
        verify that it's correct.
        """
        m = _re_cable.match (name)
        if not m:
            error ("Invalid cable name %s" % name)
            return None
        if m.group (1):
            if int (m.group (1)) != self.cnum:
                error ("Cable number in %s doesn't match chassis number %d" % (name, self.cnum))
                return
        return "%dw%02d" % (self.cnum, int (m.group (2)))

    def findcable (self, name, ctype):
        name = "c_%s" % self.normcable (name)
        if not name:
            return None
        try:
            c = self.signals[name]
            if not isinstance (c, ctype):
                error ("cable type mismatch %" % name)
        except KeyError:
            #print "adding cable", name
            c = self.signals[name] = ctype (name)
        return c
        
    def addconnector (self, slot, modinst, offset = 0):
        """Add a connector for the named slot.  Check that the slot name
        is valid and that it's not a duplicate entry.
        """
        slot = self.normslot (slot)
        if slot:
            if slot in self.connectors:
                error ("Slot %s already defined" % slot)
            else:
                c = self.connectors[slot] = Connector (slot, modinst, self, offset)
                return c

    def addmodule (self, slot, modname):
        """Add a module and its connector.  Returns the module instance
        """
        slot = self.normslot (slot)
        if not slot:
            return
        try:
            mtype = cmodule.elements[modname]
        except KeyError:
            #print "reading", modname
            mtype = cmodule.readmodule (modname, True)
        inst = self.elements[slot] = cmodule.ElementInstance (slot, modname)
        return inst
    
    def processwlist (self, wl):
        """Process a wirelist file for this chassis
        """
        for sl in _re_wmod.finditer (wl):
            inst = self.addmodule (sl.group (3), sl.group (1))
            c = self.addconnector (sl.group (3), inst)
            if c:
                c.processwlist (sl.group (4))
                #if sl.group (2):
                #    inst.generics = sl.group (2)
                if sl.group (5):
                    c = self.addconnector (sl.group (6), inst, 100)
                    c.processwlist (sl.group (7))

class Connector (object):
    """A connector for a module, in a slot
    """
    def __init__ (self, slot, inst, chassis, offset = 0):
        self.name = chassis.normslot (slot)
        self.modinst = inst
        self.offset = offset
        self.chassis = chassis

    def chwire (self, pnum, toslot, topin, wlen = 0):
        """Generate a wire name for a wire inside a chassis (twisted pair).
        Wires are named w_out_in except if the wire is long enough that we
        simulate its delay, in which case a _d is added onto the end of
        the name and the wire is hooked up to an instance of the "wire"
        element that actually implements the delay.
        """
        pin = self.modinst.eltype.pins[self.mipin (pnum)]
        if pin.dir == "out":
            wname = "w_%s_%d_%s_%s" % (self.name, pnum, toslot, topin)
            if real_length and wlen > real_length:
                wd = cmodule.ElementInstance (wname, "wire")
                self.chassis.elements[wname] = wd
                wd.addportmap (self.chassis, "o", wname)
                wname += "_d"
                wd.addportmap (self.chassis, "i", wname)
                wd.addgenericmap (self.chassis, "length", str (wlen))
        else:
            wname = "w_%s_%s_%s_%d" % (toslot, topin, self.name, pnum)
        return wname
    
    def mipin (self, pnum):
        return "p%d" % (pnum + self.offset)

    def addportmap (self, parent, formal, actual):
        self.modinst.addportmap (parent, self.mipin (pn (formal)), actual)

    def processwlist (self, wl):
        """Process the pins data from the wirelist for this connector
        """
        global curslot
        curslot = self.name
        epin = 1
        for m in _re_wline.finditer (wl):
            pnum = pn (m.group (1))
            if pnum != epin:
                error ("pins out of order, expected %d, got %d" % (epin, pnum))
            epin += 1
            if m.group (2) is None:
                # Unused pin, carry on
                continue
            try:
                mpin = self.modinst.eltype.pins[self.mipin (pnum)]
            except KeyError:
                mpin = None
            pname = "p%d" % pnum
            if m.group (3) == "x":
                # ground
                if m.group (2) == "good" or \
                   m.group (2) == "grd" or \
                   m.group (2) == "gnd" or \
                   self.chassis.normslot (m.group (2)) == self.name:
                    if mpin is not None:
                        # ignore ground pin connections that are memory
                        # power/ground rather than module signals
                        self.addportmap (self.chassis, pname, "'1'")
                else:
                    error ("Strange ground-like entry %s" % str (m.groups ()))
            elif "w" in m.group (2):
                # cable
                if mpin is None:
                    error ("pin %d undefined or out of range" % pnum)
                    continue
                dir = mpin.dir
                ptype = mpin.ptype
                wnum = int (m.group (3))
                if ptype == "misc":
                    # We ignore misc signals since they are only there
                    # to document things like jumpers and other non-logic
                    # stuff that doesn't map to VHDL
                    continue
                elif ptype == "analog":
                    # analog corresponds to a 3-bit bus in the VHDL model so
                    # we model that as a separately named signal rather than
                    # a strand in a cable (since it would make the cable
                    # not be homogeneous data type)
                    w = "c_%s_%d" % (self.chassis.normcable (m.group (2)), wnum)
                else:
                    if wnum >= 101 and wnum <= 224:
                        w = Tpwire (self.chassis, m.group (2), m.group (3),
                                    dir, ptype)
                    else:
                        w = Coaxwire (self.chassis, m.group (2), m.group (3),
                                      dir, ptype)
                    self.chassis.signals[w.cable.name] = w.cable
                self.addportmap (self.chassis, pname, w)
            else:
                # Regular slot to slot twisted pair wire
                if mpin is None:
                    error ("pin %d undefined or out of range" % pnum)
                    continue
                ptype = mpin.ptype
                if ptype == "misc":
                    # We ignore misc signals since they are only there
                    # to document things like jumpers and other non-logic
                    # stuff that doesn't map to VHDL
                    continue
                toslot = self.chassis.normslot (m.group (2))
                topin = pn (m.group (3))
                if m.group (4):
                    wlen = int (m.group (4))
                else:
                    wlen = 0
                wname = self.chwire (pnum, toslot, topin, wlen)
                self.addportmap (self.chassis, pname, wname)
        if self.mipin (epin) in self.modinst.eltype.pins:
            # next pin should not exist or we have an incomplete list
            error ("not enough pins for connector")
                
def error (text):
    if curslot:
        print "%s: %s" % (curslot, text)
    else:
        print text

def pn (pname):
    if pname.startswith ("p"):
        return int (pname[1:])
    else:
        return int (pname)
    
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
    def __init__ (self, chassis, cable, cabletype, wirenum, dir, ptype):
        cable = chassis.findcable (cable, cabletype)
        name = cable.makecname (wirenum, dir)
        cmodule.Signal.__init__ (self, name)
        self.cable = cable
        self.dir = dir
        self.ptype = ptype
        cable.dirs.add (dir)
        if wirenum in cable.strands:
            error ("wire %s already defined for %s" % (wirenum, cable.name))
        else:
            cable.strands[wirenum] = self
        
class Coaxwire (Cablewire):
    """A strand of coax terminating at some pin in this chassis.
    It may be internal or external.
    """
    def __init__ (self, chassis, coaxname, wirenum, dir, ptype = "coaxsig"):
        Cablewire.__init__ (self, chassis, coaxname, Coax, wirenum, dir, ptype)

class Tpwire (Cablewire):
    """A strand of twised-pair cable terminating at some pin in this chassis.
    These appear only in connections to the deadstart panel.
    """
    def __init__ (self, chassis, cable, wirenum, dir, ptype = "std_logic"):
        Cablewire.__init__ (self, chassis, cable, Tpcable, wirenum, dir, ptype)
        
class Cable (cmodule.hitem):
    """A cable (coax or twisted pair)
    """
    def __init__ (self, name):
        cmodule.hitem.__init__ (self, name)
        self.source = None
        self._sources = set ()
        self.dirs = set ()
        self.strands = { }
        
class Coax (Cable):
    """A coax cable.  This is modeled as separate input and output cables,
    unless the cable is internal.
    """
    def __init__ (self, name):
        Cable.__init__ (self, name)
        self.ptype = "coaxsigs"
        
    def makecname (self, wnum, dir):
        try:
            wnum = int (wnum)
            if wnum >= 90 and wnum <= 99:
                wnum -= 90
            elif wnum >= 900 and wnum <= 908:
                wnum -= 890
            else:
                return "Invalid wire number %d" % wnum
            return "%s_%s[%d]" % (self.name, dir, wnum)
        except:
            return "Invalid wire number %s" % wnum

class Tpcable (Cable):
    """A twisted pair cable.  This is modeled as just one cable because
    we only have these as inputs (from the deadstart panel)
    """
    def __init__ (self, name):
        Cable.__init__ (self, name)
        self.ptype = "tpcable"
        
    def makecname (self, wnum, dir):
        if dir != "in":
            return "Invalid direction for %s" % wnum
        try:
            x, y = divmod (int (wnum), 100)
            wnum = y * 2 + x - 3
            return "%s[%d]" % (self.name, wnum)
        except:
            return "Invalid wire number %s" % wnum
        

if __name__ == "__main__":
    topname = "cdc6600"
    opts, args = getopt.getopt (sys.argv[1:], "t:")
    if len (args) < 1:
        print "usage: %s wirelist [ wirelist ... ]" % sys.argv[0]
        sys.exit (1)
    for opt, val in opts:
        if opt == "-t":
            # top level filename (default: "cdc6600")
            if val == "-":
                topname = None
            else:
                topname = val
    for f in args:
        c = readwlist (f)
        c.finish ()
    #if topname:
    #    f = open ("%s.vhd" % topname, "w")
    #    top_vhdl (f)
    #    f.close ()
    #for ch in chassis_list:
    #    if ch is None:
    #        continue
    #    f = open ("%s.vhd" % ch.name, "w")
    #    ch.print_vhdl (f)
    #    f.close ()
    #    f = open ("%s.html" % ch.name, "w")
    #    ch.print_modmap (f)
    #    f.close ()
