#!/usr/bin/env python

"""Internet Site Controller

Copyright (C) 2011-2013 by Paul Koning

This utility connects between a serial port and a DtCyber connection
to allow an ASCII mode PPT to be a DtCyber terminal.
"""

import sys
import os
import socket
import select
import threading
import signal
import serial
import random
import time
import logging
import logging.handlers
from daemon import DaemonContext
import argparse
try:
    from Adafruit_BBIO import UART
except ImportError:
    UART = None
    
if sys.platform == "darwin":
    DEFTERM = "/dev/tty.KeySerial1"
else:
    DEFTERM = "/dev/ttyUSB0"
CYBER1 = "cyberserv.org"
DEFPORT = 8050
STOP1PORT = 8005
SPEED = 2400
RECVMAX = 1024
NEXT = '\x0d'
ANS = '\x07'
STOP1 = '\x11' # (in no-flow-control mode)
TIMEOUT = 10 * 60 # Inactivity timeout in seconds
IOTIMEOUT = 2
LOCALTIMEOUT = 2 * 60
TRACE = 2
DEFPIDFILE = "/var/run/ppt.pid"
PPTVER = "1.0.0"
PPTREV = "$LastChangedRevision$".split ()[1]

# The control sequences are:
# Exit PLATO mode, Enter PLATO mode, full screen erase, mode rewrite,
# mode 3 (text plotting), select memory M0, load Y/X.
# The two arguments supply the low Y/X value, and must be an integer
# in the range 0140 to 0177 for the first (Y) and 0100 to 0137 for
# the second (X).
#
# The "Exit PLATO mode" code is sent to force flow control off, that way
# we don't have to deal with two different character codes.
WELCOME_MSG = "\033\002\033\002\033\014\033\024\037\033\102" \
              "\033\062\044{:c}\044{:c}Press  NEXT  to begin"

# Connect error message:
CERR_MSG = "\033\062\042\140\040\130Connect error: {}"

# Program/host information message:
INFO_MSG = "\033\062\057\140\040\120Internet Site Controller {} ({})\r    on {} {}"

pptparser = argparse.ArgumentParser ()
pptparser.add_argument ("term", nargs = '?', default = DEFTERM,
                        help = "Terminal port device name (default: {})".format (DEFTERM))
pptparser.add_argument ("host", nargs = '?', default = CYBER1,
                        help = "Destination host (default: {})".format (CYBER1))
pptparser.add_argument ("port", nargs = '?', default = DEFPORT, type = int,
                        help = "Destination port (default: {})".format (DEFPORT))
pptparser.add_argument ("-d", "--daemon", action = "store_true", default = False,
                        help = "Run as daemon.  Requires a log file name to be specified.")
pptparser.add_argument ("--pid-file", metavar = "FN",
                        default = DEFPIDFILE,
                        help = "PID file, if daemon (default: {})".format (DEFPIDFILE))
pptparser.add_argument ("-L", "--log-file", metavar = "FN",
                        help = "Log file (default: log to stderr).  Required if daemon.")
pptparser.add_argument ("-e", "--log-level", default = "INFO",
                        metavar = "L",
                        choices = ("TRACE", "DEBUG", "INFO",
                                   "WARNING", "ERROR"),
                        help = "Log level (default: INFO)")
pptparser.add_argument ("-k", "--keep", type = int, default = 0, metavar = "N",
                        help = """Number of log files to keep with nightly
rotation.  Requires a log file name to be specified.
Default = no nightly rotation.""")

class pidfile:
    def __init__ (self, fn):
        self.fn = fn

    def __enter__ (self):
        try:
            f = open (self.fn, "wt")
        except Exception as exc:
            logging.exception ("failure creating pidfile %s", self.fn)
            return
        f.write ("%d\n" % os.getpid ())
        f.close ()

    def __exit__ (self, exc_type, exc_value, traceback):
        try:
            os.remove (self.fn)
        except Exception as exc:
            logging.exception ("error removing pidfile %s", self.fn)
            return
        
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
            logging.debug ("Stopping thread %s", self.name)
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
        self.term.setTimeout (IOTIMEOUT)
        while not self.stopnow:
            data = self.term.read ()
            logging.trace ("%d bytes from terminal", len (data))
            if data:
                self.lastio = time.time ()
                try:
                    self.sock.sendall (data)
                    logging.trace ("data sent to PLATO")
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
        slist = [ self.sock ]
        wlist = [ ]
        while not self.stopnow:
            r, w, x = select.select (slist, wlist, slist, IOTIMEOUT)
            if x:
                logging.debug ("Exiting due to exception from select ()")
                break
            if not r:
                logging.trace ("No data from PLATO")
                continue
            try:
                data = self.sock.recv (RECVMAX)
            except socket.error:
                logging.exception ("Receiving on socket to PLATO")
                return
            logging.trace ("%d bytes from PLATO", len (data))
            if data:
                data = data.replace (b"\377\377", b"\377")
                self.lastio = time.time ()
                self.term.write (data)
                logging.trace ("data sent to terminal")
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
    term.setTimeout (LOCALTIMEOUT)
    while True:
        pressnext (term)
        c = term.read (1)
        if c == NEXT or c == STOP1:
            return c
        if c == ANS:
            try:
                addr = socket.gethostbyname (socket.gethostname ())
            except socket.error:
                addr = "addr unknown"
            msg = INFO_MSG.format (PPTVER, PPTREV, addr, term.port)
            term.write (msg)
            time.sleep (8)

def talk (host, port, term, action):
    """ Connect to the PLATO host/port, and talk to it until timeout
    or disconnect.
    """
    interrupted = False
    if port == DEFPORT and action == STOP1:
        port = STOP1PORT
    logging.info ("Starting connection to {} port {}".format (host, port))
    try:
        sock = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
        logging.trace ("connecting")
        sock.connect ((host, port))
        logging.debug ("connected")
    except socket.error as e:
        logging.exception ("Error while connecting to %s %d", host, port)
        term.write (CERR_MSG.format (e))
        time.sleep (5)    # Allow time for people to see the message
        return
    inbound = fromcyber (sock, term)
    outbound = tocyber (sock, term)
    logging.trace ("threads active")
    # Both I/O threads are started.  Loop, mostly sleeping, looking
    # for inactivity or loss of connection.  If so, exit.
    # But first, send the supplied key to PLATO.
    sock.send (action)
    try:
        while True:
            time.sleep (IOTIMEOUT)
            now = time.time ()
            # Check inbound thread
            if not inbound.is_alive ():
                logging.debug ("Exiting due to inbound exit")
                break
            if inbound.lastio and now - inbound.lastio > TIMEOUT:
                logging.debug ("Exiting due to inbound timeout")
                break
            # Check outbound thread
            if not outbound.is_alive ():
                logging.debug ("Exiting due to outbound exit")
                break
            if outbound.lastio and now - outbound.lastio > TIMEOUT:
                logging.debug ("Exiting due to outbound timeout")
                break
    except KeyboardInterrupt:
        interrupted = True
    logging.info ("Closing down connection")
    try:
        sock.close ()
    except socket.error:
        pass
    inbound.stop ()
    outbound.stop ()
    if interrupted:
        raise KeyboardInterrupt
        
def mainloop (term, host, port):
    logging.info ("Starting isc.py for terminal {} to host {} port {}".format (term, host, port))
    if UART:
        uart = "UART{}".format (term)
        term = "/dev/ttyO{}".format (term)
        logging.info ("Configuring %s", uart)
        UART.setup (uart)
    term = serial.Serial (port = term, baudrate = SPEED,
                          parity = 'E', bytesize = 7)
    while True:
        action = getaction (term)
        talk (host, port, term, action)

def sighandler (signum, frame):
    global signalled
    signalled = True
    raise KeyboardInterrupt

if __name__ == "__main__":
    p = pptparser.parse_args ()
    daemoncontext = None
    logging.trace = trace
    logging.addLevelName (TRACE, "TRACE")
    if p.log_file:
        if p.keep:
            h = logging.handlers.TimedRotatingFileHandler (filename = p.log_file,
                                                           when = "midnight",
                                                           backupCount = p.keep)
        else:
            h = logging.FileHandler (filename = p.log_file, mode = "w")
        # If we run as daemon, we want to keep the handler's stream open
    else:
        if p.keep:
            print ("--keep requires --log-file")
            sys.exit (1)
        if p.daemon:
            print ("--daemon requires --log-file")
            sys.exit (1)
        h = logging.StreamHandler (sys.stderr)
    rootlogger = logging.getLogger ()
    fmt = logging.Formatter ("%(asctime)s: %(message)s")
    h.setFormatter (fmt)
    rootlogger.addHandler (h)
    rootlogger.setLevel (p.log_level)
    # Handle SIGTERM as a sign to quit
    global signalled
    signalled = False
    signal.signal (signal.SIGTERM, sighandler)
    try:
        addr = socket.gethostbyname (socket.gethostname ())
    except socket.error:
        addr = "addr unknown"
    msg = "ppy.py {} ({}) on {} {}".format (PPTVER, PPTREV, addr, p.term)
    logging.info (msg)
    try:
        if p.daemon:
            daemoncontext = DaemonContext (files_preserve = [ h.stream ],
                                           pidfile = pidfile (p.pid_file))
            logging.info ("Becoming daemon")
            daemoncontext.open ()
        mainloop (p.term, p.host, p.port)
    except SystemExit as exc:
        logging.info ("Exiting: {}".format (exc))
    except KeyboardInterrupt:
        if signalled:
            logging.info ("Exiting due to SIGTERM")
        else:
            logging.info ("Exiting due to Ctrl/C")
    except Exception:
        logging.exception ("Exception caught in main")
    finally:
        logging.info ("%s shut down", sys.argv[0])
        logging.shutdown ()
        if daemoncontext:
            daemoncontext.close ()
