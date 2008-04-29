#!/usr/bin/env python

"""Process CDC 6600 wiring list

By Paul Koning and Dave Redell
"""

import re
import sys
import traceback

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
curch = None
curcnum = None
ground = "ground"

module_types = { }
coaxdict = { }

portpat = re.compile (r"\s*([\w\s,]+):\s*(in|out|inout)\s+(std_logic|coaxsig|misc)\s*(?:\:=\s'.')?(\))?\s*;", re.I)
slotpat = re.compile (r"([a-r])(\d\d?)", re.I)
chslotpat = re.compile (r"(\d\d?)([a-r])(\d\d?)", re.I)

curfile = ""
curline = ""
linenum = 0

def getline (f):
    global curline, linenum
    linenum += 1
    curline = f.readline ().strip (" \t\014\n").lower ()
    #print curline
    return curline

def error (text):
    if curline:
        print "%s:%d: %s\n%s" % (curfile, linenum, text, curline)
    else:
        print text
        
def get_slot (name):
    m = slotpat.match (name)
    if m:
        return curcnum, m.group (1), int (m.group (2))
    return None

def slotname (slot):
    return "%s%d" % (slot[1], slot[2])

def get_chslot (name):
    m = chslotpat.match (name)
    if m:
        return int (m.group (1)), m.group (2), int (m.group (3))
    return None

def chslotname (slot):
    return "%d%s%d" % slot

def get_wire (name, slot, pin):
    to = (slot, pin)
    try:
        w = curch.wires[name]
        w.connect (to)
    except KeyError:
        w = curch.wires[name] = Wire (name, to)
    return w

def get_coax (name, slot, pin):
    to = (slot, pin)
    try:
        w = coaxdict[name]
        w.connect (to)
    except KeyError:
        w = coaxdict[name] = Wire (name, to)
    curch.coax[name] = w
    return w

def get_module_type (tname):
    """Retrieve a module type by type name, returning the ModuleType
    object.  If it wasn't added before, add it.
    """
    global module_types
    try:
        return module_types[tname]
    except KeyError:
        module_types[tname] = mod = ModuleType (tname)
        return mod

class Wire (object):
    """An instance of a wire (connection between two connector pins).
    """
    def __init__ (self, name, end1):
        self.name = name
        self.ends = [ ]
        self.connect (end1)

    def connect (self, end1):
        if len (self.ends) > 1:
            error ("Wire %s already has two connections" % self.name)
        else:
            self.ends.append (end1)

    def pindefs (self):
        """Return a list of pindefs for the endpoints.
        """
        d = [ ]
        for c, p in self.ends:
            d.append (c.pindef (p))
        return d

    def wiretype (self):
        if len (self.ends) != 2:
            error ("Half-connected wire %s" % self.name)
            return None
        d1, d2 = self.pindefs ()
        dir1, stype1 = d1
        dir2, stype2 = d2
        if stype1 != stype2:
            error ("Endpoint type mismatch on wire %s (%s vs. %s)" %
                   (self.name, stype1, stype2))
            return None
        if d1 == "inout" or d2 == "inout":
            if stype1 != "misc":
                error ("inout pin for wrong signal type, wire %s" % self.name)
                return None
        else:
            if d1 == d2:
                error ("wire %s not matched in to out" % self.name)
                return None
        return dir1, stype1

class Chassis (object):
    """An instance of a 6000 chassis.
    """
    def __init__ (self, num):
        self.num = num
        self.name = "chassis%d" % num
        self.modules = { }
        self.wires = { }
        self.coax = { }
        
    def add_module (self, tname, name):
        """Add a module instance for slot 'name'.  Error if that slot
        already has something in it.  Mtype is the module type object.
        """
        if name in self.modules:
            error ("Slot %s already assigned" % name)
            mi = self.modules[name]
        else:
            mi = self.modules[name] = ModuleInstance (name, tname)
        return mi

    def ports (self):
        """Return the ports definition of the chassis, as a string.
        """
        portlist = [ "      clk : in std_logic" ]
        wnames = self.coax.keys ()
        wnames.sort ()
        for w in wnames:
            wire = self.coax[w]
            wt = wire.wiretype ()
            if wt:
                dir, stype = wt
                if stype == "coaxsig":
                    portlist.append ("      %s : %s coaxsig" % (w, dir))
                else:
                    error ("coax %s unexpected type %s" % (w, stype))
        if portlist:
            return "    port (\n%s);\n" % ";\n".join (portlist)
        return ""

    def component_decls (self):
        """Return the component declarations for this chassis
        (i.e., for the module types it contains), as a string.
        """
        clist = [ ]
        mtypes = { }
        for m in self.modules.itervalues ():
            mtypes[m.Type.name] = m.Type
        mlist = mtypes.keys ()
        mlist.sort ()
        for m in mlist:
            clist.append ("  component %s\n%s  end component;\n" % (m, mtypes[m].ports ()))
        return "".join (clist)

    def wirelist (self):
        """Return the list of wires for this chassis, as signal
        declarations.  The signal names look like m32_1_q34_17 for
        the wire from pin 1 of the module in slot m32 to pin 17 of
        the module in slot q34.  The left half of the name is less
        than the right half, i.e., lower slot, or lower pin if both
        ends are the same slot.
        """
        wlist = [ ]
        wnames = self.wires.keys ()
        wnames.sort ()
        for w in wnames:
            wire = self.wires[w]
            wt = wire.wiretype ()
            if wt:
                dir, stype = wt
                wlist.append ("  signal %s : %s;\n" % (w, stype))
        return "".join (wlist)
    
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
        print >> f, "  signal one : std_logic := '1';"
        print >> f, self.wirelist ()
        print >> f, "begin -- modules"
        print >> f, "  one <= '1';"
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
        #for c in ch.coax:
        #    n = c.coaxname ()
        #    if c.to[0].startswith ("chassis"):
        #        cables[n] = 1
        #    else:
        #        cables[n] = 2
    cnames = cables.keys ()
    cnames.sort ()
    print >> f, header
    print >> f, "entity cdc6600 is\n  port (\n    clk : in std_logic;"
    clist = [ ]
    #for c in cnames:
    #    n = cables[c]
    #    if n == 2:
    #        clist.append ("    %s : inout coaxsigs" % c)
    print >> f, "%s);\nend cdc6600;\n" % ";\n".join (clist)
    print >> f, "\narchitecture chassis of cdc6600 is"
    for ch in chassis_list:
        if ch is None:
            continue
        print >> f, "  component %s\n%s  end component;" % (ch.name, ch.ports ())
    #for c in cnames:
    #    n = cables[c]
    #    if n == 1:
    #        print >> f, "  signal %s : inout coaxsigs;" % n
    print >> f, "begin -- chassis"
    for ch in chassis_list:
        if ch is None:
            continue
        print >> f, "  ch%d : %s port map (" % (int (ch.name[7:]), ch.name)
        clist = [ "    clk => clk" ]
        #for c in ch.coax:
        #    n = c.coaxname ()
        #    clist.append ("    %s => %s" % (c.name, n))
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
    def __init__ (self, name, to_chassis, to_num):
        self.name = name
        self.to = (to_chassis, to_num)
        self.Type = coaxType
        self.connections = { }

    def connect (self, pin, other, pin2):
        num = pinnum (pin)
        if num >= 90 and num <= 99:
            num -= 89
        elif num >= 900 and num <= 908:
            num = (num - 900) + 10
        else:
            error ("Pin number %s out of range" % pin)
        if pin in self.connections:
            error ("Coax %s pin %s already connected" % (self.name, pin))
        if pin2 in other.connections:
            error ("Pin %s already connected in %s module at %s" % (pin2, t2.name, other.name))
        self.connections[num] = (other, pin2)
        other.connections[num] = (self, pin)
        
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
    above 200.  Non-standard names are handled as -1000000+radix36(name).
    """
    if p.startswith ("tp"):
        return int (p[2:]) + 200
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
            error ("Error opening %s.vhd" % name)
        entpat = re.compile ("entity %s is" % name, re.I)
        ent = False
        while True:
            l = v.readline ()
            if l and not entpat.search (l):
                continue
            break
        while True:
            l = v.readline ()
            if l and not l.strip ().startswith ("port"):
                continue
            break
        if not l:
            error ("Entity %s missing" % name)
        for l in v:
            m = portpat.match (l)
            if not m:
                error ("Entity %s missing or port declaration not formatted right" % name)
                return
            pins = m.group (1).lower ()
            dir = m.group (2).lower ()
            stype = m.group (3).lower ()
            if stype != "misc" and dir not in ("in", "out"):
                error ("Unrecognized pin direction %s for %s pin %s" 
                      % (dir, name, pins))
            for pin in pins.split (","):
                pin = pin.strip ()
                if pin in self.pins:
                    print "duplicate pin", pin
                else:
                    self.pins[pin] = (dir, stype)
            if m.group (4):
                break

    def ports (self):
        """Return the ports definition of the chassis, as a string.
        They are returned grouped in, inout, out, and in numeric order
        within the group.
        """
        portlist = [ ]
        pins = self.pins.keys ()
        pins.sort (pincmp)
        for d in ("in", "inout", "out"):
            for p in pins:
                pdir, stype = self.pins[p]
                if pdir == d:
                    portlist.append ("      %s : %s %s" % (p, pdir, stype))
        return "    port (\n%s);\n" % ";\n".join (portlist)

none30 = [ None ]  * 31
class Connector (object):
    """An instance of a 30-pin connector.
    """
    def __init__ (self, slot, module, offset = 0):
        self.name = slot
        self.slotid = get_slot (slot)
        self.pins = list (none30)
        self.module = module
        self.offset = offset

    def read_pins (self, f, count):
        """Process the connector pin list.  There are 'count' pins.
        """
        for p in xrange (1, count + 1):
            l = getline (f)
            fields = l.split ()
            pin1 = fields[0]
            if int (pin1) != p:
                error ("Pin %s out of sequence" % pin1)
            if len (fields) == 1:
                # Unconnected pin, skip
                continue
            if count == 30 and fields[1] in ("cb1", "cb2", "cb3", "+6"):
                # Memory power pins, ignore
                continue
            dest = fields[1]
            pin2 = fields[2]
            if pin2.startswith ("p"):
                pin2 = pin2[1:]
            wlen = 0
            if len (fields) > 3:
                wlen = int (fields[3])
            dslot = get_slot (dest)
            if dslot:
                # Destination looks like a slot ID.
                if pin2 == "x":
                    # connected to ground, mark that
                    self.pins[p] = ground
                    continue
                else:
                    if (self.slotid, p) < (dslot, int (pin2)):
                        wname = "%s_%d_%s_%s" % (self.name, p, dest, pin2)
                    else:
                        wname = "%s_%s_%s_%d" % (dest, pin2, self.name, p)
                    w = get_wire (wname, self, p)
            else:
                wname = "%s_%s" % (dest, pin2)
                w = get_coax (wname, self, p)
            self.pins[p] = w
            
    def pindef (self, num):
        """Get the definition of the pin, from the ModuleType object.
        """
        pname = "p%d" % (num + self.offset)
        return self.module.Type.pins[pname]
    
class ModuleInstance (object):
    """An instance of a ModuleType (living in some slot in some chassis)
    """
    def __init__ (self, name, tname):
        """name is the slot name.
        """
        self.name = name
        self.tname = tname
        self.Type = get_module_type (tname)
        self.connectors = [ ]

    def add_connector (self, c):
        self.connectors.append (c)
        
    def portmap (self):
        """Return a component invocation (port map) mapping the
        connected pins of this module instance to the wire signal names.
        """
        clist = [ ]
        plist = self.Type.pins.keys ()
        plist.sort ()
        for dir in ("in", "out", "inout"):
            for p in plist:
                pdir, stype = self.Type.pins[p]
                if pdir != dir:
                    continue
                if p == "clk":
                    clist.append ("    clk => clk")
                else:
                    if p.startswith ("tp"):
                        # test point
                        continue
                    pnum = int (p[1:])
                    cnum, pnum = divmod (pnum, 100)
                    c = self.connectors[cnum]
                    w = c.pins[pnum]
                    if w:
                        if w is ground:
                            clist.append ("    %s => one" % p)
                        elif w.wiretype ():
                            # It's a valid wire, add it
                            clist.append ("    %s => %s" % (p, w.name))
                    elif dir == "in":
                        error ("Unconnected input pin %s in %s" %
                               (p, self.name))
        return "  %s : %s port map (\n%s);\n" % (self.name, self.Type.name, ",\n".join (clist))
    
def process_file (f):
    global curch, curcnum
    curch = curcnum = None
    while True:
        l = getline (f)
        if not l:
            break
        try:
            mt, slot = l.split ()
            slotid = get_chslot (slot)
            if not slotid:
                error ("Invalid slot ID %s" % slot)
            ch = slotid[0]
            if curcnum:
                if ch != curcnum:
                    error ("Chassis mismatch slot %s" % slot)
            else:
                if ch < 1 or ch > 16:
                    error ("Chassis number %d out of range" % ch)
                if not chassis_list[ch]:
                    chassis_list[ch] = Chassis (ch)
                curcnum = ch
                curch = chassis_list[ch]
            slot = slotname (slotid)
            module = curch.add_module (mt, slot)
            c = Connector (slot, module)
            module.add_connector (c)
            if mt == "mem":
                c.read_pins (f, 30)
                slot2 = getline (f)
                slotid = get_chslot (slot2)
                if not slotid:
                    error ("Invalid slot ID %s" % slot2)
                slot2 = slotname (slotid)
                c = Connector (slot2, module, 100)
                module.add_connector (c)
                c.read_pins (f, 30)
            else:
                c.read_pins (f, 28)
        except:
            error ("unexpected error")
            failure = traceback.format_exception (sys.exc_type,
                                                  sys.exc_value,
                                                  sys.exc_traceback)
            print "".join (failure)
            exit(1)

def process_list (name):
    f = open (name, "r")
    global linenum, curfile
    curfile = name
    linenum = 0
    process_file (f)

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
