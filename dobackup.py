#!/usr/bin/env python

"""Script for doing unattended backups of cyber1

This script can run as a cron job, or can be invoked interactively.
Either way, it shuts down PLATO with appropriate messages, performs a
backup while running "blackbox" to keep users informed of progresss,
then restarts PLATO.  It then sends the backup to the local backup
data server and to akdesign.dyndns.org (Paul Koning's system).
"""

# User/group and password to use for system shutdown.  This is
# expected to be a student record, with its current lesson set
# to "sysopts" (the lesson used by "1" from author mode).
# Password can be a string, or None to fetch the password from
# a separate file.  The latter is preferred for security, it allows
# the password string to be more tightly protected.

BACKUPUSER = "backups"
BACKUPGROUP = "o"
BACKUPPWD = None

# More parameters
SHUTDOWNDELAY = 15        # minutes
BACKUPITEMS = ("pack", "sys/871")

PWDFILE = "backuppwd.txt"

# Two hostnames to send the backup to, None to skip that one.
BACKUPDEST1 = None
BACKUPDEST2 = "akdesign.dyndns.org"

import sys
import os
import dtscript
import time
import subprocess

if BACKUPPWD is None:
    f = file (PWDFILE, "r")
    BACKUPPWD = f.readline ().strip ()
    f.close ()

def sendstr (pterm, str):
    """Send a string in pieces with some delays to make it reliable.
    """
    print "sending", str
    while str:
        pterm.sendstr (str[:6])
        str = str[6:]
        time.sleep (0.5)
        
def announce (pterm, str):
    sendstr (pterm, "1")
    time.sleep (1)
    sendstr (pterm, "1")
    time.sleep (1)
    sendstr (pterm, str + "\n")
    time.sleep (1)
    pterm.sendkey (pterm.BACK)
    time.sleep (1)
    sendstr (pterm, "2")
    time.sleep (1)
    pterm.sendkey (pterm.SHIFT + pterm.COPY)
    pterm.sendkey (pterm.NEXT)
    time.sleep (10)
    print "again"
    pterm.sendkey (pterm.LAB)
    time.sleep (10)
    print "again"
    pterm.sendkey (pterm.LAB)
    time.sleep (5)
    pterm.sendkey (pterm.BACK)

def doshutdown (delay):
    """Shut down PLATO in "delay" minutes.
    """
    pterm = dtscript.Pterm ()
    pterm.login (BACKUPUSER, BACKUPGROUP, BACKUPPWD, False)
    for i in range (4):
        if "S Y S T E M" in pterm.lines[5]:
            break
        if i < 3:
            time.sleep (2)
        else:
            print "Failed to log in %s/%s or start sysopts" % (BACKUPUSER, BACKUPGROUP)
            pterm.stop ()
            sys.exit (1)
    # Before we do the actual work, connect to console and
    # operator interface and check that things look ok
    cons = dtscript.Dd60 (interval = 1)
    cons.sendstr ("k,mas1.\n")
    for i in range (4):
        if "M A S T O R" in cons.screen[0][7]:
            break
        if i < 3:
            time.sleep (2)
        else:
            print "Failed to connect to Mastor K display"
            pterm.stop ()
            cons.stop ()
            sys.exit (1)
    oper = dtscript.Oper ()

    # First step: put up a 1st line message and broadcast that.
    # Repeat every 5 minutes until we've done the delay
    while delay:
        announce (pterm, "Interruption for backups in %d minutes..." % delay)
        if delay < 5:
            time.sleep (delay * 60 / 10)
            break
        delay -= 5
        time.sleep (300 / 10)

    # Second step: announce interruption for backup now
    announce (pterm, "Interruption for backups...")

    # Third step: full system backout
    sendstr (pterm, "3")
    time.sleep (1)
    pterm.sendkey (pterm.SHIFT + pterm.LAB)
    time.sleep (60)
    for i in range (20):
        if "S Y S T E M" in pterm.lines[5]:
            break
        if i < 19:
            time.sleep (2)
        else:
            print "Backout did not complete"
            pterm.stop ()
            cons.stop ()
            oper.stop ()
            sys.exit (1)

    # Shut down Mastor
    pterm.stop ()
    print "stopping mastor"
    cons.sendstr ("k,mas1.\n")
    time.sleep (2)
    cons.sendstr ("k.stop\n")

    # Wait for Mastor to exit
    for i in range (10):
        if "JSN NOT FOUND" in cons.screen[0][4]:
            break
        if i < 9:
            time.sleep (2)
        else:
            print "Mastor did not stop"
    
    # See if there is any other non-subsystem job,
    # which is likely to be cftp
    cons.sendstr ("[ab\n")
    time.sleep (1)
    cons.sendstr ("b,a\n")
    time.sleep (2)
    for i in range (5, 23):
        if cons.screen[1][i][5] == 'A':
            # First char of JSN is A, typical for user job
            print "dropping", cons.screen[1][i][5:9]
            cons.sendstr ("dro%s.\n" % cons.screen[1][i][5:9])
            time.sleep(2)

    # issue Checkpoint System
    cons.sendstr ("unlock.\n")
    time.sleep (2)
    cons.sendstr ("che\n")

    # wait for checkpoint to complete
    for i in range (10):
        if "CHECKPOINT COMPLETE" in cons.screen[1][23]:
            break
        if i < 9:
            time.sleep (2)
        else:
            print "Checkpoint did not finish"

    # Step the system
    cons.sendstr ("[step.\n")
    time.sleep (2)

    # Shut down DtCyber
    cons.stop ()
    oper.command ("UNLOCK.")
    if oper.locked:
        print "Operator interface did not unlock"
        oper.stop ()
        sys.exit (1)
    oper.command ("SHUTDOWN.")
    oper.stop ()
    print "All shut down"
    
def dostartup ():
    """Restart PLATO.
    """
    print "Starting DtCyber and PLATO"
    ret = os.system ("./dtcyber-remote.sh")
    print "Started, status is", ret
    time.sleep (5)
    pterm = dtscript.Pterm ()
    for i in range (50):
        if "Press  NEXT  to begin" in pterm.str ():
            break
        print "Waiting for PLATO ready,", i
        time.sleep (5)

class Blackbox (subprocess.Popen):
    """A specialization of subprocess for the blackbox utility.
    """
    def __init__ (self, msg):
        subprocess.Popen.__init__ (self, ("./blackbox", msg),
                                   stdin = subprocess.PIPE)

    def newmsg (self, msg):
        self.stdin.write ("%s\n" % msg)
        
    def stop (self):
        self.stdin.close ()
        for i in range (10):
            if self.poll () is not None:
                break
            print "Waiting for blackbox to exit"
            if i < 9:
                time.sleep (2)
            else:
                os.kill (self.pid, 9)
                
class Backup (subprocess.Popen):
    """Another specialization of subprocess for doing a backup
    and tracking its progress.
    """
    def __init__ (self, tarball, items):
        self.tarball = tarball
        args = ("tar", "cf", tarball) + items
        subprocess.Popen.__init__ (self, args)

    def progress (self):
        """Returns progress, in the form of the current
        size in kilobytes of the tarball.  If the backup is
        finished, return -1.
        """
        if self.poll () is not None:
            return -1
        return os.stat (self.tarball).st_size >> 10

def getsize (path):
    du = subprocess.Popen (("du", "-ks", path), stdout = subprocess.PIPE)
    result = du.stdout.read ().split ()
    return int (result[0])

def main ():
    doshutdown (SHUTDOWNDELAY)
    
