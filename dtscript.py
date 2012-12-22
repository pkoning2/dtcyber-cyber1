#!/usr/bin/env python3

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
import re

NETMAX = 4096
DEBUG = False

class MyThread (threading.Thread):
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
            self.stopnow = True
            self.join ()

class Connection (socket.socket):
    """A network connection.  Derived from socket, with some more
    read methods.
    """
    def __init__ (self, host, port):
        """Create a connection to the given host/port.
        """
        socket.socket.__init__ (self, socket.AF_INET, socket.SOCK_STREAM)
        self.connect ((host, port))
        self.pendingdata = b""

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
                print("%02x %03o %3d" % (c, c, c))
        return ret

    def readmin (self, minbytes = 1):
        """Read at least the supplied number of bytes.  Default length
        is one, i.e., read any non-null network data.
        """
        while len (self.pendingdata) < bytes:
            self.readmore ()
        ret = self.pendingdata
        self.pendingdata = b""
        return ret
        
    def readtlv (self):
        """Read a TLV coded item from the network.  Returns T and V
        as a tuple.
        """
        tl = self.read (2)
        t = tl[0]
        l = tl[1]
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
            
statre = re.compile (r"DEBUG|UNLOCKED|ALERT|(\d+) operators")
class Oper (Connection, MyThread):
    """A connection to the DtCyber operator interface.  It includes
    a thread that collects data from DtCyber and updates local
    state to reflect what it hears.
    """
    def __init__ (self, port = 5006):
        """port is the operator interface port number, default
        is 5006.
        """
        MyThread.__init__ (self)
        Connection.__init__ (self, "localhost", port)
        self.settimeout (5)
        self.fixedtext = elist ()
        self.response = ""
        self.responses = 0
        self.status = elist ()
        self.locked = True
        self.debug = False
        self.plato_alert = False
        self.statusdict = { }
        self.operators = 1
        self.start ()

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
                    i = (0o760 - y) / 16
                    try:
                        self.fixedtext[i] = v[6:]
                    except IndexError:
                        print("bad index", i, "for", x, y, size, bold, v[6:])
                elif t == 3 or t == 5:
                    # OpSyntax or OpInitialized, ignore
                    pass
                elif t == 1:
                    # OpReply
                    self.response = v
                    self.responses += 1
                elif t == 4:
                    # OpStatus
                    i = v[0]
                    v = v[1:]
                    self.status[i] = v
                    if i == 0:
                        # System status line, update some status
                        self.debug = self.plato_alert = False
                        self.locked = True
                        self.operators = 1
                        for m in statre.finditer (v):
                            if m.group (1):
                                self.operators = int (m.group (1))
                            else:
                                ms = m.group (0)
                                if ms == "DEBUG":
                                    self.debug = True
                                elif ms == "UNLOCKED":
                                    self.locked = False
                                elif ms == "ALERT":
                                    self.plato_alert = True
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

    def command (self, text):
        """Send a command string.  Returns the response from DtCyber,
        if one was received within 3 seconds.
        """
        seq = self.responses
        self.sendall (struct.pack ("<BB", 0, len (text)) + text)
        for i in range (20):
            if self.responses != seq:
                return self.response
            time.sleep (0.3)

    def __repr__ (self):
        """The representation of the object is the current status
        area (right screen of the dtoper display, but in full rather
        than limited to what fits in 30 or so lines).
        """
        retval = '\n'.join (self.status)
        while retval.endswith ("\n\n"):
            retval = retval[:-1]
        return retval

class Pterm (Connection, MyThread):
    """A connection to an NIU port, i.e., a "classic" type pterm.
    It includes a thread that collects data from DtCyber and updates
    local state to reflect what it hears.
    """
    # Keycodes
    NEXT  = 0o26
    ERASE = 0o23
    BACK  = 0o30
    STOP  = 0o32
    TAB   = 0o14
    ASSIGN = 0o15
    SUPER = 0o20
    SUB   = 0o21
    SQUARE = 0o34
    ANS   = 0o22
    COPY  = 0o33
    DATA  = 0o31
    EDIT  = 0o27
    MICRO = 0o24
    HELP  = 0o25
    LAB   = 0o35
    SHIFT = 0o40

    # A blank line
    line64 = 64 * ' '

    def __init__ (self, host = "localhost", port = 5004):
        """host is the host name to connect to, default is localhost.
        port is the pterm port number, default is 5004.
        Note that this must be an NIU ('classic' protocol) port;
        ASCII mode is not supported.
        """
        MyThread.__init__ (self)
        Connection.__init__ (self, host, port)
        self.settimeout (5)
        self.lines = [ ]
        for i in range (32):
            self.lines.append (None)
        self.fserase ()
        self.station = None
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
                byte = self.read (1)[0]
                if byte & 0o200:
                    print("output out of sync")
                    continue
                word = self.read (2)
                word = byte << 12 | \
                       ((word[0] & 0o77) << 6) | \
                       (word[1] & 0o77)
                if DEBUG:
                    print("%07o" % word)
                if word & 0o1000000:
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
                        if (word & 0o77000) == 0o42000:
                            # Station number report code
                            self.station = word & 0o777
                    elif cmd == 1:
                        # Load Mode command
                        if word & 1:
                            self.fserase ()
                        mode = (word >> 1) & 0o37
                        self.wemode = mode & 3
                        self.mode = mode >> 2
                    elif cmd == 2:
                        # Load Coordinate command
                        coord = word & 0o777
                        if word & 0o1000:
                            self.y = coord
                        else:
                            self.x = coord
                    elif cmd == 3:
                        key = (word & 0o177) + 0o200
                        self.sendkey (key)
            except socket.timeout:
                pass
            except EOFError:
                break
        self.stopnow = True
        
    def fserase (self):
        self.mode = 3          # char
        self.wemode = 3        # rewrite
        self.mem = 0           # char memory number
        self.x = 0
        self.y = 496
        self.uncover = False
        self.arrow = False     # last char was not arrow (prompt)
        for i in range (32):
            self.lines[i] = self.line64
            
    def sendkey (self, key):
        key &= 0o1777
        #print "sending key", key
        data = struct.pack ("<BB", key >> 7, (0o200 | key & 0o177))
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
       (  -1,    0o22,    0o30,    0o33,    0o31,    0o27,    0o64,    0o13,
         0o25,    0o14,    0o26,     -1,    0o35,    0o24,     -1,     -1,
         0o20,    0o34,     -1,    0o32,    0o62,     -1,     -1,     -1,
         0o12,    0o21,     -1,     -1,     -1,     -1,     -1,     -1,
        0o100,   0o176,   0o177, 0o74044,   0o044,   0o045, 0o74016,   0o047,
        0o051,   0o173,   0o050,   0o016,   0o137,   0o017,   0o136,   0o135,
        0000,   0o001,   0o002,   0o003,   0o004,   0o005,   0o006,   0o007,
        0o010,   0o011,   0o174,   0o134,   0o040,   0o133,   0o041,   0o175,
       0o74005,  0o141,   0o142,   0o143,   0o144,   0o145,   0o146,   0o147,
        0o150,   0o151,   0o152,   0o153,   0o154,   0o155,   0o156,   0o157,
        0o160,   0o161,   0o162,   0o163,   0o164,   0o165,   0o166,   0o167,
        0o170,   0o171,   0o172,   0o042, 0o74135,   0o043, 0o74130,   0o046,
       0o74121,  0o101,   0o102,   0o103,   0o104,   0o105,   0o106,   0o107,
        0o110,   0o111,   0o112,   0o113,   0o114,   0o115,   0o116,   0o117,
        0o120,   0o121,   0o122,   0o123,   0o124,   0o125,   0o126,   0o127,
        0o130,   0o131,   0o132, 0o74042, 0o74151, 0o74043, 0o74116,    0o23,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,    0o74151,  -1,
          -1,    0o74143,  -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,    0o74115,  -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
         0o141121,0o141105,0o141130,0o141116,0o141125, -1,     -1,   0o143103,
         0o145121,0o145105,0o145130,0o145125,0o151121,0o151105,0o151130,0o151125,
          -1,    0o156116,0o157121,0o157105,0o157130, -1, 0o157125,  0o012,
          -1,    0o165121,0o165105,0o165130,0o165125,0o171105, -1,     -1,
         0o101121,0o101105,0o101130,0o101116,0o101125, -1,     -1,   0o103103,
         0o105121,0o105105,0o105130,0o105125,0o111121,0o111105,0o111130,0o111125,
          -1,    0o116116,0o117121,0o117105,0o117130, -1, 0o117125,  0o013,
          -1,    0o125121,0o125105,0o125130,0o125125,0o131105, -1,   0o131125
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
            key = self.asciiToPlato[c]
            if key == -1:
                continue
            if key >> 9:
                k1 = key >> 9
                k2 = key & 0o777
                self.sendkey (k1)
                if k1 != 0o74:
                    self.sendkey (0o74)
                self.sendkey (k2)
            else:
                self.sendkey (key)
        if term:
            self.sendkey (term)

    rom = ( ":abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.\u00F7[]%\u00D7\u00AB'\"!;<>_?\u00BB ",
            "#ABCDEFGHIJKLMNOPQRSTUVWXYZ~\u00A8^\u00B4`    ~    {}&  |\u00B0     \u00B5       @\\ " )

    def char (self, c):
        """Display a character at current x/y.
        """
        c &= 0o77
        if c == 0o77:
            # uncover
            self.uncover = not self.uncover
            return
        if self.uncover:
            # handle uncover codes
            self.uncover = False
            if c == 0o10:
                # backspace
                self.x = (self.x - 8) & 0o777
            elif c == 0o11:
                # "tab"
                self.x = (self.x + 8) & 0o777
            elif c == 0o12:
                # line feed
                self.y = (self.y - 16) & 0o777
            elif c == 0o13:
                # vertical tab
                self.y = (self.y + 16) & 0o777
            elif c == 0o14:
                # form feed
                self.x, self.y = 0, 496
            elif c == 0o15:
                # carriage return
                self.x = 0
                self.y = (self.y - 16) & 0o777
            elif c == 0o16:
                # super
                self.y = (self.y + 5) & 0o777
            elif c == 0o17:
                # sub
                self.y = (self.y - 5) & 0o777
            elif c >= 0o20 and c <= 0o27:
                self.mem = c - 0o20
        else:
            cx = self.x // 8
            # line 0 is the top line
            cy = 31 - (self.y // 16)
            if self.wemode & 1:
                # mode write or rewrite
                if self.mem > 1:
                    # Not rom char, clear arrow flag
                    self.arrow = False
                    c = ' '
                else:
                    if c != 0o55:
                        # Set the arrow flag, unless this is a space.
                        self.arrow = self.mem == 0 and c == 0o76
                    c = self.rom[self.mem][c]
            else:
                # mode erase, write a space
                self.arrow = False
                c = ' '
            l = self.lines[cy]
            l1 = l[:cx]
            l2 = l[cx + 1:]
            self.lines[cy] = l1 + c + l2
            self.x = (self.x + 8) & 0o777

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
            

class Dd60 (Connection, MyThread):
    """A connection to the DtCyber console (green tubes).  It includes
    a thread that collects data from DtCyber and updates local
    state to reflect what it hears.

    The last full screen worth of content is available via attribute
    'screen' which is the left and right screens as a tuple;  each
    screen is an array of 51 strings, each 64 characters long, containing
    the content of that screen.

    Why 51?  Because while the small character height is 8, it appears
    that there is a common convention to space things 10 units apart.
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
    SETX = 0o200
    SETY = 0o210
    SETMODE = 0o240
    ENDBLOCK = 0o250
    
    def __init__ (self, port = 5007, interval = 3):
        """port is the console port number, default is 5007.
        interval is the update interval, default is 3 seconds.
        Note that the interval can be set quite short, but
        values shorter than a second or two are not recommended
        because of excessive overhead.
        """
        MyThread.__init__ (self)
        Connection.__init__ (self, "localhost", port)
        self.settimeout (5)
        self.interval = -1
        self.setinterval (interval)
        self.sendkey (0o70)                 # XON to start output flow
        self.mode = self.DOT + self.LEFT   # no display until mode set
        self.seq = 0
        self.x = 0
        self.y = 31
        self.lineheight = 10
        self.screen = self.erase ()
        self.pending = self.erase ()
        self.start ()

    def erase (self):
        left = [ ]
        right = [ ]
        for i in range (512 // self.lineheight):
            left.append (self.line64)
            right.append (self.line64)
        return left, right
    
    def __next__ (self):
        """Return next byte of Cyber output, as an integer.
        """
        return self.read (1)[0]
    
    def run (self):
        """Collect data from DtCyber, and store it away into the
        left or right screen arrays.
        """
        while True:
            try:
                if self.stopnow:
                    self.close ()
                    break
                ch = next(self)
                self.process_output (ch)
            except socket.timeout:
                pass
            except EOFError:
                break
        self.stopnow = True

    def sendkey (self, key):
        self.sendall (chr (key))
        
    def setinterval (self, interval):
        if self.interval != interval:
            self.interval = interval
            if interval > 63:
                interval = 63
            if interval < 1:
                self.sendkey (0o200 + int (interval * 50))
            else:
                self.sendkey (0o300 + int (interval))

    # This table is nearly the same as the one in charset.c, except
    # that the entry for rubout is ERASE.
    asciiToConsole = \
    (   0,      0,      0,      0,      0,      0,      0,      0,
        0o61,    0,      0o60,    0,      0,      0o60,    0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,
        0o62,    0,      0,      0,      0,      0,      0,      0,
        0o51,    0o52,    0o47,    0o45,    0o56,    0o46,    0o57,    0o50,
        0o33,    0o34,    0o35,    0o36,    0o37,    0o40,    0o41,    0o42,
        0o43,    0o44,    0,      0,      0,      0o54,    0,      0,
        0,      0o1,     0o2,     0o3,     0o4,     0o5,     0o6,     0o7,
        0o10,    0o11,    0o12,    0o13,    0o14,    0o15,    0o16,    0o17,
        0o20,    0o21,    0o22,    0o23,    0o24,    0o25,    0o26,    0o27,
        0o30,    0o31,    0o32,    0o53,    0,      0o55,    0,      0,
        0,      0o1,     0o2,     0o3,     0o4,     0o5,     0o6,     0o7,
        0o10,    0o11,    0o12,    0o13,    0o14,    0o15,    0o16,    0o17,
        0o20,    0o21,    0o22,    0o23,    0o24,    0o25,    0o26,    0o27,
        0o30,    0o31,    0o32,    0,      0,      0,      0,      0o61 )
    
    def sendstr (self, text):
        """Send a text string, converting from ASCII as we go.
        There is no delay in here, so don't send too much.
        """
        for c in text:
            key = self.asciiToConsole[ord (c)]
            if key:
                self.sendkey (key)

    def __repr__ (self):
        """The representation of the object is the current contents
        of the right screen.
        """
        return self.screentext (1)

    def screentext (self, n):
        """Returns the current contents of the specified screen
        (0 or 1), as a string, 51 lines of 64 characters each.
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
        ch = self.consoleToAscii[ch]
        if mode != self.DOT:
            if ch != ' ':
                try:
                    line = self.pending[side][self.y]
                    self.pending[side][self.y] = line[:self.x] + ch + line[self.x + 1:]
                except IndexError:
                    pass
            x = self.x + 1
            if mode == self.MEDIUM:
                x += 1
            elif mode == self.LARGE:
                x += 3
            self.x = x & 0o77

    def process_output (self, ch):
        """Process Cyber output data.  Sort control codes (upper bit set)
        from data codes (upper bit clear). 
        """
        if ch & 0o200:
            # Control code
            action = ch & 0o270
            if action == self.SETX:
                self.x = (((ch << 8) + next(self)) & 0o770) // 8
            elif action == self.SETY:
                self.y = (512 // self.lineheight) - (((ch << 8) + next(self)) & 0o777) // self.lineheight
            elif action == self.SETMODE:
                self.mode = ch & 7
            elif action == self.ENDBLOCK:
                self.screen = self.pending
                self.pending = self.erase ()
                self.seq += 1
        else:
            self.process_char (ch)

    def wait_update (self):
        """Wait until the screen has been updated twice.  This ensures
        that any keyboard input has been reflected in the currently
        visible screen text.
        """
        # Wait for any flurry of updates to settle
        time.sleep (self.interval)
        seq = self.seq
        while self.seq < seq + 2:
            time.sleep (self.interval)
            
class Console (Dd60, Pterm):
    """Subclass of Dd60 to add PLATO terminal functionality via
    the "console" program.
    """
    TERM   = Pterm.SHIFT + Pterm.ANS
    MULT   = 0o12
    DIVIDE = 0o13
    SPACE  = 0o100
    
    specials = { Pterm.NEXT   : "\n",
                 Pterm.ANS    : "[n",
                 Pterm.ASSIGN : "[a",
                 Pterm.BACK   : "[b",
                 Pterm.COPY   : "[=",
                 Pterm.DATA   : "[(",
                       DIVIDE : "[d",
                 Pterm.EDIT   : "[n",
                 Pterm.ERASE  : "\010",
                 Pterm.HELP   : "[h",
                 Pterm.LAB    : "[/",
                 Pterm.MICRO  : "[+",
                       MULT   : "[m",
                       SPACE  : " ",
                 Pterm.SQUARE : "[-",
                 Pterm.STOP   : "[)",
                 Pterm.SUB    : "[8",
                 Pterm.SUPER  : "[7",
                 Pterm.TAB    : "[[",
                       TERM   : "[9}" }

    # Shorthands for some base class methods
    _sendkey = Dd60.sendkey

    def dd60_sendstr (self, text):
        """Send a text string, converting from ASCII as we go.
        There is no delay in here, so don't send too much.
        """
        for c in text:
            key = self.asciiToConsole[ord (c)]
            if key:
                self._sendkey (key)
            
    def sendstr (self, s):
        if self.plato:
            Pterm.sendstr (self, s)
        else:
            self.dd60_sendstr (s)
        
    def __init__ (self, port = 5007, interval = 3):
        self.plato = False
        Dd60.__init__ (self, port, interval)
        
    def sendkey (self,key):
        """Convert PLATO key code to CONSOLE key sequence.
        """
        if not self.plato:
            Dd60.sendkey (self, key)
            return
        try:
            fkey = self.specials[key]
            self.dd60_sendstr (fkey)
        except KeyError:
            if key & self.SHIFT:
                self.dd60_sendstr ("[")
                key &= ~self.SHIFT
            if key & 0o100:
                # Letters
                key &= 0o77
            elif key < 0o12:
                # Digits
                key += 0o33
            Dd60.sendkey (self, key)

    def login (self, *args):
        """Override the pterm login.  We ignore arguments,
        just go to the author mode page.
        """
        self.plato = False
        self.wait_update ()
        
        # First check if we're in DIS or O26.
        while self.screen[0][1][:4] == "O26.":
            self.sendstr ("[xr.\n")
            self.wait_update ()
        while self.screen[0][1][:4] == "DIS ":
            self.sendstr ("[drop.\n")
            self.wait_update ()
        if "NEXT    Z    X" not in self.screentext (1):
            self.sendstr ("[xconsole.\n")
            self.wait_update ()

        # The console (station 0-0) starts in whatever
        # state things were left in.  So we need to get it
        # to "author mode" which may require a trip via
        # "press next to begin" and/or "lesson desired"
        while True:
            left = self.screentext (0)
            print(left)
            print(self.seq)
            if "AUTHOR MODE" in left:
                break
            elif "PRESS  NEXT  TO BEGIN" in left \
                 or "LESSON DESIRED" in left:
                # back
                self.sendstr ("[b")
            else:
                # shift-stop
                self.sendstr ("[])")
            self.wait_update ()
        self.plato = True
                
    def logout (self):
        self.plato = False
        self.wait_update ()
        while True:
            left = self.screentext (0)
            if "LESSON DESIRED" in left:
                break
            else:
                # shift-stop
                self.sendstr ("[])")
                self.wait_update ()
        # exit console utility
        self.sendstr ("[]x")
