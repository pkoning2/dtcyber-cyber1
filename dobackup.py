#!/usr/bin/env python3

"""Script for doing unattended backups of cyber1

This script can run as a cron job, or can be invoked interactively.
Either way, it shuts down PLATO with appropriate messages, performs a
backup while running "blackbox" to keep users informed of progresss,
then restarts PLATO.  It then sends the backup to the local backup
data server and to akdesign.dyndns.org (Paul Koning's system).
"""

# Parameters
SHUTDOWNDELAY = 15        # minutes from first warning to shutdown
FALLBACKDELAY = 60        # minutes to be down for PDT to PST time zone change
BACKUPITEMS = ("pack", "sys/871")
CYBER1ROOT = "/home/cyber1/D"
BACKUPDEST = "backups"    # relative to CYBER1ROOT

# Two hostnames to send the backup to, None to skip that one.
BACKUPDEST1 = None
BACKUPDEST2 = "cyber1backup@akdesign.dyndns.org"

# Base arguments for scp
# Speed limit: 1 MB/s (8 Mb/s), identity file: cyber1backup.
SCPARGS = ( "scp", "-l", "8000", "-q", "-i", "cyber1backup" )

import sys
import os
import re
import dtscript
import time
import subprocess
import shutil

mastor = "M A S T O R".encode ("dd60")
system = "S Y S T E M".encode ("dd60")
ipedit1 = "INSTALLATION PARAMETERS".encode ("dd60")
ipedit2 = "ENTER THE TIME ZONE".encode ("dd60")
nojsn = "JSN NOT FOUND".encode ("dd60")
chk = "CHECKPOINT COMPLETE".encode ("dd60")

def log (str):
    print ("%s: %s" % (time.strftime ("%T"), str))

def sendstr (term, str):
    """Send a string one char at a time for reliability.
    """
    for c in str:
        term.sendstr (c)
        time.sleep (0.2)
        
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

def startui ():
    """Start the console interface and get it ready for the real work.

    This returns the console interface object.  The console
    is at this point logged in and sitting at the Author Mode page.
    """
    # Before we do the actual work, connect to console 
    # interface and check that things look ok
    cons = dtscript.Console (interval = 1)
    cons.todsd ()
    platoup = True    # Assume PLATO is active
    cons.sendstr ("[uns\n")       # Cancel step just in case it was set
    cons.sendstr ("[k,mas1.\n")
    for i in range (4):
        if mastor in cons.screen[0][16]:
            break
        elif nojsn in cons.screen[0][9]:
            log ("PLATO is not running")
            platoup = False
            break
        time.sleep (2)
    else:
        log ("Failed to connect to Mastor K display")
        cons.stop ()
        sys.exit (1)

    # Start the CONSOLE utility and get us into sysopts (1)
    if platoup:
        cons.login ()
    return cons, platoup

def doshutdown (cons, platoup, delay, tzdelay, newtz):
    """Shut down PLATO in "delay" minutes.  "cons" is the console
    interface object.

    "platoup" says whether PLATO is active.  If False, skip the
    PLATO shutdown step.

    "tzdelay" says whether a timezone change is needed.  Non-zero means
    yes.  2 means yes and we have to be down for an hour.  "newtz" is
    the new timezone string to be set.
    """
    if platoup:
        cons.sendstr ("1")
        cons.wait_update ()
        for i in range (10):
            if system in cons.screen[0][8]:
                break
            time.sleep (2)
        else:
            log ("Failed to start sysopts")
            cons.stop ()
            sys.exit (1)

        # First step: put up a 1st line message and broadcast that.
        # Repeat every 5 minutes until we've done the delay
        while delay:
            if tzdelay == 2:
                msg = "One hour shutdown for timezone change in %d minutes" % delay
            else:
                msg = "Interruption for backups in %d minutes..." % delay
            announce (cons, msg)
            if delay < 5:
                time.sleep (delay * 60)
                break
            delay -= 5
            time.sleep (300)

        # Second step: announce interruption for backup now
        if tzdelay == 2:
            msg = "One hour shutdown for timezone change...              "
        else:
            msg = "Interruption for backups...              "
        announce (cons, msg)

        # Third step: full system backout
        log ("Starting backout")
        sendstr (cons, "3")
        time.sleep (1)
        cons.sendkey (cons.SHIFT + cons.LAB)
        time.sleep (60)
        for i in range (20):
            if system in cons.screen[0][8]:
                break
            time.sleep (2)
        else:
            log ("Backout did not complete")
            cons.stop ()
            sys.exit (1)

        # If we need a timezone change, set the new timezone now so that
        # upon restart it will be what we want it to be.
        if tzdelay:
            # Need to change timezone.  First go back to Author Mode
            cons.login ()
            cons.sendstr ("ipedit")
            cons.sendkey (cons.DATA)
            cons.wait_update ()
            ok = False
            for i in range (10):
                if ipedit1 in cons.screen[0][4]:
                    ok = True
                    break
                time.sleep (2)
            else:
                log ("Failed to start ipedit")
            if ok:
                cons.sendstr ("c")
                cons.wait_update ()
                ok = False
                for i in range (10):
                    if ipedit2 in cons.screen[0][24]:
                        ok = True
                        break
                    time.sleep (2)
                else:
                    log ("Failed to reach timezone page in ipedit")
            if ok:
                cons.sendstr (newtz)
                cons.sendkey (cons.NEXT)
                log ("Timezone updated to %s" % newtz)
                cons.wait_update ()
                cons.sendkey (cons.BACK)
                cons.wait_update ()
                cons.sendkey (cons.BACK)

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
            time.sleep (2)
        else:
            log ("Mastor did not stop")

    # See if there is any other non-subsystem job,
    # which is likely to be cftp
    cons.sendstr ("[ab\n")
    cons.wait_update ()
    cons.sendstr ("[b,a\n")
    cons.wait_update ()
    for i in range (12, 47):
        if cons.screen[1][i][5] == 1:
            # First char of JSN is A, typical for user job
            jsn = cons.screen[1][i][5:9].decode ("dd60", "ignore")
            log ("Dropping %s" % jsn)
            cons.sendstr ("[dro%s.\n" % jsn)
            cons.wait_update ()

    # All the rest is done by systemd, with some help from
    # stop-dtcyber.py
    cons.stop ()
    log ("Shutting down DtCyber")
    subprocess.call (("systemctl", "stop", "dtcyber"),
                     stderr = subprocess.STDOUT,
                     universal_newlines = True)
    log ("Shut down complete")
    
def dostartup ():
    """Restart PLATO.
    """
    log ("Starting DtCyber and PLATO")
    subprocess.call (("systemctl", "start", "dtcyber"),
                     stderr = subprocess.STDOUT,
                     universal_newlines = True)
    time.sleep (5)
    pterm = dtscript.Pterm ()
    for i in range (50):
        if "Press  NEXT  to begin" in str (pterm):
            log ("PLATO is ready")
            break
        log ("Waiting for PLATO ready, %d" % i)
        time.sleep (10)
    else:
        log ("PLATO failed to start")
    pterm.stop ()

class Blackbox (subprocess.Popen):
    """A specialization of subprocess for the blackbox utility.
    """
    def __init__ (self, msg):
        log ("Starting blackbox: %s" % msg)
        subprocess.Popen.__init__ (self, ("./blackbox", msg),
                                   stdin = subprocess.PIPE,
                                   stderr = subprocess.STDOUT,
                                   universal_newlines = True)

    def newmsg (self, msg):
        log ("Setting blackbox message: %s" % msg)
        print (msg, file = self.stdin)
        self.stdin.flush ()
        
    def stop (self):
        self.stdin.close ()
        for i in range (10):
            if self.poll () is not None:
                break
            log ("Waiting for blackbox to exit")
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
        subprocess.Popen.__init__ (self, args, stderr = subprocess.STDOUT,
                                   universal_newlines = True)

    def progress (self):
        """Returns progress, in the form of the current
        size in kilobytes of the tarball.  If the backup is
        finished, return -1.
        """
        if self.poll () is not None:
            return -1
        return os.stat (self.tarball).st_size >> 10

def getsize (path):
    result = subprocess.check_output (("du", "-ks", path),
                                      universal_newlines = True)
    size, *x = result.split ()
    return int (size)

def docopy (tarball):
    for dest in (BACKUPDEST1, BACKUPDEST2):
        if dest is None:
            continue
        log ("Copying %s to %s" % (tarball, dest))
        ret = subprocess.call (SCPARGS + (tarball, "%s:" % dest),
                            stderr = subprocess.STDOUT,
                            universal_newlines = True)
        if ret:
            log ("*** Error {} during copy to {}".format (ret, dest))

_time_re = re.compile (r"\d\d?\*\d\d\*\d\d [AP]M ([A-Z][A-Z][A-Z])")
def checktz (cons, newtz):
    """Check if the timezone needs to be changed.  Arguments are the
    console object and the current timezone string (from the host OS).
    The console should be logged in (at Author Mode).

    Returns 0 if no change, 1 if a change is needed but no long shutdown,
    and 2 if a change is needed with a 1 hour shutdown.  The short vs.
    long shutdown is because PLATO only deals with local time and many
    lessons don't handle time going backwards.  So the change in the fall
    (from daylight to standard time) requires a one hour shutdown to
    avoid backing up time by an hour.
    """
    for i in range (10):
        cons.sendkey (cons.SHIFT + cons.ERASE)
        cons.sendstr ("\nT")
        cons.wait_update ()
        m = _time_re.search (cons.screentext (0))
        if m:
            break
        time.sleep (2)
    else:
        log ("Time string not found in checktz, screen is:")
        log (cons.screentext (0))
        return 0
    curtz = m.group (1)
    if curtz == newtz:
        log ("Time zone is up to date")
        return 0
    if newtz[1] == 'S':
        # Changing to standard time, long delay
        log ("Changing from %s to %s, one hour delay" % (curtz, newtz))
        return 2
    log ("Changing from %s to %s, no delay needed" % (curtz, newtz))
    return 1

## ***TEST
#CYBER1ROOT="/buildarea/dtcyber/dtcyber"
#BACKUPITEMS=("pack/xfer",)
#SHUTDOWNDELAY = 1        # minutes
#FALLBACKDELAY = 5        # minutes

def main ():
    os.chdir (CYBER1ROOT)
    tarbase = "cyber1-%s.tar" % time.strftime ("%F")
    tarball = os.path.join (BACKUPDEST, tarbase)
    backupsize = 0
    for item in BACKUPITEMS:
        backupsize += getsize (item)
    log ("Uncompressed backup size is about %d MB" % (backupsize >> 10))
    cons, platoup = startui ()
    if platoup:
        newtz = time.strftime ("%Z")
        tzdelay = checktz (cons, newtz)
    else:
        newtz = None
        tzdelay = 0
    doshutdown (cons, platoup, SHUTDOWNDELAY, tzdelay, newtz)
    if tzdelay == 2:
        msg = time.strftime ("Cyber1 down for timezone change until around %I:%M %p",
                             time.localtime (time.time () + 3700))
    else:
        msg = "Cyber1 full backup in progress..."
    blackbox = Blackbox (msg)
    backup = Backup (tarball, BACKUPITEMS)
    elapsed = 0
    while True:
        time.sleep (30)
        if backup:
            progress = backup.progress ()
            if progress == -1:
                if tzdelay == 2 and elapsed < FALLBACKDELAY:
                    log ("Backup is finished, waiting for the rest of the hour")
                    backup = None
                    progress = backupsize
                else:
                    log ("Backup tarball has been written, restarting PLATO")
                    break
        elapsed += 0.5
        total = elapsed * backupsize / progress
        left = int (total - elapsed) + 1
        if tzdelay == 2:
            # Waiting for an hour.  Check if it's been that long and
            # the backup indeed finished.
            if elapsed < FALLBACKDELAY:
                # Not long enough yet, keep waiting
                continue
            if not backup:
                # It's time, and the backup is finished already
                break
        # Waiting for backup to finish (either not a long shutdown, or
        # backup actually took longer than an hour).  Set the message.
        if left <= 1:
            blackbox.newmsg ("Cyber1 full backup nearly done...")
        else:
            blackbox.newmsg ("Cyber1 full backup will finish in about %d minutes" % left)
    blackbox.newmsg ("Cyber1 is about to restart...")
    time.sleep (10)
    blackbox.stop ()
    dostartup ()

    # PLATO is back up, now we can compress and copy the backup
    # tarball in the background.  First see what compression tool we have
    z = open (os.devnull)
    cmd = None
    ext = ""
    try:
        # We have to gather the output from xz because (on Linux, at
        # least) it objects if stdout is /dev/null.  Bug...
        # Instead of returning a status as subprocess.call does, check_output
        # raises an exception if the exit status is nonzero.  Handle that
        # by a different "except" clause below.  
        t = subprocess.check_output (("xz", "-h"),
                                     universal_newlines = True)
        cmd = "xz"
        ext = ".xz"
    except (OSError, subprocess.CalledProcessError):
        pass
    if not cmd:
        try:
            if subprocess.call (("bzip2", "-h"), stdout = z, stderr = z,
                                universal_newlines = True) == 0:
                cmd = "bzip2"
                ext = ".bz2"
        except OSError:
            pass
    if not cmd:
        try:
            if subprocess.call (("gzip", "-h"), stdout = z, stderr = z,
                                universal_newlines = True) == 0:
                cmd = "gzip"
                ext = ".gz"
        except OSError:
            pass
    if cmd:
        log ("Compressing tarball with %s" % cmd)
        ret = subprocess.call (("nice", cmd, "-v9", tarball),
                               stderr = subprocess.STDOUT,
                               universal_newlines = True)
        if ret:
            log ("Compression failed with exit status %d" % ret)
        else:
            tarball += ext
    else:
        log ("Skipping tarball compression, no useable compressor found")

    docopy (tarball)
    
    # All done
    log ("Full backup is finished")
    
if __name__ == "__main__":
    main ()
