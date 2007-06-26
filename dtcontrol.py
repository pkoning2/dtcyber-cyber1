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
import Queue
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
                    self.window.AddPendingEvent (wx.PaintEvent ())
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
    a thread that collects data from DtCyber.
    """
    def __init__ (self, port = 5007, interval = 3):
        threading.Thread.__init__ (self)
        Connection.__init__ (self, "localhost", port)
        self.settimeout (3)
        self.stopnow = False
        self.interval = -1
        self.setinterval (interval)
        self.sendkey (070)                 # XON to start output flow
        self.inited = False
        self.block = [ ]
        self.window = None
        self.start ()

    def run (self):
        """Collect data from DtCyber, and store it away into the
        left or right screen arrays.
        """
        while True:
            try:
                if self.stopnow:
                    self.close ()
                    break
                ch = ord (self.read (1))
                if ch & 0200:
                    # Control word.  See if setx/sety, and if so
                    # collect the additional byte
                    code = ch << 8
                    if (ch < 0217):
                        code += ord (self.read (1))
                    self.block.append (code)
                    if ch == 0250:
                        # end of block
                        if self.inited:
                            #print "sending", len (self.block)
                            if self.window:
                                try:
                                    self.window.queue.put (self.block, False)
                                except Queue.Full:
                                    pass
                                self.window.AddPendingEvent (wx.PaintEvent ())
                        self.block = [ ]
                        self.inited = True    
                elif self.inited:
                    self.block.append (ch)
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

class Displaythread (threading.Thread):
    """This class wraps the processing thread for wxPython.
    The thread is started when an instance is created.
    It then creates two frames, one for the operator interface,
    one for the dd60.
    """
    
    def __init__ (self, oper = None, dd60 = None):
        self.running = False
        self.oper = oper
        self.dd60 = dd60
        threading.Thread.__init__ (self, name = "DtControl")
        self.start ()
        
    def run (self):
        """Start a thread for the wx toolkit processing.
        """
        self.app = wx.PySimpleApp ()
        #self.operframe = Operframe (self.oper, wx.ID_ANY, "Operator interface")
        #self.operframe.Show (True)
        self.dd60frame = Dd60frame (self.dd60, wx.ID_ANY, "DtCyber Console")
        self.dd60frame.Show (True)
        self.running = True
        self.app.MainLoop ()

    def stop (self):
        """Stop the display thread by closing the Frame.
        """
        if self.running:
            self.running = False
            #self.operframe.AddPendingEvent (wx.CloseEvent (wx.wxEVT_CLOSE_WINDOW))
            self.dd60frame.AddPendingEvent (wx.CloseEvent (wx.wxEVT_CLOSE_WINDOW))
            
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
        self.SetBackgroundColour (wx.BLACK)
        self.textFg = wx.Colour (0, 255, 0)
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
        oper = self.oper
        if not oper:
            return
        currentfont = None
        dc.SetTextForeground (self.textFg)
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
            y += self.smallfontextent[1] + 2
        #self.SendString (dc, 020, 060, self.font, oper.cmd, False)
        if response.startswith ('$'):
            dc.SetFont (self.lbfont)
            dc.DrawText (oper.response[1:], 020, 0764)
        else:
            dc.SetFont (self.lfont)
            dc.DrawText (oper.response, 020, 0764)
        dc.EndDrawing ()

    def OnClose (self, event = None):
        """Close the operator window.
        """
        self.Destroy ()
        if self.oper:
            self.oper.stop ()

class Dd60frame (wx.Frame):
    """
    """
    def __init__ (self, dd60, id, name):
        framestyle = (wx.MINIMIZE_BOX |
                      wx.MAXIMIZE_BOX |
                      wx.RESIZE_BORDER |
                      wx.SYSTEM_MENU |
                      wx.CAPTION |
                      wx.CLOSE_BOX |
                      wx.FULL_REPAINT_ON_RESIZE)
        wx.Frame.__init__ (self, None, id, name,
                           style = framestyle)
        self.SetSize (wx.Size (02040, 01020))
        self.SetBackgroundColour (wx.BLACK)
        self.greenPen = wx.Pen (wx.Colour (0, 255, 0))
        wx.EVT_PAINT (self, self.OnPaint)
        wx.EVT_CLOSE (self, self.OnClose)
        self.dd60 = dd60
        self.queue = Queue.Queue (3)
        self.mode = 99
        self.screen = 0
        self.x = 0
        self.y = 0
        if dd60:
            dd60.setwindow (self)
        
    def OnPaint (self, event = None):
        """This is the event handler for window repaint events,
        which is also done on window resize. 
        """
        dc = wx.PaintDC (self)
        dc.BeginDrawing ()
        dc.SetPen (self.greenPen)
        try:
            block = self.queue.get (False)
            dc.SetBackground (wx.BLACK_BRUSH)
            dc.Clear ()
        except Queue.Empty:
            block = [ ]
        #print "processing", len (block)
        for c in block:
            #print oct (c),
            if c & 0100000:
                action = (c >> 8) & 0270
                if action == 0200:
                    # Set X
                    self.x = c & 0777
                elif action == 0210:
                    # Set Y
                    self.y = c & 0777
                    if self.mode == 3:
                        # dot mode
                        x = self.x + self.screen
                        y = 01020 - self.y
                        dc.DrawPoint (x, y)
                elif action == 0220:
                    # set trace data
                    self.mode = 0
                    self.x = 0
                    self.y = 01010
                elif action == 0230:
                    # set keyboard true
                    pass
                elif action == 0240:
                    # set mode
                    c >>= 8
                    if c & 4:
                        self.screen = 01020
                    else:
                        self.screen = 020
                    self.mode = c & 3
            elif self.mode < 3:
                # Character plot mode
                size = 010 << self.mode
                x = self.x + self.screen
                y = 01020 - self.y
                self.plotChar (dc, x, y, size, c)
                self.x = (self.x + size) & 0777
        dc.EndDrawing ()
        #sys.stdout.flush ()

    def OnClose (self, event = None):
        """Close the operator window.
        """
        self.Destroy ()
        if self.dd60:
            self.dd60.stop ()

    chargen = \
      (  ( 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
           000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ),
         ( 001, 014, 014, 014, 030, 014, 014, 014, 036, 015, 001,
           002, 002, 000, 001, 000, 000, 000, 000, 000, 000, 000 ), # A
         ( 001, 010, 010, 010, 030, 002, 002, 022, 026, 022, 002,
           002, 006, 003, 003, 022, 026, 022, 002, 002, 000, 001 ), # B
         ( 012, 012, 022, 000, 007, 022, 032, 022, 016, 010, 022,
           032, 022, 000, 001, 000, 000, 000, 000, 000, 000, 000 ), # C
         ( 001, 010, 010, 010, 030, 002, 002, 022, 016, 010, 022,
           002, 002, 000, 001, 000, 000, 000, 000, 000, 000, 000 ), # D
         ( 001, 010, 010, 010, 030, 002, 002, 002, 007, 012, 000,
           003, 002, 006, 011, 010, 001, 002, 002, 002, 000, 001 ), # E
         ( 001, 010, 010, 010, 030, 002, 002, 002, 007, 012, 020,
           000, 003, 002, 001, 000, 000, 000, 000, 000, 000, 000 ), # F
         ( 022, 012, 000, 001, 002, 036, 010, 022, 032, 022, 016,
           010, 022, 032, 022, 000, 001, 000, 000, 000, 000, 000 ), # G
         ( 001, 010, 010, 010, 031, 010, 021, 000, 002, 002, 002,
           000, 011, 020, 031, 010, 010, 010, 000, 001, 000, 000 ), # H
         ( 002, 004, 000, 001, 010, 010, 010, 000, 001, 000, 000,
           000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ), # I
         ( 010, 000, 031, 012, 032, 012, 010, 010, 001, 000, 000,
           000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ), # J
         ( 001, 010, 010, 010, 031, 002, 002, 003, 006, 022, 022,
           022, 006, 022, 022, 022, 000, 001, 000, 000, 000, 000 ), # K
         ( 010, 010, 010, 031, 010, 010, 010, 000, 002, 002, 002,
           000, 001, 000, 000, 000, 000, 000, 000, 000, 000, 000 ), # L
         ( 001, 010, 010, 010, 030, 012, 024, 030, 012, 024, 030,
           010, 010, 010, 000, 001, 000, 000, 000, 000, 000, 000 ), # M
         ( 001, 010, 010, 010, 030, 012, 012, 012, 030, 010, 010,
           010, 000, 001, 000, 000, 000, 000, 000, 000, 000, 000 ), # N
         ( 010, 000, 001, 010, 012, 032, 012, 016, 012, 032, 012,
           001, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ), # O
         ( 001, 010, 010, 010, 030, 002, 002, 022, 026, 022, 002,
           002, 000, 001, 000, 000, 000, 000, 000, 000, 000, 000 ), # P
         ( 002, 000, 001, 002, 012, 016, 012, 032, 012, 016, 012,
           031, 012, 030, 001, 012, 000, 001, 000, 000, 000, 000 ), # Q
         ( 001, 010, 010, 010, 030, 002, 002, 022, 026, 022, 002,
           002, 006, 022, 022, 022, 000, 001, 000, 000, 000, 000 ), # R
         ( 020, 000, 031, 022, 032, 022, 026, 022, 002, 022, 026,
           022, 032, 022, 000, 001, 000, 000, 000, 000, 000, 000 ), # S
         ( 002, 004, 000, 001, 010, 010, 010, 001, 002, 004, 007,
           002, 002, 002, 000, 001, 000, 000, 000, 000, 000, 000 ), # T
         ( 010, 010, 010, 031, 010, 010, 014, 032, 002, 014, 010,
           010, 000, 001, 000, 000, 000, 000, 000, 000, 000, 000 ), # U
         ( 010, 010, 010, 031, 014, 014, 014, 030, 014, 014, 014,
           000, 001, 000, 000, 000, 000, 000, 000, 000, 000, 000 ), # V
         ( 010, 010, 010, 031, 010, 010, 010, 030, 012, 024, 030,
           012, 024, 030, 010, 010, 010, 000, 001, 000, 000, 000 ), # W
         ( 001, 012, 012, 012, 037, 010, 010, 010, 031, 012, 012,
           012, 000, 001, 000, 000, 000, 000, 000, 000, 000, 000 ), # X
         ( 002, 004, 000, 001, 010, 020, 001, 012, 024, 037, 012,
           024, 030, 012, 024, 000, 001, 000, 000, 000, 000, 000 ), # Y
         ( 010, 010, 010, 000, 031, 002, 002, 002, 006, 012, 012,
           012, 006, 002, 002, 002, 000, 001, 000, 000, 000, 000 ), # Z
         ( 020, 000, 001, 010, 010, 024, 032, 002, 024, 016, 010,
           024, 032, 002, 024, 000, 001, 000, 000, 000, 000, 000 ), # 0
         ( 002, 004, 000, 007, 010, 010, 010, 030, 022, 000, 001,
           000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ), # 1
         ( 010, 010, 020, 000, 001, 022, 032, 022, 026, 022, 022,
           012, 006, 002, 002, 002, 000, 001, 000, 000, 000, 000 ), # 2
         ( 020, 000, 031, 022, 032, 022, 016, 022, 002, 006, 022,
           022, 006, 002, 002, 002, 000, 001, 000, 000, 000, 000 ), # 3
         ( 002, 002, 000, 007, 010, 010, 010, 030, 012, 012, 006,
           002, 002, 002, 000, 001, 000, 000, 000, 000, 000, 000 ), # 4
         ( 020, 000, 031, 022, 032, 022, 016, 022, 002, 002, 000,
           016, 000, 002, 002, 002, 000, 001, 000, 000, 000, 000 ), # 5
         ( 010, 020, 001, 022, 032, 022, 016, 022, 032, 022, 016,
           010, 022, 032, 022, 000, 001, 000, 000, 000, 000, 000 ), # 6
         ( 002, 000, 001, 020, 014, 012, 022, 006, 002, 002, 002,
           001, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ), # 7
         ( 020, 000, 001, 020, 022, 002, 022, 026, 022, 032, 022,
           026, 022, 002, 022, 026, 022, 032, 022, 000, 001, 000 ), # 8
         ( 020, 000, 031, 022, 032, 022, 016, 010, 022, 032, 022,
           016, 022, 032, 022, 000, 001, 000, 000, 000, 000, 000 ), # 9
         ( 002, 004, 000, 001, 010, 010, 010, 031, 012, 024, 007,
           002, 002, 002, 000, 001, 000, 000, 000, 000, 000, 000 ), # +
         ( 010, 020, 000, 000, 001, 002, 002, 002, 000, 001, 000,
           000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ), # -
         ( 020, 004, 001, 012, 012, 037, 010, 001, 002, 002, 036,
           011, 030, 001, 012, 012, 000, 001, 000, 000, 000, 000 ), # *
         ( 001, 012, 012, 012, 000, 001, 000, 000, 000, 000, 000,
           000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ), # /
         ( 002, 002, 000, 007, 024, 016, 010, 024, 000, 001, 000,
           000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ), # (
         ( 002, 000, 001, 024, 016, 010, 024, 001, 000, 000, 000,
           000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ), # )
         ( 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
           000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ),
         ( 010, 000, 001, 002, 002, 002, 007, 010, 001, 002, 002,
           002, 000, 001, 000, 000, 000, 000, 000, 000, 000, 000 ), # =
         ( 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
           000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ),
         ( 001, 024, 030, 000, 001, 000, 000, 000, 000, 000, 000,
           000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 ), # ,
         ( 001, 000, 000, 001, 000, 000, 000, 000, 000, 000, 000,
           000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 )  # .
         )

    def plotChar (self, dc, xpos, ypos, size, ch):
        """Draw a character using the 545 char data.
        """
        xpos += size
        size /= 8
        dx = 1
        dy = -1
        x = y = 0
        if ch > len (self.chargen):
            return
        data = self.chargen[ch]
        segs = [ ]
        on = False
        for c in data:
            if c & 1:
                on = not on
                if on:
                    segs = [ wx.Point ( size * x, size * y ) ]
                elif len (segs) > 1:
                    dc.DrawLines (segs, xpos, ypos)
                    segs = [ ]
            xcode = (c >> 1) & 3
            ycode = (c >> 3) & 3
            if xcode == 1:
                x += dx * 2
            elif xcode == 2:
                x += dx
            elif xcode == 3:
                dx = -dx
            if ycode == 1:
                y += dy * 2
            elif ycode == 2:
                y += dy
            elif ycode == 3:
                dy = -dy
            if on:
                segs.append (wx.Point (x * size, y * size))
        if len (segs) > 1:
            dc.DrawLines (segs, xpos, ypos)
                
def o(port = 5106):
    ot = Oper (port)
    dt = Displaythread (ot)
    return dt

def d(port = 5107):
    ot = Dd60 (port)
    dt = Displaythread (None, ot)
    return dt
