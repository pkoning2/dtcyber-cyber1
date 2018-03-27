#!/usr/bin/env python3

""" Create ptermhelp.h floppy header from ptermhelp.mte floppy image

This version creates a trimmed version, with trailing zeroes omitted.
"""

import sys
if len(sys.argv) != 3:
    print ("Usage: ",sys.argv[0],"inputfile outputfile")
    print ("Takes a .MTE file as input.  Make a .H as output")
print ("Converting:",sys.argv[1],"to",sys.argv[2])
binFile = open(sys.argv[1],'rb')
outFile = open(sys.argv[2],"w")
binaryData = binFile.read()
binFile.close ()
if len (binaryData) != 1261568:
    raise ValueError ("Unexpected file size for {}".format (sys.argv[1]))
trimmedData = binaryData.rstrip (b'\000')
print ("trimmed from 1261568 to {} bytes".format (len (trimmedData)))

print ("/* {} generated from {} */".format (sys.argv[2], sys.argv[1]),
       file = outFile)

olist = list ()
for b in trimmedData:
    olist.append ("0x{:0>2x}".format (b))
    if len (olist) == 8:
        olist.append ("")
        print (", ".join (olist), file = outFile)
        olist = list ()
if olist:        
    print (", ".join (olist), file = outFile)
outFile.close()
