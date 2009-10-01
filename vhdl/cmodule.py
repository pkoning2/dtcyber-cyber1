#!/usr/bin/env python

"""Create a module definition VHDL file

Copyright (C) 2009 by Paul Koning
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

_re_k = re.compile ("(.+?)(\d+)(.*)$")
def ancmp (a, b):
    a = str (a)
    b = str (b)
    am = _re_k.match (a)
    bm = _re_k.match (b)
    if am and bm and am.group (2) and bm.group (2):
        ak = ( am.group (1), int (am.group (2)), am.group (3) )
        bk = ( bm.group (1), int (bm.group (2)), bm.group (3) )
    else:
        ak = a
        bk = b
    return cmp (ak, bk)

def sorted (it):
    """Similar to standard sorted() but does alphanumeric keys better
    """
    l = list (it)
    l.sort (ancmp)
    return l

class hitem (object):
    """A hashable container, hashed by name
    """
    def __init__ (self, name):
        self.name = str (name)

    def __str__ (self):
        return self.name

    def __eq__ (self, other):
        return self.name == str (other)

    def __hash__ (self):
        return hash (self.name)
        
class Pin (hitem):
    """Pin of a logic element type
    """
    def __init__ (self, name, dir, ptype, opt = False):
        hitem.__init__ (self, name)
        self.dir = dir
        self.ptype = ptype
        self.opt = opt
        self._sources = set ()

    def sources (self):
        return self._sources
    
    def printdecl (self):
        if self.opt:
            return "%s : %-3s %s := '1'" % (self.name, self.dir, self.ptype)
        else:
            return "%s : %-3s %s" % (self.name, self.dir, self.ptype)

    def testpoint (self):
        return self.name.startswith ("tp")
    
class ElementType (object):
    """Logic element type
    """
    def __init__ (self, name):
        global elements
        elements[name] = self
        self.name = name
        self.pins = { }
        
    def addpin (self, namelist, dir, ptype = "std_logic", opt = False):
        if dir not in ("in", "out"):
            print "Unrecognized pin direction", dir
            return
        for name in namelist:
            if name in self.pins:
                print "Pin", name, "already defined"
            else:
                self.pins[name] = Pin (name, dir, ptype)

    def inputs (self):
        """Return a list of input pins, sorted by name
        """
        return sorted ([p for p in self.pins.itervalues ()
                        if p.dir != "out"])

    def reqinputs (self):
        """Return a list of required input pins, sorted by name
        """
        return [ p for p in self.inputs () if not p.opt ]

    def optinputs (self):
        """Return a list of required input pins, sorted by name
        """
        return [ p for p in self.inputs () if p.opt ]

    def outputs (self):
        """Return a list of output pins, sorted by name
        """
        return sorted ([p for p in self.pins.itervalues ()
                        if p.dir == "out"])
        
    def printports (self):
        """Return the ports definition of this element type.
        Pins are grouped as inputs, test points, outputs, and
        alphabetically within the group.
        """
        ports = [ ]
        for p in self.inputs ():
            ports.append ("      %s" % p.printdecl ())
        o = self.outputs ()
        for p in o:
            if p.testpoint ():
                ports.append ("      %s" % p.printdecl ())
        for p in o:
            if not p.testpoint ():
                ports.append ("      %s" % p.printdecl ())
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

    def finish (self):
        """Do any actions we can't do until all the pieces have been defined
        """
        sources = set ()
        for p in self.reqinputs ():
            sources.add (p)
        sources = frozenset (sources)
        for p in self.outputs ():
            p._sources = sources


class PinInstance (object):
    """An instance of a pin belonging to an ElementInstance
    """
    def __init__ (self, parent, pin):
        self.pin = pin
        self.parent = parent

    def __getattr__ (self, name):
        return self.pin.__dict__[name]

    def __str__ (self):
        return "%s.%s" % (self.parent, self.pin)

    def sources (self):
        return set ()
    
class Signal (hitem):
    """A signal (pin or temporary) in an element instance
    """
    def __init__ (self, name):
        hitem.__init__ (self, name)
        self.source = None
        self._sources = set ()
        self.destcount = 0
        self.opt = False
        self.ptype = None
    
    def setsource (self, source):
        if self.source:
            print "Duplicate assignment to signal", self.name
            return
        if isinstance (source, PinInstance):
            if source.dir != "out":
                print "Signal source must be output pin", self.name, source.name
                return
        self.source = source
        self.ptype = source.ptype
        self.addsource (source)

    def addsource (self, source):
        self._sources.add (source)
        self.sources ()
        
    def delsource (self, source):
        self._sources.discard (source)
        
    def sources (self):
        for s in list (self._sources):
            self._sources |= s.sources ()
        if self.source:
            if not self.ptype:
                self.ptype = self.source.ptype
        return self._sources

    def printassign (self):
        if self.source:
            return "%s <= %s" % (self.name, self.source.name)
        else:
            print "No source for", self.name
            return "-- no source for %s" % self.name

class ConstSignal (Signal):
    """A signal that is a constant (not a pin)
    """
    def __init__ (self, name):
        Signal.__init__ (self, name)

    def setsource (self, source):
        pass

    def addsource (self, source):
        pass

    def delsource (self, source):
        pass

    def sources (self):
        return set ()

    def printassign (self):
        return ""

sigone = ConstSignal ("'1'")

class ElementInstance (object):
    """Instance of a logic element
    """
    def __init__ (self, name, elname):
        self.name = name
        self.eltype = elements[elname]
        self.portmap = { }
        self.iportmap = { }

    def __str__ (self):
        return self.name
    
    def addportmap (self, parent, formal, actual):
        pin = self.eltype.pins[formal]
        dir = pin.dir
        ptype = pin.ptype
        opt = False
        if actual.startswith ("-"):
            if dir != "in":
                print "Optional input flag but %s is not an input" % formal
                return
            actual = actual[1:]
            opt = True
        if dir == "out":
            # Assigning to output, must be a new signal, or one that
            # doesn't have an output yet.  Special case: '1' is a valid
            # actual
            if actual == "'1'":
                sig = onesignal
            elif actual in parent.signals:
                sig = parent.signals[actual]
                if sig.source:
                    print "duplicate output assignment to", actual
                    return
            else:
                sig = parent.signals[actual] = Signal (actual)
        else:
            # Input, can be used multiple times
            try:
                sig = parent.signals[actual]
                if sig.ptype != ptype:
                    print "signal type mismatch", sig.ptype, ptype
            except KeyError:
                # signal not yet defined, define it
                sig = parent.signals[actual] = Signal (actual)
                sig.ptype = ptype
        if dir == "out":
            sig.setsource (PinInstance (self, pin))
        else:
            sig.destcount += 1
        # Save both directions of the mapping
        self.portmap[formal] = sig
        self.iportmap[actual] = (self, pin)
        
    def promptports (self, parent):
        """Interactively build the portmap for this element
        """
        print "inputs:"
        for p in self.eltype.reqinputs ():
            pto = raw_input ("%s: " % p)
            self.addportmap (parent, p, pto)
        print "outputs:"
        for p in self.eltype.outputs ():
            pto = raw_input ("%s: " % p)
            if pto:
                self.addportmap (parent, p, pto)
        prompt_optin = True
        for p in self.eltype.optinputs ():
            if prompt_optin:
                opt = raw_input ("any optional inputs? ")
                if not opt.lower ().startswith ("y"):
                    break
            prompt_optin = False
            pto = raw_input ("%s: " % p)
            if pto:
                self.addportmap (parent, p, pto)

    def printportmap (self):
        """Return the port map.  Entries are grouped inputs
        then outputs, and alphabetically within the group.
        """
        entries = [ ]
        for p in sorted (self.portmap):
            if self.eltype.pins[p].dir == "out":
                continue
            pto = self.portmap[p]
            entries.append ("    %s => %s" % (p, pto))
        for p in sorted (self.portmap):
            if self.eltype.pins[p].dir != "out":
                continue
            pto = self.portmap[p]
            entries.append ("    %s => %s" % (p, pto))
        return """  %s : %s port map (
%s);
""" % (self.name, self.eltype.name, ",\n".join (entries))

    
class cmod (ElementType):
    """A Cyber module or element of a module
    """
    def __init__ (self, name):
        ElementType.__init__ (self, name)
        self.name = name
        self.elements = { }
        self.signals = { }
        self.generics = { }

    def addelement (self, eltype):
        if eltype not in elements or eltype == self.name:
            print "No such element"
            return
        elname = "u%d" % (len (self.elements) + 1)
        print "element %s" % elname
        e = self.elements[elname] = ElementInstance (elname, eltype)
        e.promptports (self)

    def addassign (self, to, fname):
        """Add an assignment of an output signal from a temp
        """
        if not self.istemp (fname):
            print "assignment source is not a temp signal"
            return
        if to in self.signals:
            print to, "already defined"
            return
        tsig = self.signals[to] = Signal (to)
        try:
            fsig = self.signals[fname]
        except KeyError:
            fsig = self.signals[fname] = Signal (fname)
        tsig.setsource (fsig)
        
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
        self.finish ()
        
    def printassigns (self, sigdict, comp = None):
        """Format the signal assignments from the supplied dictionary.
        If comp is supplied, include only the assignments whose
        right-hand side appears in the right hand sides of the
        portmap of comp.  Returns a pair of formatted string and
        remaining dictionary.  The remaining dictionary contains
        the signals that were not formatted (not matched in comp,
        or not a signal defined by an assignment).
        """
        assigns = [ ]
        if comp:
            cports = set (comp.portmap.values ())
        else:
            cports = None
        for p in sorted (sigdict):
            sig = sigdict[p]
            if sig.source and \
                   isinstance (sig.source, Signal) and \
                   (cports is None or sig.source in cports):
                assigns.append ("  %s;\n" % sig.printassign ())
                del sigdict[p]
        return ("".join (assigns), sigdict)
    
    def printheader (self):
        return """-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- %s
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
""" % (__doc__.split ("\n")[2], self.name.upper ())

    def istemp (self, pin):
        """Tells whether the pin is a temp signal or not.  For modules
        (things with two-character names), it's a pin if its name starts
        with "p" or "tp" and a temp otherwise.  For other elements,
        it's a temp if it starts with "t" and a "pin" (i.e., a port)
        otherwise.
        """
        pin = str (pin)
        if len (self.name) == 2:
            # Module: if it's not a pin and not a testpoint, it's a temp
            return not _re_pinname.match (pin) and \
                   not pin.startswith ("tp")
        else:
            # Element: if it starts with t but isn't a testpoint, it's a pin
            return pin.startswith ("t") and not pin.startswith ("tp")

    def finish (self):
        """Do various actions that can't be done until the module
        definition is complete
        """
        #print "finishing", self.name
        for s in self.signals.itervalues ():
            s.sources ()
        for s in self.signals.itervalues ():
            if not self.istemp (s) and not s in self.pins:
                if s.source:
                    # It has a source so it's an output
                    self.addpin ((str (s),), "out", s.ptype)
                else:
                    # Optional pins are added before calling this method
                    self.addpin ((str (s),), "in", s.ptype)
        for s in self.signals.itervalues ():
            if not self.istemp (s):
                p = self.pins[s]
                if p.dir == "out":
                    again = True
                    visited = set ()
                    while again:
                        again = False
                        sources = list (s.sources ())
                        for src in sources:
                            if src in visited:
                                continue
                            if isinstance (src, PinInstance):
                                again = True
                                visited.add (src)
                                for s2 in src.pin.sources ():
                                    if not s2.opt:
                                        try:
                                            s.addsource (src.parent.portmap[s2])
                                        except KeyError:
                                            print "Missing input", s2, "for", src
                    for src in list (s.sources ()):
                        if self.istemp (src) or isinstance (src, PinInstance):
                            s.delsource (src)
                    p._sources = frozenset (s.sources ())
                        
    def printmodule (self):
        """return module definition
        """
        eltypes = set ()
        temps = { }
        components = [ ]
        gates = [ ]
        signals = [ ]
        generics = [ ]
        sigdict = dict (self.signals)
        for e in self.elements.itervalues ():
            eltypes.add (e.eltype.name)
            #print e.eltype.name
        for s in sorted (self.signals.itervalues ()):
            if self.istemp (s):
                signals.append ("  signal %s : %s;\n" % (s.name, s.ptype))
        for e in sorted (eltypes):
            components.append (elements[e].printcomp ())
        for en in sorted (self.elements):
            e = self.elements[en]
            gates.append (e.printportmap ())
            assigns, sigdict = self.printassigns (sigdict, e)
            gates.append (assigns)
        assigns, sigdict = self.printassigns (sigdict)
        for g in sorted (self.generics):
            generics.append ("    %s : %s" % (g, self.generics[g]))
        if generics:
            generics = "generic (\n%s);\n" % ";\n".join (generics)
        else:
            generics = ""
        return """library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity %s is
%s%s
end %s;
architecture gates of %s is
%s
%s
begin -- gates
%s
%s
end gates;
""" % (self.name,
       generics,
       self.printports (),
       self.name,
       self.name,
       "\n".join (components),
       "".join (signals),
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

_re_arch = re.compile (r"entity +(.+?) +is\s+?(generic +\((.+?)\);\s+?)?port +\((.+?)\).+?end +\1.+?architecture (\w+) of \1 is.+?begin(.+?)end \w+;", re.S)
_re_portmap = re.compile (r"(\w+)\s*:\s*(\w+) port map \((.+?)\)", re.S)
_re_generic = re.compile (r"(\w+)\s*:\s*(\w+)")
_re_pinmap = re.compile (r"(\w+)\s*=>\s*(\w+|'1')")
_re_assign = re.compile (r"(\w+)\s*<=\s*(\w+)")
_re_ent = re.compile (r"entity +(.+?) +is\s+?port +\((.+?)\).+?end +\1", re.S)
_re_pin = re.compile (r"([a-z0-9, ]+):\s+(inout|in|out)\s+(std_logic|coaxsig|analog|misc)( +:= +'[01]')?")
_re_comment = re.compile (r"--.*$", re.M)
_re_pinname = re.compile (r"p\d+$")

def readmodule (modname):
    """Read a module definition VHD file and return the top
    level module object
    """
    f = open ("%s.vhd" % modname, "r")
    # Read the file, stripping comments
    mtext = _re_comment.sub ("", f.read ())
    f.close ()
    e = None
    for m in _re_arch.finditer (mtext):
        e = cmod (m.group (1))
        #print m.group (1)
        # Process any generics
        if m.group (2):
            for g in _re_generic.finditer (m.group (3)):
                e.generics[g.group (1)] = g.group (2)
        # Parse the architecture section, if it's "gates"
        gates = m.group (5) == "gates"
        if gates:
            for c in _re_portmap.finditer (m.group (6)):
                u = ElementInstance (c.group (1), c.group (2))
                #print "element", c.group (1), c.group (2)
                e.elements[c.group (1)] = u
                for pin in _re_pinmap.finditer (c.group (3)):
                    #print "pin", pin.group (1), pin.group (2)
                    u.addportmap (e, pin.group (1), pin.group (2))
            for a in _re_assign.finditer (m.group (6)):
                e.addassign (a.group (1), a.group (2))
                #print "assign", a.group (1), a.group (2)
        # Look over the entity definition to pick up pins marked
        # as optional inputs.  If we encoutered them in the architecture
        # section, update the pin type.
        # If the architecture section wasn't "gates" then process
        # all pin entries, since we don't have any other source for
        # ports information.  Ditto if the type was "misc" (wire jumpers).
        for pins in _re_pin.finditer (m.group (4)):
            dir = pins.group (2)
            ptype = pins.group (3)
            opt = False
            if dir == "inout" and ptype != "misc":
                print "Unexpected type %s for inout pin" % ptype
                continue
            if gates:
                if pins.group (4):
                    if dir != "in":
                        print "Unexpected default in", pins.group ()
                        continue
                    opt = True
                elif ptype != "misc":
                    continue
            #print pins.groups ()
            for p in pins.group (1).replace (" ", "").split (","):
                if p in e.pins or opt or not gates or ptype == "misc":
                    e.pins[p] = Pin (p, dir, ptype, opt)
        e.finish ()
    if e is None:
        print "No module found in %s.vhd" % modname
    return e
 

def stdelements ():
    f = open ("cyberdefs.vhd", "r")
    # Read the file, stripping comments
    std = _re_comment.sub ("", f.read ())
    f.close ()
    for e in _re_ent.finditer (std):
        c = ElementType (e.group (1))
        for pins in _re_pin.finditer (e.group (2)):
            dir = pins.group (2)
            ptype = pins.group (3)
            opt = False
            if pins.group (4):
                if dir != "in":
                    print "Unexpected default in", pins.group ()
                else:
                    opt = True
            pinlist = pins.group (1).replace (" ", "").split (",")
            c.addpin (pinlist, dir, ptype, opt)
        c.finish ()

# Load the standard element definitions
stdelements ()
