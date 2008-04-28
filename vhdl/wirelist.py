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

portpat = re.compile (r"\s*([\w\s,]+):\s*(in|out|inout)\s+(std_logic|coaxsig|misc)\s*(?:\:=\s'.')?(\))?\s*;", re.I)

class Fail (Exception):
    pass
class Warn (Exception):
    pass

curline = ""
linenum = 0

def getline (f):
    global curline, linenum
    linenum += 1
    curline = f.readline ().strip (" \t\014\n").lower ()
    #print curline
    return curline

class Chassis (object):
    """An instance of a 6000 chassis.
    """
    def __init__ (self, num):
        self.num = num
        self.name = "chassis%d" % num
        self.module_types = { }
        self.modules = { }
        self.coax = { }
        
    def add_module_type (self, tname):
        """Add a module type by type name, returning the ModuleType
        object.  If it was added before, return that one.
        """
        try:
            return self.module_types[tname]
        except KeyError:
            self.module_types[tname] = mod = ModuleType (tname)
            return mod

    def add_module (self, tname, name):
        """Add a module instance for slot 'name'.  Error if that slot
        already has something in it.  Mtype is the module type object.
        """
        try:
            mi = self.modules[name]
            #mod_type = self.add_module_type (tname)
            #mi.settype (tname, mod_type)
        except KeyError:
            mi = self.modules[name] = ModuleInstance (name, tname)
        return mi

    def find_module (self, name):
        """Find the module instance for slot 'name'. 
        """
        try:
            return self.modules[name]
        except KeyError:
            return self.add_module ("unknown", name)
        
    def ports (self):
        """Return the ports definition of the chassis, as a string.
        """
        portlist = [ "      clk : in std_logic" ]
        #for c in self.coax:
        #    portlist.append (self.port (c))
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
            raise Warn, "Pin number %s out of range" % pin
        if pin in self.connections:
            raise Warn, "Coax %s pin %s already connected" % (self.name, pin)
        if pin2 in other.connections:
            raise Warn, "Pin %s already connected in %s module at %s" % (pin2, t2.name, other.name)
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
            raise Warn, "Error opening %s.vhd" % name
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
            raise Warn, "Entity %s missing" % name
        for l in v:
            m = portpat.match (l)
            if not m:
                raise Warn, "Entity %s missing or port declaration not formatted right" % name
                return
            pins = m.group (1).lower ()
            dir = m.group (2).lower ()
            stype = m.group (3).lower ()
            if stype != "misc" and dir not in ("in", "out"):
                raise Warn, "Unrecognized pin direction %s for %s pin %s" \
                      % (dir, name, pins)
            for pin in pins.split (","):
                pin = pin.strip ()
                if pin in self.pins:
                    print "duplicate pin", pin
                else:
                    self.pins[pin] = (dir, stype)
            if m.group (4):
                break

    def port (self, pin):
        """Return the port definition for the specified pin,
        without the ; delimiter.
        """
        dir, stype = self.pins[pin]
        return "      %s : %s %s" % (pin, dir, stype)
    
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
                if self.pins[p] == d:
                    portlist.append (self.port (p))
        return "    port (\n%s);\n" % ";\n".join (portlist)

class ModuleInstance (object):
    """An instance of a ModuleType (living in some slot in some chassis)
    """
    def __init__ (self, name, tname):
        """name is the slot name.
        """
        self.name = name
        self.tname = tname
        self.Type = None
        self.connections = { }

    def settype (self, tname, mod_type):
        if self.Type:
            raise Warn, "Module %s type already set" % self.name
        self.tname = tname
        self.Type = mod_type
        
    def connect (self, pin, other, pin2, wlen):
        if pin2.startswith ("p"):
            pin2 = pin2[1:]
        t1 = self.Type
        if t1:
            try:
                dir1, stype1 = t1.pins[pin]
            except KeyError:
                raise Warn, "No pin %s in %s module at %s" % (pin, t1.name, self.name)
        else:
            dir1 = "inout"
            stype1 = "unknown"
        t2 = other.Type
        if t2:
            try:
                dir2, stype2 = t2.pins[pin2]
            except KeyError:
                raise Warn, "No pin %s in %s module at %s" % (pin2, t2.name, other.name)
        else:
            dir2 = "inout"
            stype2 = stype1
        if stype1 != stype2 and stype1 != "unknown":
            raise Warn, "Error: type mismatch module at %s pin %s" % \
                  (self.name, pin)
        if dir1 != "inout" and dir2 != "inout" and dir1 == dir2:
            raise Warn, "Error: %s to %s" % (dir1, dir2)
        else:
            try:
                cc = self.connections[pin]
                if cc[:2] != (other, pin2):
                    print linenum, curline
                    print "Pin %s already connected (%s, %s, %s) in module at %s" % (pin, cc[0].name, cc[1], cc[2], self.name)
                elif False and \
                         cc[2] != wlen and \
                         not (cc[2].endswith ("x") and wlen.endswith ("x")):
                    print "module %s pin %s to module %s pin %s length mismatch (%s vs. %s)" % \
                          (self.name, pin, other.name, pin2, wlen, cc[2])
                    print "module %s pin %s to module %s pin %s length mismatch (%s vs. %s)" % \
                          (other.name, pin2, self.name, pin, cc[2], wlen)
            except KeyError:
                pass
            try:
                cc = other.connections[pin2]
                if cc[:2] != (self, pin):
                    print linenum, curline
                    print "Pin %s already connected (%s, %s, %s) in module at %s" % (pin2, cc[0].name, cc[1], cc[2], other.name)
            except KeyError:
                pass
            self.connections[pin] = (other, pin2, wlen)
            other.connections[pin2] = (self, pin, wlen)

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
    
def pins (f, mod1, count):
    """Process the connector pin list.  There are 'count' pins.
    """
    current_chassis = chassis_list[curch]
    for p in xrange (count):
        l = getline (f)
        fields = l.split ()
        pin1 = fields[0]
        if int (pin1) != p + 1:
            raise Warn, "Pin %d out of sequence" % pin1
        if count == 30 and fields[1] in ("cb1", "cb2", "cb3", "+6"):
            # Memory power pins, ignore
            continue
        if len (fields) == 3:
            # Coax style connection
            cable = fields[1]
            pin2 = fields[2]
            p = int (pin2)
            if p < 90 or (p > 99 and p < 900) or p > 908:
                raise Warn, "Invalid coax pin %s" % pin2
            # Add coax wire
        elif len (fields) > 1:
            pin2 = fields[2]
            wlen = fields[3]
            if pin2 == "x":
                # "x" means ground -- logic one
                if fields[1] != mod1.name:
                    print"'x' connection to %s, expected %s" \
                              % (fields[1], mod1.name)
                # do something
            else:
                mod2 = current_chassis.find_module (fields[1])
                mod1.connect (pin1, mod2, pin2, wlen)

slotpat = re.compile (r"(\d+)(\w+)", re.I)
def process_file (f):
    global curch
    curch = None
    while True:
        l = getline (f)
        if not l:
            break
        try:
            mt, slot = l.split ()
            m = slotpat.match (slot)
            if not m:
                raise Warn, "Invalid slot ID %s" % slot
            ch = int (m.group (1))
            if curch:
                if ch != curch:
                    raise Warn, "Chassis mismatch slot %s" % slot
            else:
                if ch < 1 or ch > 16:
                    raise Fail, "Chassis number %d out of range" % cnum
                if not chassis_list[ch]:
                    chassis_list[ch] = Chassis (ch)
                curch = ch
            slot = m.group (2)
            module = chassis_list[ch].add_module (mt, slot)
            if mt == "mem":
                pins (f, module, 30)
                slot2 = getline (f)
                m = slotpat.match (slot2)
                if not m:
                    raise Warn, "Invalid slot ID %s" % slot2
                slot2 = m.group (2)
                conn2 = chassis_list[ch].add_module ("mem", slot2)
                pins (f, conn2, 30) # ??? TODO
            else:
                pins (f, module, 28)
        except Fail, msg:
            print msg
            print linenum, curline
            failure = traceback.format_exception (sys.exc_type,
                                                  sys.exc_value,
                                                  sys.exc_traceback)
            #print "".join (failure)
        except Warn, msg:
            print msg
            print linenum, curline
            failure = traceback.format_exception (sys.exc_type,
                                                  sys.exc_value,
                                                  sys.exc_traceback)
            #print "".join (failure)
        except:
            print "unexpected error in", linenum, curline
            failure = traceback.format_exception (sys.exc_type,
                                                  sys.exc_value,
                                                  sys.exc_traceback)
            print "".join (failure)
            exit(1)

def process_list (name):
    f = open (name, "r")
    global linenum
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
