#!/usr/bin/env python

"""QA script for Plato

Paul Koning    2012.02.24.    Initial version.
"""

import sys
import random
import time
import traceback
import dtscript

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

remote = raw_input ("Test via tunnel? ").lower ()[0] == 'y'
ocnt = int (raw_input ("Number of dtoper connections? "))
ccnt = int (raw_input ("Number of console connections? "))
pcnt = int (raw_input ("Number of pterm connections? "))

qathings = [ ]
try:
    try:
        for n, c in ((ocnt, QaOper), (ccnt, QaConsole), (pcnt, QaPterm)):
            qalist = [ ]
            for i in xrange (n):
                qalist.append (c (i))
            qathings.append (qalist)
        print "all started up"
        
        while True:
            time.sleep (2)
            item = random.randint (0, 250)
            for l in qathings:
                try:
                    q = l[item]
                    c = q.__class__
                    print c.__name__, item
                    r = repr (q)
                    if r.replace ("\n", "").replace (" ", ""):
                        print r
                    q.stop ()
                    l[item] = c (item)
                    break
                except IndexError:
                    item -= len (l)
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
