#!/usr/bin/env python3

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
CYBER1ROOT = "/home/cyber1/D"
BACKUPDEST = "backups"    # relative to CYBER1ROOT
TMPDEST = "/tmp"          # place where tarball goes until finished
PWDFILE = "backuppwd.txt"

# Two hostnames to send the backup to, None to skip that one.
BACKUPDEST1 = None
BACKUPDEST2 = "cyber1backup@akdesign.dyndns.org"

# Base arguments for scp
# Speed limit: 400 kb/s, identity file: cyber1backup.
SCPARGS = ( "scp", "-l", "400", "-q", "-i", "cyber1backup" )

import sys
import os
import dtscript
import time
import subprocess

if BACKUPPWD is None:
    f = file (PWDFILE, "r")
    BACKUPPWD = f.readline ().strip ()
    f.close ()

def log (str):
    print ("%s: %s" % (time.strftime ("%T"), str))
    
def sendstr (term, str):
    """Send a string in pieces with some delays to make it reliable.
    """
    while str:
        term.sendstr (str[:6])
        str = str[6:]
        time.sleep (0.5)
        
def announce (cons, str):
    log ("Announcing: %s" % str)
    sendstr (cons, "1")
    cons.wait_update ()
    sendstr (cons, "1")
    cons.wait_update ()
    sendstr (cons, str + "\n")
    cons.wait_update ()
    cons.sendkey (cons.BACK)
    cons.wait_update ()
    sendstr (cons, "2")
    cons.wait_update ()
    cons.sendkey (cons.SHIFT + cons.COPY)
    cons.sendkey (cons.NEXT)
    time.sleep (10)
    cons.sendkey (cons.LAB)
    time.sleep (10)
    cons.sendkey (cons.LAB)
    time.sleep (5)
    cons.sendkey (cons.BACK)

system = "S Y S T E M".encode ("dd60")
mastor = "M A S T O R".encode ("dd60")
nojsn = "JSN NOT FOUND".encode ("dd60")
chk = "CHECKPOINT COMPLETE".encode ("dd60")

def doshutdown (delay):
    """Shut down PLATO in "delay" minutes.
    """
    # Before we do the actual work, connect to console and
    # operator interface and check that things look ok
    cons = dtscript.Console (interval = 1)
    cons.todsd ()
    cons.sendstr ("[k,mas1.\n")
    for i in range (4):
        if mastor in cons.screen[0][16]:
            break
        if i < 3:
            time.sleep (2)
        else:
            log ("Failed to connect to Mastor K display")
            cons.stop ()
            sys.exit (1)
    oper = dtscript.Oper ()

    # Start the CONSOLE utility and get us into sysopts (1)
    cons.login ()
    cons.sendstr ("1")
    cons.wait_update ()
    for i in range (10):
        if system in cons.screen[0][8]:
            break
        if i < 9:
            time.sleep (2)
        else:
            log ("Failed to start sysopts")
            cons.stop ()
            oper.stop ()
            sys.exit (1)
    
    # First step: put up a 1st line message and broadcast that.
    # Repeat every 5 minutes until we've done the delay
    while delay:
        announce (cons, "Interruption for backups in %d minutes..." % delay)
        if delay < 5:
            time.sleep (delay * 60)
            break
        delay -= 5
        time.sleep (300)

    # Second step: announce interruption for backup now
    announce (cons, "Interruption for backups...")

    # Third step: full system backout
    log ("Starting backout")
    sendstr (cons, "3")
    time.sleep (1)
    cons.sendkey (cons.SHIFT + cons.LAB)
    time.sleep (60)
    for i in range (20):
        if system in cons.screen[0][8]:
            break
        if i < 19:
            time.sleep (2)
        else:
            log ("Backout did not complete")
            cons.stop ()
            oper.stop ()
            sys.exit (1)

    # Shut down Mastor
    cons.logout ()
    log ("Stopping mastor")
    cons.sendstr ("[k,mas1.\n")
    time.sleep (2)
    cons.sendstr ("[k.stop\n")

    # Wait for Mastor to exit
    for i in range (10):
        if nojsn in cons.screen[0][9]:
            break
        if i < 9:
            time.sleep (2)
        else:
            log ("Mastor did not stop")
    
    # See if there is any other non-subsystem job,
    # which is likely to be cftp
    cons.sendstr ("[ab\n")
    cons.wait_update ()
    cons.sendstr ("[b,a\n")
    cons,wait_update ()
    for i in range (12, 47):
        if cons.screen[1][i][5] == 1:
            # First char of JSN is A, typical for user job
            log ("dropping %s" % cons.screen[1][i][5:9])
            cons.sendstr ("[dro%s.\n" % cons.screen[1][i][5:9].decode ("dd60"))
            cons.wait_update ()

    # issue Checkpoint System
    log ("Checkpoint system")
    cons.sendstr ("[unlock.\n")
    cons.wait_update ()
    cons.sendstr ("[che\n")

    # wait for checkpoint to complete
    for i in range (10):
        if chk in cons.screen[1][48]:
            break
        if i < 9:
            time.sleep (2)
        else:
            log ("Checkpoint did not finish")

    # Step the system
    cons.sendstr ("[step.\n")
    cons.wait_update ()

    # Shut down DtCyber
    cons.stop ()
    log ("Shutting down DtCyber")
    oper.command ("UNLOCK.")
    if oper.locked:
        log ("Operator interface did not unlock")
        oper.stop ()
        sys.exit (1)
    oper.command ("SHUTDOWN.")
    oper.stop ()
    log ("Shut down complete")
    
def dostartup ():
    """Restart PLATO.
    """
    log ("Starting DtCyber and PLATO")
    subprocess.Popen (("./dtcyber-remote.sh",))
    time.sleep (5)
    pterm = dtscript.Pterm ()
    for i in range (50):
        if "Press  NEXT  to begin" in str (pterm):
            break
        log ("Waiting for PLATO ready, %d" % i)
        time.sleep (10)
    pterm.stop ()
    log ("PLATO is ready")

class Blackbox (subprocess.Popen):
    """A specialization of subprocess for the blackbox utility.
    """
    def __init__ (self, msg):
        log ("Starting blackbox: %s" % msg)
        subprocess.Popen.__init__ (self, ("./blackbox", msg),
                                   stdin = subprocess.PIPE)

    def newmsg (self, msg):
        log ("Setting blackbox message: %s" % msg)
        self.stdin.write ("%s\n" % msg)
        
    def stop (self):
        self.stdin.close ()
        for i in range (10):
            if self.poll () is not None:
                break
            log ("Waiting for blackbox to exit")
            if i < 9:
                time.sleep (2)
            else:
                os.kill (self.pid, 9)
        log ("Blackbox stopped")
                
class Backup (subprocess.Popen):
    """Another specialization of subprocess for doing a backup
    and tracking its progress.
    """
    def __init__ (self, tarball, items):
        self.tarball = tarball
        args = ("tar", "cf", tarball) + items
        log ("Starting backup of %s to %s" % (str (items), tarball))
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

def docopy (tarball):
    for dest in (BACKUPDEST1, BACKUPDEST2):
        if dest is None:
            continue
        log ("Copying %s to %s" % (tarball, dest))
        scp = subprocess.Popen (SCPARGS + (tarball, "%s:" % dest))
        scp.wait ()
        
# ***TEST
CYBER1ROOT="/Users/pkoning/Documents/svn/dtcyber"
BACKUPITEMS=("pack",)
SHUTDOWNDELAY = 1        # minutes

def main ():
    os.chdir (CYBER1ROOT)
    tarbase = "cyber1-%s.tar" % time.strftime ("%F")
    ttarball = os.path.join (TMPDEST, tarbase)
    backupsize = 0
    for item in BACKUPITEMS:
        backupsize += getsize (item)
    log ("Uncompressed backup size is about %d MB" % (backupsize >> 10))
    doshutdown (SHUTDOWNDELAY)
    blackbox = Blackbox ("Cyber1 full backup in progress...")
    backup = Backup (ttarball, BACKUPITEMS)
    elapsed = 0
    while True:
        time.sleep (30)
        progress = backup.progress ()
        if progress == -1:
            log ("Backup tarball has been written, restarting PLATO")
            break
        elapsed += 0.5
        total = elapsed * backupsize / progress
        left = int (total - elapsed) + 1
        if left <= 1:
            blackbox.newmsg ("Cyber1 full backup nearly done...")
        else:
            blackbox.newmsg ("Cyber1 full backup will finish in about %d minutes" % left)
    blackbox.newmsg ("Cyber1 is about to restart...")
    time.sleep (10)
    blackbox.stop ()
    dostartup ()

    # PLATO is back up, now we can compress and copy the backup
    # tarball in the background.
    log ("Compressing tarball")
    ret = os.system ("nice xz -v %s" % ttarball)
    ttarball += ".xz"
    tarball = os.path.join (BACKUPDEST, tarbase) + ".xz"

    # The tarball is finished, so move it to its final destination,
    # then copy it for the "push" destinations
    os.rename (ttarball, tarball)
    docopy (tarball)
    
    # All done
    log ("Full backup is finished")
    
if __name__ == "__main__":
    main ()
