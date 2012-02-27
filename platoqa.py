#!/usr/bin/env python

"""QA script for Plato

Paul Koning    2012.02.24.    Initial version.
"""

import sys
import random
import time
import traceback
import dtscript
import re

class QaOper (dtscript.Oper):
    def __init__ (self, idx):
        port = 5006
        if remote:
            port = 5106
        dtscript.Oper.__init__ (self, port)
        print "dtoper running"
        
class QaConsole (dtscript.Dd60):
    def __init__ (self, idx):
        port = 5007
        if remote:
            port = 5107
        dtscript.Dd60.__init__ (self, port, random.uniform (.1, 5.0))
        print "dd60 running"
        
class QaPterm (dtscript.Pterm):
    def __init__ (self, idx):
        port = 5004
        if remote:
            port = 5104
        dtscript.Pterm.__init__ (self, "localhost", port)
        self.login ("diag", "m")
        diagopts = "abcdefghijkmopq"
        diagopts = "aqnnnnnn"
        diagopt = diagopts[idx % len (diagopts)]
        self.sendstr ("a")
        self.waitarrow (5)
        self.sendstr ("k")
        self.waitarrow (5)
        self.sendstr (diagopt)
        print "pterm running diag option", diagopt

    def stop (self):
        #if self.isAlive ():
        #    self.logout ()
        dtscript.Pterm.stop (self)

remote = raw_input ("Test via tunnel? ").lower ()[:0] == 'y'
ocnt = int (raw_input ("Number of dtoper connections? "))
ccnt = int (raw_input ("Number of console connections? "))
pcnt = int (raw_input ("Number of pterm connections? "))

dd60conn = re.compile (r"DD6612.+? (\d+) connection", re.M)
niuconn = re.compile (r"NIU.+?(\d+) connection", re.M)
connre = re.compile (r"(\d+)\. (\w+)")

qathings = [ ]
try:
    try:
        for n, c in ((ocnt, QaOper), (ccnt, QaConsole), (pcnt, QaPterm)):
            qalist = [ ]
            for i in xrange (n):
                qalist.append (c (i))
            qathings.append (qalist)
        print "all started up"
        operlist = qathings[0]
        
        while True:
            time.sleep (2)
            item = random.randint (0, 250)
            for l in qathings:
                try:
                    q = l[item]
                    c = q.__class__
                    print "restarting", c.__name__, item
                    #r = repr (q)
                    #if r.replace ("\n", "").replace (" ", ""):
                    #    print r
                    q.stop ()
                    l[item] = c (item)
                    if operlist and operlist[0]:
                        op = operlist[0]
                        op.command ("SHOW,DEVICES.")
                        s = repr (op)
                        m1 = dd60conn.search (s)
                        m2 = niuconn.search (s)
                        cons = niu = 0
                        if m1:
                            cons = int (m1.group (1))
                        if m2:
                            niu = int (m2.group (1))
                        print "dev shows", op.operators, "op", \
                              cons, "dd60", niu, "niu connections"
                        op.command ("SHOW,CONNECTIONS.")
                        ops = cons = niu = 0
                        for m in connre.finditer (repr (op)):
                            n = int (m.group (1))
                            t = m.group (2)
                            if t == "oper":
                                ops += 1
                            elif t == "dd60":
                                cons += 1
                            elif t == "remote":
                                niu += 1
                        print "conns shows", op.operators, "op", \
                              cons, "dd60", niu, "niu connections, max", n
                except IndexError:
                    item -= len (l)
            sys.stdout.flush ()
    except KeyboardInterrupt:
        print
finally:
    print "Cleaning up"
    for l in qathings:
        for q in l:
            q.stop ()
            sys.stdout.write (".")
            sys.stdout.flush ()
    print
