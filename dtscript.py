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
DEBUG = False

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
        """Read some more network data and append it to pendingdata.
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
        if DEBUG:
            for c in ret:
                print "%02x" % ord (c),
            print
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
    def __init__ (self, port = 5006):
        """port is the operator interface port number, default
        is 5006.
        """
        threading.Thread.__init__ (self)
        Connection.__init__ (self, "localhost", port)
        self.settimeout (5)
        self.stopnow = False
        self.fixedtext = elist ()
        self.response = ""
        self.responses = 0
        self.status = elist ()
        self.locked = False
        self.debug = False
        self.statusdict = { }
        self.start ()
        self.operators = 1

    def run (self):
        """Collect data from DtCyber.  We sort out the various
        types of data we receive and salt away most of it in
        a more convenient form.

        In particular, the status lines are captured several ways:
        1. self.status is an array, each entry is a status line.
        2. self.statusdict is a dictionary indexed by a tuple
           of two integers, the channel and equipment number of
           one of the devices.  The value is the status for that device.
        3. The Oper object string representation is the whole status
           text, as it would appear on the screen (one line per entry).

        In addition, the current lock and debug states are extracted,
        as well as the current operator connection count.
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
                        if len (v) > 15:
                            self.operators = int (v[15:].split ()[0])
                        else:
                            self.operators = 1
                    else:
                        fields = v.split (None, 3)
                        if len (fields) >= 3:
                            try:
                                skey = (int (fields[1], 8), int (fields[2], 8))
                                self.statusdict[skey] = fields[3]
                            except ValueError:
                                pass
                            except IndexError:
                                self.statusdict[skey] = ""
            except socket.timeout:
                pass
            except EOFError:
                break
        self.stopnow = True

    def stop (self):
        if not self.stopnow:
            self.stopnow = True
            self.join ()

    def command (self, text):
        """Send a command string.  Returns the response from DtCyber,
        if one was received within 3 seconds.
        """
        seq = self.responses
        self.sendall (struct.pack ("<BB", 0, len (text)) + text)
        for i in xrange (20):
            if self.responses != seq:
                return self.response
            time.sleep (0.3)

    def __repr__ (self):
        """The representation of the object is the current status
        area (right screen of the dtoper display, but in full rather
        than limited to what fits in 30 or so lines).
        """
        return '\n'.join (self.status)

class Pterm (Connection, threading.Thread):
    """A connection to an NIU port, i.e., a "classic" type pterm.
    It includes a thread that collects data from DtCyber and updates
    local state to reflect what it hears.
    """
    # Keycodes
    NEXT  = 026
    ERASE = 023
    BACK  = 030
    STOP  = 032
    TAB   = 014
    ASSIGN = 015
    SUPER = 020
    SUB   = 021
    SQUARE = 034
    ANS   = 022
    COPY  = 033
    DATA  = 031
    EDIT  = 027
    MICRO = 024
    HELP  = 025
    LAB   = 035
    SHIFT = 040

    # A blank line
    line64 = 64 * ' '

    def __init__ (self, host = "localhost", port = 5004):
        """host is the host name to connect to, default is localhost.
        port is the pterm port number, default is 5004.
        Note that this must be an NIU ('classic' protocol) port;
        ASCII mode is not supported.
        """
        threading.Thread.__init__ (self)
        Connection.__init__ (self, host, port)
        self.settimeout (5)
        self.lines = [ ]
        for i in xrange (32):
            self.lines.append (None)
        self.fserase ()
        self.station = None
        self.stopnow = False
        self.start ()
        
    def run (self):
        """Collect data from DtCyber.  Text mode output is copied
        into the lines list, which is an array of 32 lines corresponding
        to the 32 lines of the coarse grid layout.  The port number
        status message is also processed.  Echo commands are handled
        by sending back the matching echo key (so this terminal looks
        like a Plato IV terminal).  Non-text output is ignored.
        """
        while True:
            try:
                if self.stopnow:
                    self.close ()
                    break
                byte = ord (self.read (1))
                if byte & 0200:
                    print "output out of sync"
                    continue
                word = self.read (2)
                word = byte << 12 | \
                       ((ord (word[0]) & 077) << 6) | \
                       (ord (word[1]) & 077)
                if DEBUG:
                    print "%07o" % word
                if word & 01000000:
                    # Data word
                    if self.mode == 3:
                        # Character plotting mode
                        self.char (word >> 12)
                        self.char (word >> 6)
                        self.char (word)
                else:
                    # control word
                    cmd = word >> 15
                    if cmd == 0:
                        # NOP command
                        if (word & 077000) == 042000:
                            # Station number report code
                            self.station = word & 0777
                    elif cmd == 1:
                        # Load Mode command
                        if word & 1:
                            self.fserase ()
                        mode = (word >> 1) & 037
                        self.wemode = mode & 3
                        self.mode = mode >> 2
                    elif cmd == 2:
                        # Load Coordinate command
                        coord = word & 0777
                        if word & 01000:
                            self.y = coord
                        else:
                            self.x = coord
                    elif cmd == 3:
                        key = (word & 0177) + 0200
                        self.sendkey (key)
            except socket.timeout:
                pass
            except EOFError:
                break
        self.stopnow = True
        
    def stop (self):
        if not self.stopnow:
            self.stopnow = True
            self.join ()

    def fserase (self):
        self.mode = 3          # char
        self.wemode = 3        # rewrite
        self.mem = 0           # char memory number
        self.x = 0
        self.y = 496
        self.uncover = False
        self.arrow = False     # last char was not arrow (prompt)
        for i in xrange (32):
            self.lines[i] = self.line64
            
    def sendkey (self, key):
        key &= 01777
        #print "sending key", key
        data = struct.pack ("<BB", key >> 7, (0200 | key & 0177))
        self.sendall (data)
        self.arrow = False

    def waitarrow (self, wait):
        """Wait the specified number of seconds for an arrow to appear.
        Returns True if there was an arrow, False if not.
        """
        delay = wait / 10.0
        if delay > 1:
            delay = 1
        while wait > 0:
            if self.arrow:
                return True
            time.sleep (delay)
            wait -= delay
        return False

    # This table is nearly the same as the one in charset.c, except
    # for the entries for ctrl-i, ctrl-j, and rubout, which are
    # TAB, NEXT, and ERASE respectively.
    asciiToPlato= \
       (  -1,    022,    030,    033,    031,    027,    064,    013,
         025,    014,    026,     -1,    035,    024,     -1,     -1,
         020,    034,     -1,    032,    062,     -1,     -1,     -1,
         012,    021,     -1,     -1,     -1,     -1,     -1,     -1,
        0100,   0176,   0177, 074044,   0044,   0045, 074016,   0047,
        0051,   0173,   0050,   0016,   0137,   0017,   0136,   0135,
        0000,   0001,   0002,   0003,   0004,   0005,   0006,   0007,
        0010,   0011,   0174,   0134,   0040,   0133,   0041,   0175,
       074005,  0141,   0142,   0143,   0144,   0145,   0146,   0147,
        0150,   0151,   0152,   0153,   0154,   0155,   0156,   0157,
        0160,   0161,   0162,   0163,   0164,   0165,   0166,   0167,
        0170,   0171,   0172,   0042, 074135,   0043, 074130,   0046,
       074121,  0101,   0102,   0103,   0104,   0105,   0106,   0107,
        0110,   0111,   0112,   0113,   0114,   0115,   0116,   0117,
        0120,   0121,   0122,   0123,   0124,   0125,   0126,   0127,
        0130,   0131,   0132, 074042, 074151, 074043, 074116,    023,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,    074151,  -1,
          -1,    074143,  -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,    074115,  -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
         0141121,0141105,0141130,0141116,0141125, -1,     -1,   0143103,
         0145121,0145105,0145130,0145125,0151121,0151105,0151130,0151125,
          -1,    0156116,0157121,0157105,0157130, -1, 0157125,  0012,
          -1,    0165121,0165105,0165130,0165125,0171105, -1,     -1,
         0101121,0101105,0101130,0101116,0101125, -1,     -1,   0103103,
         0105121,0105105,0105130,0105125,0111121,0111105,0111130,0111125,
          -1,    0116116,0117121,0117105,0117130, -1, 0117125,  0013,
          -1,    0125121,0125105,0125130,0125125,0131105, -1,   0131125
       )

    def sendstr (self, s, term = None, wait = None):
        """Send a text string, converting from ASCII as we go.
        There is no delay in here, so don't send too much.
        If term is specified, send that key after the string.
        If wait is specified, wait that long (in seconds) for
        an arrow to appear before sending the string.
        """
        if wait:
            self.waitarrow (wait)
        for c in s:
            key = self.asciiToPlato[ord (c)]
            if key == -1:
                continue
            if key >> 9:
                k1 = key >> 9
                k2 = key & 0777
                self.sendkey (k1)
                if k1 != 074:
                    self.sendkey (074)
                self.sendkey (k2)
            else:
                self.sendkey (key)
        if term:
            self.sendkey (term)

    rom = ( ":abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.\xF7[]%\xD7\xAB'\"!;<>_?\xBB ",
            "#ABCDEFGHIJKLMNOPQRSTUVWXYZ~\xA8^\xB4`    ~    {}&  |\xB0     \xB5       @\\ " )

    def char (self, c):
        """Display a character at current x/y.
        """
        if self.mem > 1:
            # not rom character, ignore it
            return
        c &= 077
        if c == 077:
            # uncover
            self.uncover = not self.uncover
            return
        if self.uncover:
            # handle uncover codes
            self.uncover = False
            if c == 010:
                # backspace
                self.x = (self.x - 8) & 0777
            elif c == 011:
                # "tab"
                self.x = (self.x + 8) & 0777
            elif c == 012:
                # line feed
                self.y = (self.y - 16) & 0777
            elif c == 013:
                # vertical tab
                self.y = (self.y + 16) & 0777
            elif c == 014:
                # form feed
                self.x, self.y = 0, 496
            elif c == 015:
                # carriage return
                self.x = 0
                self.y = (self.y - 16) & 0777
            elif c == 016:
                # super
                self.y = (self.y + 5) & 0777
            elif c == 017:
                # sub
                self.y = (self.y - 5) & 0777
            elif c >= 020 and c <= 027:
                self.mem = c - 020
        else:
            cx = self.x // 8
            # line 0 is the top line
            cy = 31 - (self.y // 16)
            if self.wemode & 1:
                # mode write or rewrite
                # Set the arrow flag, unless this is a space.
                if self.mem > 1 or c != 055:
                    self.arrow = self.mem == 0 and c == 076
                c = self.rom[self.mem][c]
            else:
                # mode erase, write a space
                self.arrow = False
                c = ' '
            l = self.lines[cy]
            l1 = l[:cx]
            l2 = l[cx + 1:]
            self.lines[cy] = l1 + c + l2
            self.x = (self.x + 8) & 0777

    def __repr__ (self):
        """The representation of the object is the current contents
        of the screen, as a string of 32 lines each 64 characters long.
        """
        return '\n'.join (self.lines)

    def login (self, user, group, passwd = None, waitauthor = True):
        self.sendkey (self.NEXT)
        self.sendstr (user, self.NEXT, 10)
        self.sendstr (group, self.SHIFT + self.STOP, 10)
        if passwd:
            self.sendstr (passwd, self.NEXT, 10)
        # If requested, handle any extraneous pages before author mode, 
        # like a message page or a consultant signon page.
        if waitauthor:
            for i in range(4):
                if self.waitarrow (5):
                    break
                self.sendkey (self.NEXT)

    def logout (self):
        while "Press  NEXT  to begin" not in str (self):
            self.sendkey (self.SHIFT + self.STOP)
            self.waitarrow (4)
            

class Dd60 (Connection, threading.Thread):
    """A connection to the DtCyber console (green tubes).  It includes
    a thread that collects data from DtCyber and updates local
    state to reflect what it hears.

    The last full screen worth of content is available via attribute
    'screen' which is the left and right screens as a tuple;  each
    screen is an array of 32 strings, each 64 characters long, containing
    the content of that screen.
    """
    # A blank line
    line64 = 64 * ' '

    # Mode codes
    LEFT = 0
    RIGHT = 4
    SMALL = 0
    MEDIUM = 1
    LARGE = 2
    DOT = 3

    # Command codes (output stream non-data codes).  Unlisted codes
    # are ignored.
    SETX = 0200
    SETY = 0210
    SETMODE = 0240
    ENDBLOCK = 0250
    
    def __init__ (self, port = 5007, interval = 3):
        """port is the console port number, default is 5007.
        interval is the update interval, default is 3 seconds.
        Note that the interval can be set quite short, but
        values shorter than a second or two are not recommended
        because of excessive overhead.
        """
        threading.Thread.__init__ (self)
        Connection.__init__ (self, "localhost", port)
        self.settimeout (5)
        self.erase ()
        self.stopnow = False
        self.interval = -1
        self.setinterval (interval)
        self.sendkey (070)                 # XON to start output flow
        self.mode = self.DOT + self.LEFT   # no display until mode set
        self.x = 0
        self.y = 31
        self.start ()
        self.screen = self.erase ()
        self.pending = self.erase ()

    def erase (self):
        left = [ ]
        right = [ ]
        for i in xrange (32):
            left.append (self.line64)
            right.append (self.line64)
        return left, right
    
    def next (self):
        """Return next byte of Cyber output, as an integer.
        """
        return ord (self.read (1))
    
    def run (self):
        """Collect data from DtCyber, and store it away into the
        left or right screen arrays.
        """
        while True:
            try:
                if self.stopnow:
                    self.close ()
                    break
                ch = self.next ()
                self.process_output (ch)
            except socket.timeout:
                pass
            except EOFError:
                break
        self.stopnow = True

    def stop (self):
        if not self.stopnow:
            self.stopnow = True
            self.join ()

    def sendkey (self, key):
        self.sendall (chr (key))
        
    def setinterval (self, interval):
        if self.interval != interval:
            self.interval = interval
            if interval > 63:
                interval = 63
            if interval < 1:
                self.sendkey (0200 + int (interval * 50))
            else:
                self.sendkey (0300 + int (interval))

    # This table is nearly the same as the one in charset.c, except
    # that the entry for rubout is ERASE.
    asciiToConsole = \
    (   0,      0,      0,      0,      0,      0,      0,      0,
        061,    0,      060,    0,      0,      060,    0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,
        062,    0,      0,      0,      0,      0,      0,      0,
        051,    052,    047,    045,    056,    046,    057,    050,
        033,    034,    035,    036,    037,    040,    041,    042,
        043,    044,    0,      0,      0,      054,    0,      0,
        0,      01,     02,     03,     04,     05,     06,     07,
        010,    011,    012,    013,    014,    015,    016,    017,
        020,    021,    022,    023,    024,    025,    026,    027,
        030,    031,    032,    053,    0,      055,    0,      0,
        0,      01,     02,     03,     04,     05,     06,     07,
        010,    011,    012,    013,    014,    015,    016,    017,
        020,    021,    022,    023,    024,    025,    026,    027,
        030,    031,    032,    0,      0,      0,      0,      061 )
    
    def sendstr (self, text):
        """Send a text string, converting from ASCII as we go.
        There is no delay in here, so don't send too much.
        """
        for c in text:
            key = self.asciiToConsole[ord (c)]
            if key == 0:
                continue
            self.sendkey (key)

    def __repr__ (self):
        """The representation of the object is the current contents
        of the right screen.
        """
        return self.screentext (1)

    def screentext (self, n):
        """Returns the current contents of the specified screen
        (0 or 1), as a string, 32 lines of 64 characters each.
        """
        return '\n'.join (self.screen[n])

    # This table comes from charset.c, replacing 0 (unused) entries
    # by blank
    consoleToAscii = \
    ( ' ',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
      'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
      'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
      'X',    'Y',    'Z',    '0',    '1',    '2',    '3',    '4',
      '5',    '6',    '7',    '8',    '9',    '+',    '-',    '*',
      '/',    '(',    ')',    ' ',    '=',    ' ',    ',',    '.',
      ' ',    ' ',    ' ',    ' ',    ' ',    ' ',    ' ',    ' ',
      ' ',    ' ',    ' ',    ' ',    ' ',    ' ',    ' ',    ' ' )

    def process_char (self, ch):
        """Process a data byte.  If not in dot mode, store it in the
        correct position of the current line.  Medium and large chars
        are handled by storing the char at the current X/Y and skipping
        one (medium) or three (large) character positions.
        """
        if self.mode & self.RIGHT:
            side = 1
        else:
            side = 0
        mode = self.mode & 3
        if mode != self.DOT:
            line = self.pending[side][self.y]
            
            self.pending[side][self.y] = line[:self.x] + self.consoleToAscii[ch] + line[self.x + 1:]
            x = self.x + 1
            if mode == self.MEDIUM:
                x += 1
            elif mode == self.LARGE:
                x += 3
            self.x = x & 077

    def process_output (self, ch):
        """Process Cyber output data.  Sort control codes (upper bit set)
        from data codes (upper bit clear). 
        """
        if ch & 0200:
            # Control code
            action = ch & 0270
            if action == self.SETX:
                self.x = (((ch << 8) + self.next ()) & 0770) / 8
            elif action == self.SETY:
                self.y = 31 - (((ch << 8) + self.next ()) & 0760) / 16
            elif action == self.SETMODE:
                self.mode = ch & 7
            elif action == self.ENDBLOCK:
                self.screen = self.pending
                self.pending = self.erase ()
        else:
            self.process_char (ch)
