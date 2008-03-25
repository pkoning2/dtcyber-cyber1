#!/usr/bin/env python

"""Process CDC 6600 wiring list

By Paul Koning and Dave Redell
"""

import re
import sys

portpat = re.compile (r"\s*([\w\s,]+):\s*(in|out|inout)\s+std_logic\s*(\))?\s*;", re.I)


class ModuleType (object):
    def __init__ (self, name):
        """Create an instance of a module type object.
        This object holds the per-type data, which is primarily the
        port definitions.
        """
        self.pins = { }
        self.instances = { }
        self.name = name
        self.readdef (name)

    def readdef (self, name):
        """Read the <name>.vhdl file to find the entity declaration
        for this module.  Process its port definition and save away
        all the pin data.
        """
        v = open (name + ".vhdl", "r")
        entpat = re.compile ("entity %s is" % name, re.I)
        ent = False
        while not entpat.search (v.readline ()):
            pass
        while not v.readline ().strip ().startswith ("port"):
            pass
        for l in v:
            m = portpat.match (l)
            if not m:
                print "Entity", name, "missing or port declaration not formatted right"
                return
            pins = m.group (1).lower ()
            dir = m.group (2).lower ()
            for pin in pins.split (","):
                pin = pin.strip ()
                if pin in self.pins:
                    print "duplicate pin", pin
                else:
                    self.pins[pin] = dir
            if m.group (3):
                break

    def add (self, instance):
        self.instances[instance.name] = instance
        instance.Type = self

class ModuleInstance (object):
    """An instance of a ModuleType (living in some slot in some chassis)
    """
    def __init__ (self, name):
        """name is the slot name.
        """
        self.name = name
        self.connections = { }
        
    def connect (self, pin, other, pin2):
        t1 = self.Type
        t2 = other.Type
        dir1 = t1.pins[pin]
        dir2 = t2.pins[pin2]
        if dir1 != "inout" and dir2 != "inout" and dir1 == dir2:
            print "Error:", dir1, "to", dir2, self.name, pin, other.name, pin2
        else:
            self.connections[pin] = (other, pin2)
            other.connections[pin2] = (self, pin)

def process_modules (f):
    """Process the modules section.  Return the instances list.
    """
    module_types = { }
    instances = { }
    for l in f:
        comment = l.find ("#")
        if comment >= 0:
            l = l[:comment]
        l = l.strip ().lower ()
        if not l:
            continue
        if l.startswith ("end modules"):
            return module_types, instances
        fields = l.split ()
        type_name = fields[0]
        try:
            mod_type = module_types[type_name]
        except KeyError:
            module_types[type_name] = mod_type = ModuleType (type_name)
        for i in fields[1:]:
            mod = ModuleInstance (i)
            mod_type.add (mod)
            instances[i] = mod

def print_inventory (types):
    """Print the module inventory given the types dictionary.
    """
    tnames = types.keys ()
    tnames.sort ()
    for k in tnames:
        instances = types[k].instances
        k2 = instances.keys ()
        k2.sort ()
        print "%s" % k.upper ()
        print " ",
        for n in k2:
            print n,
        print
        
def process_wires (f, mods):
    for l in f:
        comment = l.find ("#")
        if comment >= 0:
            l = l[:comment]
        l = l.strip ().lower ()
        if not l:
            continue
        if l.startswith ("end wires"):
            return
        fields = l.split ()
        mod1 = mods[fields[0]]
        mod2 = mods[fields[2]]
        mod1.connect (fields[1], mod2, fields[3])

def print_wires (mods):
    """Print the wires list given the instances dictionary.
    """
    slots = mods.keys ()
    slots.sort ()
    for s in slots:
        conns = mods[s].connections
        pins = conns.keys ()
        pins.sort ()
        print s.upper ()
        for p in pins:
            other, opin = conns[p]
            if other.name < s or \
                   other.name == s and opin < pin:
                continue
            print " ", p, "to", other.name.upper (), opin
            
def process_list (name):
    f = open (name, "r")
    for l in f:
        comment = l.find ("#")
        if comment >= 0:
            l = l[:comment]
        l = l.strip ().lower ()
        if not l:
            continue
        if l.startswith ("chassis"):
            pass
        elif l.startswith ("modules"):
            mtypes, mods = process_modules (f)
        elif l.startswith ("coax"):
            pass
        elif l.startswith ("wires"):
            process_wires (f, mods)
    return mtypes, mods
