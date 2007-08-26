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
        
    def setwindow (self, win):
        self.window = win
        
    def stop (self):
        if not self.stopnow:
            self.stopnow = True
            #print "telling thread to stop"
            self.join ()
            #print "thread is gone"

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

def syntaxmatch (syn, cmd):
    """Check if string 'cmd' matches syntax 'syn'.  Returns None if
    no match, or the next match if a match.  Next match means the
    input string, upcased except where the 'x' marker occurs, with
    the next character appended if a next character is defined by the
    pattern.  The next character is defined if the last pattern character
    is not '7' or 'x'.
    """
    if not cmd:
        return syn[0]
    ci = iter (cmd)
    si = iter (syn)
    ret = [ ]
    c = ci.next ()
    for s in si:
        try:
            if s == '7':
                if c <= '0' or c > '7':
                    return None
                while c >= '0' and c <= '7':
                    ret.append (c)
                    c = ci.next ()
            elif s == 'x': 
                if c == ',' or c == '\n':
                    return None
                while c != ',' and c != '\n':
                    ret.append (c)
                    c = ci.next ()
            else:
                if s != c.upper ():
                    return None
                ret.append (c.upper ())
                c = ci.next ()
        except:
            # End of string.  If the current pattern character
            # is a specific character, append that.  Return
            # the result
            if s != '7' and s != 'x':
                try:
                    s = si.next ()
                    if s != '7' and s != 'x':
                        ret.append (s)
                except StopIteration:
                    pass
            return ''.join (ret)
    # End of pattern, we should be at end of string
    try:
        ci.next ()
        # Not end of string -> no match
        return None
    except StopIteration:
        pass
    return ''.join (ret)

class Oper (Connection, threading.Thread):
    """A connection to the DtCyber operator interface.  It includes
    a thread that collects data from DtCyber and updates local
    state to reflect what it hears.
    """
    def __init__ (self, port = 5006):
        threading.Thread.__init__ (self)
        Connection.__init__ (self, "localhost", port)
        self.settimeout (1)
        self.stopnow = False
        self.fixedtext = [ ]
        self.syntax = [ ]
        self.response = ""
        self.responses = 0
        self.status = elist ()
        self.initialized = False
        self.window = None
        self.initialized = False
        self.start ()
        
    def run (self):
        """Collect data from DtCyber.  We sort out the various
        types of data we receive and salt away most of it in
        a more convenient form.
        """
        #print "oper running"
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
                    if i:
                        self.window.scrollto (i)
                elif t == 5:
                    self.initialized = True
                # Don't bother the window until we've processed
                # the data from the network
                if self.window and not self.pendingdata:
                    self.window.AddPendingEvent (wx.PaintEvent ())
                    #self.window.Refresh ()
            except socket.timeout:
                pass
            except EOFError:
                break
        self.stopnow = True
        #print "oper exiting"
        
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
        #print "dd60 running"
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
                                #self.window.Refresh ()
                        self.block = [ ]
                        self.inited = True    
                elif self.inited:
                    self.block.append (ch)
            except socket.timeout:
                pass
            except EOFError:
                break
        self.stopnow = True
        #print "dd60 exiting"

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
    
    def __init__ (self, operport = None, dd60port = None, dd60interval = None):
        self.running = False
        global frames
        frames = [ ]
        threading.Thread.__init__ (self, name = "DtControl")
        if not operport:
            operport = 5006
        if not dd60port:
            dd60port = operport + 1
        self.operport = operport
        self.dd60port = dd60port
        if not dd60interval:
            if self.dd60port == 5007:
                dd60interval = .5
            else:
                dd60interval = 3
        self.dd60interval = dd60interval
        self.start ()
        while not self.running:
            time.sleep (.2)
            
    def run (self):
        """Start a thread for the wx toolkit processing.
        """
        self.app = wx.PySimpleApp ()
        self.running = True
        #print "displaythread running"
        self.newoper (self.operport)
        self.newdd60 (self.dd60port, self.dd60interval)
        self.app.MainLoop ()
        self.running = False
        #print "displaythread end"

    def newoper (self, port):
        oper = Oper (port)
        f = Operframe (oper, wx.ID_ANY, "DtCyber operator interface")
        global frames
        frames.append (f)
        
    def newdd60 (self, port, interval):
        dd60 = Dd60 (port, interval)
        f = Dd60frame (dd60, wx.ID_ANY, "DtCyber console display")
        global frames
        frames.append (f)
        
    def stop (self):
        """Stop the display thread by closing the Frames.
        """
        if self.running:
            self.running = False
            global frames
            for f in frames:
                f.Close ()
            frames = [ ]
            
class ControlFrame (wx.Window):
    """Base class for dtcontrol frames.
    """
    def __init__ (self, thread, id, name):
        framestyle = (wx.MINIMIZE_BOX |
                      wx.MAXIMIZE_BOX |
                      wx.RESIZE_BORDER |
                      wx.SYSTEM_MENU |
                      wx.CAPTION |
                      wx.CLOSE_BOX |
                      wx.WANTS_CHARS |
                      wx.FULL_REPAINT_ON_RESIZE)
        self.frame = wx.Frame (None, id, name, style = framestyle)
        wx.Window.__init__ (self, self.frame, wx.ID_ANY)
        self.thread = thread
        self.SetBackgroundColour (wx.BLACK)
        self.frame.SetBackgroundColour (wx.BLACK)
        self.textFg = wx.Colour (0, 255, 0)
        wx.EVT_PAINT (self, self.OnPaint)
        wx.EVT_CLOSE (self.frame, self.OnClose)
        wx.EVT_CLOSE (self, self.OnChildClose)
        wx.EVT_CHAR (self, self.OnChar)
        self.frame.Show (True)
        self.Show (True)
        self.SetFocus ()

    def SetSize (self, newsize):
        wx.Window.SetSize (self, newsize)
        self.frame.SetClientSize (newsize)

    def OnChildClose (self, event = None):
        self.frame.Close ()
        
    def OnClose (self, event = None):
        """Close the window.
        """
        #print "OnClose"
        self.frame.Destroy ()
        global frames
        for i in xrange (len (frames)):
            if frames[i] == self.frame:
                del frames[i]
                break
        if self.thread:
            self.thread.stop ()
            self.thread = None
        
class Operframe (ControlFrame):
    """Simple text display window class derived from wxFrame.
    It handles repaint, close, and keyboard events.

    When instantiated, this class creats the window using the
    supplied size.
    """
    smallfont = 9
    largefont = 14
    statusoff = 2
    statuswin = 21
    rightscreen = 01040
    xmax = 02000
    ymax = 01000
    def __init__ (self, oper, id, name):
        ControlFrame.__init__ (self, oper, id, name)
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
        self.margin , ch = self.smallfontextent
        cw = self.xmax + self.margin * 2
        ch = self.ymax + self.margin * 2
        self.SetSize (wx.Size (cw, ch))
        self.width = cw
        self.cmd = ""
        self.cmdbold = False
        self.currentfont = None
        self.statustop = self.statusoff
        oper.setwindow (self)

    def SendString (self, dc, x, y, size, str, bold):
        if bold:
            if size == 010:
                font = self.sbfont
            else:
                font = self.lbfont
        elif size == 010:
            font = self.sfont
        else:
            font = self.lfont
        if font is not self.currentfont:
            dc.SetFont (font)
            self.currentfont = font
            dc.SetTextForeground (self.textFg)
        dc.DrawText (str, self.margin + x, self.margin + 01000 - y)

    def ClearString (self, dc, x, y, size, x2):
        if size == 010:
            cw, ch = self.smallfontextent
        else:
            cw, ch = self.largefontextent
        dc.Blit (self.margin + x, self.margin + 01000 - y,
                 x2 - x, ch, dc, 0, 0, wx.CLEAR)

    def statusrange (self):
        """Return pair (first, last) which is the currently displayed
        range of status lines.
        """
        oper = self.thread
        first = self.statustop
        last = first + self.statuswin
        if first > self.statusoff:
            last -= 1
        if last < len (oper.status):
            last -= 1
        return first, last
    
    def scrollto (self, pos):
        """Scroll the status display (right screen), if necessary,
        to make line 'pos' visible.
        """
        oper = self.thread
        first, last = self.statusrange ()
        if pos >= first and pos < last:
            return
        if pos > len (oper.status) - (self.statuswin - 1):
            pos = len (oper.status) - (self.statuswin - 1)
        if pos < self.statusoff:
            pos = self.statusoff
        self.statustop = pos
        #self.AddPendingEvent (wx.PaintEvent ())
        self.Refresh ()
        
    def OnPaint (self, event = None):
        """This is the event handler for window repaint events,
        which is also done on window resize. 
        """
        dc = wx.PaintDC (self)
        dc.BeginDrawing ()
        oper = self.thread
        if not oper:
            return
        self.currentfont = None
        for x, y, size, bold, text in oper.fixedtext:
            self.SendString (dc, x, y, size, text, bold)
        y = 0740 - (self.smallfontextent[1] + 2) * 2
        if oper.status:
            self.SendString (dc, self.rightscreen, 0740,
                             010, oper.status[0], False)
            first, last = self.statusrange ()
            if first > self.statusoff:
                self.ClearString (dc, self.rightscreen, y,
                                  010, self.xmax)
                self.SendString (dc, self.rightscreen, y,
                                 010, "        (more)", False)
                y -= self.smallfontextent[1] + 2
            if last < len (oper.status):
                y2 = 0740 - (self.smallfontextent[1] + 2) * \
                     (self.statuswin + 1)
                self.ClearString (dc, self.rightscreen, y2,
                                  010, self.xmax)
                self.SendString (dc, self.rightscreen, y2,
                                 010, "        (more)", False)
            for text in oper.status[first:last]:
                self.ClearString (dc, self.rightscreen, y,
                                  010, self.xmax)
                self.SendString (dc, 01040, y, 010, text, False)
                y -= self.smallfontextent[1] + 2
            self.SendString (dc, 020, 060, 020, self.cmd, self.cmdbold)
        if oper.response.startswith ('$'):
            self.SendString (dc, 020, 014, 020, oper.response[1:], True)
        else:
            self.SendString (dc, 020, 014, 020, oper.response, False)
        dc.EndDrawing ()

    def OnChar (self, event = None):
        """Process a keystroke.
        """
        oper = self.thread
        ch = chr (event.GetKeyCode ())
        if event.ControlDown ():
            ch = chr (ord (ch) & 037)
        if ch == '\027':
            # Control-W
            self.Close ()
            return
        if ch == '\r':
            ch = '\n'
        dc = wx.WindowDC (self)
        dc.BeginDrawing ()
        self.currentfont = None
        if not self.cmd:
            if ch == '-':
                self.scrollto (self.statustop - (self.statuswin - 2))
                return
            elif ch == '+':
                first, last = self.statusrange ()
                self.scrollto (last)
                return
            if oper.response:
                self.ClearString (dc, 020, 014, 020, self.xmax)
                oper.response = ""
        if ch == '\025' or ch == '[':
            # control/u or left blank, erase command
            self.ClearString (dc, 020, 060, 020, self.xmax)
            self.cmd = ""
            self.cmdbold = False
        elif ch == '\177' or ch == '\b':
            # backspace
            if self.cmd:
                # Erase the command echo area; it will be redisplayed.
                # Need to do it this way to undo any bolding.
                self.ClearString (dc, 020, 060, 020, self.xmax)
                self.cmd = self.cmd[:-1]
                self.cmdbold = False
        else:
            while True:
                cmd = self.cmd + ch
                matches = [ ]
                for s in oper.syntax:
                    match = syntaxmatch (s, cmd)
                    if match:
                        matches.append (match)
                if not matches:
                    break
                match = matches[0]
                self.cmd = match[:len (cmd)]
                if ch == '\n':
                    # Command was complete and Enter was hit
                    if self.cmd == "END.\n":
                        self.Close ()
                        return
                    else:
                        oper.command (self.cmd)
                    break
                ch = None
                if len (match) <= len (cmd):
                    # No next char
                    break
                for match in matches:
                    nch = match[len (cmd)]
                    if not ch:
                        ch = nch
                    elif ch != nch:
                        # Ambiguous next char, stop autocompleting
                        ch = None
                        break
                if not ch:
                    break
                if ch == '\n':
                    self.cmdbold = True
                    break
        self.SendString (dc, 020, 060, 020, self.cmd, self.cmdbold)
        dc.EndDrawing ()

FONTX = 680
FONTY = 200

def charpos (size, ch):
    """Return the base X/Y of the specified character in the font bitmap.
    """
    if size == 0:
        return (11 + ch * 10, 11)
    elif size == 1:
        return (21 + (ch & 037) * 20, 31 + (ch >> 5) * 20)
    else:
        return (41 + (ch & 017) * 40, 91 + (ch >> 4) * 40)

class Dd60frame (ControlFrame):
    """
    """
    margin = 20
    rightscreen = 01020
    size = 01000
    xsize = size + rightscreen + 2 * margin
    ysize = size + 2 * margin
    
    def __init__ (self, dd60, id, name):
        ControlFrame.__init__ (self, dd60, id, name)
        self.SetSize (wx.Size (self.xsize, self.ysize))
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
        self.screendc = wx.MemoryDC ()
        self.screenmap = wx.EmptyBitmap (self.xsize, self.ysize)
        self.screendc.SelectObject (self.screenmap)
        self.newBlock = False
        self.setupfonts ()
        if dd60:
            dd60.setwindow (self)

    def setupfonts (self):
        self.fontdc = wx.MemoryDC ()
        self.fontmap = wx.EmptyBitmap (FONTX, FONTY)
        self.fontdc.SelectObject (self.fontmap)
        self.fontdc.BeginDrawing ()
        self.fontdc.SetBackground (wx.BLACK_BRUSH)
        self.fontdc.Clear ()
        self.fontdc.SetPen (self.greenPen)
        for size in (0, 1, 2):
            for ch in xrange (060):
                self.setChar (self.fontdc, size, ch)
        self.fontdc.EndDrawing ()
                
    def OnPaint (self, event = None):
        """This is the event handler for window repaint events,
        which is also done on window resize. 
        """
        dc = wx.PaintDC (self)
        dc.BeginDrawing ()
        dc.SetPen (self.greenPen)
        self.screendc.BeginDrawing ()
        self.screendc.SetPen (self.greenPen)
        try:
            block = self.queue.get (False)
            if self.newBlock:
                self.screendc.Blit (0, 0,
                                    self.xsize, self.ysize,
                                    self.screendc, 0, 0, wx.CLEAR)
                self.newBlock = False
        except Queue.Empty:
            block = [ ]
        clk = time.clock ()
        for c in block:
            #print oct (c), self.mode,
            if c & 0100000:
                action = (c >> 8) & 0270
                #print oct (action)
                if action == 0200:
                    # Set X
                    self.x = c & 0777
                elif action == 0210:
                    # Set Y
                    self.y = c & 0777
                    if self.mode == 3:
                        # dot mode
                        x = self.margin + self.x + self.screen
                        y = self.margin + self.size - self.y
                        self.screendc.DrawPoint (x, y)
                elif action == 0240:
                    # set mode
                    c >>= 8
                    if c & 4:
                        self.screen = self.rightscreen
                    else:
                        self.screen = 0
                    self.mode = c & 3
                elif action == 0220:
                    # set trace data
                    self.mode = 0
                    self.x = 0
                    self.y = 01010
                elif action == 0230:
                    # set keyboard true
                    pass
                elif action == 0250:
                    # block end
                    self.newBlock = True
            elif self.mode < 3:
                # Character plot mode
                size = 010 << self.mode
                x = self.margin + self.x + self.screen
                y = self.margin + self.size - self.y 
                self.plotChar (self.screendc, x, y, self.mode, c)
                self.x = (self.x + size) & 0777
        dc.Blit (0, 0, self.xsize, self.ysize, self.screendc, 0, 0, wx.COPY)
        self.screendc.EndDrawing ()
        dc.EndDrawing ()
        print len (block), "items in", (time.clock() - clk) * 1000,"ms"

    def OnChar (self, event = None):
        """Process a keystroke.
        """
        dd60 = self.thread
        ch = chr (event.GetKeyCode ())
        if event.AltDown ():
            if ch == 'z':
                # Close window (old style)
                self.Close ()
            return
        if event.ControlDown ():
            ch = chr (ord (ch) & 037)
        if ch == '\027':
            # Control-W
            self.Close ()
            return
        if ch == '\r':
            ch = '\n'
        dd60.sendstr (ch)
        
    chargen = \
      (  ( ),
         ( 001, 014, 014, 014, 030, 014, 014, 014, 036, 015, 001,
           002, 002, 000, 001 ),                                    # A
         ( 001, 010, 010, 010, 030, 002, 002, 022, 026, 022, 002,
           002, 006, 003, 003, 022, 026, 022, 002, 002, 000, 001 ), # B
         ( 012, 012, 022, 000, 007, 022, 032, 022, 016, 010, 022,
           032, 022, 000, 001 ),                                    # C
         ( 001, 010, 010, 010, 030, 002, 002, 022, 016, 010, 022,
           002, 002, 000, 001 ),                                    # D
         ( 001, 010, 010, 010, 030, 002, 002, 002, 007, 012, 000,
           003, 002, 006, 011, 010, 001, 002, 002, 002, 000, 001 ), # E
         ( 001, 010, 010, 010, 030, 002, 002, 002, 007, 012, 020,
           000, 003, 002, 001 ),                                    # F
         ( 022, 012, 000, 001, 002, 036, 010, 022, 032, 022, 016,
           010, 022, 032, 022, 000, 001 ),                          # G
         ( 001, 010, 010, 010, 031, 010, 021, 000, 002, 002, 002,
           000, 011, 020, 031, 010, 010, 010, 000, 001 ),           # H
         ( 002, 004, 000, 001, 010, 010, 010, 000, 001 ),           # I
         ( 010, 000, 031, 012, 032, 012, 010, 010, 001 ),           # J
         ( 001, 010, 010, 010, 031, 002, 002, 003, 006, 022, 022,
           022, 006, 022, 022, 022, 000, 001 ),                     # K
         ( 010, 010, 010, 031, 010, 010, 010, 000, 002, 002, 002,
           000, 001 ),                                              # L
         ( 001, 010, 010, 010, 030, 012, 024, 030, 012, 024, 030,
           010, 010, 010, 000, 001 ),                               # M
         ( 001, 010, 010, 010, 030, 012, 012, 012, 030, 010, 010,
           010, 000, 001 ),                                         # N
         ( 010, 000, 001, 010, 012, 032, 012, 016, 012, 032, 012,
           001 ),                                                   # O
         ( 001, 010, 010, 010, 030, 002, 002, 022, 026, 022, 002,
           002, 000, 001 ),                                         # P
         ( 002, 000, 001, 002, 012, 016, 012, 032, 012, 016, 012,
           031, 012, 030, 001, 012, 000, 001 ),                     # Q
         ( 001, 010, 010, 010, 030, 002, 002, 022, 026, 022, 002,
           002, 006, 022, 022, 022, 000, 001 ),                     # R
         ( 020, 000, 031, 022, 032, 022, 026, 022, 002, 022, 026,
           022, 032, 022, 000, 001 ),                               # S
         ( 002, 004, 000, 001, 010, 010, 010, 001, 002, 004, 007,
           002, 002, 002, 000, 001 ),                               # T
         ( 010, 010, 010, 031, 010, 010, 014, 032, 002, 014, 010,
           010, 000, 001 ),                                         # U
         ( 010, 010, 010, 031, 014, 014, 014, 030, 014, 014, 014,
           000, 001 ),                                              # V
         ( 010, 010, 010, 031, 010, 010, 010, 030, 012, 024, 030,
           012, 024, 030, 010, 010, 010, 000, 001 ),                # W
         ( 001, 012, 012, 012, 037, 010, 010, 010, 031, 012, 012,
           012, 000, 001 ),                                         # X
         ( 002, 004, 000, 001, 010, 020, 001, 012, 024, 037, 012,
           024, 030, 012, 024, 000, 001 ),                          # Y
         ( 010, 010, 010, 000, 031, 002, 002, 002, 006, 012, 012,
           012, 006, 002, 002, 002, 000, 001 ),                     # Z
         ( 020, 000, 001, 010, 010, 024, 032, 002, 024, 016, 010,
           024, 032, 002, 024, 000, 001 ),                          # 0
         ( 002, 004, 000, 007, 010, 010, 010, 030, 022, 000, 001 ), # 1
         ( 010, 010, 020, 000, 001, 022, 032, 022, 026, 022, 022,
           012, 006, 002, 002, 002, 000, 001 ),                     # 2
         ( 020, 000, 031, 022, 032, 022, 016, 022, 002, 006, 022,
           022, 006, 002, 002, 002, 000, 001 ),                     # 3
         ( 002, 002, 000, 007, 010, 010, 010, 030, 012, 012, 006,
           002, 002, 002, 000, 001 ),                               # 4
         ( 020, 000, 031, 022, 032, 022, 016, 022, 002, 002, 000,
           016, 000, 002, 002, 002, 000, 001 ),                     # 5
         ( 010, 020, 001, 022, 032, 022, 016, 022, 032, 022, 016,
           010, 022, 032, 022, 000, 001 ),                          # 6
         ( 002, 000, 001, 020, 014, 012, 022, 006, 002, 002, 002,
           001 ),                                                   # 7
         ( 020, 000, 001, 020, 022, 002, 022, 026, 022, 032, 022,
           026, 022, 002, 022, 026, 022, 032, 022, 000, 001 ),      # 8
         ( 020, 000, 031, 022, 032, 022, 016, 010, 022, 032, 022,
           016, 022, 032, 022, 000, 001 ),                          # 9
         ( 002, 004, 000, 001, 010, 010, 010, 031, 012, 024, 007,
           002, 002, 002, 000, 001 ),                               # +
         ( 010, 020, 000, 000, 001, 002, 002, 002, 000, 001 ),      # -
         ( 020, 004, 001, 012, 012, 037, 010, 001, 002, 002, 036,
           011, 030, 001, 012, 012, 000, 001 ),                     # *
         ( 001, 012, 012, 012, 000, 001 ),                          # /
         ( 002, 002, 000, 007, 024, 016, 010, 024, 000, 001 ),      # (
         ( 002, 000, 001, 024, 016, 010, 024, 001 ),                # )
         ( ),                                                       # space
         ( 010, 000, 001, 002, 002, 002, 007, 010, 001, 002, 002,
           002, 000, 001 ),                                         # =
         ( ),                                                       # $
         ( 001, 024, 030, 000, 001 ),                               # ,
         ( 001, 000, 000, 001 )                                     # .
         )

    def plotChar (self, dc, xpos, ypos, size, ch):
        """Draw a character using the 545 char data.
        """
        chx, chy = charpos (size, ch)
        chx -= 2 << size
        chy -= 8 << size
        xpos -= 5 << size
        ypos -= 5 << size
        size = 10 << size
        #print xpos, ypos, chx, chy, size, ch
        dc.Blit (xpos, ypos, size, size,
                 self.fontdc, chx, chy, wx.OR)

    def setChar (self, dc, size, ch):
        """Draw a character into the supplied DC.  This is done
        at setup; the DC is a MemoryDC.
        """
        xpos, ypos = charpos (size, ch)
        size = 1 << size
        #print xpos, ypos, size, ch
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
                    #print ch, xpos, ypos, segs
                    dc.DrawLines (segs, xpos, ypos)
                    segs = [ ]
                elif segs:
                    x, y = segs[0]
                    #print ch, xpos, ypos, segs
                    dc.DrawPoint (x + xpos, y + ypos)
            xcode = (c >> 1) & 3
            ycode = (c >> 3) & 3
            if xcode == 1:
                x += dx * 2
            elif xcode == 2:
                x += dx
            elif xcode == 3:
                dx = -dx
                xcode = 0
            if ycode == 1:
                y += dy * 2
            elif ycode == 2:
                y += dy
            elif ycode == 3:
                dy = -dy
                ycode = 0
            if on and (xcode or ycode):
                segs.append (wx.Point (x * size, y * size))
        #print ch, xpos, ypos, segs
        if len (segs) > 1:
            dc.DrawLines (segs, xpos, ypos)
        elif segs:
            x, y = segs[0]
            dc.DrawPoint (x + xpos, y + ypos)

def main (args):
    operport = dd60port = None
    if args:
        operport = int (args[0])
    if len (args) > 1:
        dd60port = int (args[1])
    d = Displaythread (operport, dd60port)
    while not d.running:
        time.sleep (1)
    while d.running:
        time.sleep (1)
    
if __name__ == "__main__":
    main (sys.argv[1:])
    
