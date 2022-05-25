#!/usr/bin/env python3

"""Utility for accessing PLATO packs and masterfiles.

Copyright (c) 2006-2022  Paul Koning

This program can operate on PLATO packs and masterfiles.  Packs are
accessed as DtCyber disk images.  Masterfiles (CDC style virtual
packs) can be accessed directly (if you have a masterfile image copy
available) or as NOS files contained in a NOS disk image.  
"""

import os
import sys
import re
import argparse
import datetime
import tarfile

sys.path.append (os.path.normpath (os.path.join (os.path.dirname (sys.argv[0]), "..")))
import nosio

disp_dec = ':abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.#[]%"_!&\'?<>@\\^;'
disp_enc = { c : i for (i, c) in enumerate (disp_dec) }

DEFTS = datetime.datetime (1973, 1, 1)

packtypes = (
    "master   a",
    "backup   b",
    "general  c",
    "binary   d"
)

filetypes = (
    None,
    "tutor",
    "(2)",
    "curriculum",    # or old-style course if created before 78/11/01.
    "datafile",
    "compass",
    "group",
    "dataset",
    "plm curriculum", # or pnotes if created before 80/08/01.
    "group notes",
    "(10)",
    "nameset",
    "account",
    "catalog",
    "(14)",
    "clm module",
    "pnotes",
    "document",
    "pcd3",
    "(19)",
    "mailfile",
    "exambase"
)
    
# These file types are nameset-like
NSTYPES = { 3, 6, 8, 9, 11, 13, 16, 18 }
# These file types have "rmts" and "ndir" fields in the file info word
RMTTYPES = { 3, 6, 7, 8, 11, 13, 16, 18 }
DATASET = 7
TUTOR = 1
COMPASS = 5
SOURCE = (TUTOR, COMPASS)
GROUP = 6

blktypes = (
    "source",
    "com", 
    "chars", 
    "micro", 
    "llist", 
    "vocab", 
    "lines", 
    "text",
    "binary",
    "list",
    "access",
)

TEXTBLK = (0, 7, 9)

AMASK = 0o77777777777777000000
GMASK = 0o77777777777777770000
TOP1  = 0o77000000000000000000
TOP2  = 0o77770000000000000000

pdname = "4;;;;;;;;;"     # pack directory "name"

class Pack:
    """Encapsulates a disk pack (container file), either in classic
    or packed encoding.
    """
    def __init__ (self, p, pname = None):
        self.packed = not p.unpacked
        self.mf = p.master_file
        if self.mf:
            ui = p.user_index or 0o377773
            self.nospack = nosio.nosdisk (p.pack)
            if self.mf == "*":
                # Hack to print the NOS catalog.  Also ident while
                # we're at it.
                self.nospack.ident ()
                self.nospack.catlist ()
                self.nospack.close ()
                sys.exit (0)
            self.pack = self.nospack.open (self.mf, ui)
            p.flaw_map = False
            self.name = pname or self.mf
        else:
            self.pack = nosio.platodisk (p.pack)
            self.name = pname or p.pack

    def readpd (self, p):
        "Read the pack directory"
        pdstart = 12 if p.flaw_map else 1
        pdir = self.pack.readblk (pdstart)
        self.pdhdr = pdir[:5]
        self.packname = wtostr (pdir[0])
        self.packtype = wtostr (pdir[1])
        self.pdtype, self.pdsize, self.stotal, self.sused = \
             getvfd (pdir[2], 6, 6, -12, 18, 18)
        ft, self.fused = getvfd (pdir[3], -24, 18, 18)
        self.ftotal = ft
        self.inflth = il = pdir[4]
        bmlen, bmbits = divmod (self.stotal, 48)
        if bmbits:
            bmlen += 1
        self.pdblks = (self.inflth + (self.ftotal * 2) + 320 - 1) // 320
        self.pdparts = (self.pdblks + 6) // 7
        if self.pdparts > 10 or self.stotal > 20000:
            # Pack dir or total parts counts look way out of line, fail
            raise ValueError ("Not a valid pack directory")
        for i in range (1, self.pdblks):
            pdir.extend (self.pack.readblk (i + pdstart))
        self.bitmap = pdir[5:5 + bmlen]
        if bmbits:
            self.bitmap[-1] &= ~((1 << bmbits) - 1)
        self.names = pdir[il:il + ft]
        self.infos = pdir[il + ft:il + 2 * ft]

    def finfo (self, d, ftype):
        "Read the directory block information"
        ret = list ()
        blocks = d[2]
        if d[3] >> 59:
            # New format
            names = d[192:192 + blocks]
            infos = d[64:64 + blocks]
            extrai = 4
        else:
            infos = d[5:5 + blocks]
            names = d[5 + blocks:5 + 2 * blocks]
            extrai = 5 + 2 * blocks
        ctime = wtostr (d[extrai], 9)
        if ctime:
            try:
                ctime = datetime.datetime.strptime (ctime, " %m/%d/%y")
            except Exception:
                print ("bad creation time", ctime, "in", wtostr (d[0]))
                ctime = DEFTS
        else:
            ctime = DEFTS            
        mtime = wtostr (d[extrai + 1], 9) + wtostr (d[extrai + 23])
        if mtime:
            try:
                mtime = datetime.datetime.strptime (mtime, " %m/%d/%y %H.%M.%S.")
            except Exception:
                print ("bad last edit time", mtime, "in", wtostr (d[0]))
                mtime = ctime
        else:
            # If there is no last edit date/time (never
            # edited?) use the create time.
            mtime = ctime
        for s, off, count, *extra in infotab:
            fun = wtostr
            chktype = None
            if extra:
                fun = extra[0]
                if len (extra) > 1:
                    check = extra[1]
                    if isinstance (check, int):
                        if ftype != check:
                            continue
                    else:
                        if ftype not in check:
                            continue
            words = (count + 9) // 10
            ret.append (s)
            if off is None:
                ret.append (self.packname)
            else:
                if off < 0:
                    off += 4
                else:
                    off += extrai
                ret.append (fun (d[off:off + words], count))
        return "".join (ret), names, infos, ctime, mtime
        
    def doinfo (self, p):
        self.readpd (p)
        print ("pack {}, type {}".format (self.packname, self.packtype))
        print ("pack directory size {} ({} parts)".format (self.pdblks, self.pdparts))
        print (" {} files used, {} total".format (self.fused, self.ftotal))
        print (" {} parts used, {} total".format (self.sused, self.stotal))
        print (" pinflth", self.inflth)
        print (" pack model {}, pack dir size {}".format (self.pdtype, self.pdsize))
        
    def dolist (self, p):
        self.doinfo (p)
        for fn, fiw in zip (self.names, self.infos):
            fn = wtostr (fn)
            rmts, ndir, ftype, blks, sblk = getvfd (fiw, -12, 6, 6, 6, 6, -6, 18)
            if p.verbose:
                sblk *= 7
                blks *= 7
                nftype = ftype
                if ftype < 0o40:
                    dblk = self.pack.readblk (sblk)
                    acct = wtostr (dblk[40] & AMASK)
                    ftype = disp_dec[ftype]
                else:
                    acct = ""
                    ftype = " "
                if nftype == DATASET:
                    print ("{:<10s}  {:>3d} {}  {:>5d} {:0>20o} {:>3d}     {}"
                           .format (fn, blks, ftype, sblk, fiw,
                                    rmts, acct))
                elif nftype in RMTTYPES:
                    print ("{:<10s}  {:>3d} {}  {:>5d} {:0>20o} {:>3d} {:>3d} {}"
                           .format (fn, blks, ftype, sblk, fiw,
                                    rmts, ndir, acct))
                else:
                    print ("{:<10s}  {:>3d} {}  {:>5d} {:0>20o}         {}"
                           .format (fn, blks, ftype, sblk, fiw, acct))
            else:
                print (fn)
            if fn == pdname:
                break

    def doread (self, p):
        if not p.file:
            print ("Nothing to copy")
            return
        if not p.output:
            if p.info:
                p.pipe = True
            else:
                p.output = "."
        if not p.file and (len (p.file) > 1 or p.file[0] == "*") and \
           not os.path.isdir (p.output):
            print ("Multiple file transfer but output is not a directory")
            return
        self.readpd (p)
        count = 0
        for fn in p.file:
            if fn == "*":
                for idx in range (self.ftotal):
                    fn = wtostr (self.names[idx])
                    if fn == pdname:
                        break
                    fiw = self.infos[idx]
                    if (fiw >> 30) > 0o37:
                        # Not a normal file, skip it
                        continue
                    if self.doread1 (fn, idx, p):
                        count += 1
                        if p.verbose and not p.info:
                            print ("copied", fn, file = sys.stderr)
            else:
                fnw = dtow (fn)
                try:
                    idx = self.names.index (fnw)
                except ValueError:
                    print ("PLATO file {} not found".format (fn))
                    continue
                self.doread1 (fn, idx, p)
        if count:
            print (count, "files copied")
            
    def doread1 (self, fn, idx, p):
        fiw = self.infos[idx]
        ftype, blks, sblk = getvfd (fiw, -24, 6, 6, -6, 18)
        if ftype > 0o37:
            print ("Not a valid file", fn, "type", ftype)
            return False
        sblk *= 7
        blks *= 7
        if p.pipe:
            f = sys.stdout
        else:
            if os.path.isdir (p.output):
                ofn = os.path.join (p.output, fn)
            else:
                ofn = p.output
            f = open (ofn, "wt")
        dirblk = self.pack.readblk (sblk)
        s, names, infos, ctime, mtime = self.finfo (dirblk, ftype)
        print (s, file = f)
        # Now transfer the file content, either a block by block copy
        # if this is a source type file, or a dump of the data blocks
        # (as a unit) if not.  But if -d was specified, dump the
        # requested blocks a block at a time, for any file type.
        if p.info:
            # Nothing else to do
            pass
        elif ftype in SOURCE:
            blki = iter (range (1, blks))
            for blk in blki:
                name = names[blk]
                info = infos[blk]
                partial, btype, blkcnt, blklen, dblk = \
                    getvfd (info, 1, 5, -27, 9, 9, 9)
                if not blklen:
                    continue
                print (blklabel (name, info, blk), file = f)
                d = self.pack.readblk (sblk + dblk)[:blklen]
                if btype in TEXTBLK:
                    # Some sort of text block, format that
                    fmtsource (d, p, f)
                else:
                    for i in range (blkcnt - 1):
                        blk = next (blki)
                        info2 = infos[blk]
                        partial, btype, blkcnt, blklen, dblk = \
                            getvfd (info2, 1, 5, -27, 9, 9, 9)
                        d.extend (self.pack.readblk (sblk + dblk)[:blklen])
                    nosio.dump (d, f)
        else:
            # Non-source file, just read all the data and dump that.
            print (file = f)
            data = list ()
            for blk in range (1, blks):
                data.extend (self.pack.readblk (sblk + blk))
            nosio.dump (data, f)
        if f is not sys.stdout:
            f.close ()
            if p.dates:
                ctime = ctime.timestamp ()
                mtime = mtime.timestamp ()
                # There doesn't seem to be a way to set just mtime, so
                # instead set both mtime and atime to the saved value.
                if sys.platform == "darwin":
                    # On MacOS, there exists a creation time
                    # attribute, with a somewhat odd way of setting
                    # it.  It's taken from the mtime value provided
                    # that is less than the currently saved create
                    # time.  So we set create time first, then modify
                    # time.
                    os.utime (ofn, (mtime, ctime))
                os.utime (ofn, (mtime, mtime))
        return True

def getvfd (w, *fields):
    """Extract listed fields from the word.  The field arguments give
    field sizes in bits, negative means skip this (no return value),
    positive is a returned data item.
    """
    ret = list ()
    pos = 60
    for f in fields:
        if f < 0:
            pos += f
            assert pos >= 0, pos
        else:
            pos -= f
            assert pos >= 0, pos
            ret.append ((w >> pos) & ((1 << f) - 1))
    return ret

def wtod (w):
    "Translate a 60-bit word to 10 display code characters"
    dc = list ()
    for s in range (54, -1, -6):
        dc.append (disp_dec[(w >> s) & 0o77])
    return "".join (dc)

def wtostr (w, count = 99999):
    if isinstance (w, list):
        s = "".join (wtod (i) for i in w)
    else:
        s = wtod (w)
    return s[:count].rstrip (":")

def showuse (w, count):
    if w[0] == 1:
        return "multi-lesson -use-"
    return wtostr (w, count)

def wtostat (w, x):
    return "{}-{}".format (*divmod (w[0] & 0o7777, 32))

def fmtnum (w, x):
    return str (w[0])

def codewd (w, x):
    w = w[0]
    if not w:
        return "blank--open to all"
    if w & TOP1:
        return "**********"
    w <<= 6
    if w & TOP1:
        return "//////////"
    w <<= 6
    if w & TOP1:
        s = wtostr (w)
        if s == "'n'o'n'e":
            return "no match permitted"
        return "group {}".format (wtostr (w))
    w <<= 6
    if w & TOP1:
        return "account {}".format (wtostr (w))
    return "//////////"

def blklabel (name, info, blk):
    """Return the block label string given name, info word, and block
    number.
    """
    part, pb = divmod (blk, 7)
    part += 1
    pb = disp_dec[pb + 1]
    partial, btype, eyr, emon, eday, ehr, emin = \
        getvfd (info, 1, 5, 7, 4, 5, 5, 6)
    if partial:# and btype == 0:
        partout = " out"
    else:
        partout = ""
    if blk:
        try:
            btype = blktypes[btype]
        except IndexError:
            btype = "({})".format (btype)
    else:
        btype = "fileinfo"
    return "* /---   block={:0>2d}-{} {}           {:0>2d}/{:0>2d}/{:0>2d}. {:0>2d}.{:0>2d}    {} {}" \
           .format (part, pb, wtostr (name), (eyr + 1973) % 100, emon, eday, 
                    ehr, emin, btype, partout)

def sysacc (w, x):
    if w[0] >> 59:
        return "restricted"
    return "allowed"

def dtow (d):
    d = d[:10]
    while len (d) < 10:
        d += ":"
    ret = 0
    for c in d:
        ret = (ret << 6) | disp_enc[c]
    return ret

def strtow (s):
    ret = list ()
    for i in range (0, len (s), 10):
        ret.append (dtow (s[i:i + 10]))
    if not ret or (ret[-1] & 0o7777):
        ret.append (0)
    return ret

def ppdump (data, f = sys.stdout):
    "Dump buffer of PPU words (12 bit words)"
    prev = pstart = None
    for off in range (0, len (data), 8):
        dc = list ()
        line = data[off:off + 8]
        if line == prev:
            if pstart is None:
                pstart = off
            pend = off
        else:
            # Not the same as before, or no "before"
            if pstart is not None:
                print ("        lines {:0>6o} through {:0>6o} same as above".format (pstart, pend), file = f)
            prev = line
            pstart = None
            dline = [ "{:0>6o}/ ".format (off) ]
            for w in line:
                if w:
                    dline.append ("{:0>4o} ".format (w))
                else:
                    dline.append ("---- ")
                dline.append (" ")
                for s in 6, 0:
                    dc.append (disp_dec[(w >> s) & 0o77])
            dline.extend (dc)
            dline = "".join (dline)
            print (dline, file = f)
    if pstart is not None:
        print ("        lines {:0>6o} through {:0>6o} same as above".format (pstart, pend), file = f)

def fmtsource (data, p, f = sys.stdout):
    "Format a block of source text"
    off = 0
    end = len (data)
    while off < end:
        m = 0
        if (data[off] & TOP2) == 0:
            # modword
            m = data[off]
            off += 1
        for eol in range (off, end):
            if (data[eol] & 0o7777) == 0:
                eol += 1
                break
        else:
            eol = end
        if ((m >> 17) & 1) and not p.deleted:
            # Deleted line, skip it
            off = eol
            continue
        if p.modwords:
            if m:
                who = wtostr (m << 12, 5)
                dbit, myr, mmon, mday = getvfd (m, -42, 1, -1, 7, 4, 5)
                dbit = "D" if dbit else " "
                myr = (myr + 73) % 100
                modw = "{:<5s} {}{:0>2d}.{:0>2d}.{:0>2d}" \
                       .format (who, dbit, myr, mmon, mday)
            else:
                modw = "               "
            print (modw, wtostr (data[off:eol]), file = f)
        else:
            print (wtostr (data[off:eol]), file = f)
        off = eol

infotab = (
     (  "file name -------- ", -4, 10),
     ("\nfile type -------- ", -3, 9),
     ("\npack name -------- ", None, 10),
     ("\naccount ---------- ", 36, 7),
     ("\nupdate level ----- ", 49, 10, fmtnum),
     ("\ncreated ----------", 0, 9),
     ("\nlast edited ------", 1, 9),
     ("", 23, 10),
     ("\n         by ------ ", 24, 18),
     (" of ", 26, 8),
     ("\n         at ------ ", 25, 10, wtostat),
     ("\nchange code ------ ", 2, 10, codewd),
     ("\ninspect code ----- ", 3, 10, codewd),
     ("\ncommon code ------ ", 27, 10, codewd, TUTOR),
     ("\nfile access code - ", 27, 10, codewd, COMPASS),
     ("\ndata file -------- ", 27, 10, wtostr, GROUP),
     ("\n-use- code ------- ", 28, 10, codewd, TUTOR),
     ("\nrouter lesson ---- ", 28, 10, wtostr, GROUP),
     ("\n-jumpout- code --- ", 29, 10, codewd, TUTOR),
     ("\ninstructor file -- ", 29, 10, wtostr, GROUP),
     ("\n-attach- code ---- ", 41, 10, codewd, TUTOR),
     ("\nsystem access ---- ", 34, 10, sysacc),
     ("\n-use-d lesson ---- ", 35, 10, showuse, TUTOR),
     ("\nsubmit block ----- ", 35, 10, wtostr, COMPASS),
     ("\ncharset lesson --- ", 30, 10, wtostr, SOURCE),
     ("\ncharset block ---- ", 31, 10, wtostr, SOURCE),
     ("\ntemplate record -- ", 31, 18, wtostr, GROUP),
     ("\nmicro lesson ----- ", 32, 10, wtostr, SOURCE),
     ("\nmicro block ------ ", 33, 10, wtostr, SOURCE),
     ("\nnotes file ------- ", 38, 10, wtostr, TUTOR),
     ("\nprocessor -------- ", 44, 10, wtostr, SOURCE),
     ("\neditor ----------- ", 48, 10, wtostr, SOURCE),
     ("\nauthor name ------ ", 4, 30),
     ("\nauthor dept. ----- ", 7, 20),
     ("\nauthor phone ----- ", 9, 20),
     ("\nsubject ---------- ", 11, 20),
     ("\naudience --------- ", 13, 20),
     ("\ndescription ------ ", 15, 80)
)

pfparser = argparse.ArgumentParser ()
pfparser.add_argument ("pack", help = "File name of pack container file")
pfparser.add_argument ("file", nargs = "*", help = "PLATO file to read")
pfparser.add_argument ("--model", default = "di",
                       help = "Drive model")
pfparser.add_argument ("--tar-file", "-t",
                       help = "Tar archive containing pack container file")
pfparser.add_argument ("-o", "--output",
                       help = """Output file or directory.  Default is
                                 current directory for file copy, output
                                 to terminal (stdout) for other commands""")
pfparser.add_argument ("-p", "--pipe", action = "store_true", default = False,
                       help = "Send file output to stdout")
pfparser.add_argument ("--unpacked", action = "store_true", default = False,
                       help = "Unpacked (old style) container file")
pfparser.add_argument ("-l", "--list", action = "store_true",
                       default = False, help = "List the file names")
pfparser.add_argument ("-i", "--info", action = "store_true",
                       default = False, help = "Show pack information")
pfparser.add_argument ("-v", "--verbose", action = "store_true",
                       default = False, help = "Display more details")
pfparser.add_argument ("-m", "--modwords",
                       action = "store_true", default = False,
                       help = "Show modwords in text output")
pfparser.add_argument ("-D", "--deleted",
                       action = "store_true", default = False,
                       help = "Include deleted lines (implies --modwords)")
pfparser.add_argument ("-F", "--flaw-map",
                       action = "store_true", default = False,
                       help = "Pack has a flaw map")
pfparser.add_argument ("-M", "--master-file", metavar = "MF",
                       help = "Name of master file on the NOS pack. "
                              "Use '*' to print NOS file catalog.")
pfparser.add_argument ("-U", "--user-index", metavar = "UI",
                       help = "User index of master file on the NOS pack")
pfparser.add_argument ("--dates", action = "store_true", default = False,
                       help = "Preserve file modify date")


def main ():
    p = pfparser.parse_args ()
    pack = Pack (p)
    if p.file:
        pack.doread (p)
    elif p.list:
        pack.dolist (p)
    elif p.info:
        pack.doinfo (p)

if __name__ == "__main__":
    main ()
    
