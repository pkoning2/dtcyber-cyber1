#!/usr/bin/env python3

import sys
import io
import os
import re

import wx
import PyPDF2
import struct
import wlist
import cmodule

pdf_pat = re.compile (r"\S+\.pdf", re.I)
lsp_pat = re.compile (r"^\s+")
tsp_pat = re.compile (r"\s+$", re.M)
first_pat = re.compile (r"# Starting page (\d+)", re.I)

pdf_path = os.path.expanduser ("~/Documents/cdc/6600 field service")

"""
Sample code for extracting images from PDF files from:
 http://stackoverflow.com/questions/2693820/extract-images-from-pdf-without-resampling-in-python

Links:
PDF format: http://www.adobe.com/content/dam/Adobe/en/devnet/acrobat/pdfs/pdf_reference_1-7.pdf
CCITT Group 4: https://www.itu.int/rec/dologin_pub.asp?lang=e&id=T-REC-T.6-198811-I!!PDF-E&type=items
Extract images from pdf: http://stackoverflow.com/questions/2693820/extract-images-from-pdf-without-resampling-in-python
Extract images coded with CCITTFaxDecode in .net: http://stackoverflow.com/questions/2641770/extracting-image-from-pdf-with-ccittfaxdecode-filter
TIFF format and tags: http://www.awaresystems.be/imaging/tiff/faq.html
"""

def tiff_header_for_CCITT(width, height, img_size, CCITT_group=4):
    tiff_header_struct = '<' + '2s' + 'h' + 'l' + 'h' + 'hhll' * 8 + 'h'
    return struct.pack(tiff_header_struct,
                       b'II',  # Byte order indication: Little indian
                       42,  # Version number (always 42)
                       8,  # Offset to first IFD
                       8,  # Number of tags in IFD
                       256, 4, 1, width,  # ImageWidth, LONG, 1, width
                       257, 4, 1, height,  # ImageLength, LONG, 1, lenght
                       258, 3, 1, 1,  # BitsPerSample, SHORT, 1, 1
                       259, 3, 1, CCITT_group,  # Compression, SHORT, 1, 4 = CCITT Group 4 fax encoding
                       262, 3, 1, 0,  # Threshholding, SHORT, 1, 0 = WhiteIsZero
                       273, 4, 1, struct.calcsize(tiff_header_struct),  # StripOffsets, LONG, 1, len of header
                       278, 4, 1, height,  # RowsPerStrip, LONG, 1, lenght
                       279, 4, 1, img_size,  # StripByteCounts, LONG, 1, size of image
                       0  # last IFD
                       )

class scan:
    def __init__ (self, fn):
        self.pdf_file = open (fn, 'rb')
        self.reader = PyPDF2.PdfFileReader (self.pdf_file)
        self.pages = self.reader.getNumPages ()

    def bitmap (self, pnum):
        page = self.reader.getPage (pnum)
        xObject = page['/Resources']['/XObject'].getObject()
        for obj in xObject:
            if xObject[obj]['/Subtype'] == '/Image':
                """
                The  CCITTFaxDecode filter decodes image data that has been encoded using
                either Group 3 or Group 4 CCITT facsimile (fax) encoding. CCITT encoding is
                designed to achieve efficient compression of monochrome (1 bit per pixel) image
                data at relatively low resolutions, and so is useful only for bitmap image data, not
                for color images, grayscale images, or general data.

                K < 0 --- Pure two-dimensional encoding (Group 4)
                K = 0 --- Pure one-dimensional encoding (Group 3, 1-D)
                K > 0 --- Mixed one- and two-dimensional encoding (Group 3, 2-D)
                """
                if xObject[obj]['/Filter'] == '/CCITTFaxDecode':
                    if xObject[obj]['/DecodeParms']['/K'] == -1:
                        CCITT_group = 4
                    else:
                        CCITT_group = 3
                    width = xObject[obj]['/Width']
                    height = xObject[obj]['/Height']
                    data = xObject[obj]._data  # sorry, getData() does not work for CCITTFaxDecode
                    img_size = len (data)
                    data = tiff_header_for_CCITT (width, height, img_size, CCITT_group) + data
                    stream = io.BytesIO (data)
                    bm = wx.Bitmap (wx.Image (stream, wx.BITMAP_TYPE_TIFF))
                    return bm

class Module:
    """A module's entry in a wire list.
    """
    def __init__ (self, mod = None):
        self.mem = False
        self.mtype = None
        self.slot = self.modname = self.generic = ""
        self.slots = [ "", None ]
        self.slotnum = 0
        if mod:
            gd = mod.groupdict ()
            self.generic = gd["generic"] or ""
            self.setslot (gd["slot"], gd["slot2"])
            self.setmod (gd["mod"])
            self.hcomments = [ gd["hcomment"], gd["hcomment2"] ]
            pins = [ [ ], [ ] ]
            for side, spins in (0, gd["pins"]), (1, gd["pins2"]):
                if not spins:
                    break
                for g in wlist._re_wline.finditer (spins):
                    pnum = int (g.group (1))
                    mod = (g.group (2) or "").replace (" ", "")
                    pin = (g.group (3) or "").replace ("I", "1")
                    wlen = (g.group (4) or "").replace ("I", "1")
                    pins[side].append ([g.group (1), mod, pin, wlen, g.group (5)])
            self.pins = pins
        else:
            pins = list ()
            for i in range (28):
                pins.append ([ "{}".format (i + 1), "", "", "", None ])
            self.pins = [ pins, None ]
            self.offsets = [ 0.52, None, 1.0, None ]
        self.voff = defvoff
            
    def __str__ (self):
        ret = list ()
        for c in range (self.mem + 1):
            slot = self.slots[c]
            hc = self.hcomments[c]
            if c:
                s = slot
            else:
                s = "{}{}\t{}".format (self.modname, self.generic, slot)
            if hc:
                s = "{}{}".format (s, hc)
            s = tsp_pat.sub ("", s)
            ret.append (s)
            for p in self.pins[c]:
                s = "{}\t{}\t{}\t{}\t".format (*p[:4])
                s = tsp_pat.sub ("", s)
                if p[4]:
                    s = "{}{}".format (s, p[4])
                ret.append (s)
        return '\n'.join (ret)
    
    def setmod (self, modname):
        modname = modname.lower ()
        self.mem = modname == "mem"
        self.modname = modname.upper ()
        try:
            self.mtype = cmodule.elements[modname]
        except KeyError:
            #print ("reading", modname)
            try:
                self.mtype = cmodule.readmodule (modname, True)
                # Fill in pinnames because that doesn't get done by
                # readmodule, TBD why not.
                for pl, p in self.mtype.pins.items ():
                    for pn in pl.split ("_"):
                        self.mtype.pinnames[pn] = p
            except FileNotFoundError:
                print ("Module", modname.upper (),
                       "not defined, at", self.slot)
                cmodule.elements[modname] = None
            
    def setslot (self, slot, slot2 = None):
        self.slot = slot
        self.mem = slot2 is not None
        self.slots = [ slot, slot2 ]
        m2 = wlist._re_chslot.match (slot)
        if m2:
            self.slotnum = int (m2.group (3))
            left = self.slotnum & 1
            if self.mem and self.slotnum == 18:
                # Memory slot, slot 18 is at the left side of the page.
                # Apart from that one case, left == odd applies.
                left = 1
            self.right = not left
        else:
            self.slotnum = 0
            self.right = False
        if self.mem:
            # Offsets to the memory module pins
            self.offsets = [ 0.29, 0.52, 0.77, 1.0 ]
        else:
            # Offsets to non-memory pins
            self.offsets = [ 0.52, None, 1.0, None ]

wlheader = """#
# OCR from {}
#
# That document is one of the CDC 6600 "Chassis Tabs" manuals
# which document the detailed hardware design of the 6600 as
# created by Seymour Cray and his team at Control Data.
#
# Scans supplied from the Computer History Museum collection
# by Al Kossow.
#
"""

class scanwin (wx.Window):
    """Display window class derived from wx.Window which displays a portion of
    a selected PDF page (of the first/only image on that page, that is).

    It handles repaint and close events.
    """
    def __init__ (self, parent, id, size, pos, fn):
        wx.Window.__init__ (self, parent, id, pos, size)
        self.Bind (wx.EVT_PAINT, self.OnPaint)
        self.Bind (wx.EVT_CLOSE, self.OnClose)
        self.doShow = True
        self.SetBackgroundColour (wx.WHITE)
        self.bitmap = self.page = None
        self.pdf = scan (fn)

    def setbitmap (self, page, offset, voff):
        page -= 1
        if self.page != page:
            self.bitmap = self.pdf.bitmap (page)
        self.offset = offset
        self.voff = voff
        self.page = page
        self.Refresh (False)

    def OnPaint (self, event = None):
        """This is the event handler for window repaint events,
        which is also done on window resize. 
        """
        dc = wx.PaintDC (self)
        if self.bitmap:
            # Figure out the scale
            w, h = self.Size
            h = h / (1 - 2 * self.voff)
            bw, bh = self.bitmap.Size
            s = min (h / bh, 1.0)
            dx = -(bw * self.offset - w / s)
            dy = (-h * self.voff) / s
            dc.SetLogicalScale (s, s)
            dc.DrawBitmap (self.bitmap, dx, dy)

    def OnClose (self, event = None):
        """Close the window
        """
        self.bitmap = None

class entrywin (wx.Window):
    """Display window class derived from wx.Window which displays a 
    data entry panel.
    """
    def __init__ (self, parent, id, pos):
        wx.Window.__init__ (self, parent, id, pos)
        self.parent = parent
        self.NavigateIn ()
        self.Bind (wx.EVT_NAVIGATION_KEY, self.nextfield)
        self.Bind (wx.EVT_CLOSE, self.OnClose)
        self.Bind (wx.EVT_TEXT_ENTER, self.nextline)
        self.Bind (wx.EVT_TEXT, self.newtext)
        self.Bind (wx.EVT_CHILD_FOCUS, self.cfocus)
        self.display = display
        self.doShow = True
        self.font = wx.Font (14, wx.FONTFAMILY_MODERN,
                             wx.FONTSTYLE_NORMAL,
                             wx.FONTWEIGHT_NORMAL)
        dc = wx.MemoryDC ()
        dc.SetFont (self.font)
        self.fontextent = dc.GetTextExtent ("A")
        cw, ch = self.fontextent
        sz = wx.Size (cw * 6, ch * 1.5)
        cs = wx.TE_PROCESS_ENTER
        pitch = cw * 9
        lead = ch * 1.7
        self.mtype = wx.TextCtrl (self, 2, pos = wx.Point (20, lead),
                                  size = sz, style = cs)
        self.slot  = wx.TextCtrl (self, 3, pos = wx.Point (20 + 1.5 * pitch, lead),
                                  size = sz, style = cs)
        self.lines = [ (None, None, self.mtype, self.slot, None) ]
        for i in range (1, 31):
            y = (i + 2) * lead
            lt = "{:>2d}".format (i)
            lbl  = wx.StaticText (self, label = lt, pos = wx.Point (10, y),
                                  size = wx.Size (30, ch * 1.5))
            mod  = wx.TextCtrl (self, i * 4 + 1, pos = wx.Point (40 + 0 * pitch, y),
                                size = sz, style = cs)
            pin  = wx.TextCtrl (self, i * 4 + 2, pos = wx.Point (40 + 1 * pitch, y),
                                size = sz, style = cs)
            wlen = wx.TextCtrl (self, i * 4 + 3, pos = wx.Point (40 + 2 * pitch, y),
                                size = sz, style = cs)
            comment = wx.StaticText (self, label = "-", pos = wx.Point (30 + 3 * pitch, y),
                                     size = wx.Size (70, ch * 1.5),
                                     style = wx.ST_NO_AUTORESIZE)
            self.lines.append ((lbl, mod, pin, wlen, comment))
        self.SetClientSize (wx.Size (70 + 3 * pitch, lead * 36))

    def load (self, mod, pins, slot, pins2):
        self.mod = mod
        self.mtype.ChangeValue (mod.modname)
        self.slot.ChangeValue (slot)
        self.pins = pins
        self.pins2 = pins2
        for i, r in enumerate (self.lines):
            if i:
                if pins2:
                    i += 100
                i = "p{}".format (i)
                defined = mod.mtype is None or i in mod.mtype.pinnames
                for c in r:
                    c.Enable (defined)
        for r in self.lines[29], self.lines[30]:
            for c in r:
                c.Show (mod.mem)
        if mod.mem:
            self.maxline = 30
        else:
            self.maxline = 28
        assert len (pins) == self.maxline
        for pnum1, pin in enumerate (pins):
            pnum = pnum1 + 1
            lb, tmod, tpin, tlen, tcomment = self.lines[pnum]
            tmod.ChangeValue (pin[1])
            tpin.ChangeValue (pin[2])
            tlen.ChangeValue (pin[3])
            c = pin[4] or ""
            c = lsp_pat.sub ("", c)
            tcomment.SetLabel (c)

    def setcomment (self, pnum = None):
        if not pnum:
            id = wx.Window.FindFocus ().Id
            pnum = id // 4
        if not pnum:
            return
        tcomment = self.lines[pnum][4]
        d = wx.TextEntryDialog (self, "Pin {} comment".format (pnum),
                                value = tcomment.Label)
        d.ShowModal ()
        c = c2 = d.Value
        if c:
            c = lsp_pat.sub ("", c)
            if not c.startswith ("#"):
                c = "# " + c
            c2 = '\t' + c
        tcomment.SetLabel (c)
        self.parent.setmod ()
        self.pins[pnum - 1][4] = c2
        self.lines[pnum][1].SetFocus ()

    def cleanfields (self):
        for i in range (1, 29):
            lb, tmod, tpin, tlen, tcomment = self.lines[i]
            if not lb.Enabled and \
              (tmod.Value or tpin.Value or tlen.Value or tcomment.GetLabel ()):
              tmod.Value = tpin.Value = tlen.Value = ""
              tcomment.SetLabel ("")
                
    def shiftright (self):
        id = wx.Window.FindFocus ().Id
        pnum, id = divmod (id, 4)
        if not pnum:
            return
        for f in range (3, id - 1, -1):
            self.lines[pnum][f + 1].Value = self.lines[pnum][f].Value
        self.lines[pnum][id].Value = ""
        
    def setmtype (self, mtype):
        mod = self.mod
        mod.setmod (mtype)
        for i, r in enumerate (self.lines):
            if i:
                if self.pins2:
                    i += 100
                i = "p{}".format (i)
                defined = i in mod.mtype.pinnames
                for c in r:
                    c.Enable (defined)
        for r in self.lines[29], self.lines[30]:
            for c in r:
                c.Show (mod.mem)
        if mod.mem:
            self.maxline = 30
        else:
            self.maxline = 28
        
    def top (self):
        # Position at the first pin
        self.curline = 0
        self.field = 3
        wx.PostEvent (self, wx.NavigationKeyEvent ())
        
    def nextline (self, event):
        while self.curline < self.maxline:
            self.curline += 1
            self.field = 1
            t = self.lines[self.curline][1]
            if t.Enabled:
                t.SetFocus ()
                return
        else:
            event.Skip ()

    def nextfield (self, event):
        self.field += 1
        if self.field > 3:
            self.nextline (event)
        else:
            self.lines[self.curline][self.field].SetFocus ()

    def up (self):
        l = self.curline
        while l > 1:
            l -= 1
            t = self.lines[l][self.field]
            if t.Enabled:
                self.curline = l
                t.SetFocus ()
                return
            
    def down (self):
        l = self.curline
        while l <= 28:
            l += 1
            t = self.lines[l][self.field]
            if t.Enabled:
                self.curline = l
                t.SetFocus ()
                return
            
    def newtext (self, event):
        l, c = divmod (event.Id, 4)
        r = self.lines[l]
        t = r[c]
        if l:
            self.pins[l - 1][c] = t.Value
        else:
            # top row changed
            if c == 2:
                # module type
                mtype = t.Value
                if len (mtype) > 1:
                    self.setmtype (mtype)
            elif c == 3:
                # slot
                slot = t.Value
                if len (slot) > 2:
                    self.mod.setslot (slot)
        if l and c == 1:
            lb, mod, pin, wlen, tcomment = r
            if mod.Value.lower () == "x":
                mod.Value = "GND"
                pin.Value = "X"
                wlen.Value = "2"
                self.nextline (event)
        self.parent.setmod ()
        event.Skip ()

    def cfocus (self, event = None):
        id = wx.Window.FindFocus ().Id
        l, c = divmod (id, 4)
        self.curline = l
        self.field = c
        
    def OnClose (self, event = None):
        """Close the window
        """
        self.Destroy ()

class topframe (wx.Frame):
    wscale = 0.28
    
    def __init__ (self, display, id, name, fn, wl, wl_fn):
        framestyle = (wx.MINIMIZE_BOX |
                      wx.MAXIMIZE_BOX |
                      wx.RESIZE_BORDER |
                      wx.SYSTEM_MENU |
                      wx.CAPTION |
                      wx.CLOSE_BOX |
                      wx.FULL_REPAINT_ON_RESIZE)
        self.display = display
        self.doShow = True
        x, y, w, h = display.ClientArea
        fh = h
        fw = h * self.wscale
        tm = y
        wx.Frame.__init__ (self, None, id, name,
                           pos = wx.Point (w / 5, y), style = framestyle)
        self.wl = wl
        self.wl_fn = wl_fn
        h, t = wl
        self.header = h.splitlines ()
        m = first_pat.match (self.header[-1])
        if m:
            page = int (m.group (1))
        else:
            page = 4
        self.firstpage = page
        self.pages = list ()
        self.modified = False
        next = None
        for m in wlist._re_wmod.finditer (t):
            mod = Module (m)
            if not next or (mod.slotnum != next and not mod.mem):
                curpage = [ None, None ]
                self.pages.append (curpage)
            curpage[mod.right] = mod
            next = not mod.right and mod.slotnum + 1
        if self.pages[0][0]:
            self.minindex = 0
        else:
            self.minindex = 0
        self.maxindex = len (self.pages) * 4 - 1
        lp = self.pages[-1]
        lm = lp[1]
        if not lm:
            self.maxindex -= 2
            lm = lp[0]
        if not lm.pins[1]:
            self.maxindex -= 1
        self.index = self.minindex
        self.sframe = scanwin (self, wx.ID_ANY, wx.Size (fw, fh), wx.Point (0, 0), fn)
        self.Bind (wx.EVT_CHAR_HOOK, self.key)
        self.Bind (wx.EVT_TEXT_ENTER, self.nextslot)
        self.sframe.Show (True)
        self.eframe = entrywin (self, wx.ID_ANY, wx.Point (fw, 0))
        self.eframe.Show (True)
        self.showpage ()
        self.SetClientSize (wx.Size (fw + self.eframe.Size.width, fh))

    def key (self, event = None):
        k = event.GetKeyCode ()
        ctrl = event.RawControlDown ()
        shift = event.ShiftDown ()
        if not ctrl:
            if k == wx.WXK_UP:
                self.eframe.up ()
            elif k == wx.WXK_DOWN:
                self.eframe.down ()
            else:
                event.Skip ()
            return
        if 32 < k < 127:
            k = chr (k).lower ()
        if k == 'n':
            if shift:
                # Shift/Ctrl/N, show next scan
                self.firstpage += 1
                self.setmod ()
                self.showpage ()
            else:
                # Ctrl/N, show next part
                self.nextslot ()
        elif k == 'p':
            if shift:
                # Shift/Ctrl/P, show previous scan
                self.firstpage = max (1, self.firstpage - 1)
                self.setmod ()
                self.showpage (False)
            else:
                # Ctrl/P, show previous part
                self.prevslot ()
        elif k == "g":
            # Ctrl/G, go to specified page number
            d = wx.TextEntryDialog (self, "Page number", value = str (self.curpage))
            d.SetMaxLength (3)
            d.ShowModal ()
            g = (int (d.Value) - self.firstpage) * 4
            self.index = max (self.minindex, min (g, self.maxindex))
            self.showpage ()
        elif k == "a":
            # Ctrl/A, add a module
            lp, lsp = divmod (self.maxindex, 4)
            ls = lsp // 2
            lmod = self.pages[lp][ls]
            slot = lmod.slot
            m = wlist._re_chslot.match (slot)
            if lmod.slotnum == 42:
                row = m.group (2)
                if row == "R":
                    event.Skip ()
                    return
                slot = "{}{:c}1".format (m.group (1), ord (row) + 1)
            else:
                slot = "{}{}{}".format (m.group (1), m.group (2), lmod.slotnum + 1)
            mod = Module ()
            mod.setslot (slot)
            if mod.right:
                self.pages[lp][1] = mod
            else:
                self.pages.append ([ mod, None ])
            if lmod.mem:
                self.maxindex += 1
            else:
                self.maxindex += 2
            self.index = self.maxindex
            self.showpage ()
        elif k == 's':
            # Ctrl/S, save file
            backup = self.wl_fn + "~"
            if not os.path.exists (backup):
                os.rename (self.wl_fn, backup)
            fp = "# Starting page {}".format (self.firstpage)
            m = first_pat.match (self.header[-1])
            if m:
                self.header[-1] = fp
            else:
                self.header.append (fp)
            with open (self.wl_fn, "wt") as f:
                print ('\n'.join (self.header), file = f)
                for p in self.pages:
                    for m in p:
                        if m:
                            print (m, file = f)
            self.setmod (False)
        elif k == "3":
            self.eframe.setcomment ()
        elif k == 'r' or k == wx.WXK_RIGHT:
            # Ctrl/R or Ctrl/rightarrow, shift fields right
            self.eframe.shiftright ()
        elif k == 'c':
            # Ctrl/C, clear out fields for unused pins
            self.eframe.cleanfields ()
        else:
            event.Skip ()

    def nextslot (self, event = None):
        if self.index == self.maxindex:
            return
        self.index += 1
        self.showpage ()

    def prevslot (self):
        if self.index == self.minindex:
            return
        self.index -= 1
        self.showpage (False)

    def setmod (self, mod = True):
        self.modified = mod
        self.settitle ()

    def settitle (self):
        mod = "*" if self.modified else ""
        title = "{} Page {}.{}".format (mod, self.curpage, self.subpage + 1)
        self.Title = title
        
    def showpage (self, fwd = True):
        page, subpage = divmod (self.index, 4)
        right, pins2 = divmod (subpage, 2)
        mod = self.pages[page][right]
        pins = mod and mod.pins[pins2]
        if pins:
            self.curpage = page + self.firstpage
            self.subpage = subpage
            self.settitle ()
            self.sframe.setbitmap (self.curpage, mod.offsets[subpage], mod.voff)
            self.eframe.top ()
            self.eframe.load (mod, pins, mod.slots[pins2], pins2)
        elif fwd:
            self.nextslot ()
        else:
            self.prevslot ()
        
class scanApp (wx.App):
    def start (self, args):
        """Start the wx App main loop.
        This finds a font, then opens up a Frame
        """
        global display, defvoff
        self.app = wx.App ()
        display = wx.Display ()
        wl_fn = args[0]
        wl = wlist.readfile (wl_fn, False)
        m = pdf_pat.search (wl[0])
        if len (args) > 1:
            defvoff = float (args[1])
        else:
            defvoff = 0.08
        if len (args) > 2:
            pdf_filename = args[2]
        elif m:
            pdf_filename = os.path.join (pdf_path, m.group (0))
        else:
            print ("No PDF file name")
            sys.exit (1)
        print (wl_fn, pdf_filename)
        self.tf = topframe (display, wx.ID_ANY, "Scan display",
                            pdf_filename, wl, wl_fn)
        self.tf.Show (True)
        self.running = True
        print ("entering mainloop")
        self.app.MainLoop ()
        print ("mainloop exited")
        # Come here only on application exit.
        display = None
    
def main (args):
    app = scanApp ()
    app.start (args)
    
if __name__ == "__main__":
    main (sys.argv[1:])
