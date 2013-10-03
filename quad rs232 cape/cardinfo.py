#!

"""Classes for creating BeagleBone cape card-ID EEPROM content
"""

from collections import namedtuple
import struct
import time

_pin = namedtuple ("PinMap", "number name signal")
class PinMap (_pin):
    """A simple class to describe a single pin assignment for
    the BeagleBone expansion connectors.
    """
    pins = dict ()
    def __new__ (cls, number, name, signal):
        n = _pin.__new__ (cls, number, name, signal)
        cls.pins[n.name] = n
        return n

PinMap (88, "P9-22", "UART2_RXD")
PinMap (90, "P9-21", "UART2_TXD")
PinMap (92, "P9-18", "I2C1_SDA")
PinMap (94, "P9-17", "I2C1_SCL")
PinMap (96, "P9-42", "GPIO0_7")
PinMap (98, "P8-35", "UART4_CTSN")
PinMap (100, "P8-33", "UART4_RTSN")
PinMap (102, "P8-31", "UART5_CTSN")
PinMap (104, "P8-32", "UART5_RTSN")
PinMap (106, "P9-19", "I2C2_SCL")
PinMap (108, "P9-20", "I2C2_SDA")
PinMap (110, "P9-26", "UART1_RXD")
PinMap (112, "P9-24", "UART1_TXD")
PinMap (114, "P9-41", "CLKOUT2")
PinMap (116, "P8-19", "EHRPWM2A")
PinMap (118, "P8-13", "EHRPWM2B")
PinMap (120, "P8-14", "GPIO0_26")
PinMap (122, "P8-17", "GPIO0_27")
PinMap (124, "P9-11", "UART4_RXD")
PinMap (126, "P9-13", "UART4_TXD")
PinMap (128, "P8-25", "GPIO1_0")
PinMap (130, "P8-24", "GPIO1_1")
PinMap (132, "P8-5", "GPIO1_2")
PinMap (134, "P8-6", "GPIO1_3")
PinMap (136, "P8-23", "GPIO1_4")
PinMap (138, "P8-22", "GPIO1_5")
PinMap (140, "P8-3", "GPIO1_6")
PinMap (142, "P8-4", "GPIO1_7")
PinMap (144, "P8-12", "GPIO1_12")
PinMap (146, "P8-11", "GPIO1_13")
PinMap (148, "P8-16", "GPIO1_14")
PinMap (150, "P8-15", "GPIO1_15")
PinMap (152, "P9-15", "GPIO1_16")
PinMap (154, "P9-23", "GPIO1_17")
PinMap (156, "P9-14", "EHRPWM1A")
PinMap (158, "P9-16", "EHRPWM1B")
PinMap (160, "P9-12", "GPIO1_28")
PinMap (162, "P8-26", "GPIO1_29")
PinMap (164, "P8-21", "GPIO1_30")
PinMap (166, "P8-20", "GPIO1_31")
PinMap (168, "P8-18", "GPIO2_1")
PinMap (170, "P8-7", "TIMER4")
PinMap (172, "P8-9", "TIMER5")
PinMap (174, "P8-10", "TIMER6")
PinMap (176, "P8-8", "TIMER7")
PinMap (178, "P8-45", "GPIO2_6")
PinMap (180, "P8-46", "GPIO2_7")
PinMap (182, "P8-43", "GPIO2_8")
PinMap (184, "P8-44", "GPIO2_9")
PinMap (186, "P8-41", "GPIO2_10")
PinMap (188, "P8-42", "GPIO2_11")
PinMap (190, "P8-39", "GPIO2_12")
PinMap (192, "P8-40", "GPIO2_13")
PinMap (194, "P8-37", "UART5_TXD")
PinMap (196, "P8-38", "UART5_RXD")
PinMap (198, "P8-36", "UART3_CTSN")
PinMap (200, "P8-34", "UART3_RTSN")
PinMap (202, "P8-27", "GPIO2_22")
PinMap (204, "P8-29", "GPIO2_23")
PinMap (206, "P8-28", "GPIO2_24")
PinMap (208, "P8-30", "GPIO2_25")
PinMap (210, "P9-29", "SPI1_D0")
PinMap (212, "P9-30", "SPI1_D1")
PinMap (214, "P9-28", "SPI1_CS0")
PinMap (216, "P9-27", "GPIO3_19")
PinMap (218, "P9-31", "SPI1_SCLK")
PinMap (220, "P9-25", "GPIO3_21")
PinMap (222, "P8-39", "AIN0")
PinMap (224, "P8-40", "AIN1")
PinMap (226, "P8-37", "AIN2")
PinMap (228, "P8-38", "AIN3")
PinMap (230, "P9-33", "AIN4")
PinMap (232, "P8-36", "AIN5")
PinMap (234, "P9-35", "AIN6")

_pinuse = namedtuple ("PinUse", "pin dir slew rx pull mux")

class PinUse (_pinuse):
    """A class to describe the usage of a BeagleBone expansion card pin.
    """
    UNUSED = 0   # Value indicating an unused pin
    OUT = 2
    IN = 1
    IO = 3
    SLOW = 1
    FAST = 0
    ENABLE = 1
    DISABLE = 0
    UP = 2
    DOWN = 0
    NO = 1
    
    def __new__ (cls, name, **kwds):
        """Allocate a new pin usage object.  The pin name (e.g., P8-21) is
        required.  Other arguments (in keyword form) are dir, slew, rx, pull,
        and mux; most of these may be omitted.  Argument values are:

        dir: IN, OUT, IO (required)
        slew: SLOW, FAST (default is FAST)
        rx: DISABLE, ENABLE (default is DISABLE)
        pull: UP, DOWN, NO (default is NO)
        mux: 0..7 (default is 0)
        """
        args = dict ()
        name = name.upper ()
        for f in cls._fields:
            args[f] = 0
        args["pull"] = cls.NO
        args["pin"] = PinMap.pins[name]
        args.update (kwds)
        return _pinuse.__new__ (cls, **args)

    def encode (self):
        """Return the encoding (2 byte unsigned integer) of the pin usage
        defined by this object.  Refer to the BeagleBone SRM, description
        of the pin usage field in the Expansion Board EEPROM, for details
        on this encoding.
        """
        return 0x8000 | (self.dir << 13) | (self.slew << 6) | \
               (self.rx << 5) | (self.pull << 3) | self.mux
    
class CardInfo (object):
    """Information about a BeagleBone cape card, from which the
    I2C card info flash memory data can be generated.
    """
    LAYOUT = struct.Struct (">4s2s32s4s16s16sh12s74Hhhhh")
    MAGIC = "\xaa\x55\x33\xee"
    FMTREV = "A1"
    
    def __init__ (self, name, **kwds):
        """Define the card info object.  Name (the board name) is
        required; other attributes are optional and must be keyword
        arguments.  Arguments are:

        name, version, mfg, partnum, assembly, i33, i5, isys5, isrc,
        avail.

        Most of these match the fields by (roughly) that name shown in the
        BeagleBone SRM description of the Expansion Board EEPROM.

        "assembly" is the assembly code component of the serial number.
        The rest of the serial number is generated automatically.

        The "i" fields represent the parameters for currents for the various
        supply voltages. i33 is VDD_3V3B_CURRENT.  i5 is VDD_5V_CURRENT.
        isys5 is SYS_V5_CURRENT.  isrc is the "DC Supplied" current.

        "avail" is a string which specifies data to be supplied in the
        "Available" field, i.e., the unspecified field starting after the
        fixed elements at offset 244.
        """
        # Set defaults
        self.version = self.mfg = self.partnum = ""
        self.assembly = "____"
        self.pins = 0
        self.i33 = self.i5 = self.isys5 = self.isrc = 0
        self.avail = ""
        self.__dict__.update (kwds)
        self.name = name
        # Initially no pins are in use
        self.pinuse = dict ()
        # Set the serial number
        datecode = time.strftime ("%W%y")
        serfn = "%s_%s.serno" % (datecode, self.partnum)
        try:
            f = open (serfn, "r")
            serno = int (f.readline ().strip ())
            f.close ()
        except IOError:
            serno = 0
        serno += 1
        f = open (serfn, "w")
        print >> f, serno
        f.close ()
        self.serno = "%s%4s%04d" % (datecode, self.assembly, serno)
        
    def addpin (self, name, **kwds):
        if name in self.pinuse:
            raise ValueError ("Duplicate pin assignment")
        self.pinuse[name] = PinUse (name, **kwds)
            
    def encode (self):
        if self.i33 > 1500 or self.i5 > 1500 or self.isys5 > 1500:
            raise ValueError ("Excessive current")
        pinuse = [ PinUse.UNUSED ] * 74
        self.pins = len (self.pinuse)
        # Add in power/ground pin use
        self.pins += 8    # ground
        if self.i33:
            self.pins += 2
        if self.i5 or self.isrc:
            self.pins += 2
        if self.isys5:
            self.pins += 2
            
        for u in self.pinuse.itervalues ():
            i = u.pin.number / 2 - 88
            pinuse[i] = u.encode ()
        pinuse.extend ([self.i33, self.i5, self.isys5, self.isrc])
        return self.LAYOUT.pack (self.MAGIC, self.FMTREV, self.name,
                                 self.version, self.mfg, self.partnum,
                                 self.pins, self.serno, *pinuse) + self.avail
    
