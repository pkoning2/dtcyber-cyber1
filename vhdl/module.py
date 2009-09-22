#!/usr/bin/env python

"""Create a module definition VHDL file

-- Copyright (C) 2009 by Paul Koning
"""

import readline
import re

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

_re_k = re.compile ("(.+?)(\d*)")
def ancmp (a, b):
    am = _re_k.match (a)
    bm = _re_k.match (b)
    if am.group (2) and bm.group (2):
        ak = ( am.group (1), int (am.group (2)) )
        bk = ( bm.group (1), int (bm.group (2)) )
    else:
        ak = a
        bk = b
    #print a, b, ak, bk
    return cmp (ak, bk)

def sorted (it):
    """Similar to standard sorted() but does alphanumeric keys better
    """
    l = list (it)
    l.sort (ancmp)
    return l

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
            ports.append ("      %s : out %s" % (p, t))
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
    def __init__ (self, name, elname, prompt = True):
        self.name = name
        self.eltype = elements[elname]
        self.portmap = { }
        if prompt:
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
        
    def printassigns (self, assigndict, comp = None):
        """Format the signal assignments from the supplied dictionary.
        If comp is supplied, include only the assignments whose
        right-hand side appears in the right hand sides of the
        portmap of comp.  Returns a pair of formatted string and
        remaining dictionary.  The remaining dictionary contains
        the assignments that were not formatted (not matched in comp).
        """
        assigns = [ ]
        if comp:
            cports = comp.portmap.values ()
        else:
            cports = None
        for p in sorted (assigndict):
            pto = assigndict[p]
            if cports and pto not in cports:
                continue
            assigns.append ("  %s <= %s;\n" % (p, pto))
            del assigndict[p]
        return ("".join (assigns), assigndict)
    
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
        assigndict = dict (self.assigns)
        for e in self.elements.itervalues ():
            eltypes[e.eltype.name] = 1
            #print e.eltype.name
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
            assigns, assigndict = self.printassigns (assigndict, e)
            gates.append (assigns)
        assigns, assigndict = self.printassigns (assigndict)
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
       assigns)

    def write (self):
        """Write module definition to a file
        """
        slices = { }
        f = file ("%s.vhd" % self.name, "w")
        print >> f, self.printheader ()
        for e in self.elements:
            if "slice" in self.elements[e].eltype.name:
                slices[self.elements[e].eltype.name] = 1
        for slice in sorted (slices):
            print >> f, elements[slice].printmodule ()
        print >> f, self.printmodule ()
        f.close ()

_re_arch = re.compile (r"architecture gates of (\w+?) is.+?begin(.+?)end gates;", re.S)
_re_portmap = re.compile (r"(\w+) : (\w+) port map \((.+?)\)", re.S)
_re_pinmap = re.compile (r"(\w+) => (\w+)")
_re_assign = re.compile (r"(\w+) <= (\w+)")

def readmodule (modname):
    """Read a module definition VHD file and return the top
    level module object
    """
    f = open ("%s.vhd" % modname, "r")
    mtext = f.read ()
    f.close ()
    e = None
    for m in _re_arch.finditer (mtext):
        e = cmod (m.group (1))
        #print m.group (1)
        for c in _re_portmap.finditer (m.group (2)):
            u = element (c.group (1), c.group (2), False)
            #print "element", c.group (1), c.group (2)
            e.elements[c.group (1)] = u
            for pin in _re_pinmap.finditer (c.group (3)):
                #print "pin", pin.group (1), pin.group (2)
                u.portmap[pin.group (1)] = pin.group (2)
        for a in _re_assign.finditer (m.group (2)):
            e.addassign (a.group (1), a.group (2))
            #print "assign", a.group (1), a.group (2)
        e.printmodule ()
    return e
 
_re_ent = re.compile (r"entity +(.+?) +is\s+?port +\((.+?)\).+?end +\1", re.S)
_re_pin = re.compile (r"([a-z0-9, ]+): +(in|out) +(std_logic|coaxsig)( +:= +'[01]')?")

def stdelements ():
    f = open ("cyberdefs.vhd", "r")
    std = f.read ()
    f.close ()
    for e in _re_ent.finditer (std):
        c = eltype (e.group (1))
        for pins in _re_pin.finditer (e.group (2)):
            dir = pins.group (2)
            ptype = pins.group (3)
            if pins.group (4):
                if dir != "in":
                    print "Unexpected default in", pins.group ()
                else:
                    dir = "optin"
            pinlist = pins.group (1).replace (" ", "").split (",")
            c.addpin (pinlist, dir, ptype)
        #print c.name, c.printports ()

# Load the standard element definitions
stdelements ()
