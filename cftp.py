#!/usr/bin/env python

"""Cyber FTP client and server

This program talks to the Cyber mini-FTP, taking either the client or
the server (daemon) role depending on command line arguments.  It only
transfers single-record files since record marks (or file marks) have
no representation in the Unix file system.  (At some point this could
be added if desired, by storing the data in tape format as a .tap
file.)

The protocol is a simple client-server protocol, transfering exactly
one file per connection, in either direction.  The protocol transfers
60-bit words, packed into 7.5 bytes big endian byte order.  Control
transfers are either 2 or 4 words (always an even length though that's
not always needed, that way it comes across as an integral number of
bytes).  Data transfers are also in words, rounded up to an even number
to avoid fractional bytes.

The description below shows the messages as 60-bit values, written
in CDC standard notation as if they were VFD arguments.  See the
CDC Compass manual for an explanation...

Hello message, server to client.  This is sent upon connection:
   60/MTU, 60/0
   
Operation request, client to server:
   42/filename, 24/0
   1/put, 1/direct, 46/0, 12/MTU

"MTU" is the receive buffer size, in words.  The data transfers are
broken up into chunks no larger than MTU.  MTU should be even.

"put" is 1 if the transfer is from client to server, 0 if server to client
"direct" is 1 if the server side file is (or should be created as) a
direct file.

Response, server to client:
   60/0 and 3 words of padding if ok
   60/errno and 3 words of message if error

If ok, then the transfer starts.  The encoding is the same no matter
what direction it goes in.  For a "get", the transfer immediately
follows the reponse from the server; for a "put", the client can
start the transfer as soon as it receives the (ok) response.
The transfer is broken up in chunks of length "MTU" or less.  Each
chunk is marked as regular data, EOR (the last data of a "record"),
EOF (which has no data) or EOI (which has no data).  Chunks follow
one after the other until the EOI chunk, which ends the transfer.
Alternatively, transfer can be aborted with an "abort" chunk (which
has no data).

Note that regular (non-EOR) transfers will normally be exactly
MTU in size, though this is not actually required.

Chunk header:
  60/wordcount, 60/0 for regular data
  60/wordcount, 60/1 for EOR
  60/0, 60/2 for EOF
  60/0, 60/3 for EOI
  60/0, 60/4 for abort

The chunk header is followed by "wordcount" rounded up to an even number
words of data, which means 7.5 * wordcount bytes.

Once the transfer completes, the receiver of the data replies with
a final status reply, and then does a "shutdown" on the connection.

Status reply:
  60/0 and 3 words of padding if ok
  60/errno and 3 words of message if error
"""

import os
import sys
import getopt
import time
import socket
import struct

MTU = 200
NETMAX = MTU * 15
CFTP = 6021

NORMAL = 0
EOR = 1
EOF = 2
EOI = 3
ABORT = 4
def usage ():
    print "usage: %s [-dv] {get | put} host[:port] lfn rfn" % sys.argv[0]
    print "       %s [-v] -D [port]" % sys.argv[0]

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
            if verbose:
                print "Connected to", host, port
        self.pendingdata = ""
        self.daemon = daemon
        
    def accept (self):
        if not self.daemon:
            print "Wrong accept call"
        dsock, fromaddr = socket.socket.accept (self)
        if verbose:
            print "Accepted connection from", fromaddr[0], fromaddr[1]
        self.dsock = dsock
        self.pendingdata = ""

    def shutdown (self, flag = socket.SHUT_RDWR):
        if self.daemon:
            sock = self.dsock
        else:
            sock = self
        try:
            socket.socket.shutdown (sock, flag)
        except socket.error:
            pass
        self.pendingdata = ""
        
    def readmore (self):
        """Read some more network data and append it to pendingdata.
        """
        if self.daemon:
            sock = self.dsock
        else:
            sock = self
        new = sock.recv (NETMAX)
        #if verbose:
        #    print len (new), "bytes received:", repr (new)
        if not new:
            sock.close ()
            self.dsock = None
            self.pendingdata = ""
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
        words = bytes2words (self.read (wc2bc (words)))
        if verbose:
            print "Read %d words:" % len (words)
            for w in words:
                print " %020o" % w
        return words
    
    def sendwords (self, words):
        """Send the supplied list of words."""
        self.sendall (words2bytes (words))
        if verbose:
            print "Sent %d words" % len (words)
            for w in words:
                print " %020o" % w
        
def wc2bc (w):
    """Convert word count to byte count"""
    pairs, odd = divmod (w, 2)
    return pairs * 15 + odd * 15

def bc2wc (b):
    """Convert byte count to word count"""
    pairs, odd = divmod (b, 15)
    if odd:
        print "warning: funny byte count", b, pairs, odd
    return pairs * 2

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
    """Convert a list of display code words to ascii.
    Conversion stops at end of line."""
    cl = [ ]
    for c in l:
        cl.append (dw2a (c))
        if (c & 077) == 0:
            break
    return "".join (cl)

def transfer (sock, f, inbound, mtu):
    even, odd = divmod (mtu, 2)
    if odd:
        print "Warning, odd MTU", mtu
    bytemax = even * 15
    if inbound:
        while True:
            wc, flags = sock.readwords (2)
            if wc == 0:
                if flags == EOI:
                    sock.sendwords ((0, 0, 0, 0))
                    print "Done"
                    break
                else:
                    print "inbound transfer, wc 0, flags", flags
            else:
                data = sock.read (wc2bc (wc))
                f.write (data[:wc2bc (wc)])
    else:
        while True:
            data = f.read (bytemax)
            if data:
                wc = bc2wc (len (data))
                sock.sendwords ((wc, 0))
                sock.sendall (data[:wc2bc (wc)])
            else:
                # end of file, send EOR then EOI
                sock.sendwords ((0, EOR))
                sock.sendwords ((0, EOI))
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
        print "error %d opening %s: %s" % (err.errno, lfn, err.strerror)
        return
    except:
        print "unexpected error opening", lfn
        return
    i = host.find (":")
    if i < 0:
        port = CFTP
    else:
        port = int (host[i + 1:])
        host = host[:i]
    sock = Connection (host, port)
    cmtu, pad = sock.readwords (2)
    nosfn = afn2d (rfn)
    flags = 0
    if put:
        flags += 1 << 59
    if direct:
        flags += 1 << 58
    if cmtu < 10:
        print "Bad MTU from server", cmtu
    cmtu = min (cmtu, MTU)
    flags += cmtu
    sock.sendwords ((nosfn, flags))
    resp = sock.readwords (4)
    if resp[0]:
        print op, rfn, "error code", resp[0], d2a (resp[1:])
        sock.close ()
        return
    transfer (sock, locfile, not put, cmtu)
    
def cftpd (args):
    """Server side operation.  This takes no arguments, but simply
    listens for connections and does what was asked.  It runs until
    interrupted.
    """
    if args:
        port = int (args[0])
    else:
        port = CFTP
    sock = Connection (None, port, True)
    while True:
        try:
            sock.accept ()
            sock.sendwords ((MTU, 0))
            reqfn, flags = sock.readwords (2)
            put = ((flags >> 59) & 1) != 0
            direct = ((flags >> 58) & 1) != 0
            cmtu = flags & 07777
            if put:
                action = "put"
                mode = "wb"
            else:
                action = "get"
                mode = "rb"
            rfn = dw2a (reqfn)
            print "Request is", action, "of", rfn, "mtu", cmtu
            if cmtu > MTU or cmtu < 10:
                print "Error, bad MTU from client", cmtu
                sock.shutdown ()
                continue
            if direct:
                print "(direct access)"
            try:
                locfile = open (rfn, mode)
            except (IOError,OSError), err:
                print "error %d opening %s: %s" % (err.errno, rfn, err.strerror)
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
            transfer (sock, locfile, put, cmtu)
        except KeyboardInterrupt:
            print
            if verbose:
                print len (sock.pendingdata), "bytes pending"
                print repr (sock.pendingdata)
                raise
            sock.close ()
            break
        except (IOError, OSError, EOFError), err:
            # some sort of network error; close connection and wait for another
            print "error %d transfering %s: %s" % (err.errno, rfn, err.strerror)
            sock.close ()
            
def main (args):
    opts, args = getopt.getopt (args, "dDv")
    daemon = False
    direct = False
    global verbose
    verbose = False
    for opt, val in opts:
        if opt == "-D":
            daemon = True
        elif opt == "-d":
            direct = True
        elif opt == "-v":
            verbose = True
    if daemon:
        cftpd (args)
    else:
        if len (args) != 4:
            usage ()
            sys.exit (1)
        cftp (args[0], args[1], args[2], args[3], direct)
        
if __name__ == "__main__":
    main (sys.argv[1:])
    
