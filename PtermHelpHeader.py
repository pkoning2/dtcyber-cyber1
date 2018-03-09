# create ptermhelp.h floppy header from ptermhelp.mte floppy image
import sys
if len(sys.argv) != 3:
    print ("Usage: ",sys.argv[0],"inputfile outputfile")
    print ("Takes a .MTE file as input.  Make a .H as output")
print ("Converting:",sys.argv[1],"to",sys.argv[2])
binFile = open(sys.argv[1],'rb')
outFile = open(sys.argv[2],"w")
binaryData = binFile.read(1261568)
offset = 0
for k in range(0, 9856*8):
    outFile.write("/* " + hex(offset) + " */\t")
    str = ""
    for i in range(offset, offset+15):
        offset += 1
        str += (hex(binaryData[i])+","+"\t")
    str += (hex(binaryData[i+1]))
    offset += 1
    if (offset < 1261568):
        str += ","
    outFile.write(str+"\n")
    print(str)
outFile.close()
