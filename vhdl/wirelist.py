#!/usr/bin/env python

"""Process CDC 6600 wiring list

By Paul Koning and Dave Redell
"""

import re
import sys

header = """-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;


"""

chassis_list = [ None ] * 17

portpat = re.compile (r"\s*([\w\s,]+):\s*(in|out|inout)\s+std_logic\s*(?:\:=\s'.')?(\))?\s*;", re.I)

class Fail (Exception):
    pass
class Warn (Exception):
    pass

class Chassis (object):
    """An instance of a 6000 chassis.
    """
    def __init__ (self, num):
        self.name = "chassis%d" % num
        self.module_types = { }
        self.modules = { }
        self.coax = [ ]
        
    def add_module_type (self, tname):
        """Add a module type by type name, returning the ModuleType
        object.  If it was added before, return that one.
        """
        try:
            return self.module_types[tname]
        except KeyError:
            self.module_types[tname] = mod = ModuleType (tname)
            return mod

    def add_module (self, mtype, name):
        """Add a module instance for slot 'name'.  Error if that slot
        already has something in it.  Mtype is the module type object.
        """
        if name in self.modules:
            raise Warn, "Duplicate module instance for slot %s" % name
        self.modules[name] = ModuleInstance (name, mtype)

    def find_module (self, name):
        """Find the module instance for slot 'name'.  If the name
        looks like "coax1", return that coax instead.
        """
        if name.startswith ("coax"):
            try:
                return self.coax[int (name[4:]) - 1]
            except IndexError:
                raise Warn, "No %s defined" % name
        try:
            return self.modules[name]
        except KeyError:
            raise Warn, "No defined module in slot %s" % name
        
    def add_coax (self, num, to_chassis, to_num):
        """Add a coax to the given chassis and coax number.  Num is the
        one-based number of the coax (at this side).  They must be
        added in order.
        """
        if num != len (self.coax) + 1:
            raise Warn, "Coax %d is out of sequence" % num
        self.coax.append (Coax (num, to_chassis, to_num))

    def port (self, c):
        """Return the port definition for the specified coax,
        without the ; delimiter.
        """
        return "      %s : inout coaxsigs" % c.name
    
    def ports (self):
        """Return the ports definition of the chassis, as a string.
        """
        portlist = [ "      clk : in std_logic" ]
        for c in self.coax:
            portlist.append (self.port (c))
        if portlist:
            return "    port (\n%s);\n" % ";\n".join (portlist)
        return ""

    def component_decls (self):
        """Return the component declarations for this chassis
        (i.e., for the module types it contains), as a string.
        """
        clist = [ ]
        mlist = self.module_types.keys ()
        mlist.sort ()
        for m in mlist:
            clist.append ("  component %s\n%s  end component;\n" % (m, self.module_types[m].ports ()))
        return "".join (clist)

    def wires (self):
        """Return the list of wires for this chassis, as signal
        declarations.  The signal names look like m32_1_q34_17 for
        the wire from pin 1 of the module in slot m32 to pin 17 of
        the module in slot q34.  The left half of the name is less
        than the right half, i.e., lower slot, or lower pin if both
        ends are the same slot.
        """
        wlist = [ ]
        for mname, m in self.modules.iteritems ():
            for pin, p in m.connections.iteritems ():
                other, pin2 = p
                if isinstance (other, Coax) or \
                       (other.name, pin2) < (mname, pin):
                    continue
                wlist.append (wirename (m, pin, other, pin2))
        wlist.sort ()
        return "  signal %s : std_logic;\n" % " : std_logic;\n  signal ".join (wlist)

    def print_vhdl (self, f):
        """Print the VHDL entity definition and structural model
        for this chassis.  f is the file to write to.  We don't
        build the whole string because that will be too much stuff...
        """
        print >> f, header
        print >> f, "entity %s is" % self.name
        print >> f, self.ports ()
        print >> f, "end %s;" % self.name
        print >> f, "\narchitecture modules of %s is" % self.name
        print >> f, self.component_decls ()
        print >> f, self.wires ()
        print >> f, "begin -- modules"
        clist = self.modules.keys ()
        clist.sort ()
        for slot in clist:
            c = self.modules[slot]
            print >> f, c.portmap ()
        print >> f, "end modules;"

def top_vhdl (f):
    """Print the top-level VHDL for the whole system.  This is the
    interconnect between chassis and from the chassis to the
    outside world.  All interconnects are modeled as coax, which is
    an array of 19 inout logic signals.
    """
    cables = { }
    for ch in chassis_list:
        if ch is None:
            continue
        for c in ch.coax:
            n = c.coaxname ()
            if c.to[0].startswith ("chassis"):
                cables[n] = 1
            else:
                cables[n] = 2
    cnames = cables.keys ()
    cnames.sort ()
    print >> f, header
    print >> f, "entity cdc6600 is\n  port (\n    clk : in std_logic;"
    clist = [ ]
    for c in cnames:
        n = cables[c]
        if n == 2:
            clist.append ("    %s : inout coaxsigs" % c)
    print >> f, "%s);\nend cdc6600;\n" % ";\n".join (clist)
    print >> f, "\narchitecture chassis of cdc6600 is"
    for ch in chassis_list:
        if ch is None:
            continue
        print >> f, "  component %s\n%s  end component;" % (ch.name, ch.ports ())
    for c in cnames:
        n = cables[c]
        if n == 1:
            print >> f, "  signal %s : inout coaxsigs;" % n
    print >> f, "begin -- chassis"
    for ch in chassis_list:
        if ch is None:
            continue
        print >> f, "  ch%d : %s port map (" % (int (ch.name[7:]), ch.name)
        clist = [ "    clk => clk" ]
        for c in ch.coax:
            n = c.coaxname ()
            clist.append ("    %s => %s" % (c.name, n))
        print >> f, "%s);" % ",\n".join (clist)
    print >> f, "end chassis;"

class CoaxType (object):
    """Used to make references to coax look like references to module
    instances.
    """
    def __init__ (self):
        self.name = "Coax"
        self.pins = { }
        for p in xrange (19):
            self.pins["p%d" % (p + 1)] = "inout"

coaxType = CoaxType ()

class Coax (object):
    """A coax connection (more precisely, the end within a given
    chassis of that coax connection).
    """
    def __init__ (self, num, to_chassis, to_num):
        self.num = num
        self.name = "coax%d" % num
        self.to = (to_chassis, to_num)
        self.Type = coaxType
        self.connections = { }

    def connect (self, pin, other, pin2):
        num = pinnum (pin)
        if num < 1 or num > 19:
            raise Warn, "Pin number %s out of range" % pin
        if pin in self.connections:
            raise Warn, "Coax %s pin %s already connected" % (self.name, pin)
        if pin2 in other.connections:
            raise Warn, "Pin %s already connected in %s module at %s" % (pin2, t2.name, other.name)
        self.connections[pin] = (other, pin2)
        other.connections[pin2] = (self, pin)
        
    def coaxname (self):
        """Like wirename but for coax cables.
        """
        this = self.name
        num = self.num
        other, num2 = self.to
        if other.startswith ("coax") and (other, num2) < (this, num):
            this, num, other, num2 = other, num2, this, num
        return "%s_%s_%s_%s" % (this, num, other, num2)
    
def pinnum (p):
    """Convert pin name to number.  Test point are treated as pins
    above 100.  Non-standard names are handled as -1000000+radix36(name).
    """
    if p.startswith ("tp"):
        return int (p[2:]) + 100
    elif p.startswith ("p"):
        return int (p[1:])
    return int (p, 36) - 1000000

def pincmp (p1, p2):
    """Comparison function for sorting pin names in numeric order.
    """
    return pinnum (p1) - pinnum (p2)

def wirename (this, pin, other, pin2):
    """Return the signal name for the connection between the two
    given endpoints.
    """
    if isinstance (this, Coax):
        return "%s(%d)" % (this.name, pinnum (pin))
    if isinstance (other, Coax):
        return "%s(%d)" % (other.name, pinnum (pin2))
    if (other.name, pin2) < (this.name, pin):
        this, pin, other, pin2 = other, pin2, this, pin
    return "%s_%s_%s_%s" % (this.name, pin, other.name, pin2)

class ModuleType (object):
    def __init__ (self, name):
        """Create an instance of a module type object.
        This object holds the per-type data, which is primarily the
        port definitions.
        """
        self.pins = { }
        self.name = name
        self.readdef (name)

    def readdef (self, name):
        """Read the <name>.vhd file to find the entity declaration
        for this module.  Process its port definition and save away
        all the pin data.
        """
        try:
            v = open (name + ".vhd", "r")
        except:
            raise Warn, "Error opening %s.vhd" % name
        entpat = re.compile ("entity %s is" % name, re.I)
        ent = False
        while not entpat.search (v.readline ()):
            pass
        while not v.readline ().strip ().startswith ("port"):
            pass
        for l in v:
            m = portpat.match (l)
            if not m:
                raise Warn, "Entity %s missing or port declaration not formatted right" % name
                return
            pins = m.group (1).lower ()
            dir = m.group (2).lower ()
            if dir not in ("in", "out"):
                raise Warn, "Unrecognized pin direction %s for %s pin %s" \
                      % (dir, name, pins)
            for pin in pins.split (","):
                pin = pin.strip ()
                if pin in self.pins:
                    print "duplicate pin", pin
                else:
                    self.pins[pin] = dir
            if m.group (3):
                break

    def port (self, pin):
        """Return the port definition for the specified pin,
        without the ; delimiter.
        """
        dir = self.pins[pin]
        return "      %s : %s std_logic" % (pin, dir)
    
    def ports (self):
        """Return the ports definition of the chassis, as a string.
        They are returned grouped in then out, and in numeric order
        within the group.
        """
        portlist = [ ]
        pins = self.pins.keys ()
        pins.sort (pincmp)
        for p in pins:
            if self.pins[p] == "in":
                portlist.append (self.port (p))
        for p in pins:
            if self.pins[p] != "in":
                portlist.append (self.port (p))
        return "    port (\n%s);\n" % ";\n".join (portlist)

class ModuleInstance (object):
    """An instance of a ModuleType (living in some slot in some chassis)
    """
    def __init__ (self, name, mod_type):
        """name is the slot name.
        """
        self.name = name
        self.Type = mod_type
        self.connections = { }
        
    def connect (self, pin, other, pin2):
        t1 = self.Type
        t2 = other.Type
        try:
            dir1 = t1.pins[pin]
        except KeyError:
            raise Warn, "No pin %s in %s module at %s" % (pin, t1.name, self.name)
        try:
            dir2 = t2.pins[pin2]
        except KeyError:
            raise Warn, "No pin %s in %s module at %s" % (pin2, t2.name, other.name)
        if dir1 != "inout" and dir2 != "inout" and dir1 == dir2:
            raise Warn, "Error: %s to %s" % (dir1, dir2)
        else:
            if pin in self.connections:
                raise Warn, "Pin %s already connected in %s module at %s" % (pin, t1.name, self.name)
            if pin2 in other.connections:
                raise Warn, "Pin %s already connected in %s module at %s" % (pin2, t2.name, other.name)
            self.connections[pin] = (other, pin2)
            other.connections[pin2] = (self, pin)

    def portmap (self):
        """Return a component invocation (port map) mapping the
        connected pins of this module instance to the wire signal names.
        """
        clist = [ ]
        if "clk" in self.Type.pins:
            clist.append ("    clk => clk")
        plist = self.connections.keys ()
        plist.sort ()
        for pin in plist:
            other, pin2 = self.connections[pin]
            wire = wirename (self, pin, other, pin2)
            clist.append ("    %s => %s" % (pin, wire))
        return "  %s : %s port map (\n%s);\n" % (self.name, self.Type.name, ",\n".join (clist))
    
def process_modules (l):
    """Process a line in the modules section.  
    """
    fields = l.split ()
    type_name = fields[0]
    mod_type = current_chassis.add_module_type (type_name)
    for i in fields[1:]:
        current_chassis.add_module (mod_type, i)

def process_wires (l):
    """Process a line in the wires section.
    """
    fields = l.split ()
    mod1 = current_chassis.find_module (fields[0])
    pin1 = fields[1]
    mod2 = current_chassis.find_module (fields[2])
    pin2 = fields[3]
    if not pin1.startswith ("p"):
        pin1 = "p" + pin1
    if not pin2.startswith ("p"):
        pin2 = "p" + pin2
    mod1.connect (pin1, mod2, pin2)

def skip (l):
    pass

def process_chassis (l):
    cnum = int (l.split ()[1])
    if cnum < 1 or cnum > 16:
        raise Fail, "Chassis number %d out of range" % cnum
    global current_chassis, chassis_list
    if chassis_list[cnum]:
        raise Fail, "Chassis %d already defined" % cnum
    chassis_list[cnum] = current_chassis = Chassis (cnum)

def process_coax (l):
    """Process a line in the coax section.
    """
    fields = l.split ()
    current_chassis.add_coax (int (fields[0]), fields[1], fields[2])

def process_list (name):
    f = open (name, "r")
    action = skip
    for l in f:
        comment = l.find ("#")
        if comment >= 0:
            l = l[:comment]
        l = l.strip ().lower ()
        if not l:
            continue
        try:
            if l.startswith ("chassis "):
                process_chassis (l)
            elif l == "modules":
                action = process_modules
            elif l == "coax":
                action = process_coax
            elif l == "wires":
                action = process_wires
            else:
                action (l)
        except Fail, msg:
            print msg
            print " ", l
            action = skip
        except Warn, msg:
            print msg
            print " ", l

if __name__ == "__main__":
    if len (sys.argv) < 2:
        print "usage: %s wirelist [ wirelist ... ]" % sys.argv[0]
        sys.exit (1)
    for f in sys.argv[1:]:
        process_list (f)
    f = open ("cdc6600.vhd", "w")
    top_vhdl (f)
    f.close ()
    for ch in chassis_list:
        if ch is None:
            continue
        f = open ("%s.vhd" % ch.name, "w")
        ch.print_vhdl (f)
        f.close ()
