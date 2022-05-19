#!/usr/bin/env python3

"""Module for NOS device I/O

"""

import struct
from abc import abstractmethod, ABCMeta
import re

# Build some struct objects for quick unpacking of data
u60 = struct.Struct (">Q")   # 60-bit data in disk images
u32 = struct.Struct ("<I")   # TAP file length field
cws = struct.Struct ("BBB")  # disk sector control word (in 3 bytes)

m60 = 0o77777777777777777777
m48 = 0o77777777777777000000
alnum_re = re.compile ("[a-z0-9]+", re.I)

class device (object):
    """Base class for NOS devices.
    """
    def __init__ (self, fn):
        self.f = open (fn, "rb")

    def close (self):
        if self.f:
            self.f.close ()
        self.f = None

    def seek (self, n):
        self.f.seek (n)

    def srt (self, data):
        """This mimics the NOS routine SRT from common deck COMCSRT.
        Input is the beginning of the record.  Return value is the
        record name (upper case), record type, date, time, os,
        language processor, and comment.  Record type is a string,
        corresponding to the standard record types:
        TEXT,PP,REL,OVL,ULIB,OPL,OPLC,OPLD,ABS,PPU,CAP,PROC,PPL.
        Date, time, os, processor, and comment are obtained from the
        7700 (PRFX) table, and are returned as empty strings if there
        is no such table.  The copyright is stripped from the comment
        if it is found in the usual form, matching the logic in the
        CATALOG utility.
        """
        h = data[0]
        # Set default answers
        fn = dt = tm = os = proc = cm = ""
        tp = "TEXT"
        p = 0
        if (h >> 48) == 0o7700:
            # PRFX (7700) table
            wc = (h >> 36) & 0o7777
            p = wc + 1
            assert len (data) > p
            h2 = data[1]
            fn = wtod (h2 & m48, uc = True)
            if wc == 0:
                # UPDATE compressed text
                return fn, "TEXT", dt, tm, os, proc, cm
            else:
                dt = wtod (data[2])
                tm = wtod (data[3])
                os = wtod (data[4], uc = True)
                proc = wtod (data[5])
                cm = list ()
                for d in data[8:15]:
                    c = wtod (d, uc = True)
                    if c == "COPYRIGHT " and cm:
                        # By convention, if we encounter COPYRIGHT at
                        # word boundary, stop before that point.
                        break
                    cm.append (c)
                cm = "".join (cm)
                if h2 & 0o777777:
                    # MODIFY compressed text
                    return fn, "TEXT", dt, tm, os, proc, cm
        else:
            # No prefix table, assume record name is in first word
            fn = wtod (h & m48, uc = True)
            
        # Continue analyzing after the prefix table, if there was one.
        h = data[p]
        if (h >> 48) == 0o7000:
            # LDSET (7000) table
            wc = (h >> 36) & 0o7777
            if wc:
                # Skip over 7000 table if not OPLD
                p += wc + 1
        h = data[p]

        # See if it's a PROC
        if fn[:6] == ".PROC,":
            fn = (data[0] << 36) + (data[1] >> 24)
            fn = wtod (fn & m48, uc = True)
            m = alnum_re.match (fn)
            if m:
                fn = m.group (0)
            else:
                fn = ""
            return fn, "PROC", dt, tm, os, proc, cm

        # See if it's a PP (program starts with 6PPM table, which
        # has a 3-character program name and, and if the name starts
        # with a letter, a non-zero load address)
        fn2 = wtod (h, False)
        wc = h & 0o7777
        if fn2[:3].isalnum () and ((h >> 36) & 0o77) == 0 \
               and (fn[0].isnumeric () or ((h >> 24) & 0o7777) != 0):
            if wc == 0:
                return fn, "TEXT", dt, tm, os, proc, cm
            return fn, "PP", dt, tm, os, proc, cm

        # Something else, get table type at start of program
        tb = h >> 48
        if tb == 0o3400:
            # Relocatable
            return fn, "REL", dt, tm, os, proc, cm
        if tb == 0o7600:
            # User library
            return fn, "ULIB", dt, tm, os, proc, cm
        if tb == 0o7001:
            # Old program library
            return fn, "OPL", dt, tm, os, proc, cm
        if tb == 0o7002:
            # OPL common deck
            return fn, "OPLC", dt, tm, os, proc, cm
        if tb == 0o7000:
            # OPL directory
            return fn, "OPLD", dt, tm, os, proc, cm
        if tb == 0o6000:
            # CAP
            return fn, "CAP", dt, tm, os, proc, cm
        if tb == 0o6100:
            # 16 bit (180 series) PP program
            return fn, "PPL", dt, tm, os, proc, cm
        if tb == 0o5400:
            # EACPM
            if ((h >> 36) & 0o7777) == 0:
                # Overlay level 0,0 means "absolute"
                return fn, "ABS", dt, tm, os, proc, cm
            # Otherwise it's OVL
            return fn, "OVL", dt, tm, os, proc, cm
        if tb == 0o5300:
            # ACPM
            return fn, "OVL", dt, tm, os, proc, cm
        if tb == 0o5100:
            # EASCM
            return fn, "ABS", dt, tm, os, proc, cm
        if tb == 0o5200:
            # PPU (7000 series peripheral program)
            return fn, "PPU", dt, tm, os, proc, cm
        if tb == 0o5000:
            # Overlay
            return fn, "OVL", dt, tm, os, proc, cm
        # Otherwise, call it text
        return fn, "TEXT", dt, tm, os, proc, cm
            
    def readrec (self):
        """Read a NOS record (which most people would call a "file"),
        from the current position through the EOR/EOF/EOI.

        Returns file name (lower case), data (list of words), and
        EOR/EOF/EOI level number (EOR = 0, EOF > 0, EOI = -1).  File
        name is taken from the second word if the first word is a 7700
        (PRFX) table; otherwise it is taken from the first word.
        """
        data, eor = self.read ()
        if eor or not data:
            return "", [ ], eor
        d1 = data[0]
        p = d1 >> 48
        if p == 0o7700:
            # Prefix (7700) table, get the name from there.
            fn = wtod (data[1] & m48)
        else:
            fn = wtod (d1 & m48)
        while eor is None:
            d2, eor = self.read ()
            data.extend (d2)
        return fn, data, eor

    def catrec (self):
        """Return a catalog line for the next record and the record
        type (None if EOF/EOI or empty record)
        """
        fn, data, eor = self.readrec ()
        if eor:
            if eor < 0:
                return "* EOI *", None
            return "* EOF *", None
        if not data:
            return "(00)", None
        dt = tm = os = cm = ""
        dlen = len (data)
        h = data[0]
        if (h >> 48) == 0o7700:
            # Prefix table, don't count that in reported length
            dlen -= 1 + ((h >> 36) & 0o7777)
        fn, tp, dt, tm, os, proc, cm = self.srt (data)
        return "%-7s %-4s %6o %s %s" % (fn, tp, dlen, dt, cm), tp

    def catalog (self):
        """Return a catalog for the whole file (from current position
        to EOI).
        """
        ret = [ ]
        while True:
            s, tp = self.catrec ()
            ret.append (s)
            if s == "* EOI *":
                break
            if tp == "ULIB":
                # Library, skip its contents
                while tp and tp != "OPLD":
                    s, tp = self.catrec ()
        return '\n'.join (ret)

class tape (device):
    """Basic tape.  This reads a .tap file (standard DtCyber tape image file).
    """
    def __init__ (self, fn):
        super ().__init__ (fn)
        self.bnum = -1
        
    def read (self, sec = None):
        """Read the next block from a tape image file.  Returns the
        block as read, or None if end of file (tape mark).
        """
        bc1 = self.f.read (4)
        if not bc1:
            return None
        bc = u32.unpack (bc1)[0]
        if not bc:
            # Tape mark
            return None
        data = self.f.read (bc)
        # Read the trailing block length
        bc2 = self.f.read (4)
        if bc1 != bc2:
            raise IOError ("Inconsistent length fields in TAP file")
        self.bnum += 1
        return data

    def words (self, data):
        """Convert data (a byte string) into a list of 60-bit words.
        """
        wl = [ ]
        l = len (data)
        for i in range (0, l, 15):
            wl.append ((u60.unpack_from (data, i)[0] >> 4) & 0o77777777777777777777)
            if l >= i + 15:
                wl.append (u60.unpack_from (data, i + 7)[0] & 0o77777777777777777777)
        return wl
        
    def seek (self, n):
        if n:
            raise IOError ("Invalid tape seek")
        super ().seek (0)
        self.bnum = -1
        
class nositape (tape):
    """NOS I-format tape.
    """
    def read (self, sec = None):
        """Read a tape block from a NOS I-format tape.
        Returns the block data as a list of words, and the end of record
        indication.  End of record indication is None (not EOR/EOF/EOI)
        or the level number (EOR = 0, EOF > 0, EOI = -1).
        """
        data = super ().read ()
        if not data:
            # EOI
            return [ ], -1
        bc = len (data)
        # NOS I-format tapes have data that is a multiple of a 60-bit
        # CPU word, plus a 48 bit (4 PP word) trailer.  The trailer
        # immediately follows the data.  If the number of words is odd,
        # there is another 12 bits of zero (unused) after the trailer,
        # for reasons that are not clear.  (Source: NosICreate.c)
        wc, r = divmod (bc - 6, 15)
        if r == 0:
            wc *= 2
            trailer = u60.unpack (b"\x00\x00" + data[-6:])[0]
            data = data[:-6]
        elif r == 9:
            wc = wc * 2 + 1
            trailer = u60.unpack (b"\x00" + data[-8:-1])[0] >> 4
            data = data[:-7]
        else:
            raise IOError ("Unexpected block length %d" % bc)
        trailer &= 0o00007777777777777777
        bc = trailer >> 36
        bnum = (trailer >> 12) & 0o77777777
        lnum = trailer & 0o7777
        if self.bnum != bnum:
            raise IOError ("Block number mismatch, expected %d, actual %d"
                           % (bnum, self.bnum))
        data = self.words (data)[:(bc - 4) // 5]
        if bc == 0o5004:
            # Full data, so not EOR/EOF
            return data, None
        # EOR/EOF
        return data, lnum

class disk (device):
    """Basic disk.
    """
    def readsec (self, count = 0):
        """Read raw data from the supplied file descriptor.
        Stop after "count" words if non-zero.
        The data read is returned as a list of 60 bit words.
        """
        wl = [ ]
        while count == 0 or len (wl) < count:
            w = self.f.read (15)
            if len (w) < 8:
                break
            wl.append (u60.unpack (w[:8])[0] >> 4)
            if len (w) == 15:
                wl.append (u60.unpack (w[7:15])[0] & 0o77777777777777777777)
            else:
                break
        return wl

    def read (self, sec):
        """Read data from a disk image at the given sector
        number.  Return data is a 64 entry list of 60-bit
        words plus the two control words (from the start of
        the sector) which are 12 bit values.
        """
        self.seek (sec * 512, 0)
        cw = self.f.read (3)
        if not cw:
            return [ ], 0, 0
        b1, b2, b3 = cws.unpack (cw)
        self.cw1 = (b1 << 4) + (b2 >> 4)
        self.cw2 = ((b2 & 0x0f) << 8) + b3
        wl = self.readsec (64)
        return wl, self.cw1, self.cw2

class nosdisk (disk):
    """NOS file structured disk.
    """
    def read (self, trk = None, sec = None, syssec = False):
        """Read a sector from the specified NOS logical track and
        sector address.  Returns the block data as a list of words,
        and the end of record indication.  End of record indication is
        None (not EOR/EOF/EOI) or the level number (EOR = 0, EOF > 0,
        EOI = -1), same as NOS tape read.  The next track/sector are
        saved in the object state, and are used as default position
        for the next read.
        By default, the system sector is skipped; if syssec is True,
        it will be returned.
        """
        if trk is not None:
            self.trk = trk
        if sec is not None:
            self.sec = sec
        data, cw1, cw2 = super().read (self.ltop (self.trk, self.sec))
        # Set next track/sector and compute data length and eor
        dlen = 64
        eor = None
        if cw1 == 0o3777:
            # System sector, next is first data sector
            self.sec = 1
            if not syssec:
                # Skipping the system sector, so read the next sector
                # after that and return its data.
                return self.read ()
        elif self.cw1 != 0:
            # Link to next
            dlen = cw2
            if dlen < 64:
                # Short sector, so EOR
                eor = 0
            if self.cw1 < 0o4000:
                self.sec += 1
            else:
                self.trk = cw1 & 0o3777
                self.sec = 0
        else:
            # EOF/EOI
            dlen = 0
            if cw2 == 0:
                eor = -1    # EOI
            else:
                # EOF, process link to next
                eor = 0o17
                if cw2 < 0o4000:
                    self.sec += 1
                else:
                    self.trk = cw2 & 0o3777
                    self.sec = 0
        return data[:dlen], eor
    
    @abstractmethod
    def ltop (self, trk, sec):
        """Convert NOS logical track and sector to a disk image 512
        byte sector offset.  This has to be supplied by the subclass.
        """

class di (nosdisk):
    """NOS DI (844-21) type disk.
    """
    def ltop (self, trk, sec):
        """Translate NOS logical track/sector to sector offset.
        """
        pu, d = divmod (sec, 0o153)
        ht = (trk >> 1) & 1
        hc = trk & 1
        pc = (trk & 0o3774) >> 2
        b = ht + d * 2
        c, ps = divmod (b, 0o30)
        pt = c + hc * 0o11
        return ((pc * 19) + pt) * 24 + ps
    
class dj (nosdisk):
    """NOS DJ (844-41) type disk.
    """
    def ltop (self, trk, sec):
        """Translate NOS logical track/sector to sector offset.
        """
        pu, d = divmod (sec, 0o343)
        ht = trk & 1
        pc = (trk & 0o3776) >> 1
        b = ht + d * 2
        pt, ps = divmod (b, 0o30)
        return ((pc * 19) + pt) * 24 + ps

class dm (nosdisk):
    """NOS DM (885) type disk.
    """
    def ltop (self, trk, sec):
        """Translate NOS logical track/sector to sector offset.
        """
        pu, d = divmod (sec, 0o1200)
        ht = trk & 1
        pc = (trk & 0o3776) >> 1
        b = ht + d * 2
        pt, ps = divmod (b, 0o40)
        return ((pc * 40) + pt) * 32 + ps

class platodisk (disk):
    """Plato disk (not NOS file structured).
    """
    def read (self, blk):
        """Read a PLATO block from a disk image at the
        given block number.  (Blocks are 5 sectors each.)
        Return data is a 320 entry list of 60 bit words,
        without the control words because they are not
        used in PLATO.
        """
        wl = [ ]
        for i in range (5):
            s, cw1, cw2 = super ().read (blk * 5 + i)
            wl.extend (s)
        return wl

d2alc = ":abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.#[]%\"_!&'?<>@\\^;"
d2auc = d2alc.upper ()

def wtod (w, lmode = True, uc = False):
    """Convert a 60 bit word to display code.  If lmode is False,
    don't pay attention to end of line (2 or more 00 characters).  If
    lmode is True or defaulted, strip an end of line.  If lmode is
    anything else, strip end of line and append that (presumably a \n
    character).  Return the translated string.
    """
    cl = [ ]
    if uc:
        d2a = d2auc
    else:
        d2a=d2alc
    for i in (54, 48, 42, 36, 30, 24, 18, 12, 6, 0):
        # In line mode, stop as soon as we have only zeroes left
        if (w & ((1 << (i + 6)) - 1)) == 0 and lmode:
            # If we found zero before the last character,
            # that's a line ending, so insert the newline
            # if one was requested.
            if lmode is not True and i:
                cl.append (lmode)
            break
        cl.append (d2a[(w >> i) & 0o77])
    return "".join (cl)

# PLATO code to ASCII, four strings corresponding to the plain, shifted,
# access, and shifted access.
p2a = ( "\001abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.\xf7[]%\xd7\xab\001\001\001\001<>\001\001\001;",
        "\001ABCDEFGHIJKLMNOPQRSTUVWXYZ\001\001\001\001\001\001_'\001\001\001\001\001\001{}&\001\001\"!\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001:",
        "\001\001\001\001\001\001\001\xe6\xf8\001\xe5\xe4\001\xb5\001\xb0\001\001\001\001\001\001\001\001\001\xf6\001\xab\xbb\001\001\001@\xbb\001\001\001&\001\001\\\001\001#\001\001\001\001\001{}\001\xba\001\001\001\001\001\001\001\001\001\001~",
        "\001\001\001\xa9\001\001\xc6\xd8|\xc5\xc4\001\001\001\001\001\001\001\001\001\001\001\001\001\xd6\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001\001" )

def wtop (w, shift, access, lmode = True):
    """Convert a 60 bit word to PLATO code.  If lmode is False,
    don't pay attention to end of line (2 or more 00 characters).  If
    lmode is True or defaulted, strip an end of line.  If lmode is
    anything else, strip end of line and append that (presumably a \n
    character).  Return the translated string, shift state, and access
    state.
    """
    cl = [ ]
    for i in (54, 48, 42, 36, 30, 24, 18, 12, 6, 0):
        # In line mode, stop as soon as we have only zeroes left
        if (w & ((1 << (i + 6)) - 1)) == 0 and lmode:
            # If we found zero before the last character,
            # that's a line ending, so insert the newline
            # if one was requested.
            if lmode is not True and i:
                cl.append (lmode)
            break
        c = (w >> i) & 0o77
        if c == 0o70:
            shift = not shift
        elif c == 0o76:
            access = not access
        else:
            s = 0
            if shift:
                s = 1
            if access:
                s += 2
            cl.append (p2a[s][c])
            shift = access = False
    return "".join (cl), shift, access


def dump (wl):
    """Dump a list of 60 bit words in octal and display code.
    """
    for i in xrange (0, len (wl) - 1, 2):
        print ("%03o/ %020o %020o %s %s" % (i, wl[i], wl[i + 1],
                                            wtod(wl[i], False),
                                            wtod(wl[i + 1], False)))
    # Do the last odd word if the length was odd
    if len (wl) & 1:
        print ("%03o/ %020o                      %s" % (len (wl), wl[-1],
                                                        wtod(wl[-1], False)))

def wordstod (data, uc = False):
    """Convert a list of data words to the corresponding display
    code text.  uc is False (default) for lower case, True for upper
    case.
    """
    ret = [ wtod (d, '\n', uc) for d in data ]
    return "".join (ret)

def wordstop (data):
    """Convert a list of data words to the corresponding PLATO code
    text.
    """
    ret = [ ]
    shift = access = False
    for d in data:
        s, shift, access = wtop (d, shift, access, '\n')
        ret.append (s)
        if (d & 0o7777) == 0:
            # end of line, reset shift/access
            shift = access = False
    return "".join (ret)

def textrecs (t, mfn = None, plato = False):
    """Read records from t, and send all text records (including PROC
    ones) to text files of the same name.  If "mfn" is specified, copy
    only the record of that name.  Code conversion is display code
    if "plato" is False (default), PLATO code otherwise.
    """
    if mfn:
        mfn = mfn.lower ()
    if plato:
        cvt = wordstop
    else:
        cvt = wordstod
    count = 0
    while True:
        fn, data, eor = t.readrec ()
        if eor:
            # EOF or EOI encountered, stop
            break
        fn, tp, *x = t.srt (data)
        fn = fn.lower ()
        if mfn and fn != mfn:
            # Looking for a name match, skip if it isn't
            continue
        if tp not in ("TEXT", "PROC"):
            # Not a text type record, skip it
            continue
        if tp == "TEXT":
            data = data[1:]
        with open (fn, "wt") as f:
            print ("writing", fn)
            f.write (cvt (data))
            count += 1
        if mfn:
            # Asking for a match and found one, quit
            break
    print (count, "files written")
    
