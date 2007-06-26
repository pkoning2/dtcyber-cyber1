#!/usr/bin/env python

"""Combined user interface for DtCyber

This package combines dd60 (the console display) and dtoper
(the operator interface for DtCyber) into a single wxPython
based utility.
"""

import os
import sys
import time
import socket
import threading
import struct
import wx

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
            time.sleep (0.3)
        
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
        threading.Thread.__init__ (self)
        Connection.__init__ (self, "localhost", port)
        self.settimeout (5)
        self.stopnow = False
        self.fixedtext = [ ]
        self.syntax = [ ]
        self.response = ""
        self.responses = 0
        self.status = elist ()
        self.initialized = False
        self.window = None
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
                    # Store the unpacked data in the fixedtext list
                    x, y, size, bold = struct.unpack ("<HHBB", v[:6])
                    self.fixedtext.append ((x, y, size, bold, v[6:]))
                elif t == 3:
                    # OpSyntax -- save in syntax list.
                    self.syntax.append (v)
                elif t == 5:
                    self.initialized = True
                elif t == 1:
                    # OpReply
                    self.response = v
                    self.responses += 1
                elif t == 4:
                    # OpStatus
                    i = ord (v[0])
                    v = v[1:]
                    self.status[i] = v
                if self.window:
                    self.window.Refresh ()
            except socket.timeout:
                pass
            except EOFError:
                break
        self.stopnow = True

    def setwindow (self, win):
        self.window = win
        
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
        return '\n'.join (self.status)

class Dd60 (Connection, threading.Thread):
    """A connection to the DtCyber console (green tubes).  It includes
    a thread that collects data from DtCyber and updates local
    state to reflect what it hears.
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
        threading.Thread.__init__ (self)
        Connection.__init__ (self, "localhost", port)
        self.settimeout (5)
        self.erase ()
        self.stopnow = False
        self.interval = -1
        self.setinterval (interval)
        self.sendkey (070)                 # XON to start output flow
        self.mode = self.DOT + self.LEFT   # no display until mode set
        self.x = self.y = 0
        self.endblock = True
        self.start ()

    def erase (self):
        self.left = [ ]
        self.right = [ ]
        for i in xrange (32):
            self.left.append (self.line64)
            self.right.append (self.line64)

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
        return self.screen (1)

    def screen (self, n):
        if n:
            return '\n'.join (self.right)
        else:
            return '\n'.join (self.left)

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
            screen = self.left
        else:
            screen = self.right
        mode = self.mode & 3
        if mode != self.DOT:
            line = screen[self.y]
            
            screen[self.y] = line[:self.x] + self.consoleToAscii[ch] + line[self.x + 1:]
            x = self.x + 1
            if mode == self.MEDIUM:
                x += 1
            elif mode == self.LARGE:
                x += 3
            self.x = x & 077

    def process_output (self, ch):
        """Process Cyber output data.  Sort control codes (upper bit set)
        from data codes (upper bit clear).  Whenever we receive an output
        byte after the end of block code, do a full erase to prepare the
        screen image for another block of data.
        """
        if self.endblock:
            self.erase ()
            self.endblock = False
        if ch & 0200:
            # Control code
            action = ch & 0270
            if action == self.SETX:
                self.x = (((ch << 8) + self.next ()) & 0770) / 8
            elif action == self.SETY:
                self.y = (((ch << 8) + self.next ()) & 0760) / 16
            elif action == self.SETMODE:
                self.mode = ch & 7
            elif action == self.ENDBLOCK:
                self.endblock = True
        else:
            self.process_char (ch)

class Displaythread (threading.Thread):
    """This class wraps the processing thread for wxPython.
    The thread is started when an instance is created.
    It then creates two frames, one for the operator interface,
    one for the dd60.
    """
    
    def __init__ (self, oper):
        self.running = False
        self.oper = oper
        threading.Thread.__init__ (self, name = "DtControl")
        self.start ()
        
    def run (self):
        """Start a thread for the wx toolkit processing.
        """
        self.app = wx.PySimpleApp ()
        self.operframe = Operframe (self.oper, wx.ID_ANY, "Operator interface")
        self.operframe.Show (True)
        self.running = True
        self.app.MainLoop ()

    def stop (self):
        """Stop the display thread by closing the Frame.
        """
        if self.running:
            self.running = False
            self.operframe.AddPendingEvent (wx.CloseEvent (wx.wxEVT_CLOSE_WINDOW))
            #self.consoleframe.AddPendingEvent (wx.CloseEvent (wx.wxEVT_CLOSE_WINDOW))
            
class Operframe (wx.Frame):
    """Simple text display window class derived from wxFrame.
    It handles repaint, close, and keyboard events.

    When instantiated, this class creats the window using the
    supplied size.
    """
    smallfont = 9
    largefont = 14
    def __init__ (self, oper, id, name):
        framestyle = (wx.MINIMIZE_BOX |
                      wx.MAXIMIZE_BOX |
                      wx.RESIZE_BORDER |
                      wx.SYSTEM_MENU |
                      wx.CAPTION |
                      wx.CLOSE_BOX |
                      wx.FULL_REPAINT_ON_RESIZE)
        wx.Frame.__init__ (self, None, id, name,
                           style = framestyle)
        fontenum = wx.FontEnumerator ()
        fontenum.EnumerateFacenames (fixedWidthOnly = True)
        fontlist = fontenum.GetFacenames ()
        lucida = None
        for f in fontlist:
            f = str (f)
            if "lucida" in f.lower ():
                lucida = f
                break
        self.sfont = wx.Font (self.smallfont, wx.FONTFAMILY_MODERN,
                             wx.FONTSTYLE_NORMAL,
                             wx.FONTWEIGHT_NORMAL)
        self.sbfont = wx.Font (self.smallfont, wx.FONTFAMILY_MODERN,
                               wx.FONTSTYLE_NORMAL,
                               wx.FONTWEIGHT_BOLD)
        self.lfont = wx.Font (self.largefont, wx.FONTFAMILY_MODERN,
                             wx.FONTSTYLE_NORMAL,
                             wx.FONTWEIGHT_NORMAL)
        self.lbfont = wx.Font (self.largefont, wx.FONTFAMILY_MODERN,
                               wx.FONTSTYLE_NORMAL,
                               wx.FONTWEIGHT_BOLD)
        if lucida:
            self.sfont.SetFaceName (lucida)
            self.sbfont.SetFaceName (lucida)
            self.lfont.SetFaceName (lucida)
            self.lbfont.SetFaceName (lucida)
        dc = wx.MemoryDC ()
        dc.SetFont (self.sfont)
        self.smallfontextent = dc.GetTextExtent ("a")
        dc.SetFont (self.lfont)
        self.largefontextent = dc.GetTextExtent ("a")
        cw , ch = self.smallfontextent
        self.margin = cw
        cw = cw * 132 + self.margin * 2
        ch = ch * 32 + self.margin * 2
        self.SetSize (wx.Size (cw, ch))
        wx.EVT_PAINT (self, self.OnPaint)
        wx.EVT_CLOSE (self, self.OnClose)
        self.oper = oper
        oper.setwindow (self)
        
    def OnPaint (self, event = None):
        """This is the event handler for window repaint events,
        which is also done on window resize. 
        """
        dc = wx.PaintDC (self)
        dc.BeginDrawing ()
        #dc.Clear ()
        oper = self.oper
        if not oper:
            return
        currentfont = None
        dc.SetPen (wx.BLACK_PEN)
        for x, y, size, bold, text in oper.fixedtext:
            if bold:
                if size == 010:
                    font = self.sbfont
                else:
                    font = self.slfont
            elif size == 010:
                font = self.sfont
            else:
                font = self.lfont
            if font is not currentfont:
                dc.SetFont (font)
                currentfont = font
            dc.DrawText (text, x, 01000 - y)
        y = 040
        dc.SetFont (self.sfont)
        for text in oper.status:
            dc.DrawText (text, 01040, y)
            y += self.smallfontextent[1]
        #self.SendString (dc, 020, 060, self.font, oper.cmd, False)
        dc.SetFont (self.lfont)
        dc.DrawText (oper.response, 020, 0764)
        dc.EndDrawing ()

    def OnClose (self, event = None):
        """Close the operator window.
        """
        self.Destroy ()
        if self.oper:
            self.oper.stop ()
        
def o(port):
    ot = Oper (port)
    dt = Displaythread (ot)
    return dt
