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

portpat = re.compile (r"\s*([\w\s,]+):\s*(in|out|inout)\s+(std_logic|coaxsig|analog|misc)\s*(?:\:=\s'.')?(\))?\s*;", re.I)
slotpat = re.compile (r"([a-r])(\d\d?)", re.I)
chslotpat = re.compile (r"(\d\d?)([a-r])(\d\d?)", re.I)
vhdlcommentpat = re.compile (r"--.*$")

curfile = ""
curline = ""
linenum = 0

commentpat = re.compile (r"#.*$")
def getline (f):
    global curline, linenum
    while True:
        linenum += 1
        curline = f.readline ()
        if not curline:
            return
        curline = commentpat.sub ("", curline).strip (" \t\014\n").lower ()
        #print curline
        if curline:
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
    The endpoints are given by the elements of the "ends" list.  Each
    element is a pair of slot and pin number.  The slot is a
    ModuleInstance object.
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

    def endids (self):
        """Returns a list of endpoints.  Each endpoint is a pair of
        chassis/slot ID and pin number.  The chassis/slot ID is a pair
        of chassis number and slot name.
        """
        d = [ ]
        for c, p in self.ends:
            d.append ((c.chslotid (), p))
        return d

    def wiretype (self, verbose = True):
        ids = self.endids ()
        if len (self.ends) != 2:
            if verbose:
                error ("Half-connected wire %s (only at %s %d)" %
                       (self.name, ids[0][0][1], ids[0][1]))
            return None
        d1, d2 = self.pindefs ()
        dir1, stype1 = d1
        dir2, stype2 = d2
        if stype1 != stype2:
            if verbose:
                error ("Endpoint type mismatch on wire %s (%s vs. %s)" %
                   (self.name, stype1, stype2))
            return None
        if dir1 == "inout" or dir2 == "inout":
            if stype1 != "misc":
                if verbose:
                    error ("inout pin for wrong signal type, wire %s" % self.name)
                return None
        else:
            if dir1 == dir2:
                if verbose:
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
            mi = self.modules[name] = ModuleInstance (self.num, name, tname)
        return mi

    def portlist (self):
        """Return the port defintions of the chassis, as a list of
        tuples.  Each tuple consists of signal name, direction
        name, and type name.
        """
        portlist = [ ]
        for clk in ("clk1", "clk2", "clk3" , "clk4"):
            portlist.append ((clk, "in", "std_logic"))
        wnames = self.coax.keys ()
        wnames.sort ()
        for w in wnames:
            wire = self.coax[w]
            ends = wire.endids ()
            if len (ends) == 2:
                e1, e2 = ends
                if e1[0][0] == e2[0][0]:
                    # Same chassis (which has to be this chassis)
                    # at both ends -- don't put this coax wire into ports.
                    continue
                wt = wire.wiretype ()  # Check for consistency
                if not wt:
                    continue
            c, p = wire.ends[0]
            if ends[0][0][0] != self.num:
                # first endpoint is the other chassis
                c, p = wire.ends[1]
            wt = c.pindef (p)
            dir, stype = wt
            #if stype == "coaxsig" or stype == "analog":
            portlist.append ((w, dir, stype))
            #else:
            #    error ("coax %s unexpected type %s" % (w, stype))
        return portlist
        
    def ports (self):
        """Return the ports definition of the chassis, as a string.
        Coax cables are included unless they are in-chassis, i.e.,
        the same chassis number for both endpoints.
        """
        pl = [ ]
        for p in self.portlist ():
            pl.append ("      %s : %s %s" % p)
        if pl:
            return "    port (\n%s);\n" % ";\n".join (pl)
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
        Coax cables are included if they are in-chassis, i.e., the
        same chassis number for both endpoints.
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
        wnames = self.coax.keys ()
        wnames.sort ()
        for w in wnames:
            wire = self.coax[w]
            ends = wire.endids ()
            if len (ends) == 2:
                e1, e2 = ends
                if e1[0][0] != e2[0][0]:
                    # Different chassis, so it goes into ports not signals
                    continue
                wt = wire.wiretype ()  # Check for consistency
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
        print >> f, "  signal zero : std_logic := '0';"
        print >> f, "  signal one : std_logic := '1';"
        print >> f, self.wirelist ()
        print >> f, "begin -- modules"
        print >> f, "  zero <= '0';"
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
    for ch in chassis_list:
        if ch is None:
            continue
    print >> f, header
    print >> f, "entity cdc6600 is\n  port ("
    clist = [ "    clk1, clk2, clk3, clk4 : in std_logic" ]
    wnames = coaxdict.keys ()
    wnames.sort ()
    for w in wnames:
        wire = coaxdict[w]
        ends = wire.endids ()
        if len (ends) == 1:
            # Half-connected coax, so it goes to the outside world
            c, p = wire.ends[0]
            wt = c.pindef (p)
            dir, stype = wt
            if stype == "coaxsig" or stype == "analog":
                clist.append ("    %s : %s %s" % (w, dir, stype))
    print >> f, "%s);\nend cdc6600;\n" % ";\n".join (clist)
    print >> f, "\narchitecture chassis of cdc6600 is"
    for ch in chassis_list:
        if ch is None:
            continue
        print >> f, "  component %s\n%s  end component;" % (ch.name, ch.ports ())
    for w in wnames:
        wire = coaxdict[w]
        wt = wire.wiretype ()  # Check for consistency
        if not wt:
            continue
        ends = wire.endids ()
        e1, e2 = ends
        if e1[0][0] == e2[0][0]:
            # Same chassis (which has to be this chassis)
            # at both ends -- don't put this coax wire into top level signals
                continue
        c, p = wire.ends[0]
        wt = c.pindef (p)
        dir, stype = wt
        print >> f, "  signal %s : %s;" % (w, stype)
    print >> f, "begin -- chassis"
    for ch in chassis_list:
        if ch is None:
            continue
        print >> f, "  ch%d : %s port map (" % (int (ch.name[7:]), ch.name)
        pl = ch.portlist ()
        clist = [ ]
        for p, d, t in pl:
            clist.append ("    %s => %s" % (p, p))
        print >> f, "%s);" % ",\n".join (clist)
    print >> f, "end chassis;"

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
            l = vhdlcommentpat.sub ("", l).strip ().lower ()
            if not l:
                continue
            if name == "pq":
                print l
            m = portpat.match (l)
            if not m:
                error ("Entity %s missing or port declaration not formatted right" % name)
                return
            pins = m.group (1)
            dir = m.group (2)
            stype = m.group (3)
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
            if dest == "good" or dest == "gnd" or dest == "grd":
                dest = self.name
            pin2 = fields[2]
            if pin2.startswith ("p"):
                pin2 = pin2[1:]
            wlen = 0
            if len (fields) > 3:
                wlen = int (fields[3])
            dslot = get_slot (dest)
            if dslot:
                # Normalize the name
                dest = slotname (dslot)
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
        try:
            return self.module.Type.pins[pname]
        except KeyError:
            print pname, self.name, self.module.tname
            raise

    def chslotid (self):
        return self.module.chslotid ()
    
class ModuleInstance (object):
    """An instance of a ModuleType (living in some slot in some chassis)
    """
    def __init__ (self, ch, name, tname):
        """name is the slot name.
        """
        self.chassis = ch
        self.name = name
        self.tname = tname
        self.Type = get_module_type (tname)
        self.connectors = [ ]

    def add_connector (self, c):
        self.connectors.append (c)

    def chslotid (self):
        return (self.chassis, self.name)
    
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
                if p.startswith ("clk"):
                    clist.append ("    %s => %s" % (p, p))
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
                        elif w.wiretype (False):
                            # It's a valid wire, add it
                            clist.append ("    %s => %s" % (p, w.name))
                        elif dir == "in":
                            # TEMP: tie half-connected inputs to idle
                            # if they are twisted pair, to outside if coax
                            if stype == "coaxsig" or stype == "analog":
                                clist.append ("    %s => %s" % (p, w.name))
                            else:
                                clist.append ("    %s => one" % p)
                        elif dir == "out" and \
                             (stype == "coaxsig" or stype == "analog"):
                            # TEMP: hook up coax (to the outside)
                            clist.append ("    %s => %s" % (p, w.name))
                    elif dir == "in":
                        error ("Unconnected input pin %s in %s" %
                               (p, self.name))
                        # TEMP: tie unconnected inputs to idle
                        if stype == "coaxsig" or stype == "analog":
                            clist.append ("    %s => zero" % p)
                        else:
                            clist.append ("    %s => one" % p)
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
