#!/usr/bin/env python

"""Bridge utility for PPT to DtCyber

Copyright (C) 2011 by Paul Koning

This utility connects between a serial port and a DtCyber connection
to allow an ASCII mode PPT to be a DtCyber terminal.
"""

import sys
import socket
import threading
import serial

TERM = "/dev/tty.KeySerial1"
CYBER1 = "cyberserv.org"
PORT = 8005
SPEED = 2400
RECVMAX = 1024

class tocyber (threading.Thread):
    """A class for the thread that sends data to DtCyber
    """
    def __init__ (self, sock, term):
        threading.Thread.__init__ (self)
        self.term = term
        self.sock = sock
        self.start ()
        
    def run (self):
        """Read from the terminal and send to the socket.
        """
        while True:
            data = self.term.read ()
            self.sock.sendall (data)
            
class fromcyber (threading.Thread):
    """A class for the thread that receives data from DtCyber
    """
    def __init__ (self, sock, term):
        threading.Thread.__init__ (self)
        self.term = term
        self.sock = sock
        self.start ()
        
    def run (self):
        """Read from the socket and send to the terminal.
        """
        while True:
            data = self.sock.recv (RECVMAX)
            self.term.write (data)
            
if __name__ == "__main__":
    host = CYBER1
    term = TERM
    if len (sys.argv) > 2:
        TERM = sys.argv[2]
    if len (sys.argv) > 1:
        host = sys.argv[1]
    sock = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
    sock.connect ((host, PORT))
    term = serial.Serial (port = term, baudrate = SPEED)
    inbound = fromcyber (sock, term)
    outbound = tocyber (sock, term)
    while True:
        t = raw_input ()
        term.write (t)
        
