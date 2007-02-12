#!/usr/bin/env python

"""Tool for scripting interaction with DtCyber

This package provides a collection of classes for interacting with
DtCyber via any of the externally visible network ports: console,
operator interface, pterm (NIU form only).
"""

import os
import sys
import time
import socket
import threading
import struct

NETMAX = 4096

class Connection (socket.socket):
    """A network connection.  Derived from socket, with some more
    read methods.
    """
    def __init__ (self, host, port):
        """Create a connection to the given host/port.
        """
        socket.socket.__init__ (self, socket.AF_INET, socket.SOCK_STREAM)
        self.connect ((host, port))
        self.pendingdata = ""

    def readmore (self):
        """Read some more network data and add it to pendingdata.
        """
        new = self.recv (NETMAX)
        if not new:
            self.close ()
            raise EOFError
        self.pendingdata += new
            
    def read (self, bytes):
        """Read exactly the supplied number of bytes.
        """
        while len (self.pendingdata) < bytes:
            self.readmore ()
        ret = self.pendingdata[:bytes]
        self.pendingdata = self.pendingdata[bytes:]
        return ret

    def readmin (self, minbytes = 1):
        """Read at least the supplied number of bytes.  Default length
        is one, i.e., read any non-null network data.
        """
        while len (self.pendingdata) < bytes:
            self.readmore ()
        ret = self.pendingdata
        self.pendingdata = ""
        return ret
        
    def readtlv (self):
        """Read a TLV coded item from the network.  Returns T and V
        as a tuple.
        """
        tl = self.read (2)
        t = ord (tl[0])
        l = ord (tl[1])
        v = self.read (l)
        return t, v
    
class elist (list):
    """A list that automatically grows when you index it.  Skipped
    entries are set to empty strings.
    """
    def __setitem__ (self, i, val):
        if len (self) > abs (i):
            list.__setitem__ (self, i, val)
        elif i < 0:
            raise IndexError
        else:
            while len (self) < i:
                self.append ("")
            self.append (val)
            
class Oper (Connection, threading.Thread):
    """A connection to the DtCyber operator interface.  It includes
    a thread that collects data from DtCyber and updates local
    state to reflect what it hears.
    """
    def __init__ (self, host, port):
        threading.Thread.__init__ (self)
        Connection.__init__ (self, host, port)
        self.settimeout (5)
        self.stopnow = False
        self.fixedtext = elist ()
        self.response = ""
        self.responses = 0
        self.status = elist ()
        self.locked = False
        self.debug = False
        self.start ()

    def run (self):
        """Collect data from DtCyber.  We sort out the various
        types of data we receive and salt away most of it in
        a more convenient form.
        """
        while True:
            try:
                if self.stopnow:
                    self.close ()
                    break
                t, v = self.readtlv ()
                if t == 2:
                    # OpText -- fixed text (left screen material)
                    x, y, size, bold = struct.unpack ("<HHBB", v[:6])
                    i = (0760 - y) / 16
                    self.fixedtext[i] = v[6:]
                elif t == 3 or t == 5:
                    # OpSyntax or OpInitialized, ignore
                    pass
                elif t == 1:
                    # OpReply
                    self.response = v
                    self.responses += 1
                elif t == 4:
                    # OpStatus
                    i = ord (v[0])
                    v = v[1:]
                    self.status[i] = v
                    if i == 0:
                        # System status line, update locked and debug status
                        self.locked = not v.startswith ("UNLOCKED")
                        self.debug = v[10:15] == "DEBUG"
            except socket.timeout:
                pass
            except EOFError:
                break
        self.stopnow = True

    def stop (self):
        if not self.stopnow:
            self.stopnow = True
            self.join ()

    def sendall (self, text):
        """Send specified string, don't return until all
        has been sent.
        """
        while True:
            sent = self.send (text)
            if sent == len (text):
                return
            text = text[sent:]
            sleep (0.3)
        
    def command (self, text):
        """Send a command string.
        """
        # TODO: why does this need to be sent in two parts?
        self.sendall (struct.pack ("<BB", 0, len (text)))
        self.sendall (text)
        
