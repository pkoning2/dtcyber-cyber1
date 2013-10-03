#!

"""Card ID data generator for quad RS232 card.
"""

ADDR = 0x54

import sys
from cardinfo import CardInfo, PinUse

q = CardInfo ("Quad RS232 converter", version = "00A0", mfg = "AK Design",
              partnum = "quad_rs232", i33 = 30, assembly = "Q232")

# Skip pin allocation because of conflicts with BB-UARTn
if True:
    # UART1 RX, TX
    q.addpin ("P9-26", dir = PinUse.IN, rx = PinUse.ENABLE, mux = 0)
    q.addpin ("P9-24", dir = PinUse.OUT, mux = 0)
    # UART2 RX, TX
    q.addpin ("P9-22", dir = PinUse.IN, rx = PinUse.ENABLE, mux = 1)
    q.addpin ("P9-21", dir = PinUse.OUT, mux = 1)
    # UART4 RX, TX
    q.addpin ("P9-11", dir = PinUse.IN, rx = PinUse.ENABLE, mux = 6)
    q.addpin ("P9-13", dir = PinUse.OUT, mux = 6)
    # UART5 RX, TX
    q.addpin ("P8-38", dir = PinUse.IN, rx = PinUse.ENABLE, mux = 4)
    q.addpin ("P8-37", dir = PinUse.OUT, mux = 4)

print "Finished generating data for", q.name, q.serno

data = q.encode ()

e = "/sys/bus/i2c/devices/i2c-1/1-%04x/eeprom" % ADDR
try:
    f = open (e, "rb")
    # Read the existing config content, the standard 244 bytes
    current = f.read (244)
    f.close ()
    i2c = True
except IOError:
    i2c = False

if i2c:
    if current[:4] == CardInfo.MAGIC:
        print "It appears that the card at %02x is already programmed" % ADDR
        print "Board name:  ", current[6:38]
        print "Manufacturer:", current[42:58]
        print "Serial no:   ", current[76:88]
        ok = raw_input ("Proceed anyway (YES/no) [no]? :")
    else:
        print "Card at %02x is not yet programmed" % ADDR
        ok = raw_input ("Proceed (YES/no) [no]? :")
    if ok != "YES":
        sys.exit (1)
    print "Ok, writing new data"
    f = open (e, "wb")
    f.write (data)
    f.close ()
    f = open (e, "rb")
    check = f.read (244)
    if check != data:
        print "ERROR: data read back does not match"
        print repr (data)
        print repr (check)
else:
    print "Writing encoded data to card.dat"
    f = open ("card.dat", "wb")
    f.write (data)
    f.close ()
    
