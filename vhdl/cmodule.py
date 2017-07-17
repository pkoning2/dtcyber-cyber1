#!/usr/bin/env python3

"""Create a module definition VHDL file

Copyright (C) 2009-2017 by Paul Koning
"""

import readline
import re
import sys
import time
import os
from builtins import sorted as b_sorted

vhdl_keywords = ("if", "in", "is", "to")

def modname (name):
    if name in vhdl_keywords:
        return "mod_" + name
    else:
        return name
    
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

def init_completions ():
    readline.parse_and_bind ("tab: complete")
    readline.parse_and_bind ("set show-all-if-ambiguous on")
    readline.set_completer (complete_list)
    completions ()

elements = { }

_re_k = re.compile (r"(.+?)(\d+)(.*)$")
def ankey (a):
    a = str (a)
    am = _re_k.match (a)
    if am and am.group (2):
        return ( am.group (1), int (am.group (2)), am.group (3) )
    return (a,)

# Override the standard "sorted" to use the above key function
def sorted (a):
    return b_sorted (a, key = ankey)

class hitem (object):
    """A container, hashed and sorted by name
    """
    def __init__ (self, name):
        self.name = str (name)

    def __str__ (self):
        return self.name

    def __eq__ (self, other):
        return self.name == str (other)

    def __hash__ (self):
        return hash (self.name)
        
    def __lt__ (self, other):
        return self.name < str (other)
    
    def __gt__ (self, other):
        return self.name > str (other)
    
class Pin (hitem):
    """Pin of a logic element type
    """
    def __init__ (self, name, dir, ptype, opt = False, optval = None):
        hitem.__init__ (self, name)
        self.dir = dir
        self.ptype = ptype
        self.opt = opt
        self.optval = optval
        self._sources = set ()

    def sources (self):
        return self._sources
    
    def printdecl (self):
        if self.opt:
            return "%s : %-3s %s := %s" % (self.name, self.dir,
                                            self.ptype, self.optval)
        else:
            return "%s : %-3s %s" % (self.name, self.dir, self.ptype)

    def testpoint (self):
        return self.name.startswith ("tp")

stdpins = { "reset" : "logicsig",
            "sysclk" : "clocks" }
def stdpin (s):
    return s in stdpins

class Generic (hitem):
    """Generic of a logic element type
    """
    def __init__ (self, name, ptype, defval = None):
        hitem.__init__ (self, name)
        self.ptype = ptype
        self.defval = defval

    def printdecl (self):
        if self.defval:
            return "%s : %s%s" % (self.name, self.ptype, self.defval)
        else:
            return "%s : %s" % (self.name, self.ptype)
            
class ElementType (object):
    """Logic element type
    """
    def __init__ (self, name):
        global elements
        elements[name] = self
        self.name = name
        self.pins = { }
        self.pinnames = { }
        self.generics = { }
        
    def addpin (self, namelist, dir, ptype = "logicsig",
                opt = False, optval = None):
        if dir not in ("in", "out"):
            print("Unrecognized pin direction", dir)
            return
        for name in namelist:
            if self.name == "pa" and self.name == "p3":
                raise Exception
            if name in self.pins:
                print("Pin", name, "already defined")
            else:
                self.pins[name] = p = Pin (name, dir, ptype, opt, optval)
                # In case a list of signals is passed in
                name = str (name)
                if "_" in name:
                    for n in name.split ("_"):
                        self.pinnames[n] = p

    def inputs (self):
        """Return a list of input pins, sorted by name
        """
        return sorted ([p for p in self.pins.values ()
                        if p.dir != "out"])

    def reqinputs (self):
        """Return a list of required input pins
        """
        return [ p for p in self.inputs () if not p.opt ]

    def optinputs (self):
        """Return a list of required input pins
        """
        return [ p for p in self.inputs () if p.opt ]

    def outputs (self):
        """Return a list of output pins, sorted by name
        """
        return sorted ([p for p in self.pins.values ()
                        if p.dir == "out"])
        
    def printports (self):
        """Return the ports definition of this element type.
        Pins are grouped as inputs, test points, outputs, and
        alphabetically within the group.
        """
        generics = [ ]
        ports = [ ]
        for g in sorted (self.generics):
            generics.append ("      %s" % self.generics[g].printdecl ())
        if generics:
            generics = "    generic (\n%s);\n" % ";\n".join (generics)
        else:
            generics = ""
        for p in self.inputs ():
            ports.append ("      %s" % p.printdecl ())
        o = self.outputs ()
        for p in o:
            if p.testpoint ():
                ports.append ("      %s" % p.printdecl ())
        for p in o:
            if not p.testpoint ():
                ports.append ("      %s" % p.printdecl ())
        if ports:
            ports = """    port (
%s);
""" % ";\n".join (ports)
        else:
            ports = ""
        return generics + ports

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
    def __init__ (self, name, opt = False):
        hitem.__init__ (self, name)
        self.source = None
        self._sources = set ()
        self.destcount = 0
        self.opt = opt
        self.optval = None
        self.ptype = None
        self.aliases = set ()

    def setsource (self, source):
        if self.source and self.source != source:
            print("Duplicate assignment to signal", self.name)
            return
        if isinstance (source, PinInstance):
            if source.dir != "out":
                print("Signal source must be output pin", self.name, source.name)
                return
        self.source = source
        self.ptype = source.ptype
        self.addsource (source)

    def addsource (self, source):
        if not isinstance (source, ConstSignal):
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
            print("No source for", self.name)
            return "-- no source for %s" % self.name

class ConstSignal (Signal):
    """A signal that is a constant (not a pin)
    """
    def __init__ (self, name):
        Signal.__init__ (self, name)
        self.ptype = "logicsig"
        
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
sigzero = ConstSignal ("'0'")

class ElementInstance (object):
    """Instance of a logic element
    """
    def __init__ (self, name, elname, parent):
        self.name = name
        self.parent = parent
        self.eltype = elements[modname (elname)]
        self.portmap = { }
        self.iportmap = { }
        self.genericmap = { }
        # Automatically connect up reset and clock pins, if present
        for pin in self.eltype.pins:
            if stdpin (pin):
                try:
                    s = self.parent.signals[pin]
                except KeyError:
                    s = self.parent.signals[pin] = Signal (pin)
                    s.ptype = stdpins[pin]
                self.addportmap (self.parent, pin, pin)
        
    def __str__ (self):
        return self.name

    def addgenericmap (self, parent, formal, actual):
        """Add a generic map entry.  "formal" is the generic name;
        "actual" is the generic item value.
        """
        if formal in self.eltype.generics:
            self.genericmap[formal] = actual
        else:
            print("Generic map for unknown generic %s" % formal)
            
    def addportmap (self, parent, formal, actual):
        """Add a port map entry.  "formal" is the element type pin name;
        "actual" is the signal to map to it, or a string naming a signal.
        """
        # First see if it's an alias -- a name of a pin that has multiple
        # names because it's a fanout
        try:
            pin = self.eltype.pinnames[formal]
        except KeyError:
            try:
                pin = self.eltype.pins[formal]
            except KeyError:
                if actual is not sigone:
                    print("Port map to unknown pin %s" % formal)
                return
        dir = pin.dir
        ptype = pin.ptype
        opt = False
        if isinstance (actual, str):
            if actual.startswith ("-"):
                if dir != "in":
                    print("Optional input flag but %s is not an input" % formal)
                    return
                actual = actual[1:]
                opt = True
            if actual == "'1'":
                actual = sigone
            elif actual == "'0'":
                actual = sigzero
            else:
                actual = parent.findsignal (actual, opt)
        if dir == "out":
            # Assigning to output, must be a new signal, or one that
            # doesn't have an output yet.  Special case: '1' is a valid
            # actual
            if actual.source:
                print("duplicate output assignment to", actual)
                return
        else:
            # Input, can be used multiple times
            if actual.ptype:
                if actual.ptype != ptype and \
                       not isinstance (actual, ConstSignal):
                    print("signal type mismatch", self.name, actual, actual.ptype, ptype)
            else:
                actual.ptype = ptype
        if dir == "out":
            actual.setsource (PinInstance (self, pin))
        else:
            actual.destcount += 1
        # Save both directions of the mapping
        if pin.name in self.portmap and (not ptype or ptype == "logicsig"):
            # Multiple outputs for an output signal, that's a fanout.
            # Make the new actual an alias of the previous one.
            oa = self.portmap[pin.name]
            if "(" in actual.name or "(" in oa.name:
                # Check for ( is there to filter out coax cables that
                # carry logic signals.  That doesn't happen in the regular
                # inter-chassis connections but it does on the DD60 cabling
                # in chassis 12.
                #print("Adding alias coax signal:", \
                #      self.name, self.eltype.name, pin.name, \
                #      actual.name, oa.name, pinsig)
                #oa.aliases.add (oa.name)
                #oa.aliases.add (actual.name)
                if "(" in actual.name:
                    parent.aliases[actual.name] = (self, pin)
                if "(" in oa.name:
                    parent.aliases[oa.name] = (self, pin)
                self.portmap[pin.name] = actual
            else:
                #print ("adding alias %s for %s" % (actual.name, oa.name))
                oa.aliases.add (oa.name)
                oa.aliases.add (actual.name)
                parent.aliases[actual.name] = oa
                del parent.signals[actual]
        else:
            self.portmap[pin.name] = actual
        self.iportmap[actual] = (self, pin)
        
    def promptports (self, parent):
        """Interactively build the portmap for this element
        """
        print("inputs:")
        for p in self.eltype.reqinputs ():
            pto = getport (p)
            self.addportmap (parent, p, pto)
        print("outputs:")
        for p in self.eltype.outputs ():
            pto = getport (p)
            if pto:
                self.addportmap (parent, p, pto)
        prompt_optin = True
        for p in self.eltype.optinputs ():
            if prompt_optin:
                opt = input ("any optional inputs? ")
                if not opt.lower ().startswith ("y"):
                    break
            prompt_optin = False
            pto = getport (p)
            if pto:
                self.addportmap (parent, p, pto)

    def printportmap (self):
        """Return the port map.  Entries are grouped inputs
        then outputs, and alphabetically within the group.
        The generic map, if present, is also formatted preceding
        the port map.
        """
        if self.genericmap:
            generics = [ ]
            for g in sorted (self.genericmap):
                generics.append ("    %s => %s" % (g, self.genericmap[g]))
            generics = """generic map (
%s)
  """ % ",\n".join (generics)
        else:
            generics = ""
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
        return """  %s : %s %sport map (
%s);
""" % (self.name, self.eltype.name, generics, ",\n".join (entries))

    
_re_strip = re.compile ("[ \t\n]+")
_re_empty = re.compile ("# *$")

class cmod (ElementType):
    """A Cyber module or element of a module
    """
    def __init__ (self, name = None):
        if not name:
            name = input ("Module name: ")
            try:
                os.stat (name + ".vhd")
                print("%s.vhd already exists")
                raise OSError
            except OSError:
                pass
        self.modname = name = modname (name)
        if name.startswith ("mod_") and len (name) == 6:
            self.ismodule = True
            self.modname = name[-2:]
        else:
            self.ismodule = (len (name) == 2)
        ElementType.__init__ (self, name)
        self.firstyear = self.cyears = None
        self.oldtext = None
        self.name = name
        self.elements = { }
        self.signals = { }
        self.aliases = { }
        self.sourcehdr = ""

    def setheader (self, text):
        text = text.splitlines ()
        while text and _re_empty.match (text[0]):
            del text[0]
        while text and _re_empty.match (text[-1]):
            del text[-1]
        text = '\n'.join (text)
        self.sourcehdr = text.replace ("#", "--")
        
    def nextelement (self):
        n = len (self.elements) + 1
        while True:
            elname = "u%d" % n
            if elname not in self.elements:
                return elname
            n += 1
            
    def addelement (self, eltype):
        if eltype not in elements or eltype == self.name:
            print("No such element")
            return
        elname = self.nextelement ()
        print("element %s" % elname)
        e = self.elements[elname] = ElementInstance (elname, eltype, self)
        try:
            e.promptports (self)
        except EOFError:
            print()
            del self.elements[elname]
            
    def findsignal (self, name, opt = False):
        """Find a currently defined signal, or create a new one
        """
        try:
            return self.signals[name]
        except KeyError:
            s = self.signals[name] = Signal (name, opt)
            return s

    def addassign (self, to, fname):
        """Add an assignment of an output signal from a temp
        """
        #print ("assign %s <= %s" % (to, fname))
        if fname == "'0'":
            fsig = sigzero
        elif fname == "'1'":
            fsig = sigone
        else:
            fsig = self.findsignal (fname)
        if fsig is sigzero or fsig is sigone or \
               fsig.ptype and fsig.ptype != "logicsig":
            if to in self.signals:
                print(to, "already defined")
                return
            tsig = self.findsignal (to)
            tsig.setsource (fsig)
        else:
            for s in self.signals.values ():
                if to in s.aliases:
                    print(to, "already defined")
                    return
            fsig.aliases.add (to)

    def addelements (self):
        init_completions ()
        while True:
            tlist = [ e for e in elements if e != self.name ]
            completions (tlist)
            eltype = input ("element: ")
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

    ohdr = None
    header = """-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) {} by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- {} module
--
-------------------------------------------------------------------------------
"""

    def printheader (self):
        ohdr = None
        now = str (time.localtime ().tm_year)
        if self.ohdr:
            ohdr = self.ohdr
            m = _re_cright.search (ohdr)
            if m:
                firstyear = m.group (1)
                cyears = m.group (1) + m.group (2)
                if now not in cyears:
                    if firstyear == now:
                        cyears = firstyear
                    else:
                        cyears = "{}-{}".format (firstyear, now)
                    ohdr =_re_cright.sub ("Copyright (C) {}".format (cyears), ohdr)
            else:
                ohdr = None
        if not ohdr:
            ohdr = self.header.format (now, self.name.upper ())
        return ohdr

    def isinternal (self, pin):
        """Tells whether the pin is an internal signal or not.  For modules
        (things with two-character names), it's a pin if its name starts
        with "p" or "tp" and a temp otherwise.  For other elements,
        it's a temp if it starts with "t" and a "pin" (i.e., a port)
        otherwise.
        """
        pin = str (pin)
        if self.ismodule:
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
        for s in self.signals.values ():
            if s.ptype is None:
                s.ptype = "logicsig"
        for s in self.signals.values ():
            if not self.isinternal (s) and not s in self.pins:
                if not s.source:
                    # It has no source so it's an input
                    #print "adding", s, s.ptype, s.opt
                    self.addpin ((str (s),), "in", s.ptype, s.opt, s.optval)
        for s in list(self.signals.values ()):
            if s.aliases:
                oname = s.name
                if not (s.destcount or \
                        (s in self.pins and self.pins[s].dir == "in")):
                    if self.isinternal (oname):
                        s.aliases.discard (oname)
                    elif not "_" in oname:
                        # Add original name, if we haven't already built
                        # the composite name
                        s.aliases.add (oname)
                a = sorted (s.aliases)
                an = "_".join (a)
                #print ("signal %s, aliases %s" % (str(s), an))
                if s.destcount or \
                       (s in self.pins and self.pins[s].dir == "in"):
                    #print ("new signal set from", s)
                    tsig = self.findsignal (an)
                    tsig.setsource (s)
                else:
                    #print ("changing signal %s to name %s" % (s, an))
                    del self.signals[s.name]
                    s.name = an
                    self.signals[an] = s
        for s in self.signals.values ():
            s.sources ()
        for s in self.signals.values ():
            if not self.isinternal (s) and not s in self.pins:
                if s.source:
                    # It has a source so it's an output
                    self.addpin ((str (s),), "out", s.ptype)
        for s in self.signals.values ():
            if not self.isinternal (s):
                p = self.pins[s]
                if p.dir == "out":
                    again = True
                    visited = set ()
                    while again:
                        again = False
                        sources = list (s.sources ())
                        for src in sources:
                            if src in visited or src.opt:
                                continue
                            if isinstance (src, PinInstance):
                                again = True
                                visited.add (src)
                                for s2 in src.pin.sources ():
                                    if not s2.opt:
                                        try:
                                            s.addsource (src.parent.portmap[s2])
                                        except KeyError:
                                            print(self.name, "Missing input", s2, "for", src)
                    for src in list (s.sources ()):
                        if self.isinternal (src) or isinstance (src, PinInstance):
                            s.delsource (src)
                    srcpins = set ()
                    for src in s.sources ():
                        try:
                            srcpins.add (self.pins[src])
                        except KeyError:
                            print("key error", src, "not in pins for", self.name)
                            raise
                    p._sources = frozenset (srcpins)
                        
    def printmodule (self):
        """return module definition
        """
        eltypes = set ()
        temps = { }
        components = [ ]
        gates = [ ]
        signals = [ ]
        sigdict = dict (self.signals)
        for e in self.elements.values ():
            eltypes.add (e.eltype.name)
            #print e.eltype.name
        for s in sorted (self.signals.values ()):
            if self.isinternal (s):
                signals.append ("  signal %s : %s;\n" % (s.name, s.ptype))
        for e in sorted (eltypes):
            components.append (elements[e].printcomp ())
        for en in sorted (self.elements):
            e = self.elements[en]
            gates.append (e.printportmap ())
            assigns, sigdict = self.printassigns (sigdict, e)
            gates.append (assigns)
        assigns, sigdict = self.printassigns (sigdict)
        if not gates and not assigns.strip ():
            raise Exception("Empty architecture")
        return """use work.sigs.all;

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
       "".join (signals),
       "\n".join (gates),
       assigns)

    def write (self, writedep = False):
        """Write module definition to a file.  Writedep says whether to
        write a .d file (dependencies file).  If the definition is
        unchanged (except for layout and comments), don't write it.
        """
        slices = set ()
        deps = set ()
        newtext = [ ]
        for e in self.elements:
            if "slice" in self.elements[e].eltype.name and \
                   self.elements[e].eltype.name.startswith (self.name):
                slices.add (self.elements[e].eltype.name)
            else:
                deps.add (self.elements[e].eltype.name)
        try:
            for slice in sorted (slices):
                newtext.append (elements[slice].printmodule ())
            newtext.append (self.printmodule ())
        except Exception as msg:
            print("Module %s: %s" % (self.name, msg))
            raise
            return
        newtext = '\n'.join (newtext)
        if self.oldtext:
            n = _re_comment.sub ("", newtext)
            n = _re_strip.sub (" ", n).strip ()
            o = _re_strip.sub (" ", self.oldtext)
        else:
            n = "new"
            o = "old"
        if n == o:
            print("Module %s is unchanged" % self.name)
        else:
            f = open ("%s.vhd" % self.modname, "w")
            self.cyears = None
            print(self.printheader (), file=f)
            print(newtext, file=f)
            f.close ()
        if writedep:
            deps.discard ("wire")
            df = open ("%s.d" % self.name, "w")
            print("%s.o: %s.vhd" % (self.name, self.name), end=' ', file=df)
            for dep in sorted (deps):
                if dep.startswith ("mod_"):
                    dep = dep[4:]
                print("%s.o" % dep, end=' ', file=df)
            print(file=df)
            df.close ()

_re_arch = re.compile (r"entity +(.+?) +is\s+?(generic +\((.+?)\);\s+?)?port +\((.+?)\).+?end +\1.+?architecture (\w+) of \1 is.+?begin(.+?)end \w+;", re.S)
_re_portmap = re.compile (r"(\w+)\s*:\s*(\w+) port map \((.+?)\)", re.S)
_re_generic = re.compile (r"(\w+)\s*:\s*(\w+)( +:= +.+)?")
_re_pinmap = re.compile (r"(\w+)\s*=>\s*(\w+|'1')")
_re_assign = re.compile (r"(\w+)\s*<=\s*(['\w]+)")
_re_pin = re.compile (r"([a-z0-9, ]+):\s+(inout|in|out)\s+([a-z0-9_]+)( +:= +'[01]')?")
_re_comment = re.compile (r"--.*$", re.M)
_re_chdr = re.compile ("(--.*\n)+", re.M)
_re_pinname = re.compile (r"p\d+(_|$)")
_re_cright = re.compile (r"copyright \(c\) (\d+)((?:[-, ]+\d+)*)", re.I)

def readmodule (modname):
    """Read a module definition VHD file and return the top
    level module object
    """
    f = open ("%s.vhd" % modname, "r")
    #print "reading module", modname
    # Read the file, stripping comments
    mtext = f.read ()
    ohdr = None
    m = _re_chdr.match (mtext)
    if m:
        ohdr = m.group (0)
        mtext = mtext[m.end ():]
    mtext = _re_comment.sub ("", mtext)
    f.close ()
    e = None
    for m in _re_arch.finditer (mtext):
        e = cmod (m.group (1))
        e.ohdr = ohdr
        e.oldtext = mtext.strip ()
        #print m.group (1)
        # Process any generics
        if m.group (2):
            for g in _re_generic.finditer (m.group (3)):
                e.generics[g.group (1)] = Generic (g.group (1),
                                                   g.group (2),
                                                   g.group (3))
        # Parse the architecture section, if it's "gates"
        gates = (m.group (5) == "gates" or m.group (5) == m.group (1))
        if gates:
            for c in _re_portmap.finditer (m.group (6)):
                u = ElementInstance (c.group (1), c.group (2), e)
                #print "element", c.group (1), c.group (2)
                e.elements[c.group (1)] = u
                for pin in _re_pinmap.finditer (c.group (3)):
                    #print "pin", pin.group (1), pin.group (2)
                    u.addportmap (e, pin.group (1), pin.group (2))
            # Do assignments as aliases.
            if len (modname) == 2:
                for a in _re_assign.finditer (m.group (6)):
                    e.addassign (a.group (1), a.group (2))
                    #print "assign", a.group (1), a.group (2)
        # Look over the entity definition to pick up pins marked as
        # optional inputs.  If we encountered them in the architecture
        # section, update the pin type.
        # If the architecture section wasn't "gates" then process
        # all pin entries, since we don't have any other source for
        # ports information.  Ditto if the type was "misc" (wire jumpers).
        e.finish ()
        for pins in _re_pin.finditer (m.group (4)):
            dir = pins.group (2)
            ptype = pins.group (3)
            opt = False
            optval = None
            if dir == "inout" and ptype != "misc":
                print("Unexpected type %s for inout pin" % ptype)
                continue
            if gates:
                if pins.group (4):
                    if dir != "in":
                        print("Unexpected default in", pins.group ())
                        continue
                    opt = True
                    optval = pins.group (4)[-3:]
            #print pins.groups ()
            for p in pins.group (1).replace (" ", "").split (","):
                if p not in e.pinnames:
                    e.pins[p] = Pin (p, dir, ptype, opt, optval)
        # Do this again to handle pins we added above
        e.finish ()
    if e is None:
        print("No module found in %s.vhd" % modname)
    return e

def getport (p):
    while True:
        pto = input ("%s: " % p)
        if "=" in pto or pto in elements:
            print("Invalid port", pto)
        else:
            return pto
        
def stdelements ():
    global stdnames
    stdnames = set ()
    f = open ("cyberdefs.vhd", "r")
    # Read the file, stripping comments
    std = _re_comment.sub ("", f.read ())
    f.close ()
    for e in _re_arch.finditer (std):
        c = ElementType (e.group (1))
        stdnames.add (c.name)
        # Process any generics
        if e.group (2):
            for g in _re_generic.finditer (e.group (3)):
                c.generics[g.group (1)] = Generic (g.group (1),
                                                   g.group (2),
                                                   g.group (3))
        for pins in _re_pin.finditer (e.group (4)):
            dir = pins.group (2)
            ptype = pins.group (3)
            opt = False
            if pins.group (4):
                if dir != "in":
                    print("Unexpected default in", pins.group ())
                else:
                    opt = True
            pinlist = pins.group (1).replace (" ", "").split (",")
            c.addpin (pinlist, dir, ptype, opt)
        c.finish ()

def allmodules ():
    # Read all modules (files with names of the form xy.vhd).
    # On completion, dictionary "modules" contains the modules
    # that were read.
    global modules
    modules = { }
    for mn in [ n[:2] for n in os.listdir (".") if len (n) == 6 and
                n.endswith (".vhd") ]:
        print ("loading", mn)
        modules[mn] = readmodule (mn)

# Load the standard element definitions
stdelements ()
#print ("standard names", stdnames)
