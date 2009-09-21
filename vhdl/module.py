#!/usr/bin/env python

"""Create a module definition VHDL file

-- Copyright (C) 2009 by Paul Koning
"""

import readline

def completions (c = None):
    global _completions
    if c:
        _completions = c
    else:
        _completions = list ()

def complete_list (start, i):
    for c in _completions:
        if c.startswith (start):
            if i:
                i -= 1
            else:
                return c
    return None

readline.parse_and_bind ("tab: complete")
readline.parse_and_bind ("set show-all-if-ambiguous on")
readline.set_completer (complete_list)
completions ()

elements = { }

class eltype (object):
    """Logic element type
    """
    def __init__ (self, name):
        self.name = name
        self.pins = { }
        global elements
        elements[name] = self
        
    def addpin (self, namelist, dir, ptype = "std_logic"):
        if dir not in ("in", "out", "optin"):
            print "Unrecognized pin direction", dir
            return
        for name in namelist:
            self.pins[name] = (dir, ptype)

    def printports (self):
        """Return the ports definition of this element type.
        Pins are grouped as inputs, test points, outputs, and
        alphabetically within the group.
        """
        ports = [ ]
        for p in sorted (self.pins):
            d, t = self.pins[p]
            if d == "out":
                continue
            if d == "optin":
                ports.append ("      %s : in  %s := '1'" % (p, t))
            else:
                ports.append ("      %s : in  %s" % (p, t))
        for p in sorted (self.pins):
            if not p.startswith ("tp"):
                continue
            ports.append ("      %s : %s %s" % (p, d, t))
        for p in sorted (self.pins):
            d, t = self.pins[p]
            if d != "out" or p.startswith ("tp"):
                continue
            ports.append ("      %s : %s %s" % (p, d, t))
        return """    port (
%s);
""" % ";\n".join (ports)

    def printcomp (self):
        """Return the component definition of this element type
        """
        return """  component %s
%s
  end component;
""" % (self.name, self.printports ())


class element (object):
    """Instance of a logic element
    """
    def __init__ (self, name, elname):
        self.name = name
        self.eltype = elements[elname]
        self.portmap = { }
        print "inputs:"
        for p in sorted (self.eltype.pins):
            if self.eltype.pins[p][0] == "in":
                pto = raw_input ("%s: " % p)
                self.portmap[p] = pto
        print "outputs:"
        for p in sorted (self.eltype.pins):
            if self.eltype.pins[p][0] == "out":
                pto = raw_input ("%s: " % p)
                if pto:
                    self.portmap[p] = pto
        prompt_optin = True
        for p in sorted (self.eltype.pins):
            if self.eltype.pins[p][0] == "optin":
                if prompt_optin:
                    opt = raw_input ("any optional inputs? ")
                    if not opt.lower ().startswith ("y"):
                        break
                prompt_optin = False
                pto = raw_input ("%s: " % p)
                if pto:
                    self.portmap[p] = pto

    def printportmap (self):
        """Return the port map.  Entries are grouped inputs
        then outputs, and alphabetically within the group.
        """
        entries = [ ]
        for p in sorted (self.portmap):
            if self.eltype.pins[p][0] == "out":
                continue
            pto = self.portmap[p]
            if pto.startswith ("-"):
                pto = pto[1:]
            entries.append ("    %s => %s" % (p, pto))
        for p in sorted (self.portmap):
            if self.eltype.pins[p][0] != "out":
                continue
            pto = self.portmap[p]
            entries.append ("    %s => %s" % (p, pto))
        return """  %s : %s port map (
%s);
""" % (self.name, self.eltype.name, ",\n".join (entries))

    
class cmod (eltype):
    """A Cyber module
    """
    def __init__ (self, name):
        eltype.__init__ (self, name)
        self.name = name
        self.elements = { }
        self.assigns = { }
        self.elcount = 0

    def addelement (self, eltype):
        if eltype not in elements or eltype == self.name:
            print "No such element"
            return
        self.elcount += 1
        elname = "u%d" % self.elcount
        print "element %s" % elname
        self.elements[elname] = element (elname, eltype)

    def addassign (self, pin, temp):
        """Add an assignment of an output signal from a temp
        """
        self.assigns[pin] = temp
        self.addpin ((pin,), "out", "std_logic")
        
    def addelements (self):
        while True:
            tlist = [ e for e in elements if e != self.name ]
            completions (tlist)
            eltype = raw_input ("element: ")
            completions ()
            if not eltype:
                break
            if "=" in eltype:
                # Assignment
                pin, temp = eltype.split ("=")
                self.addassign (pin, temp)
            else:
                self.addelement (eltype)
        # This updates various lists
        self.printmodule ()
        
    def printassigns (self):
        assigns = [ ]
        for p in sorted (self.assigns):
            assigns.append ("%s <= %s;\n" % (p, self.assigns[p]))
        return "".join (assigns)
    
    def printheader (self):
        return """-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- %s module
--
-------------------------------------------------------------------------------
""" % self.name.upper ()

    def istemp (self, pin):
        """Tells whether the pin is a temp signal or not
        """
        if "slice" in self.name:
            return pin.startswith ("t") and not pin.startswith ("tp")
        else:
            # If it's not a pin and not a testpoint, it's a temp
            return pin == "p" or \
                   (not pin.startswith ("p") and \
                    not pin.startswith ("tp"))

    def printmodule (self):
        """return module definition
        """
        eltypes = { }
        temps = { }
        components = [ ]
        gates = [ ]
        sigs = [ ]
        for e in self.elements.itervalues ():
            eltypes[e.eltype.name] = 1
            for p, pto in e.portmap.iteritems ():
                dir, ptype = e.eltype.pins[p]
                if dir == "optin":
                    dir = "in"
                if self.istemp (pto):
                    temps[pto] = ptype
                else:
                    if pto.startswith ("-"):
                        pto = pto[1:]
                        dir = "optin"
                    self.addpin ((pto, ), dir, ptype)
        for t in sorted (temps):
            v = temps[t]
            sigs.append ("  signal %s : %s;\n" % (t, v))
        for e in sorted (eltypes):
            components.append (elements[e].printcomp ())
        for en in sorted (self.elements):
            e = self.elements[en]
            gates.append (e.printportmap ())
        return """library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity %s is
%s
end %s;
architecture gates of %s is
%s
%s
begin -- gates
%s
%s
end gates;
""" % (self.name,
       self.printports (),
       self.name,
       self.name,
       "\n".join (components),
       "".join (sigs),
       "\n".join (gates),
       self.printassigns ())

    def write (self):
        """Write module definition to a file
        """
        f = file ("%s.vhd" % self.name, "w")
        print >> f, self.printheader ()
        for slice in [ e for e in elements if "slice" in e ]:
            print >> f, elements[slice].printmodule ()
        print >> f, self.printmodule ()
        f.close ()

# The standard cyber module elements
c = eltype ("inv")
c.addpin (("a", ), "in", "std_logic")
c.addpin (("y", ), "out", "std_logic")
c = eltype ("inv2")
c.addpin (("a", ), "in", "std_logic")
c.addpin (("y", "y2" ), "out", "std_logic")
c = eltype ("g2")
c.addpin (("a", "b" ), "in", "std_logic")
c.addpin (("y", "y2" ), "out", "std_logic")
c = eltype ("g3")
c.addpin (("a", "b", "c" ), "in", "std_logic")
c.addpin (("y", "y2" ), "out", "std_logic")
c = eltype ("g4")
c.addpin (("a", "b", "c", "d" ), "in", "std_logic")
c.addpin (("y", "y2" ), "out", "std_logic")
c = eltype ("g5")
c.addpin (("a", "b", "c", "d", "e" ), "in", "std_logic")
c.addpin (("y", "y2" ), "out", "std_logic")
c = eltype ("g6")
c.addpin (("a", "b", "c", "d", "e", "f" ), "in", "std_logic")
c.addpin (("y", "y2" ), "out", "std_logic")
c = eltype ("cxdriver")
c.addpin (("a", ), "in", "std_logic")
c.addpin (("y", ), "out", "coaxsig")
c = eltype ("cxreceiver")
c.addpin (("a", ), "in", "coaxsig")
c.addpin (("y", ), "out", "std_logic")
c = eltype ("rsflop")
c.addpin (("s", "r" ), "in", "std_logic")
c.addpin (("s2", "s3", "s4", "r2", "r3", "r4" ), "optin", "std_logic")
c.addpin (("q", "qb" ), "out", "std_logic")
c = eltype ("latch")
c.addpin (("d", "clk" ), "in", "std_logic")
c.addpin (("r", ), "optin", "std_logic")
c.addpin (("q", "qb" ), "out", "std_logic")
