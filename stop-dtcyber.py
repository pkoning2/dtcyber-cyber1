#!/usr/bin/env python3

"""Helper script for fairly cleanly shutting down dtcyber

This script is used by the systemd machinery to shut down the
dtcyber service.  It is fairly clean in that the OS is shut down
and the dtcyber emulator is then cleanly stopped.  It does not,
however, shut down any applications.  In particular, a dtcyber
instance running PLATO needs additional steps to shut down PLATO
cleanly before this script is invoked.
"""

import time
import dtscript

system = "S Y S T E M".encode ("dd60")
chk = "CHECKPOINT COMPLETE".encode ("dd60")

def log (str):
    print ("%s: %s" % (time.strftime ("%T"), str))

def sendstr (term, str):
    """Send a string one char at a time for reliability.
    """
    for c in str:
        term.sendstr (c)
        time.sleep (0.2)
        
def startui ():
    """Start the user interfaces and get them ready for the real work.

    This returns the console and operator interface objects.   The console
    is in DSD and STEP mode is not in effect.
    """
    # Before we do the actual work, connect to console and
    # operator interface and check that things look ok
    cons = dtscript.Console (interval = 1)
    cons.todsd ()
    cons.sendstr ("[uns\n")       # Cancel step just in case it was set
    oper = dtscript.Oper ()
    return cons, oper

def doshutdown (cons, oper):
    """Shut down DtCyber.  "cons" and "oper" are the  user interface objects.
    """
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
    
def main ():
    cons, oper = startui ()
    doshutdown (cons, oper)
    
if __name__ == "__main__":
    main ()
    
