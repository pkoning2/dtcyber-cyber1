#!/usr/bin/env python

"""Cyber FTP client and server

This program talks to the Cyber mini-FTP, taking either the client or
the server (daemon) role depending on command line arguments.  It only
transfers single-record files since record marks (or file marks) have
no representation in the Unix file system.  (At some point this could
be added if desired, by storing the data in tape format as a .tap
file.)
"""

import os
import sys
import getopt
import time
import socket
import struct

NETMAX = 8192
CFTP = 6021

def usage ():
    print "usage: %s [-d] {get | put} host lfn rfn" % sys.argv[0]
    print "       %s -D" % sys.argv[0]

class Connection (socket.socket):
    """A network connection.  Derived from socket, with some more
    read methods.
    """
    def __init__ (self, host, port = CFTP, daemon = False):
        """Create a connection to the given host/port.
        If daemon is True, listen for incoming connections.
        """
        socket.socket.__init__ (self, socket.AF_INET, socket.SOCK_STREAM)
        if daemon:
            self.setsockopt (socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.bind (("0.0.0.0", port))
            self.listen (5)
            self.dsock = None
        else:
            self.connect ((socket.gethostbyname (host), port))
        self.pendingdata = ""
        self.nextbyte = 0
        self.daemon = daemon
        
    def accept (self):
        if not self.daemon:
            print "Wrong accept call"
        dsock, fromaddr = socket.socket.accept (self)
        self.dsock = dsock

    def shutdown (self, flag = socket.SHUT_RDWR):
        if self.daemon:
            sock = self.dsock
        else:
            sock = self
        try:
            socket.socket.shutdown (sock, flag)
        except socket.error:
            pass
    def readmore (self):
        """Read some more network data and append it to pendingdata.
        """
        if self.daemon:
            sock = self.dsock
        else:
            sock = self
        new = sock.recv (NETMAX)
        if not new:
            sock.close ()
            self.dsock = None
            raise EOFError
        self.pendingdata += new
            
    def read (self, bytes):
        """Read exactly the supplied number of bytes.
        """
        while len (self.pendingdata) - self.nextbyte < bytes:
            self.readmore ()
        ret = self.pendingdata[self.nextbyte:self.nextbyte + bytes]
        self.nextbyte += bytes
        return ret

    def readmin (self, minbytes = 1):
        """Read at least the supplied number of bytes.  Default length
        is one, i.e., read any non-null network data.
        """
        while len (self.pendingdata) - self.nextbyte < bytes:
            self.readmore ()
        ret = self.pendingdata[self.nextbyte:]
        self.pendingdata = ""
        self.nextbyte = 0
        return ret
        
    def sendall (self, text):
        """Send specified string, don't return until all
        has been sent.
        """
        if self.daemon:
            sock = self.dsock
        else:
            sock = self
        while True:
            sent = sock.send (text)
            if sent == len (text):
                return
            text = text[sent:]
            time.sleep (0.3)

    def readwords (self, words):
        """Read exactly the supplied number of words."""
        return bytes2words (self.read (wc2bc (words)))

    def sendwords (self, words):
        """Send the supplied list of words."""
        self.sendall (words2bytes (words))
        
def wc2bc (w):
    """Convert word count to byte count"""
    pairs, odd = divmod (w, 2)
    return pairs * 15 + odd * 8

def bc2wc (b):
    """Convert byte count to word count"""
    pairs, odd = divmod (b, 15)
    if odd and odd != 8:
        print "warning: funny byte count", b, pairs, odd
    return pairs * 2 + odd / 8

def words2bytes (wl):
    """Convert string of words to string of bytes"""
    bl = [ ]
    odd = False
    for w in wl:
        if odd:
            bytes = struct.pack (">Q", w | carry)
            bl.append (bytes)
            odd = False
        else:
            bytes = struct.pack (">Q", w << 4)
            bl.append (bytes[:7])
            carry = (w & 0x0f) << 60
            odd = True
    if odd:
        bl.append (chr (carry >> 56))
    return ''.join (bl)

def bytes2words (bl):
    """Convert string of bytes to list of words"""
    wl = [ ]
    odd = False
    while bl:
        if odd:
            if len (bl) < 8:
                if len (bl) != 1:
                    print "incorrect string length, ends with",
                    len(bl), "bytes"
                break
            w = struct.unpack (">Q", bl[:8])[0]
            w &= (1 << 60) - 1
            wl.append (w)
            bl = bl[8:]
            odd = False
        else:
            if len (bl) < 8:
                print "incorrect string length, ends with", len(bl), "bytes"
                break
            w = struct.unpack (">Q", bl[:8])[0]
            w >>= 4
            wl.append (w)
            bl = bl[7:]
            odd = True
    return wl

def afn2d (name):
    """Convert an ASCII file name to display code."""
    c = 0
    shift = 54
    for n in name:
        n = n.lower ()
        if n >= 'a' and n <= 'z':
            n = ord (n) - ord ('a') + 1
        elif n >= '0' and n <= '9':
            n = ord (n) - ord ('0') + 033
        else:
            break
        c += (n << shift)
        shift -= 6
        if shift < 18:
            break
    return c

def a2d (s):
    """Convert an ASCII string to a sequence of display code words."""
    cl = [ ]
    c = 0
    shift = 54
    for n in s.lower ():
        d = ":abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.#[]%\"_!&'?<>@\\^;".find (n)
        if d >= 0:
            c += (d << shift)
            shift -= 6
            if shift < 0:
                cl.append (c)
                c = 0
                shift = 54
    if shift != 54:
        cl.append (c)
    return cl

def dw2a (w):
    """Convert a name in one word of display code to ascii."""
    name = [ ]
    shift = 54
    while shift >= 0:
        c = (w >> shift) & 077
        if c:
            name.append (":abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.#[]%\"_!&'?<>@\\^;"[c])
            shift -= 6
        else:
            break
    return "".join (name)

def d2a (l):
    """Convert a list of display code words to ascii."""
    cl = [ ]
    for c in l:
        cl.append (dw2a (c))
    return "".join (cl)

def transfer (sock, f, inbound):
    if inbound:
        while True:
            wc, flags = sock.readwords (2)
            if wc == 0:
                sock.sendwords ((0, 0, 0, 0))
                if flags == 2:
                    print "Done"
                else:
                    print "inbound transfer ended with unexpected flags", flags
                break
            else:
                f.write (sock.read (wc2bc (wc)))
    else:
        while True:
            data = f.read (1500)
            if data:
                wc = bc2wc (len (data))
                sock.sendwords ((wc, 0))
                sock.sendall (data[:wc2bc (wc)])
            else:
                sock.sendwords ((0, 2))
                status  = sock.readwords (4)
                if status[0]:
                    print "outbound transfer ended with error code", status[0], d2a (status[1:])
                else:
                    print "Done"
                break
    sock.shutdown ()
    f.close ()

def cftp (op, host, lfn, rfn, direct):
    """Client side operation.  Arguments are:
    op:      "get" or "put"
    host:    host name or address
    lfn:     local file name (file name at this end)
    rfn:     remote file name (must conform to NOS name rules)
    direct:  True if -d ("rfn" is a direct access file) is present
    """
    op = op.lower ()
    if op == "get":
        put = False
    elif op == "put":
        put = True
    else:
        print "Unrecognized action", op
        return
    if put:
        mode = "rb"
    else:
        mode = "wb"
    try:
        locfile = open (lfn, mode)
    except (IOError,OSError), err:
        print "error %d opening %s: %s" % (err.errno, err.strerror, lfn)
        return
    except:
        print "unexpected error opening", lfn
        return
    sock = Connection (host)
    nosfn = afn2d (rfn)
    if put:
        nosfn += 2
    if direct:
        nosfn += 1
    sock.sendwords ((nosfn, 0))
    resp = sock.readwords (4)
    if resp[0]:
        print op, rfn, "error code", resp[0], d2a (resp[1:])
        sock.close ()
        return
    transfer (sock, locfile, not put)
    
def cftpd ():
    """Server side operation.  This takes no arguments, but simply
    listens for connections and does what was asked.  It runs until
    interrupted.
    """
    sock = Connection (None, daemon = True)
    try:
        while True:
            sock.accept ()
            req, flags = sock.readwords (2)
            put = (req & 2) != 0
            direct = (req & 1) != 0
            if put:
                action = "put"
                mode = "wb"
            else:
                action = "get"
                mode = "rb"
            rfn = dw2a (req & (-4))
            print "Request is", action, "of", rfn
            if direct:
                print "(direct access)"
            try:
                locfile = open (rfn, mode)
            except (IOError,OSError), err:
                print "error %d opening %s: %s" % (err.errno, err.strerror, rfn)
                msg = [ err.errno ]
                msg.extend (a2d (err.strerror))
                while len (msg) < 4:
                    msg.append (0)
                sock.sendwords (msg[:4])
                sock.shutdown ()
                continue
            except:
                print "Error opening", rfn
                sock.sendwords ((9999, 0, 0, 0))
                sock.shutdown ()
                continue
            sock.sendwords ((0, 0, 0, 0))
            transfer (sock, locfile, put)
    except KeyboardInterrupt:
        print
        sock.close ()
        
def main (args):
    opts, args = getopt.getopt (args, "dD")
    daemon = False
    direct = False
    for opt, val in opts:
        if opt == "-D":
            daemon = True
        elif opt == "-d":
            direct = True
    if daemon:
        cftpd ()
    else:
        if len (args) != 4:
            usage ()
            sys.exit (1)
        cftp (args[0], args[1], args[2], args[3], direct)
        
if __name__ == "__main__":
    main (sys.argv[1:])
    
