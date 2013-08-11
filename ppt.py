#!/usr/bin/env python

"""Bridge utility for PPT to DtCyber

Copyright (C) 2011-2013 by Paul Koning

This utility connects between a serial port and a DtCyber connection
to allow an ASCII mode PPT to be a DtCyber terminal.
"""

import sys
import socket
import threading
import serial
import random
import time
import logging
import logging.handlers
#from daemon import DaemonContext

DEFTERM = "/dev/tty.KeySerial1"
CYBER1 = "cyberserv.org"
NEXTPORT = 8050
STOP1PORT = 8005
SPEED = 2400
RECVMAX = 1024
NEXT = '\x0d' #'\x16'
STOP1 = '\x11' #'\x3a'    # (in no-flow-control mode)
TIMEOUT = 10 * 60 # Inactivity timeout in seconds
TRACE = 2
DEFPIDFILE = "/var/run/ppt.pid"
LOGFILE = "/var/log/ppt.log"
LOGKEEP = 5
LOG_LEVEL = "DEBUG"

# The control sequences are:
# Exit PLATO mode, Enter PLATO mode, full screen erase, mode rewrite,
# mode 3 (text plotting), select memory M0, load X/Y.
# The two arguments supply the low X/Y value, and must be an integer
# in the range 0140 to 0177 for the first and 0100 to 0137 for the second.
#
# The "Exit PLATO mode" code is sent to force flow control off, that way
# we don't have to deal with two different character codes.
WELCOME_MSG = "\033\002\033\002\033\014\033\024\037\033\102" \
              "\033\062\044{:c}\044{:c}Press  NEXT  to begin"

# This one is like the one in the "logging" module but with the
# decimal comma corrected to a decimal point.
def formatTime(self, record, datefmt=None):
    """
    Return the creation time of the specified LogRecord as formatted text.
    """
    ct = self.converter (record.created)
    if datefmt:
        s = time.strftime (datefmt, ct)
    else:
        t = time.strftime ("%Y-%m-%d %H:%M:%S", ct)
        s = "%s.%03d" % (t, record.msecs) # the use of % here is internal
    return s
logging.Formatter.formatTime = formatTime

def trace (msg, *args, **kwargs):
    logging.log (TRACE, msg, *args, **kwargs)
    
class StopThread (threading.Thread):
    """A thread with stop method.
    """
    def __init__ (self):
        threading.Thread.__init__ (self)
        self.stopnow = False

    def stop (self):
        """Stop the thread associated with this connection.  The actual
        handling of "stopnow" needs to go into the class that uses this.
        """
        if not self.stopnow and self.isAlive ():
            logging.trace ("Stopping thread %s", self)
            self.stopnow = True
            self.join ()

class tocyber (StopThread):
    """A class for the thread that sends data to DtCyber
    """
    def __init__ (self, sock, term):
        StopThread.__init__ (self)
        self.term = term
        self.sock = sock
        self.lastio = None
        self.start ()
        
    def run (self):
        """Read from the terminal and send to the socket.
        """
        self.term.setTimeout (10)
        while not self.stopnow:
            data = self.term.read ()
            if data:
                self.lastio = time.time ()
                try:
                    self.sock.sendall (data)
                except socket.error:
                    logging.exception ("Sending on socket to PLATO")
                    return
            
class fromcyber (StopThread):
    """A class for the thread that receives data from DtCyber
    """
    def __init__ (self, sock, term):
        StopThread.__init__ (self)
        self.term = term
        self.sock = sock
        self.lastio = None
        self.start ()
        
    def run (self):
        """Read from the socket and send to the terminal.
        """
        while not self.stopnow:
            try:
                data = self.sock.recv (RECVMAX)
            except socket.error:
                logging.exception ("Receiving on socket to PLATO")
                return
            if data:
                data = data.replace (b"\377\377", b"\377")
                self.lastio = time.time ()
                self.term.write (data)
            else:
                logging.debug ("EOF reading socket to PLATO")
                return

def pressnext (term):
    """Send a Press NEXT to begin message to the terminal, at a random
    X/I near 128, 128.
    """
    msg = WELCOME_MSG.format (random.randint (0140, 0177),
                              random.randint (0100, 0137))
    term.write (msg)

def getaction (term):
    """Not connected state: send Press NEXT to begin message to the terminal,
    and look for NEXT or SHIFT-STOP.  Return the keystroke that was read.
    False otherwise.
    """
    logging.debug ("Starting local interaction")
    while True:
        pressnext (term)
        c = term.read (1)
        if c == NEXT or c == STOP1:
            return c

def talk (host, term, action):
    """ Connect (port chosen based on the "action" key code) to the
    PLATO host, and talk to it until timeout or disconnect.
    """
    if action == STOP1:
        port = STOP1PORT
    else:
        port = NEXTPORT
    logging.debug ("Starting connection to %s port %d", host, port)
    try:
        sock = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
        logging.trace ("connecting")
        sock.connect ((host, port))
        logging.trace ("connected")
    except socket.error:
        logging.exception ("Connecting")
        return
    inbound = fromcyber (sock, term)
    outbound = tocyber (sock, term)
    logging.trace ("threads active")
    # Both I/O threads are started.  Loop, mostly sleeping, looking
    # for inactivity or loss of connection.  If so, exit.
    try:
        while True:
            time.sleep (10)
            now = time.time ()
            # Check inbound thread
            if not inbound.is_alive () or now - inbound.lastio > TIMEOUT:
                break
            # Check outbound thread
            if not outbound.is_alive () or now - outbound.lastio > TIMEOUT:
                break
    except KeyboardInterrupt:
        pass
    logging.trace ("Closing down connection")
    try:
        sock.close ()
    except socket.error:
        pass
    inbound.stop ()
    outbound.stop ()
        
def mainloop (term, host):
    logging.info ("Starting ppt.py for terminal %s to host %s", term, host)
    term = serial.Serial (port = term, baudrate = SPEED,
                          parity = 'E', bytesize = 7)
    while True:
        action = getaction (term)
        talk (host, term, action)
    
if __name__ == "__main__":
    host = CYBER1
    term = DEFTERM
    logging.addLevelName (TRACE, "TRACE")
    #h = logging.handlers.TimedRotatingFileHandler (filename = LOGFILE,
    #                                               when = "midnight",
    #                                               backupCount = LOGKEEP)
    h = logging.StreamHandler (sys.stderr)
    rootlogger = logging.getLogger ()
    fmt = logging.Formatter ("%(asctime)s: %(threadName)s: %(message)s")
    h.setFormatter (fmt)
    rootlogger.addHandler (h)
    rootlogger.setLevel (LOG_LEVEL)
    if len (sys.argv) > 1:
        term = sys.argv[1]
    if len (sys.argv) > 2:
        host = sys.argv[2]
    mainloop (term, host)
        
