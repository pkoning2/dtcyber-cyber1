#!/usr/bin/env python3

"""Process CDC 6600 wiring list

Copyright (C) 2008-2017 Paul Koning

This program reads design data files for CDC 6000 machines, and produces
as output an equivalent VHDL model.  There are three kinds of input files:

1. Chassis wire lists.  These contain the text from the "chassis tabs"
   documents published by CDC.  A wire list is a sequence of module slot
   entries.  Each slot entry begins with a header line which lists the
   module type (a two letter string) and the slot position.  It is followed
   by the pin connections, one per pin in order.  Most modules have
   28 pins, but memory modules have 30 and also have two connectors; the
   pins for the second connector are preceded by a line that lists the
   connector slot position without a module type.

   Slot positions in the header have the chassis number, row letter, and
   column number, for example "5A12".  Slot labels in the pin lists also
   have this format but the chassis number is typically omitted (but if
   present, it must match the number of the current chassis).

   The module type may be followed by an argument in parentheses, which
   gives the "generic map" data for that module.  This is used with PP
   memory, to supply the bank number (PP number) which is used in the
   simulation to read the initial content.

   Pin entries have four fields: the pin number, destination slot label,
   destination pin number, and wire length in inches.  Special cases are:
   a. Ground: given by "good", "grd", "grn", "gnd" or the slot label in
      the destination slot field, "x" in the pin field, and optionally
      "2" in the length field.
   b. Coax connections: the cable label (e.g., "W12") in the destination
      slot field, and strand number (e.g., "902") in the pin field,
      length field unused.
   c. Model signals: "reset" or a clock phase such as "t05" in the 
      destination slot field, other fields unused.
   d. A power designation such as "CB3" or "+6" in the destination slot
      field -- these appear in memory connections and are ignored.

   For ease of modifying stuff via the "corrections" files, wire "length"
   may instead be given as "t" followed by a delay in ns (a multiple of 
   5 ns since that's the simulation granularity).  That syntax isn't found
   in the original wire lists but the correction files may use it when 
   they need to set specific signal delays.

2. Coax wire lists.  These contain the text from the "cable tabs" document
   published by CDC.  Each line lists the two end points of a cable,
   for example "1W2  5W13"

3. Correction files.  These are created as part of the modeling effort,
   for several purposes.  One is to correct typos and other obvious errors
   in the chassis wire lists, since the published documents are not
   entirely accurate.  The other purpose is to make changes for the
   VHDL model to work correctly.  Such changes may include adjustments
   to wire lengths, or replacements of the inverter based clock tree by
   explicit clock phase assignments.  Correction files are a sequence
   of lines of several types:
   a. Module type change: a slot position and new module type.
   b. Module deletion: a slot position and "--".
   c. Wire replacement: slot position, pin number, slot position, pin
      number (the two end points) and wire length
   d. Wire replacement with coax connection: slot position, pin number,
      cable label, strand number.
   e. Wire replacement with model signal: slot position, pin number,
      model signal name.
   f. Wire deletion: slot position, pin number, "--"
   
   In each of these, the firs slot position includes the chassis number,
   for the second slot position it may be omitted (but if supplied must
   match).

In all files, the content is case-insensitive.  Comments are permitted,
with the # prefix.  Fields are separated by a single tab.
"""

import re
import sys
import os
import traceback
import argparse
import cmodule
import time

chassis_list = { }
curslot = None
real_length = 60     # simulate wire delay for wires this long, 0 to disable
real_length = 0

_pat_wline = "(\\d+)(?:[ \t]+(\\w ?\\w*)(?:[ \t]+(\\+?\\w+)(?:[ \t]+(t?\\w+))?)?)?([ \t]*.*(?:\n#.*)*)?"
# This next one ends up with a whole lot of groups partly because of
# the line pattern, so use named groups for sanity.
_re_wmod = re.compile ("^(?P<mod>[a-z]+)(?P<generic>\\(.+?\\))?[ \t]+(?P<slot>\\w+)(?P<hcomment>[ \t]*#.*)?\n+"
                       "(?P<pins>(?:(?:" + _pat_wline + ")\n+){28,30})"
                       "((?P<slot2>\\w+)(?P<hcomment2>[ \t]*#.*)?\n+(?P<pins2>(?:(?:" + _pat_wline + ")\n+){30}))?",
                       re.M | re.I)
_re_wline = re.compile ("^" + _pat_wline, re.M | re.I)
_re_chslot = re.compile (r"(0?[1-9]|1[0-6])?([a-rz])(0[1-9]|[5-9]|[1-3][0-9]?|4[0-2]?)$", re.I)
_re_cable = re.compile (r"(\d+)?w(\d+)(?:#.*)?$", re.I)
_re_cables = re.compile ("^(\\d+w\\d+)[ \t]+(\\d+w\\d+)", re.M | re.I)
_re_header = re.compile ("(#.*\n)+")
_re_tsp = re.compile ("[ \t]+$", re.M)
_re_cstrip = re.compile (r"[ \t]*#.*$")
_re_clkref = re.compile (r"t(\d+)(b?)$", re.I)

header = """-------------------------------------------------------------------------------
--
-- CDC 6600 model -- {{}}
--
-- Converted from wire lists using {} by Paul Koning
--
{{}}
--
-- NOTE: This is a generated file, created {}.
--
-------------------------------------------------------------------------------
""".format (os.path.basename (sys.argv[0]),
            time.strftime ("%Y/%m/%d %H.%M.%S"))

def cstrip (l):
    return _re_cstrip.sub ("", l)

def normslot (chnum, slot, silent = False):
    """Normalize a slot name.  Strip off leading chassis number,
    if present (must be correct if present).  Make slot number 2 digits.
    """
    m = _re_chslot.match (slot)
    if not m:
        if not silent:
            error ("Invalid module slot ID {}", slot)
        return None
    if m.group (1):
        if chnum and int (m.group (1)) != chnum:
            error ("Module slot ID {} chassis number is not {}", slot, chnum)
            return None
    return "%s%02d" % (m.group (2), int (m.group (3)))

class Cyber (cmodule.cmod):
    """An instance of a Cyber (top level object)
    """
    chassisname = "Top level"
    
    def __init__ (self):
        cmodule.cmod.__init__ (self, "cdc6600")

    def isinternal (self, sig):
        return isinstance (sig, Cable)
    
    def printheader (self):
        return header.format (self.chassisname, self.sourcehdr)

    def printassigns (self, sigdict, comp = None):
        return ("", dict ())
    
    def finish (self):
        """Check for internal consistency
        """
        # Hook up the standard signals
        for w in self.signals.values ():
            if not isinstance (w, Cable):
                # Default signals
                self.addpin ((w,), "in", w.ptype)
        # Handle any loose input cables
        for c in self.elements.values ():
            for s in c.eltype.pins.values ():
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
            error ("Chassis number {} out of range", cnum)
            return
        ctname = "chassis%d" % cnum
        ciname = "ch%d" % cnum
        try:
            c = self.elements[ciname]
        except KeyError:
            # Check if the element type exists
            cmodule.elements[ctname]
            c = self.elements[ciname] = cmodule.ElementInstance (ciname,
                                                                 ctname, self)
        return c

    def processconns (self, cables):
        """Process the saved connections text
        """
        if not cables:
            return
        for l in cables.splitlines ():
            l = cstrip (l)
            if not l:
                continue
            m = _re_cables.match (l)
            if not m:
                print ("unrecognized cable entry", l)
                continue
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

def findmodule (modname):
    try:
        mtype = cmodule.elements[modname]
    except KeyError:
        #print "reading", modname
        mtype = cmodule.readmodule (modname)
    return mtype

class Chassis (cmodule.cmod):
    """The definition of a 6000 chassis.
    """
    def __init__ (self, num):
        cmodule.cmod.__init__ (self, "chassis%d" % num)
        self.cnum = num
        self.chassisname = "chassis {}".format (num)
        self.connectors = { }
        self.delays = 0
        
    def printheader (self):
        return header.format (self.chassisname, self.sourcehdr)

    def isinternal (self, sig):
        return (isinstance (sig, Wire) and \
                sig.ptype != "analog") or sig.name in self.aliases
    
    def printassigns (self, sigdict, comp = None):
        assigns = [ ]
        for to, src in self.aliases.items ():
            slot, pin = src
            if comp is not None and comp.name == slot.name:
                c = self.connectors[slot.name]
                w = c.chwire (slot, pin)
                assigns.append ("  %s <= %s;" % (to, w.name))
        if assigns:
            assigns.append ("")
        return ("\n".join (assigns), sigdict)
    
    def finish (self):
        """Check for internal consistency: missing inputs required
        for used outputs of modules, wires connected at only one
        end.  Also do things that can't be done until this point:
        provide default inputs for input pins that aren't connected
        (ghdl requires that, silly thing), and define chassis "pins"
        for signals that go outside.
        """
        # Make sure we define wires that are used only to attach
        # multiple coax signals via aliasing.
        for to, src in list (self.aliases.items ()):
            slot, pin = src
            c = self.connectors[slot.name]
            minst = c.modinst
            p = c.modpin (pin)
            curmap = minst.portmap[p]
            w = c.chwire (slot, pin)
            if "(" in curmap.name:
                # Current connection is a coax wire, but we need
                # multiple connections and coax is a port so you can't
                # use it as the source for an assignment.  Replace it
                # by a regular wire (signal) instead, and we'll alias
                # the current coax as well as whatever else needs this
                # pin set to that wire.
                #print ("replacing", curmap, "by", w)
                self.aliases[curmap] = src
                minst.portmap[p] = w
        for m in sorted (self.elements):
            m = self.elements[m]
            unused = set ()
            for p in m.eltype.pins.values ():
                if p.dir == "in" and p not in m.portmap:
                    unused.add (p)
                    if p.ptype == "logicsig":
                        m.addportmap (self, p, "'1'")
                    else:
                        # Coax idle state is 0 not 1
                        m.addportmap (self, p, "'0'")
        for w in list(self.signals.values ()):
            if isinstance (w, Cable):
                for dir in ("in", "out"):
                    cname = "%s_%s" % (w, dir)
                    try:
                        del self.signals[cname]
                    except KeyError:
                        pass
        for w in self.signals.values ():
            if isinstance (w, Cable):
                for dir in ("in", "out"):
                    if dir in w.dirs:
                        self.addpin (("%s_%s" % (w, dir),), dir, w.ptype)
            elif w.ptype == "analog":
                self.addpin ((w,), "out", w.ptype)
            elif w.name.startswith ("sysclk"):
                if "sysclk" not in self.pins:
                    self.addpin (("sysclk",), "in", "clocks")
            elif not isinstance (w, Wire):
                # Default signals
                self.addpin ((w,), "in", w.ptype)
                
    def normslot (self, slot):
        """Normalize a slot name.  Strip off leading chassis number,
        if present (must be correct if present).  Make slot number 2 digits.
        """
        return normslot (self.cnum, slot)
        
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
                error ("cable type mismatch {}", name)
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
                error ("Slot {} already defined", slot)
            else:
                c = self.connectors[slot] = Connector (slot, modinst,
                                                       self, offset)
                return c

    def addmodule (self, slot, modname):
        """Add a module and its connector.  Returns the module instance
        """
        slot = self.normslot (slot)
        if not slot:
            return
        mtype = findmodule (modname)
        inst = self.elements[slot] = cmodule.ElementInstance (slot, modname,
                                                              self)
        return inst
    
def wire_delay (wlen):
    """Return the wire delay for a wire of the specified length
    in inches.  Delay is returned in units of 5 ns, since that is
    the timing granularity we use in this model, rounded down to an
    integer.  So short wires (under 47 inches) are handled as
    having zero delay.
    """
    # Nominal twisted pair delay is 1.3 ns per foot.
    if not wlen:
        return 0
    try:
        if wlen.lower ().startswith ("t"):
            # Delay given in ns
            wlen = int (wlen[1:])
            wlen, x = divmod (wlen, 5)
            if not 0 <= wlen < 6 or x:
                raise ValueError
        else:
            wlen = int (wlen)
            if wlen < 1 or wlen > 180:
                raise ValueError
            if wlen < real_length:
                wlen = 0
            # Convert length to delay time, truncated to 5 ns multiple.
            wlen = int (((wlen / 12.) * 1.3) / 5.)
    except (TypeError, ValueError):
        error ("invalid wire length {} pin {} in {}",
               wlen, pnum, curslot)
    return wlen

class Connector (object):
    """A connector for a module, in a slot
    """
    def __init__ (self, slot, inst, chassis, offset = 0):
        self.name = chassis.normslot (slot)
        self.modinst = inst
        self.offset = offset
        self.chassis = chassis

    def chwire (self, scon, sport, wdelay = 0):
        """Generate a Wire object for a wire inside a chassis (twisted pair).
        Wires are named by their source port, which is given by scon/sport.
        In many cases, several pins correspond to a single port, when the
        same output signal appears on multiple pins; in such cases, the
        port has a name like p3_p7 and that is the name use to form the
        wire signal name.
        If a wire is long enough to need its delay simulated, we also
        generate wire names with wd<n> prefixes, where n is the delay in 
        5 ns units.  Each 5 ns delay uses a "delay" element.  The wiring,
        from source to destination, looks like (for a 2 unit delay):
           w_a01_p3_p5, wire, wd1_a01_p3_p5, wire, wd2_a01_p3_p5
        """
        sname = scon.name
        if wdelay:
            wname = "wd{}_{}_{}".format (wdelay, sname, sport)
        else:
            wname = "w_{}_{}".format (sname, sport)
        try:
            return self.chassis.signals[wname]
        except KeyError:
            try:
                return self.chassis.aliases[wname]
            except KeyError:
                pass
        # Wire is not defined yet.  Define it.
        w = Wire (wname)
        if wdelay:
            # Delayed version of some output.  We handle this by
            # creating a unit delay element, fed by the desired signal
            # delayed one less unit (recursively).
            self.chassis.delays += 1
            wdname = "ud{}".format (self.chassis.delays)
            wd = cmodule.ElementInstance (wdname, "unit_delay", self)
            self.chassis.elements[wdname] = wd
            w2 = self.chwire (scon, sport, wdelay - 1)
            wd.addportmap (self.chassis, "i", w2)
            wd.addportmap (self.chassis, "o", w)
            #print ("added", wdname, "for", w2, "to", wname)
        else:
            scon.addportmap (self.chassis, sport, w)
        self.chassis.signals[w] = w
        return w
    
    def mipin (self, pnum):
        return "p%d" % (pnum + self.offset)

    def modpin (self, pnum):
        if isinstance (pnum, int):
            pnum = self.mipin (pnum)
        try:
            return self.modinst.eltype.pinnames[pnum]
        except KeyError:
            try:
                return self.modinst.eltype.pins[pnum]
            except KeyError:
                return None
        
    def addportmap (self, parent, formal, actual):
        self.modinst.addportmap (parent, formal, actual)

    def processwlist (self, pinlist, wl):
        """Process the pins data from the wirelist for this connector.
        The argument is a list of wire entries, in pin order.  Each
        entry is None (for no connection) or a triple corresponding
        to the three columns of the wire list.  They are: for twisted
        pair wires, the destination slot, pin, and wire length; and
        for coax, the cable name and cable strand number, with the third
        field unused.  Grounds (logic 1) are shown as destination
        of "grd".  Destination may also be a standard signal defined
        by the VHDL model, such as "reset" or "t10".
        """
        global curslot
        curslot = self.name
        pnum = 0
        for m in pinlist:
            pnum += 1
            if m is None:
                # Unused pin, carry on
                continue
            dslot, dpin, wlen = m
            if dslot.startswith ("cb") or dslot.startswith ("+"):
                # Memory power connections, skip those since we don't
                # model them
                continue
            mpin = self.modpin (pnum)
            pname = "p%d" % pnum
            if dslot == "grd":
                # ground
                if mpin is not None:
                    # ignore ground pin connections that are memory
                    # power/ground rather than module signals
                    self.addportmap (self.chassis, pname, "'1'")
            elif "w" in dslot:
                # cable
                if mpin is None:
                    error ("pin {} undefined or out of range in {}",
                           pnum, curslot)
                    continue
                dir = mpin.dir
                ptype = mpin.ptype
                if dpin in ("in", "out"):
                    # Whole cable -- for fake modules that model things
                    # at a different VHDL level
                    if dpin != dir:
                        error ("direction mismatch for pin {} in {}",
                               pname, curslot)
                    if ptype == "coaxsigs":
                        ctype = Coax
                    else:
                        ctype = Tpcable
                    cable = self.chassis.findcable (self.chassis.normcable (dslot), ctype)
                    cable.dirs.add (dir)
                    w = "%s_%s" % (cable.name, dir)
                    self.chassis.signals[cable] = cable
                else:
                    try:
                        wnum = int (dpin)
                    except (TypeError, ValueError):
                        error ("invalid pin {}: {} in {}", pnum, dpin, curslot)
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
                        w = "c_%s_%d" % (self.chassis.normcable (dslot), wnum)
                    else:
                        if wnum >= 101 and wnum <= 224:
                            w = Tpwire (self.chassis, dslot, dpin,
                                        dir, ptype)
                        elif (wnum >= 90 and wnum <= 99) or \
                             (wnum >= 900 and wnum <= 908):
                            w = Coaxwire (self.chassis, dslot, dpin,
                                          dir, ptype)
                        else:
                            error ("invalid cable wire number {} in {} pin {}",
                                   wnum, curslot, pnum)
                            w = None
                        if w:
                            self.chassis.signals[w.cable.name] = w.cable
                if w:
                    self.addportmap (self.chassis, pname, w)
            elif dslot:
                # Regular slot to slot twisted pair wire
                if mpin is None:
                    error ("pin {} undefined or out of range in {}",
                           pnum, curslot)
                    continue
                dir = mpin.dir
                ptype = mpin.ptype
                pname = mpin.name
                if ptype == "misc":
                    # We ignore misc signals since they are only there
                    # to document things like jumpers and other non-logic
                    # stuff that doesn't map to VHDL
                    continue
                if dslot == "reset" or dslot.startswith ("t") or \
                  dslot == "'0'" or dslot == "'1'":
                    if dir != "in":
                        error ("wrong pin direction {} for model signal {}",
                               dir, dslot)
                    if dslot.startswith ("t"):
                        # Clock phase reference.  These are of the
                        # form t<n> or t<n>b for positive or inverted
                        # clock phase respectively.  n is the offset
                        # in ns from the reference point, a multiple
                        # of 5.  We convert this to one of the
                        # elements of the sysclk bus (0 to 19 for
                        # positive, 20 to 39 for inverted clock).
                        m = _re_clkref.match (dslot)
                        if not m:
                            error ("invalid clock reference {}", dslot)
                        phase, inv = m.groups ()
                        try:
                            phase = int (phase)
                            if not 0 <= phase < 100 or phase % 5:
                                raise ValueError
                        except ValueError:
                            error ("Invalid clock offset {}", phase)
                        phase //= 5
                        if inv:
                            phase += 20
                        dslot = "sysclk({})".format (phase)
                    w = dslot
                else:
                    toslot = self.chassis.normslot (dslot)
                    topin = pn (dpin)
                    try:
                        tcon = self.chassis.connectors[toslot]
                    except KeyError:
                        error ("Connection to unused slot {}", toslot)
                        continue
                    tmpin = tcon.modpin (topin)
                    reverse = wl[toslot]
                    if reverse.slot == toslot:
                        reverse = reverse.pins
                    else:
                        reverse = reverse.pins2
                    reverse = reverse[topin - 1]
                    if reverse:
                        rslot, rpin, rlen = reverse
                        rslot = self.chassis.normslot (rslot)
                        if rpin.startswith ("p"):
                            rpin = rpin[1:]
                        try:
                            rpin = int (rpin)
                        except Exception:
                            pass
                        if rslot != self.name or rpin != pnum:
                            error ("Other end of wire does not match: {} {} vs. {} {}",
                                   rslot, rpin, self.name, pnum)
                    else:
                        error ("Other end of wire missing: {} {} {} {}",
                               self.name, pnum, toslot, topin)
                    if dir != "in":
                        # We make the connection when we see the input
                        # end of the wire, but at this point we'll check
                        # that the other end is indeed an input.
                        if dir == "out":
                            if tmpin.dir != "in":
                                error ("wire is not in to out, {} {} {} {}",
                                       self.name, pnum, toslot, topin)
                        else:
                            error ("Unexpected direction {} for pin {}",
                                   dir, pnum)
                        continue
                    # Check the wire lengths at both ends, and issue a
                    # warning if they differ in a way that changes the
                    # modeled delay.
                    wd = wire_delay (wlen)
                    rd = wire_delay (rlen)
                    if wd != rd:
                        print ("Inconsistent wire lengths: "
                               "{} {} {} vs. {} {} {}".format
                               (self.name, pnum, wlen, dslot, topin, rlen))
                        wd = max (wd, rd)
                    w = self.chwire (tcon, tmpin.name, wd)
                self.addportmap (self.chassis, pname, w)
        if self.mipin (pnum + 1) in self.modinst.eltype.pins:
            # next pin should not exist or we have an incomplete list
            error ("not enough pins for connector")

def normcable (name, cnum = 0):
    """Normalize a cable name.  Make cable number 2 digits, and
    prefix with chassis number.  If chassis number was already present,
    verify that it matched the supplied number.  If "cnum" is zero,
    get the number from the name (in which case the number is required)
    and return a tuple of chassis number and normalized name.
    """
    m = _re_cable.match (name)
    if not m:
        error ("Invalid cable name {}", name)
        return None
    if m.group (1):
        if cnum:
            if int (m.group (1)) != cnum:
                error ("Cable number in {} doesn't match chassis number {}",
                       name, self.cnum)
                return
        else:
            cnum = int (m.group (1))
            return "%dw%02d" % (cnum, int (m.group (2))), cnum
    elif cnum == 0:
        error ("Chassis number missing in cable name {}", name)
        return
    return "%dw%02d" % (cnum, int (m.group (2)))

def error (text, *args):
    if args:
        text = text.format (*args)
    if curslot:
        text = "{}: ".format (curslot) + text
    print(text)

_re_pnum = re.compile (r"p(\d+)")
def pn (pname):
    m = _re_pnum.match (pname)
    if m:
        return int (m.group (1))
    else:
        try:
            return int (pname)
        except (TypeError, ValueError):
            error ("invalid pin {}", pname)

def findchassis (cnum):
    """Return the chassis object, allocating it if necessary.
    """
    if cnum < 1 or cnum > 16:
        error ("Chassis number {} out of range", cnum)
        return
    try:
        c = chassis_list[cnum]
    except KeyError:
        c = chassis_list[cnum] = Chassis (cnum)
    return c

def readfile (fn, lc = True):
    """Read an input file, return as lower case text
    """
    with open (fn, "r", encoding = "utf_8_sig") as f:
        text = f.read ()
    text = text.replace ("\014", "")
    text = _re_tsp.sub ("", text)
    hm = _re_header.match (text)
    if hm:
        text = text[hm.end ():]
        hm = hm.group (0).splitlines ()
        if lc and hm[-1].startswith ("# Starting page"):
            del hm[-1]
        hm = '\n'.join (hm)
    else:
        hm = ""
    if lc:
        text = text.lower ()
    return hm, text

def parse_connector (cnum, curslot, text):
    epin = 1
    pins = list ()
    for m in _re_wline.finditer (text):
        pnum = pn (m.group (1))
        if pnum != epin:
            error ("pins out of order in {}, expected {}, got {}",
                   curslot, epin, pnum)
        epin += 1
        if m.group (2) is None:
            # Unused pin, carry on
            pins.append (None)
            continue
        pname = "p%d" % pnum
        if m.group (3) == "x":
            # ground
            if m.group (2) == "good" or \
               m.group (2) == "grd" or \
               m.group (2) == "gnd" or \
               normslot (cnum, m.group (2)) == curslot:
                pins.append (("grd", "x", "2"))
            else:
                error ("Strange ground-like entry {} in {} pin {}",
                       m.groups (), curslot, pnum)
        else:
            pins.append ((m.group (2), m.group (3), m.group (4)))
    return pins

class Slottext:
    def __init__ (self, cnum, slot, gd):
        self.slot = slot
        self.modtype = gd["mod"]
        self.pins = parse_connector (cnum, slot, gd["pins"])
        if gd["slot2"]:
            self.slot2 = normslot (cnum, gd["slot2"])
            self.pins2 = parse_connector (cnum, self.slot2, gd["pins2"])
        else:
            self.slot2 = self.pins2 = None
        self.generic = gd["generic"]
    
def parse_chassis (cnum, text):
    """Parse the text from a chassis wirelist into its elements.  We do
    this separate from generating the VHDL objects so we can apply
    corrections to the wire list after parsing it but before generating
    the model.
    """
    modules = dict ()
    for sl in _re_wmod.finditer (text):
        gd = sl.groupdict ()
        slot = normslot (cnum, gd["slot"])
        modules[slot] = m = Slottext (cnum, slot, gd)
        if m.slot2:
            modules[m.slot2] = modules[slot]
    return modules

class Wire (cmodule.Signal):
    """A wire (twisted pair) between two connector pins in the same chassis.
    """
    def __init__ (self, name, ptype = "logicsig"):
        cmodule.Signal.__init__ (self, name)
        self.ptype = ptype
        
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
            error ("wire {} already defined for {}", wirenum, cable.name)
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

def generate_vhdl (chassis, cables, cheader, p):
    # Process the (corrected) text for each chassis
    modnames = { "cyberdefs" }
    for cnum, cdata in sorted (chassis.items ()):
        header, cdata = cdata
        if verbose:
            print ("processing chassis", cnum)
        ch = findchassis (cnum)
        ch.setheader (header)
        # First add all the connectors so we have all the pin
        # information available when we're stringing the wires.
        for slot, m in sorted (cdata.items ()):
            if slot != m.slot:
                # To allow for corrections that refer to the second
                # connector of two-connector modules (memory), both
                # connectors are in the directionary, referring to the
                # same Module object.  The second connector can be
                # recognized by the fact that its name differs from the
                # primary connector name attribute of the module.
                continue
            inst = ch.addmodule (slot, m.modtype)
            modnames.add (m.modtype)
            if m.generic:
                for formal in inst.eltype.generics:
                    inst.addgenericmap (ch, formal, m.generic)
                    break
            ch.addconnector (slot, inst)
            if m.slot2:
                ch.addconnector (m.slot2, inst, 100)
        # Now run the wires
        for slot, m in sorted (cdata.items ()):
            if slot != m.slot:
                # To allow for corrections that refer to the second
                # connector of two-connector modules (memory), both
                # connectors are in the directionary, referring to the
                # same Module object.  The second connector can be
                # recognized by the fact that its name differs from the
                # primary connector name attribute of the module.
                continue
            c = ch.connectors[slot]
            c.processwlist (m.pins, cdata)
            if m.slot2:
                c = ch.connectors[m.slot2]
                c.processwlist (m.pins2, cdata)
        
    for c in chassis_list.values ():
        if verbose:
            print ("finalizing", c.name)
        c.finish ()
    # Process the coax cables
    if verbose:
        print ("processing coax cables")
    toplevel.processconns (cables)
    if verbose:
        print ("finalizing top level model")
    toplevel.finish ()
    if p.top != "-":
        toplevel.name = p.top
        toplevel.setheader (cheader)
        if verbose:
            print ("writing top level model to", p.top)
        toplevel.write (False)
    for cname, ch in chassis_list.items ():
        if verbose:
            print ("writing", ch.name)
        ch.write (False)
    if p.depend_file:
        if verbose:
            print ("writing dependency file", p.depend_file)
        dest = [ p.depend_file ]
        if p.top != "-":
            dest.append ("{}.vhd".format (p.top))
        for cname in sorted (chassis_list):
            dest.append ("chassis{}.vhd".format (cname))
        dest = ' '.join (dest)
        src = p.wlist + p.patch + [ sys.argv[0] ]
        for m in sorted (modnames):
            src.append ("{}.vhd".format (m))
        src = ' '.join (src)
        with open (p.depend_file, "wt") as f:
            print ("{} : {}".format (dest, src), file = f)

def delwire (ch, cnum, p):
    # p is a pin entry (destination slot/pin/length)
    if verbose > 1:
        print ("delwire", cnum, p)
    dslot, dpin, *rest = p
    dslot = normslot (cnum, dslot, True)
    try:
        dmod = ch[dslot]
    except KeyError:
        return
    if dmod.slot == dslot:
        pins = dmod.pins
    else:
        pins = dmod.pins2
    dpin = int (dpin)
    pins[dpin - 1] = None
    
def process_corr (header, text, chassis):
    for l in text.splitlines ():
        l = cstrip (l)
        if not l:
            continue
        slot, *rest = l.split ('\t')
        m = _re_chslot.match (slot)
        if not m:
            error ("missing chassis number in {}", l)
            continue
        cnum = int (m.group (1))
        slot = normslot (cnum, slot)
        try:
            hdr, ch = chassis[cnum]
        except KeyError:
            error ("Unknown chassis number in {}", l)
            continue
        try:
            mod = ch[slot]
        except KeyError:
            error ("Unknown slot position in {}", l)
            continue
        # Figure out what to do, depending on what follows the slot position
        if len (rest) == 1:
            # Module change
            modtype = rest[0]
            if modtype == "--":
                if verbose > 1:
                    print ("removing module at", slot)
                # Delete wires going to input pins, but not outputs,
                # so that we'll catch cases of deleting a module while
                # others still depend on the data it generates.
                mtype = findmodule (mod.modtype)
                for pn, pinfo in mtype.pins.items ():
                    if pinfo.dir == "in":
                        assert pn[0] == 'p'
                        pnum = int (pn[1:])
                        if pnum < 100:
                            p = mod.pins[pnum - 1]
                            if p:
                                delwire (ch, cnum, p)
                        else:
                            p = mod.pins2[pnum - 101]
                            if p:
                                delwire (ch, cnum, p)
                del ch[slot]
            else:
                error ("todo: module change not yet implemented")
        else:
            # Wire change
            if slot == mod.slot:
                pins = mod.pins
            else:
                pins = mod.pins2
            pnum, dslot, *rest = rest
            try:
                pnum = int (pnum)
            except ValueError:
                error ("invalid pin number in {}", l)
            if pnum < 1 or pnum > len (pins):
                error ("pin number out of range in {}", l)
            p = pins[pnum - 1]
            if p:
                delwire (ch, cnum, p)
                pins[pnum - 1] = None
            if dslot == "--":
                # Delete existing wire, nothing left to do
                pass
            elif _re_chslot.match (dslot):
                # New or replacement wire
                dslot = normslot (cnum, dslot)
                dpnum, wlen = rest
                try:
                    dmod = ch[dslot]
                except KeyError:
                    error ("unknown destination slot in {}", l)
                if dslot == dmod.slot:
                    dpins = dmod.pins
                else:
                    dpins = dmod.pins2
                try:
                    dpnum = int (dpnum)
                except ValueError:
                    error ("invalid destination pin number in {}", l)
                if dpnum < 1 or dpnum > len (dpins):
                    error ("destination pin number out of range in {}", l)
                pins[pnum - 1] = (dslot, str (dpnum), wlen)
                dpins[dpnum - 1] = (slot, str (pnum), wlen)
                if verbose > 1:
                    print ("new wire", pins[pnum -1], dpins[dpnum-1])
            else:
                # coax or model signal
                rep = [ dslot ] + rest
                while len (rep) < 3:
                    rep.append ("")
                pins[pnum - 1] = rep
                #print (rep)
                
ap = argparse.ArgumentParser ()
ap.add_argument ("wlist", nargs = "+", metavar = "FN",
                 help = "Wire list, cable list, or corrections file")
ap.add_argument ("-t", "--top", default = "cdc6600", metavar = "FN",
                 help = "Top level file, '-' to disable")
ap.add_argument ("-v", "--verbose", default = 0, action = "count",
                 help = "Display additional output")
ap.add_argument ("-r", "--report", metavar = "FN",
                 help = "Report file name")
ap.add_argument ("-p", "--patch", action = "append",
                 metavar = "FN", default = list (),
                 help = "Specify correction (patch) file name; may be repeated")
ap.add_argument ("-d", "--depend-file", metavar = "FN",
                 help = "Name of dependency file to generate")

def main ():
    global verbose
    p = ap.parse_args ()
    verbose = p.verbose
    chassis = dict ()
    cables = ""
    cheader = None
    for fn in p.wlist:
        print ("reading", fn)
        header, text = readfile (fn)
        m = _re_wmod.search (text)
        if m:
            # Looks like a chassis definition.  Pick up the chassis number.
            m2 = _re_chslot.match (m.groupdict ()["slot"])
            if m2 and m2.group (1):
                cnum = int (m2.group (1))
                if cnum in chassis:
                    error ("Duplicate chassis definition for {}", cnum)
                else:
                    chassis[cnum] = (header, parse_chassis (cnum, text))
            else:
                error ("Chassis number not found in {}", fn)
        else:
            # Not chassis, assume it's cables
            cables += text
            if not cheader:
                cheader = header
    # We've read all the wire and cable lists.  Now look for any
    # corrections files, and apply whatever changes those call for.
    for fn in p.patch:
        print ("reading correction file", fn)
        header, text = readfile (fn)
        process_corr (header, text, chassis)
    # We now have corrected text, process it into VHDL module
    # definitions, and write the results.
    generate_vhdl (chassis, cables, cheader, p)

if __name__ == "__main__":
    main ()
    
