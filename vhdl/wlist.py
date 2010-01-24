#!/usr/bin/env python

"""Process CDC 6600 wiring list

Copyright (C) 2008, 2009 Paul Koning
"""

import re
import sys
import traceback
import getopt
import cmodule

chassis_list = { }
curslot = None
real_length = 60     # simulate wire delay for wires this long, None to disable

_re_wstrip = re.compile ("(^[\014\\s]+|\\s*#.*$)", re.M)
_re_wmod = re.compile ("([a-z]+)(\\(.+?\\))?\t(\\w+)\n+((?:\\d+(?:\t+.+)?\n+)+)((\\w+)\n+((?:\\d+(?:\t+.+)?\n+)+))?")
_re_wline = re.compile ("^(\\d+)(?:$|\t+(\\w+)\t+(\\w+)(:?\t(\\d+))?)?", re.M)
_re_chslot = re.compile (r"(0?[1-9]|1[0-6])?([a-rz])(0[1-9]|[5-9]|[1-3][0-9]?|4[0-2]?)$", re.I)
_re_cable = re.compile (r"(\d+)?w(\d+)$")
_re_cables = re.compile (r"^(\d+w\d+)\s+(\d+w\d+)", re.M)

class Cyber (cmodule.cmod):
    """An instance of a Cyber (top level object)
    """
    def __init__ (self):
        cmodule.cmod.__init__ (self, "cdc6600")
        self.conntext = None
        
    header = """-------------------------------------------------------------------------------
--
-- CDC 6600 model -- top level
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
        return self.header

    def isinternal (self, sig):
        return isinstance (sig, Cable)
    
    def printassigns (self, sigdict, comp = None):
        return ("", dict ())
    
    def finish (self):
        """Check for internal consistency
        """
        self.processconns2 ()
        # Hook up the standard signals
        for w in self.signals.itervalues ():
            if not isinstance (w, Cable):
                # Default signals
                self.addpin ((w,), "in", w.ptype)
        # Handle any loose input cables
        for c in self.elements.itervalues ():
            for s in c.eltype.pins.itervalues ():
                if s.dir == "in" and s not in c.portmap:
                    if s.ptype == "coaxsigs":
                        c.addportmap (self, s, "idlecoax")
                    else:
                        # Twisted pair cables are for the deadstart panel,
                        # bring that out
                        s2 = cmodule.Signal (s.name)
                        s2.ptype = s.ptype
                        c.addportmap (self, s, s2)
                        self.addpin ((s2,), "in", s2.ptype)
                        
    def findchassis (self, cnum):
        """Return the chassis instance object, allocating it if necessary.
        """
        if cnum < 1 or cnum > 16:
            error ("Chassis number %d out of range" % cnum)
            return
        ctname = "chassis%d" % cnum
        ciname = "ch%d" % cnum
        try:
            c = self.elements[ciname]
        except KeyError:
            # Check if the element type exists
            cmodule.elements[ctname]
            c = self.elements[ciname] = cmodule.ElementInstance (ciname, ctname)
            addstd (self, c)
        return c

    def processconns (self, text):
        """Process a connections file ("coax tabs")
        """
        if self.conntext:
            self.conntext += text
        else:
            self.conntext = text
            
    def processconns2 (self):
        """Process the saved connections text
        """
        if not self.conntext:
            return
        for m in _re_cables.finditer (self.conntext):
            end1, c1 = normcable (m.group (1))
            end2, c2 = normcable (m.group (2))
            if end1 == end2:
                # Local connection (internal to the chassis)
                try:
                    ch = self.findchassis (c1)
                    n1 = "c_%s_in" % end1
                    n2 = "c_%s_out" % end1
                    if n1 in ch1.eltype.pins and n2 in ch1.eltype.pins:
                        sig = Coax ("c_%s" % end1)
                        ch.addportmap (toplevel, n1, sig)
                        ch.addportmap (toplevel, n2, sig)
                        self.signals[sig] = sigw
                except KeyError:
                    # Ignore missing chassis
                    pass
            else:
                try:
                    ch1 = self.findchassis (c1)
                    ch2 = self.findchassis (c2)
                    sig = Coax ("c_%s" % end1)
                    n1 = "c_%s_in" % end1
                    n2 = "c_%s_out" % end1
                    n3 = "c_%s_in" % end2
                    n4 = "c_%s_out" % end2
                    if n1 in ch1.eltype.pins and n4 in ch2.eltype.pins:
                        sig = Coax (n1)
                        ch1.addportmap (toplevel, n1, sig)
                        ch2.addportmap (toplevel, n4, sig)
                        self.signals[sig] = sig
                    if n2 in ch1.eltype.pins and n3 in ch2.eltype.pins:
                        sig = Coax (n2)
                        ch1.addportmap (toplevel, n2, sig)
                        ch2.addportmap (toplevel, n3, sig)
                        self.signals[sig] = sig
                except KeyError:
                    # Ignore missing chassis
                    pass
                
# Instantiate the top level object
toplevel = Cyber ()

class Chassis (cmodule.cmod):
    """The definition of a 6000 chassis.
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

    def isinternal (self, sig):
        return isinstance (sig, Wire) and \
               sig.ptype != "analog"
    
    def printassigns (self, sigdict, comp = None):
        return ("", dict ())
    
    def finish (self):
        """Check for internal consistency: missing inputs required
        for used outputs of modules, wires connected at only one
        end.  Also do things that can't be done until this point:
        provide default inputs for input pins that aren't connected
        (ghdl requires that, silly thing), and define chassis "pins"
        for signals that go outside.
        """
        for w in sorted (self.signals):
            w = self.signals[w]
            if isinstance (w, Wire) and \
                   not (w.source and w.destcount == 1) and \
                   w.ptype != "analog":
                if not w.source:
                    print "%s: %s has no source" % (w.destname, w)
                if not w.destcount:
                    print "%s: %s has no destination" % (w.sourcename, w)
                elif w.destcount > 1:
                    print "%s: %s has multiple destinations" % (w.sourcename, w)
        for m in sorted (self.elements):
            m = self.elements[m]
            unused = set ()
            for p in m.eltype.pins.itervalues ():
                if p.dir == "in" and p not in m.portmap:
                    unused.add (p)
                    if p.ptype == "logicsig":
                        m.addportmap (self, p, "'1'")
                    else:
                        # Coax idle state is 0 not 1
                        m.addportmap (self, p, "'0'")
            # Don't do the missing input check; there are lots of cases
            # where inputs are left unconnected because the logic is
            # such that they are don't cares.
            #for pn in m.portmap:
            #    p = m.eltype.pins[pn]
            #    if p.dir == "out":
            #        for src in p.sources ():
            #            if src in unused:
            #                print "%s: missing input %s for output %s" % (m, src, pn)
        for w in self.signals.values ():
            if isinstance (w, Cable):
                for dir in ("in", "out"):
                    cname = "%s_%s" % (w, dir)
                    try:
                        del self.signals[cname]
                    except KeyError:
                        pass
        for w in self.signals.itervalues ():
            if isinstance (w, Cable):
                for dir in ("in", "out"):
                    if dir in w.dirs:
                        self.addpin (("%s_%s" % (w, dir),), dir, w.ptype)
            elif w.ptype == "analog":
                self.addpin ((w,), "out", w.ptype)
            elif not isinstance (w, Wire):
                # Default signals
                self.addpin ((w,), "in", w.ptype)
                
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
        return normcable (name, self.cnum)

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
        addstd (self, inst)
        return inst
    
    def processwlist (self, wl):
        """Process a wirelist file for this chassis
        """
        for sl in _re_wmod.finditer (wl):
            inst = self.addmodule (sl.group (3), sl.group (1))
            c = self.addconnector (sl.group (3), inst)
            if c:
                c.processwlist (sl.group (4))
                if sl.group (2):
                    for formal in inst.eltype.generics:
                        inst.addgenericmap (self, formal, sl.group (2))
                        break
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
                
    def chwire (self, pnum, toslot, topin, dir, wlen = 0):
        """Generate a Wire object for a wire inside a chassis (twisted pair).
        Wires are named w_out_in except if the wire is long enough that we
        simulate its delay, in which case the name is wd_out_in 
        and the wire is hooked up to an instance of the "wire"
        element that actually implements the delay.
        """
        end1 = "%s_%d" % (self.name, pnum)
        end2 = "%s_%s" % (toslot, topin)
        if dir == "out":
            try:
                w = self.chassis.signals["w_%s_%s" % (end1, end2)]
            except KeyError:
                # Wire is not defined yet.  Define it
                w = Wire (end1, end2)
                if real_length and wlen > real_length:
                    wdname = "wd_%s" % w
                    wd = cmodule.ElementInstance (wdname, "wire")
                    self.chassis.elements[wdname] = wd
                    w2 = Wire (end1, end2, "d")
                    self.chassis.signals[w] = w
                    wd.addportmap (self.chassis, "o", w)
                    w = w2
                    wd.addportmap (self.chassis, "i", w)
                    wd.addgenericmap (self.chassis, "length", str (wlen))
                self.chassis.signals[w] = w
        else:
            try:
                w = self.chassis.signals["w_%s_%s" % (end2, end1)]
            except KeyError:
                w = Wire (end2, end1)
                self.chassis.signals[w] = w
        return w
    
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
                if m.group (3) in ("in", "out"):
                    # Whole cable -- for fake modules that model things
                    # at a different VHDL level
                    if m.group (3) != dir:
                        error ("direction mismatch for pin %s" % pname)
                    if ptype == "coaxsigs":
                        ctype = Coax
                    else:
                        ctype = Tpcable
                    cable = self.chassis.findcable (self.chassis.normcable (m.group (2)), ctype)
                    cable.dirs.add (dir)
                    w = "%s_%s" % (cable.name, dir)
                    self.chassis.signals[cable] = cable
                else:
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
                        elif (wnum >= 90 and wnum <= 99) or \
                             (wnum >= 900 and wnum <= 908):
                            w = Coaxwire (self.chassis, m.group (2), m.group (3),
                                          dir, ptype)
                        else:
                            error ("invalid cable wire number %d" % wnum)
                            w = None
                        if w:
                            self.chassis.signals[w.cable.name] = w.cable
                if w:
                    self.addportmap (self.chassis, pname, w)
            else:
                # Regular slot to slot twisted pair wire
                if mpin is None:
                    error ("pin %d undefined or out of range" % pnum)
                    continue
                dir = mpin.dir
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
                w = self.chwire (pnum, toslot, topin, dir, wlen)
                self.addportmap (self.chassis, pname, w)
        if self.mipin (epin) in self.modinst.eltype.pins:
            # next pin should not exist or we have an incomplete list
            error ("not enough pins for connector")

def addstd (parent, inst):
    """ Automatically connect up reset and clock pins, if present
    """
    for stdpin in ("reset", "clk1", "clk2", "clk3", "clk4", "clk40"):
        if stdpin in inst.eltype.pins:
            try:
                s = parent.signals[stdpin]
            except KeyError:
                s = parent.signals[stdpin] = cmodule.Signal (stdpin)
                s.ptype = "logicsig"
            inst.addportmap (parent, stdpin, stdpin)
            
def normcable (name, cnum = 0):
    """Normalize a cable name.  Make cable number 2 digits, and
    prefix with chassis number.  If chassis number was already present,
    verify that it matched the supplied number.  If "cnum" is zero,
    get the number from the name (in which case the number is required)
    and return a tuple of chassis number and normalized name.
    """
    m = _re_cable.match (name)
    if not m:
        error ("Invalid cable name %s" % name)
        return None
    if m.group (1):
        if cnum:
            if int (m.group (1)) != cnum:
                error ("Cable number in %s doesn't match chassis number %d" % (name, self.cnum))
                return
        else:
            cnum = int (m.group (1))
            return "%dw%02d" % (cnum, int (m.group (2))), cnum
    elif cnum == 0:
        error ("Chassis number missing in cable name %s" % name)
        return
    return "%dw%02d" % (cnum, int (m.group (2)))

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

def findchassis (cnum):
    """Return the chassis object, allocating it if necessary.
    """
    if cnum < 1 or cnum > 16:
        error ("Chassis number %d out of range" % cnum)
        return
    try:
        c = chassis_list[cnum]
    except KeyError:
        c = chassis_list[cnum] = Chassis (cnum)
    return c

def readfile (fn):
    """Read an input file, stripping comments and the like.
    """
    f = open (fn, "r")
    text = _re_wstrip.sub ("", f.read ()).lower ()
    f.close ()
    return text

def process_file (fn):
    """Process a file -- either a chassis definition file or
    the cable connections file.
    """
    text = readfile (fn)
    m = _re_wmod.search (text)
    if m:
        # Looks like a chassis definition.  Pick up the chassis number.
        m2 = _re_chslot.match (m.group (3))
        if m2 and m2.group (1):
            cnum = int (m2.group (1))
            c = findchassis (cnum)
            c.processwlist (text)
        else:
            error ("Chassis number not found in %s" % fn)
    else:
        toplevel.processconns (text)
        
class Wire (cmodule.Signal):
    """A wire (twisted pair) between two connector pins in the same chassis.
    The wire is named w_out_in where out and in are slot_pin.
    """
    def __init__ (self, end1, end2, prefix = ""):
        name = "w%s_%s_%s" % (prefix, end1, end2)
        cmodule.Signal.__init__ (self, name)
        self.sourcename = end1
        self.destname = end2
        self.ptype = "logicsig"
        
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
    def __init__ (self, chassis, cable, wirenum, dir, ptype = "logicsig"):
        Cablewire.__init__ (self, chassis, cable, Tpcable, wirenum, dir, ptype)
        
class Cable (cmodule.hitem):
    """A cable (coax or twisted pair)
    """
    def __init__ (self, name):
        cmodule.hitem.__init__ (self, name)
        self.source = None
        self._sources = set ()
        self.destcount = 0
        self.dirs = set ()
        self.strands = { }

    def setsource (self, pin):
        pass
    
class Coax (Cable):
    """A coax cable.  This is modeled as separate input and output cables.
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
            return "%s_%s(%d)" % (self.name, dir, wnum)
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
            return "%s_in(%d)" % (self.name, wnum)
        except:
            return "Invalid wire number %s" % wnum
        

if __name__ == "__main__":
    topname = "cdc6600"
    opts, args = getopt.getopt (sys.argv[1:], "t:")
    if len (args) < 1:
        print "usage: %s wirelist [ wirelist ... ] [ cablelist ]" % sys.argv[0]
        sys.exit (1)
    for opt, val in opts:
        if opt == "-t":
            # top level filename (default: "cdc6600")
            if val == "-":
                topname = None
            else:
                topname = val
    for f in args:
        print "processing", f
        process_file (f)
    for c in chassis_list.itervalues ():
        c.finish ()
    toplevel.finish ()
    if topname:
        toplevel.name = topname
        toplevel.write (False)
    for cname in sorted (chassis_list):
        ch = chassis_list[cname]
        ch.write (True)
