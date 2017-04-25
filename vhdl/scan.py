#!/usr/bin/env python3

import sys
import io
import os
import re

import wx
import PyPDF2
import struct
import wlist

pdf_pat = re.compile (r"\S+\.pdf", re.I)

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
    def __init__ (self, mod):
        gd = mod.groupdict ()
        self.mtype = gd["mod"]
        self.slot = gd["slot"]
        m2 = wlist._re_chslot.match (self.slot)
        self.slotnum = int (m2.group (3))
        left = self.slotnum & 1
        if gd["pins2"] and self.slotnum == 18:
            # Memory slot, slot 18 is at the left side of the page.
            # Apart from that one case, left == odd applies.
            left = 1
        self.right = not left
        self.mem = gd["slot2"] is not None
        pins = [ [ ], [ ] ]
        self.slots = [ self.slot, gd["slot2"] ]
        for side, spins in (0, gd["pins"]), (1, gd["pins2"]):
            if not spins:
                break
            for g in wlist._re_wline.finditer (spins):
                pnum = int (g.group (1))
                mod = g.group (2) or ""
                pin = g.group (3) or ""
                wlen = g.group (4) or ""
                pins[side].append ([g.group (1), mod, pin, wlen, g.group (5)])
        self.pins = pins
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

    def setbitmap (self, page, offset):
        page -= 1
        if self.page != page:
            self.bitmap = self.pdf.bitmap (page)
        self.offset = offset
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
            bw, bh = self.bitmap.Size
            s = min (h / bh, 1.0)
            off = bw * self.offset - w / s
            dc.SetLogicalScale (s, s)
            dc.DrawBitmap (self.bitmap, -off, 0)

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
        pitch = cw * 10
        lead = ch * 1.7
        self.mtype = wx.TextCtrl (self, 2, pos = wx.Point (20, lead),
                                  size = sz, style = cs)
        self.slot  = wx.TextCtrl (self, 3, pos = wx.Point (20 + 1.5 * pitch, lead),
                                  size = sz, style = cs)
        self.lines = [ (None, None, self.mtype, self.slot) ]
        for i in range (1, 29):
            y = (i + 2) * lead
            lt = "{:>2d}".format (i)
            lbl  = wx.StaticText (self, label = lt, pos = wx.Point (20, y), size = sz)
            mod  = wx.TextCtrl (self, i * 4 + 1, pos = wx.Point (20 + 1 * pitch, y),
                                size = sz, style = cs)
            pin  = wx.TextCtrl (self, i * 4 + 2, pos = wx.Point (20 + 2 * pitch, y),
                                size = sz, style = cs)
            wlen = wx.TextCtrl (self, i * 4 + 3, pos = wx.Point (20 + 3 * pitch, y),
                                size = sz, style = cs)
            self.lines.append ((lbl, mod, pin, wlen))
        self.SetClientSize (wx.Size (20 + 4 * pitch, lead * 36))

    def load (self, mod, pins, slot):
        self.mtype.Value = mod.mtype
        self.slot.Value = slot
        for pin in pins:
            pnum = int (pin[0])
            if pnum > 28:
                continue
            lb, tmod, tpin, tlen = self.lines[pnum]
            tmod.Value = pin[1]
            tpin.Value = pin[2]
            tlen.Value = pin[3]
        
    def top (self):
        self.curline = 0
        self.field = 1
        wx.PostEvent (self, wx.NavigationKeyEvent ())
        
    def nextline (self, event):
        if self.curline == 28:
            event.Skip ()
        else:
            self.curline += 1
            self.field = 1
            self.lines[self.curline][1].SetFocus ()

    def nextfield (self, event):
        self.field += 1
        if self.field > 3:
            self.nextline (event)
        else:
            self.lines[self.curline][self.field].SetFocus ()

    def newtext (self, event):
        l, c = divmod (event.Id, 4)
        if l and c == 1:
            lb, mod, pin, wlen = self.lines[l]
            if mod.Value.lower () == "x":
                mod.Value = "GND"
                pin.Value = "X"
                wlen.Value = "2"
                self.nextline (event)
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
    def __init__ (self, display, id, name, fn, page, wl):
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
        fw = h / 4
        tm = y
        wx.Frame.__init__ (self, None, id, name,
                           pos = wx.Point (w / 4, y), style = framestyle)
        self.wl = wl
        h, t = wl
        self.pages = list ()
        next = None
        for m in wlist._re_wmod.finditer (t):
            mod = Module (m)
            if not next or (mod.slotnum != next and not mod.mem):
                curpage = [ None, None ]
                self.pages.append (curpage)
            curpage[mod.right] = mod
            next = not mod.right and mod.slotnum + 1
        self.firstpage = page
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
        if k < 32 or k > 126 or not ctrl:
            event.Skip ()
            return
        k = chr (k).lower ()
        if k == 'n':
            if shift:
                # Shift/Ctrl/N, show next scan
                self.firstpage += 1
                self.showpage ()
            else:
                # Ctrl/N, show next part
                self.nextslot ()
        elif k == 'p':
            if shift:
                # Shift/Ctrl/P, show previous scan
                self.firstpage = max (1, self.firstpage - 1)
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
            
    def showpage (self, fwd = True):
        page, subpage = divmod (self.index, 4)
        right, pins2 = divmod (subpage, 2)
        mod = self.pages[page][right]
        pins = mod and mod.pins[pins2]
        if pins:
            self.curpage = page + self.firstpage
            title = "Page {}.{}".format (self.curpage, subpage + 1)
            self.Title = title
            self.sframe.setbitmap (self.curpage, mod.offsets[subpage])
            self.eframe.top ()
            self.eframe.load (mod, pins, mod.slots[pins2])
        elif fwd:
            self.nextslot ()
        else:
            self.prevslot ()
        
class scanApp (wx.App):
    def start (self, args):
        """Start the wx App main loop.
        This finds a font, then opens up a Frame
        """
        global display
        self.app = wx.App ()
        display = wx.Display ()
        wl_fn = args[0]
        wl = wlist.readfile (wl_fn, False)
        m = pdf_pat.search (wl[0])
        if len (args) > 1:
            pdf_filename = args[1]
        elif m:
            pdf_filename = os.path.join (pdf_path, m.group (0))
        else:
            print ("No PDF file name")
            sys.exit (1)
        print (wl_fn, pdf_filename)
        self.tf = topframe (display, wx.ID_ANY, "Scan display", pdf_filename, 4, wl)
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
