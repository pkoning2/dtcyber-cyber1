#!/usr/bin/env python3

import sys
import re
from collections.abc import Iterable

import wlist

_re_phase = re.compile ("t?(\d+)(b)?", re.I)

mods = dict ()

def S (slot):
    m2 = wlist._re_chslot.match (slot)
    x, row, col = m2.groups ()
    if col.startswith ('0'):
        col = col[1:]
    return "{}{}{}".format (cnum, row.upper (), col)

def TP (n):
    return "TP{}".format (n)

def T (phase, b):
    return "T{:0>2d}{}".format (phase % 100, "B" if b else "")

def wire_delay (l):
    return wlist.wire_delay (l + 20)

class Io:
    def __init__ (self, ipins, i2tp, tp, i2o, opins):
        self.ipins = ipins
        self.tp = tp
        self.tpinv = i2tp & 1
        self.tpdelay = 5 * i2tp
        self.ioinv = i2o & 1
        self.iodelay = 5 * i2o
        self.opins = tuple (sorted (opins))
        
class Module:
    def __init__ (self, name):
        self.pinset = set ()
        self.name = name
        mods[name] = self
        self.ipins = dict ()
        self.tps = dict ()
        
    def io (self, ipins, i2tp, tp, i2o, opins):
        assert tp not in self.tps
        if not isinstance (ipins, Iterable):
            ipins = [ ipins ]
        assert not (set (ipins) & self.pinset)
        self.pinset.update (ipins)
        if not isinstance (opins, Iterable):
            opins = [ opins ]
        assert not (set (opins) & self.pinset)
        self.pinset.update (opins)
        io = Io (ipins, i2tp, tp, i2o, opins)
        for ipin in ipins:
            self.ipins[ipin] = io
        self.tps[tp] = io

class Ioinst:
    def __init__ (self, ipin, phase, b, parent, ioinfo):
        self.ipin = ipin
        self.phase = phase
        self.b = b
        self.parent = parent
        self.info = ioinfo
        self.isclock = False
        
class Minst:
    def __init__ (self, slot, name):
        self.slot = slot
        self.mtype = mods[name]
        self.ipins = dict ()

    def addin (self, ipin, phase, b):
        info = self.mtype.ipins[ipin]
        self.ipins[ipin] = Ioinst (ipin, phase, b, self, info)
        
tc = Module ("tc")
tc.io ((3, 5), 2, 2, 3, (15, 7, 6, 4, 9, 11, 13, 1, 8, 10, 12, 2))
tc.io ((14, 16), 2, 6, 3, (25, 23, 26, 24, 22, 20, 18, 28, 21, 19, 17, 27))
hq = Module ("hq")
hq.io (12, 2, 1, 2, (14,))
hq.io (10, 0, 0, 0, ())
hq.io ((11, 16), 1, 2, 2, (2, 4, 6, 8))
hq.io ((13, 18), 1, 3, 2, (1, 3, 5, 7))
hq.io ((15, 17), 1, 5, 2, (22, 24, 26, 28))
hq.io ((19, 20), 1, 6, 2, (21, 23, 25, 27))
td = Module ("td")
td.io ((1, 3), 1, 1, 2, (6, 8, 10, 12, 14))
td.io ((2, 4), 1, 2, 2, (5, 7, 9, 11, 13))
td.io ((25, 27), 1, 5, 2, (16, 18, 20, 22, 24))
td.io ((26, 28), 1, 6, 2, (15, 17, 19, 21, 23))
tp = Module ("tp")
tp.io (1, 1, 1, 3, (11, 9, 7, 5, 3))
tp.io (14, 2, 3, 3, (2, 4, 6, 8, 10, 12))
tp.io ((13, 15), 1, 4, 3, (17, 19, 21, 23, 25))
tp.io ((27, 28), 2, 6, 3, (16, 18, 20, 22, 24, 26))
th = Module ("th")
th.io (1, 1, 1, 2, (14, 8, 7))
th.io (6, 1, 2, 2, (5, 9, 11, 13))
th.io (3, 1, 3, 2, (2, 12, 10, 4))
th.io (26, 1, 4, 2, (23, 19, 17, 27))
th.io (25, 1, 5, 2, (16, 18, 20, 24))
th.io (28, 1, 6, 2, (22, 21, 15))
ti = Module ("ti")
ti.io ((1, 3, 5, 7), 1, 1, 1, 13)
ti.io ((2, 4, 6, 8), 1, 2, 1, 9)
ti.io ((10, 12, 14), 1, 3, 1, 11)
ti.io ((15, 17, 19), 1, 4, 1, 16)
ti.io ((21, 23, 25, 27), 1, 5, 1, 18)
ti.io ((22, 24, 26, 28), 1, 6, 1, 20)

fn = sys.argv[1]

header, text = wlist.readfile (fn)
m = wlist._re_wmod.search (text)
m2 = wlist._re_chslot.match (m.groupdict ()["slot"])
cnum = int (m2.group (1))
ch = wlist.parse_chassis (cnum, text)

result = list ()

slot = wlist.normslot (cnum, input ("Start slot: "))
slotdata = ch[slot]
pin = int (input ("Start pin: "))
phase = int (input ("Start phase: "))

top = Minst (slot, slotdata.modtype)
top.addin (pin, phase, 0)
clocktree = { slot : top }

more = True
while more:
    more = False
    print ("examining clock tree")
    for m in list (clocktree.values ()):
        print ("looking at", m.slot)
        slotdata = ch[m.slot]
        slotpins = slotdata.pins
        for pnum, i in list (m.ipins.items ()):
            print ("looking at", m.slot, "input pin", pnum)
            info = i.info
            complete = True
            for i2 in info.ipins:
                if i2 not in m.ipins and slotpins[i2 - 1] is not None \
                  and slotpins[i2 - 1][1] != "x":
                    complete = False
            if complete and not i.isclock:
                # This I/O is now recognized as a clock
                i.isclock = True
                print (m.slot, "input pin", pnum, "is a clock")
                for o in info.opins:
                    print ("for", m.slot, "input pin", pnum, "output pin", o)
                    dest = slotpins[o - 1]
                    if dest:
                        dslot, dpin, wlen = dest
                        dslot = wlist.normslot (cnum, dslot)
                        dslotdata = ch[dslot]
                        dpin = int (dpin)
                        wlen = int (wlen)
                        wdelay = wire_delay (wlen) * 5
                        ophase = i.phase + info.iodelay + wdelay
                        ob = i.b ^ info.ioinv
                        try:
                            dmod = clocktree[dslot]
                        except KeyError:
                            mt = dslotdata.modtype
                            if mt in mods:
                                print ("new tree module", mt, "at", dslot)
                                dmod = clocktree[dslot] = Minst (dslot, mt)
                                more = True
                            else:
                                print ("leaf connection to", mt, "at", dslot)
                                continue
                        try:
                            dio = dmod.ipins[dpin]
                        except KeyError:
                            print ("new clock input", dpin, T (ophase, ob), "slot", dslot)
                            more = True
                            dmod.addin (dpin, ophase, ob)

# At this point, "clocktree" is a dictionary containing all the modules
# that participate in the clock distribution tree.  There are a couple
# of cases:
#
# 1. Intermediate module: all outputs go to other modules in the tree.
# 2. Leaf module: at least some outputs go to modules that are not
#    clock tree modules.
# a. Clock only modules: every section of this module is part of the
#    clock tree.
# b. Mixed modules: some sections of this module handle other (non-clock)
#    signals.
#
# The way we handle these:
# 1. Intermediate modules are ignored, since we are going to generate
#    final clocks direct to the clock users.
# 2. For leaf modules, we ask for the correct clock phase (since the
#    calculation here is only approximate) and generate the correction
#    line for that clock phase (adjusted for delays) for each output pin
#    that doesn't go to another clock module.
# a. Clock only modules are deleted.
# b. In mixed modules, the inputs for clock sections are disconnected
#    (which leaves that whole section unused since its outputs are no
#    longer used).

delmodules = list ()
delinputs = list ()
leafoutputs = list ()
for slot, m in sorted (clocktree.items ()):
    mi = m.mtype
    inputs = set ()
    clocksections = 0
    slotdata = ch[slot]
    slotpins = slotdata.pins
    # We want to visit each section of the module once.  We saw
    # individual inputs during the tree walk, which gives sections
    # multiple times for sections that have more than one input.  So
    # use the section definitions (Io objects) from the module
    # definition instead.
    tps = sorted (mi.tps.items ())
    for tpnum, tp in tps:
        pnum = tp.ipins[0]
        if tpnum and pnum in m.ipins:
            # Ok, this section is a clock section.
            i = m.ipins[pnum]
            if not i.isclock:
                continue
            clocksections += 1
            # Remember the input pin numbers so we can disconnect
            # them, if we don't delete the whole module.
            inputs.update (m.ipins)
            # Find the leaf outputs (ones that connect to places that
            # are not part of the clock tree)
            outputs = list ()
            for o in tp.opins:
                dest = slotpins[o - 1]
                if not dest:
                    continue
                dslot, dpin, wlen = dest
                dslot = wlist.normslot (cnum, dslot)
                dpin = int (dpin)
                if wlen:
                    wlen = int (wlen)
                else:
                    wlen = 0
                try:
                    dmod = clocktree[dslot]
                    try:
                        dsec = dmod.ipins[dpin]
                        if dsec.isclock:
                            continue
                    except KeyError:
                        pass
                except KeyError:
                    pass
                # If we get here, either the module isn't in the clock
                # tree, or the particular section this wire goes to
                # isn't.
                outputs.append ((dslot, dpin, wlen))
            # We've walked through the outputs. See if there are any
            # leaf outputs.
            if outputs:
                # There are, so we need to confirm the clock phase and
                # generate the correction statements.
                tpphase = i.phase + tp.tpdelay
                tpb = i.b ^ tp.tpinv
                while True:
                    ph = input ("{} {} {}: ".format (slot, TP (tpnum),
                                                     T (tpphase, tpb)))
                    if ph:
                        tm = _re_phase.match (ph)
                        if not tm:
                            print ("Bad input")
                            continue
                        tpphase, tpb = tm.groups ()
                        tpphase = int (tpphase)
                        tpb = int (bool (tpb))
                    break
                ophase = tpphase - tp.tpdelay + tp.iodelay
                ob = tpb ^ tp.tpinv ^ tp.ioinv
                c = "# {} {} {}".format (S (slot), TP (tpnum),
                                         T (tpphase, tpb))
                leafoutputs.append (c)
                #print (c)
                for dslot, dpin, wlen in outputs:
                    wldelay = wire_delay (wlen) * 5
                    c = "{}\t{}\t{}".format (S (dslot), dpin,
                                             T (ophase + wldelay, ob))
                    leafoutputs.append (c)
                    #print (c)
    # Done with this module.  See if it is all clock
    if clocksections == len (mi.tps):
        # Every section was clock, so delete this module
        delmodules.append ("{}\t--".format (S (slot)))
    else:
        # Disconnect the sections that are clock
        for i in inputs:
            delinputs.append ("{}\t{}\t--".format (S (slot), i))

with open ("chassis{}clocks.corr".format (cnum), "wt") as f:
    print ("# Clock signals, replacing clock tree for chassis", cnum, file = f)
    print ('\n'.join (leafoutputs), file = f)
    if delmodules:
        print ("# Delete unused clock tree modules", file = f)
        print ('\n'.join (delmodules), file = f)
    if delinputs:
        print ("# Disconnect unused clock tree inputs", file = f)
        print ('\n'.join (delinputs), file = f)
