#!/usr/bin/python

"""Convert an FSU printout to a file ready to be given to pldisk,
i.e., each ascii character corresponds to a single cdc 6-bit code.
Insert block label markers, convert upper case to shift codes,
parse all the backspace composites, etc.
"""

import time
import sys
import os
import re

_line1_re = re.compile (r"-+( not condensed -+)?block (\d+)-([a-g]) +'(.*?)'")
_line2_re = re.compile (r" +last edited: (\d+)/(\d+)/(\d+)")

def blockheader (line1, line2):
    """Parse a block header (two lines), and return the pldisk format
    block label string.
    """
    m1 = _line1_re.match (line1)
    m2 = _line2_re.match (line2)
    if not m1 or not m2:
        print "Unrecognized block header:"
        print line1
        print line2
        return ""
    if m1.group (1):
        out = "" #out = "out"
    else:
        out = ""
    return "**/---   block=%02d-%s %-10s           %02d/%02d/%02d. 00.00    source %s \n" % \
           (int (m1.group (2)), m1.group (3), m1.group (4), 
            int (m2.group (3)), int (m2.group (2)), int (m2.group (1)), out)

_bs_re = re.compile ("((['^]?.)?(\b['^]?.)+)|(\(c\)\b\b'6\b'6)")
_bs_dict = { "<\b-" : "_",
             "=\b/" : "^=",
             "<\b(" : "^0",
             ")\b>" : "^1",
             ">\b'6" : "^>",
             "<\b'6" : "^<",
             "';\b-" : "#",
             "/\b^/" : '"',
             "-\b>" : "^6",
             "^\b|\bv" : "^,",
             "\b," : "^c",
             "\b^" : "^x",
             "\b'7" : "^e",
             "\b'," : "^u",
             "\b~" : "^n",
             "(c)\b\b'6\b'6" : "^'c",
             "^\bv\b+" : "^d",
             "^/\b)" : "^l",
             "'o\b/" : "^'t",
             "o\b/" : "^s" }
def _bs_sub (m):
    if m.group (0) in _bs_dict:
        return _bs_dict[m.group (0)]
    return m.group (0)

_uc_re = re.compile (r"[A-Z_'?\"!:{}\\]")
_uc_dict = { "_" : "'6",
             "'" : "'7",
             "|" : "^'i",
             "?" : "'/",
             '"' : "',",
             "!" : "'.",
             ":" : "';",
             "{" : "'[",
             "}" : "']",
             "\\": "^/" }
def _uc_sub (m):
    if m.group (0) in _uc_dict:
        return _uc_dict[m.group (0)]
    return "'" + m.group (0).lower ()

def recode (ln):
    """Translate a line of PLATO printout to ASCII display codes.
    """
    ln = ln.replace ('\000', '?')
    ln = _uc_re.sub (_uc_sub, ln)
    ln = _bs_re.sub (_bs_sub, ln)
    ln = ln.replace ('\b', "' ")
    return ln + '\n'

def main (infile, outfile):
    """Process the input file, generating the output file.
    """
    if infile == outfile:
        print "input and output filenames match"
        return
    inf = open (infile, "r")
    indata = iter (inf)
    outdata = [ ]
    try:
        while True:
            ln = indata.next ().rstrip ('\n')
            if ln.startswith ('\032'):
                raise StopIteration
            while len (ln) == 0:
                ln = indata.next ().rstrip ('\n')
                if len (ln) and not ln.startswith ("        Lesson  "):
                    if ln.startswith ('\032'):
                        raise StopIteration
                    print "Unexpected line:", ln
                    break
                else:
                    indata.next ()
                    indata.next ()
                    ln = indata.next ().rstrip ('\n')
            if not ln.startswith ("        "):
                if ln.startswith ('\032'):
                    raise StopIteration
                print "Unexpected start of line in:", ln
                continue
            ln = ln[8:]
            if ln.startswith ("------------------------------------block") or \
               ln.startswith ("------ not condensed ---------------block"):
                outdata = outdata[:-2]
                outdata.append (blockheader (ln, indata.next ()))
                indata.next ()
                indata.next ()
                continue
            outdata.append (recode (ln))
    except StopIteration:
        inf.close ()
        outf = open (outfile, "w")
        outf.writelines (outdata)
        outf.close ()
        
if __name__ == "__main__":
    if len (sys.argv) != 3:
        print "usage: %s infile outfile" % sys.argv[0]
        sys.exit (1)
    main (*sys.argv[1:])
    
