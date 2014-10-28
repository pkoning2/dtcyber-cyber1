#!/usr/bin/env python3

"""Tool for scripting interaction with DtCyber

This package provides a collection of classes for interacting with
DtCyber via any of the externally visible network ports: console,
operator interface, pterm (NIU form only).
"""

import os
import sys
import time
import socket
import threading
import struct
import re
import copy
import unicodedata

NETMAX = 4096
DEBUG = False

class MyThread (threading.Thread):
    """A thread with stop method.
    """
    def __init__ (self):
        threading.Thread.__init__ (self)
        self.stopnow = False

    def stop (self):
        """Stop the thread associated with this connection.  The actual
        handling of "stopnow" needs to go into the class that uses this.
        """
        if not self.stopnow and self.isAlive ():
            self.stopnow = True
            self.join ()

class Connection (socket.socket):
    """A network connection.  Derived from socket, with some more
    read methods.
    """
    def __init__ (self, host, port):
        """Create a connection to the given host/port.
        """
        socket.socket.__init__ (self, socket.AF_INET, socket.SOCK_STREAM)
        self.connect ((host, port))
        self.pendingdata = b""

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
        if DEBUG:
            for c in ret:
                print ("%02x %03o %3d" % (c, c, c))
        return ret

    def readmin (self, minbytes = 1):
        """Read at least the supplied number of bytes.  Default length
        is one, i.e., read any non-null network data.
        """
        while len (self.pendingdata) < bytes:
            self.readmore ()
        ret = self.pendingdata
        self.pendingdata = b""
        return ret
        
    def readtlv (self):
        """Read a TLV coded item from the network.  Returns T and V
        as a tuple.
        """
        tl = self.read (2)
        t = tl[0]
        l = tl[1]
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
            sleep (0.3)
        
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
            
statre = re.compile (r"DEBUG|UNLOCKED|ALERT|(\d+) operators")
class Oper (Connection, MyThread):
    """A connection to the DtCyber operator interface.  It includes
    a thread that collects data from DtCyber and updates local
    state to reflect what it hears.
    """
    def __init__ (self, port = 5006):
        """port is the operator interface port number, default
        is 5006.
        """
        MyThread.__init__ (self)
        Connection.__init__ (self, "localhost", port)
        self.settimeout (5)
        self.fixedtext = elist ()
        self.response = ""
        self.responses = 0
        self.status = elist ()
        self.locked = True
        self.debug = False
        self.plato_alert = False
        self.statusdict = { }
        self.operators = 1
        self.start ()

    def run (self):
        """Collect data from DtCyber.  We sort out the various
        types of data we receive and salt away most of it in
        a more convenient form.

        In particular, the status lines are captured several ways:
        1. self.status is an array, each entry is a status line.
        2. self.statusdict is a dictionary indexed by a tuple
           of two integers, the channel and equipment number of
           one of the devices.  The value is the status for that device.
        3. The Oper object string representation is the whole status
           text, as it would appear on the screen (one line per entry).

        In addition, the current lock and debug states are extracted,
        as well as the current operator connection count.
        """
        while True:
            try:
                if self.stopnow:
                    self.close ()
                    break
                t, v = self.readtlv ()
                if t == 2:
                    # OpText -- fixed text (left screen material)
                    x, y, size, bold = struct.unpack ("<HHBB", v[:6])
                    i = (0o760 - y) // 16
                    v = v[6:].decode ("ascii", "ignore")
                    try:
                        self.fixedtext[i] = v
                    except IndexError:
                        print ("bad index", i, "for", x, y, size, bold, v)
                elif t == 3 or t == 5:
                    # OpSyntax or OpInitialized, ignore
                    pass
                elif t == 1:
                    # OpReply
                    self.response = v.decode ("ascii", "ignore")
                    self.responses += 1
                elif t == 4:
                    # OpStatus
                    i = v[0]
                    v = v[1:]
                    v = v.decode ("ascii", "ignore")
                    self.status[i] = v
                    if i == 0:
                        # System status line, update some status
                        self.debug = self.plato_alert = False
                        self.locked = True
                        self.operators = 1
                        for m in statre.finditer (v):
                            if m.group (1):
                                self.operators = int (m.group (1))
                            else:
                                ms = m.group (0)
                                if ms == "DEBUG":
                                    self.debug = True
                                elif ms == "UNLOCKED":
                                    self.locked = False
                                elif ms == "ALERT":
                                    self.plato_alert = True
                    else:
                        fields = v.split (None, 3)
                        if len (fields) >= 3:
                            try:
                                skey = (int (fields[1], 8), int (fields[2], 8))
                                self.statusdict[skey] = fields[3]
                            except ValueError:
                                pass
                            except IndexError:
                                self.statusdict[skey] = ""
            except socket.timeout:
                pass
            except EOFError:
                break
        self.stopnow = True

    def command (self, text):
        """Send a command string.  Returns the response from DtCyber,
        if one was received within 3 seconds.
        """
        seq = self.responses
        self.sendall (struct.pack ("<BB", 0, len (text)) + text.encode ("ascii", "ignore"))
        for i in range (20):
            if self.responses != seq:
                return self.response
            time.sleep (0.3)

    def __repr__ (self):
        """The representation of the object is the current status
        area (right screen of the dtoper display, but in full rather
        than limited to what fits in 30 or so lines).
        """
        retval = '\n'.join (self.status)
        while retval.endswith ("\n\n"):
            retval = retval[:-1]
        return retval

""" Python Character Mapping Codec plato generated from './plato.txt' with gencodec.py.

"""#"

import codecs

### Codec APIs

class PlatoCodec (codecs.Codec):

    def encode (self, input, errors = 'strict'):
        return codecs.charmap_encode (input, errors, plato_encoding_table)

    def decode(self,input, errors = 'strict'):
        return codecs.charmap_decode (input, errors, plato_decoding_table)

class PlatoIncrementalEncoder (codecs.IncrementalEncoder):
    def encode (self, input, final = False):
        return codecs.charmap_encode (input, self.errors, plato_encoding_table)[0]

class PlatoIncrementalDecoder (codecs.IncrementalDecoder):
    def decode(self, input, final = False):
        return codecs.charmap_decode (input, self.errors, plato_decoding_table)[0]

class PlatoStreamWriter (PlatoCodec ,codecs.StreamWriter):
    pass

class PlatoStreamReader (PlatoCodec, codecs.StreamReader):
    pass

### encodings module API

def plato_getregentry (name):
    if name == "plato":
        return codecs.CodecInfo (
            name = 'plato',
            encode = PlatoCodec ().encode,
            decode = PlatoCodec ().decode,
            incrementalencoder = PlatoIncrementalEncoder,
            incrementaldecoder = PlatoIncrementalDecoder,
            streamreader = PlatoStreamReader,
            streamwriter = PlatoStreamWriter,
            )
    elif name == "platokb":
        return codecs.CodecInfo (
            name = 'platokb',
            encode = PlatoKbCodec ().encode,
            decode = PlatoCodec ().decode,
            incrementalencoder = PlatoKbIncrementalEncoder,
            incrementaldecoder = PlatoIncrementalDecoder,
            streamreader = PlatoStreamReader,
            streamwriter = PlatoKbStreamWriter,
            )

### Decoding Table

plato_decoding_table = (
    ':'         #  0x00 -> COLON
    'a'         #  0x01 -> LATIN SMALL LETTER A
    'b'         #  0x02 -> LATIN SMALL LETTER B
    'c'         #  0x03 -> LATIN SMALL LETTER C
    'd'         #  0x04 -> LATIN SMALL LETTER D
    'e'         #  0x05 -> LATIN SMALL LETTER E
    'f'         #  0x06 -> LATIN SMALL LETTER F
    'g'         #  0x07 -> LATIN SMALL LETTER G
    'h'         #  0x08 -> LATIN SMALL LETTER H
    'i'         #  0x09 -> LATIN SMALL LETTER I
    'j'         #  0x0A -> LATIN SMALL LETTER J
    'k'         #  0x0B -> LATIN SMALL LETTER K
    'l'         #  0x0C -> LATIN SMALL LETTER L
    'm'         #  0x0D -> LATIN SMALL LETTER M
    'n'         #  0x0E -> LATIN SMALL LETTER N
    'o'         #  0x0F -> LATIN SMALL LETTER O
    'q'         #  0x10 -> LATIN SMALL LETTER P
    'q'         #  0x11 -> LATIN SMALL LETTER Q
    'r'         #  0x12 -> LATIN SMALL LETTER R
    's'         #  0x13 -> LATIN SMALL LETTER S
    't'         #  0x14 -> LATIN SMALL LETTER T
    'u'         #  0x15 -> LATIN SMALL LETTER U
    'v'         #  0x16 -> LATIN SMALL LETTER V
    'w'         #  0x17 -> LATIN SMALL LETTER W
    'x'         #  0x18 -> LATIN SMALL LETTER X
    'y'         #  0x19 -> LATIN SMALL LETTER Y
    'z'         #  0x1A -> LATIN SMALL LETTER Z
    '0'         #  0x1B -> DIGIT ZERO
    '1'         #  0x1C -> DIGIT ONE
    '2'         #  0x1D -> DIGIT TWO
    '3'         #  0x1E -> DIGIT THREE
    '4'         #  0x1F -> DIGIT FOUR
    '5'         #  0x20 -> DIGIT FIVE
    '6'         #  0x21 -> DIGIT SIX
    '7'         #  0x22 -> DIGIT SEVEN
    '8'         #  0x23 -> DIGIT EIGHT
    '9'         #  0x24 -> DIGIT NINE
    '+'         #  0x25 -> PLUS SIGN
    '-'         #  0x26 -> HYPHEN-MINUS
    '*'         #  0x27 -> ASTERISK
    '/'         #  0x28 -> SOLIDUS
    '('         #  0x29 -> LEFT PARENTHESIS
    ')'         #  0x2A -> RIGHT PARENTHESIS
    '$'         #  0x2B -> DOLLAR SIGN
    '='         #  0x2C -> EQUALS SIGN
    ' '         #  0x2D -> SPACE
    ','         #  0x2E -> COMMA
    '.'         #  0x2F -> FULL STOP
    '\xf7'      #  0x30 -> DIVISION SIGN
    '['         #  0x31 -> LEFT SQUARE BRACKET
    ']'         #  0x32 -> RIGHT SQUARE BRACKET
    '%'         #  0x33 -> PERCENT SIGN
    '\xd7'      #  0x34 -> MULTIPLICATION SIGN
    '\xab'      #  0x35 -> LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    "'"         #  0x36 -> APOSTROPHE
    '"'         #  0x37 -> QUOTATION MARK
    '!'         #  0x38 -> EXCLAMATION MARK
    ';'         #  0x39 -> SEMICOLON
    '<'         #  0x3A -> LESS-THAN SIGN
    '>'         #  0x3B -> GREATER-THAN SIGN
    '_'         #  0x3C -> LOW LINE
    '?'         #  0x3D -> QUESTION MARK
    '\xbb'      #  0x3E -> RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    '\ufffe'    #  0x3F -> UNDEFINED
    '#'         #  0x40 -> NUMBER SIGN
    'A'         #  0x41 -> LATIN CAPITAL LETTER A
    'B'         #  0x42 -> LATIN CAPITAL LETTER B
    'C'         #  0x43 -> LATIN CAPITAL LETTER C
    'D'         #  0x44 -> LATIN CAPITAL LETTER D
    'E'         #  0x45 -> LATIN CAPITAL LETTER E
    'F'         #  0x46 -> LATIN CAPITAL LETTER F
    'G'         #  0x47 -> LATIN CAPITAL LETTER G
    'H'         #  0x48 -> LATIN CAPITAL LETTER H
    'I'         #  0x49 -> LATIN CAPITAL LETTER I
    'J'         #  0x4A -> LATIN CAPITAL LETTER J
    'K'         #  0x4B -> LATIN CAPITAL LETTER K
    'L'         #  0x4C -> LATIN CAPITAL LETTER L
    'M'         #  0x4D -> LATIN CAPITAL LETTER M
    'N'         #  0x4E -> LATIN CAPITAL LETTER N
    'O'         #  0x4F -> LATIN CAPITAL LETTER O
    'P'         #  0x50 -> LATIN CAPITAL LETTER P
    'Q'         #  0x51 -> LATIN CAPITAL LETTER Q
    'R'         #  0x52 -> LATIN CAPITAL LETTER R
    'S'         #  0x53 -> LATIN CAPITAL LETTER S
    'T'         #  0x54 -> LATIN CAPITAL LETTER T
    'U'         #  0x55 -> LATIN CAPITAL LETTER U
    'V'         #  0x56 -> LATIN CAPITAL LETTER V
    'W'         #  0x57 -> LATIN CAPITAL LETTER W
    'X'         #  0x58 -> LATIN CAPITAL LETTER X
    'Y'         #  0x59 -> LATIN CAPITAL LETTER Y
    'Z'         #  0x5A -> LATIN CAPITAL LETTER Z
    '~'         #  0x5B -> TILDE
    '\xa8'      #  0x5C -> DIAERESIS
    '^'         #  0x5D -> CIRCUMFLEX ACCENT
    '\xb4'      #  0x5E -> ACUTE ACCENT
    '`'         #  0x5F -> GRAVE ACCENT
    '\u2191'    #  0x60 -> UPWARDS ARROW
    '\u2192'    #  0x61 -> RIGHTWARDS ARROW
    '\u2193'    #  0x62 -> DOWNWARDS ARROW
    '\u2190'    #  0x63 -> LEFTWARDS ARROW
    '\u223c'    #  0x64 -> TILDE OPERATOR
    '\u03a3'    #  0x65 -> GREEK CAPITAL LETTER SIGMA
    '\u0394'    #  0x66 -> GREEK CAPITAL LETTER DELTA
    '\u222a'    #  0x67 -> UNION
    '\u2229'    #  0x68 -> INTERSECTION
    '{'         #  0x69 -> LEFT CURLY BRACKET
    '}'         #  0x6A -> RIGHT CURLY BRACKET
    '&'         #  0x6B -> AMPERSAND
    '\u2260'    #  0x6C -> NOT EQUAL TO
    ' '         #  0x6D -> SPACE
    '|'         #  0x6E -> VERTICAL LINE
    '\xb0'      #  0x6F -> DEGREE SIGN
    '\u2261'    #  0x70 -> IDENTICAL TO
    '\u03b1'    #  0x71 -> GREEK SMALL LETTER ALPHA
    '\u03b2'    #  0x72 -> GREEK SMALL LETTER BETA
    '\u03b4'    #  0x73 -> GREEK SMALL LETTER DELTA
    '\u03bb'    #  0x74 -> GREEK SMALL LETTER LAMBDA
    '\u03bc'    #  0x75 -> GREEK SMALL LETTER MU
    '\u03c0'    #  0x76 -> GREEK SMALL LETTER PI
    '\u03c1'    #  0x77 -> GREEK SMALL LETTER RHO
    '\u03c3'    #  0x78 -> GREEK SMALL LETTER SIGMA
    '\u03c9'    #  0x79 -> GREEK SMALL LETTER OMEGA
    '\u2264'    #  0x7A -> LESS-THAN OR EQUAL TO
    '\u2265'    #  0x7B -> GREATER-THAN OR EQUAL TO
    '\u0398'    #  0x7C -> GREEK CAPITAL LETTER THETA
    '@'         #  0x7D -> COMMERCIAL AT
    '\\'        #  0x7E -> REVERSE SOLIDUS
    '\n'        #  0x7F -> Use as a way to get newline
)
# Decoding table must be 256 chars long
plato_decoding_table += '\ufffe' * (256 - len (plato_decoding_table))

### Encoding table
plato_encoding_table=codecs.charmap_build(plato_decoding_table)

### Decoding Table

platokb_decoding_table = (
    '0'         #  0x00 -> DIGIT ZERO
    '1'         #  0x01 -> DIGIT ONE
    '2'         #  0x02 -> DIGIT TWO
    '3'         #  0x03 -> DIGIT THREE
    '4'         #  0x04 -> DIGIT FOUR
    '5'         #  0x05 -> DIGIT FIVE
    '6'         #  0x06 -> DIGIT SIX
    '7'         #  0x07 -> DIGIT SEVEN
    '8'         #  0x08 -> DIGIT EIGHT
    '9'         #  0x09 -> DIGIT NINE
    '\xd7'      #  0x0A -> MULTIPLICATION SIGN
    '\xf7'      #  0x0B -> DIVISION SIGN
    '\t'        #  0x0C -> HORIZONTAL TABULATION
    '\xab'      #  0x0D -> LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    '+'         #  0x0E -> PLUS SIGN
    '-'         #  0x0F -> HYPHEN-MINUS
    '\ufffe'    #  0x10 -> UNDEFINED
    '\ufffe'    #  0x11 -> UNDEFINED
    '\ufffe'    #  0x12 -> UNDEFINED
    '\x7f'      #  0x13 -> DELETE
    '\ufffe'    #  0x14 -> UNDEFINED
    '\ufffe'    #  0x15 -> UNDEFINED
    '\n'        #  0x16 -> LINE FEED
    '\ufffe'    #  0x17 -> UNDEFINED
    '\ufffe'    #  0x18 -> UNDEFINED
    '\ufffe'    #  0x19 -> UNDEFINED
    '\ufffe'    #  0x1A -> UNDEFINED
    '\ufffe'    #  0x1B -> UNDEFINED
    '\ufffe'    #  0x1C -> UNDEFINED
    '\ufffe'    #  0x1D -> UNDEFINED
    '\ufffe'    #  0x1E -> UNDEFINED
    '\ufffe'    #  0x1F -> UNDEFINED
    '<'         #  0x20 -> LESS-THAN SIGN
    '>'         #  0x21 -> GREATER-THAN SIGN
    '['         #  0x22 -> LEFT SQUARE BRACKET
    ']'         #  0x23 -> RIGHT SQUARE BRACKET
    '$'         #  0x24 -> DOLLAR SIGN
    '%'         #  0x25 -> PERCENT SIGN
    '_'         #  0x26 -> LOW LINE
    "'"         #  0x27 -> APOSTROPHE
    '*'         #  0x28 -> ASTERISK
    '('         #  0x29 -> LEFT PARENTHESIS
    '\u222a'    #  0x2A -> UNION
    '\u2229'    #  0x2B -> INTERSECTION
    '\ufffe'    #  0x2C -> UNDEFINED
    '\ufffe'    #  0x2D -> UNDEFINED
    '\u03a3'    #  0x2E -> GREEK CAPITAL LETTER SIGMA
    '\u0394'    #  0x2F -> GREEK CAPITAL LETTER DELTA
    '\ufffe'    #  0x30 -> UNDEFINED
    '\ufffe'    #  0x31 -> UNDEFINED
    '\ufffe'    #  0x32 -> UNDEFINED
    '\ufffe'    #  0x33 -> UNDEFINED
    '\ufffe'    #  0x34 -> UNDEFINED
    '\ufffe'    #  0x35 -> UNDEFINED
    '\ufffe'    #  0x36 -> UNDEFINED
    '\ufffe'    #  0x37 -> UNDEFINED
    '\ufffe'    #  0x38 -> UNDEFINED
    '\ufffe'    #  0x39 -> UNDEFINED
    '\ufffe'    #  0x3A -> UNDEFINED
    '\ufffe'    #  0x3B -> UNDEFINED
    '\ufffe'    #  0x3C -> UNDEFINED
    '\ufffe'    #  0x3D -> UNDEFINED
    '\ufffe'    #  0x3E -> UNDEFINED
    '\ufffe'    #  0x3F -> UNDEFINED
    ' '         #  0x40 -> SPACE
    'a'         #  0x41 -> LATIN SMALL LETTER A
    'b'         #  0x42 -> LATIN SMALL LETTER B
    'c'         #  0x43 -> LATIN SMALL LETTER C
    'd'         #  0x44 -> LATIN SMALL LETTER D
    'e'         #  0x45 -> LATIN SMALL LETTER E
    'f'         #  0x46 -> LATIN SMALL LETTER F
    'g'         #  0x47 -> LATIN SMALL LETTER G
    'h'         #  0x48 -> LATIN SMALL LETTER H
    'i'         #  0x49 -> LATIN SMALL LETTER I
    'j'         #  0x4A -> LATIN SMALL LETTER J
    'k'         #  0x4B -> LATIN SMALL LETTER K
    'l'         #  0x4C -> LATIN SMALL LETTER L
    'm'         #  0x4D -> LATIN SMALL LETTER M
    'n'         #  0x4E -> LATIN SMALL LETTER N
    'o'         #  0x4F -> LATIN SMALL LETTER O
    'p'         #  0x50 -> LATIN SMALL LETTER P
    'q'         #  0x51 -> LATIN SMALL LETTER Q
    'r'         #  0x52 -> LATIN SMALL LETTER R
    's'         #  0x53 -> LATIN SMALL LETTER S
    't'         #  0x54 -> LATIN SMALL LETTER T
    'u'         #  0x55 -> LATIN SMALL LETTER U
    'v'         #  0x56 -> LATIN SMALL LETTER V
    'w'         #  0x57 -> LATIN SMALL LETTER W
    'x'         #  0x58 -> LATIN SMALL LETTER X
    'y'         #  0x59 -> LATIN SMALL LETTER Y
    'z'         #  0x5A -> LATIN SMALL LETTER Z
    '='         #  0x5B -> EQUALS SIGN
    ';'         #  0x5C -> SEMICOLON
    '/'         #  0x5D -> SOLIDUS
    '.'         #  0x5E -> FULL STOP
    ','         #  0x5F -> COMMA
    '\x08'      #  0x60 -> BACKSPACE
    'A'         #  0x61 -> LATIN CAPITAL LETTER A
    'B'         #  0x62 -> LATIN CAPITAL LETTER B
    'C'         #  0x63 -> LATIN CAPITAL LETTER C
    'D'         #  0x64 -> LATIN CAPITAL LETTER D
    'E'         #  0x65 -> LATIN CAPITAL LETTER E
    'F'         #  0x66 -> LATIN CAPITAL LETTER F
    'G'         #  0x67 -> LATIN CAPITAL LETTER G
    'H'         #  0x68 -> LATIN CAPITAL LETTER H
    'I'         #  0x69 -> LATIN CAPITAL LETTER I
    'J'         #  0x6A -> LATIN CAPITAL LETTER J
    'K'         #  0x6B -> LATIN CAPITAL LETTER K
    'L'         #  0x6C -> LATIN CAPITAL LETTER L
    'M'         #  0x6D -> LATIN CAPITAL LETTER M
    'N'         #  0x6E -> LATIN CAPITAL LETTER N
    'O'         #  0x6F -> LATIN CAPITAL LETTER O
    'P'         #  0x70 -> LATIN CAPITAL LETTER P
    'Q'         #  0x71 -> LATIN CAPITAL LETTER Q
    'R'         #  0x72 -> LATIN CAPITAL LETTER R
    'S'         #  0x73 -> LATIN CAPITAL LETTER S
    'T'         #  0x74 -> LATIN CAPITAL LETTER T
    'U'         #  0x75 -> LATIN CAPITAL LETTER U
    'V'         #  0x76 -> LATIN CAPITAL LETTER V
    'W'         #  0x77 -> LATIN CAPITAL LETTER W
    'X'         #  0x78 -> LATIN CAPITAL LETTER X
    'Y'         #  0x79 -> LATIN CAPITAL LETTER Y
    'Z'         #  0x7A -> LATIN CAPITAL LETTER Z
    ')'         #  0x7B -> RIGHT PARENTHESIS
    ':'         #  0x7C -> COLON
    '?'         #  0x7D -> QUESTION MARK
    '!'         #  0x7E -> EXCLAMATION MARK
    '"'         #  0x7F -> QUOTATION MARK
    # Codes 80 and up represent ACCESS codes
    '\u226e'    #  0x80 -> NOT LESS-THAN
    '\u226f'    #  0x81 -> NOT GREATER-THAN
    '\ufffe'    #  0x82 -> UNDEFINED
    '\ufffe'    #  0x83 -> UNDEFINED
    '\ufffe'    #  0x84 -> UNDEFINED
    '@'         #  0x85 -> COMMERCIAL AT
    '\xbb'      #  0x86 -> RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    '\ufffe'    #  0x87 -> UNDEFINED
    '\ufffe'    #  0x88 -> UNDEFINED
    '\ufffe'    #  0x89 -> UNDEFINED
    '\xb0'      #  0x8A -> DEGREE SIGN
    '\ufffe'    #  0x8B -> UNDEFINED
    '\ufffe'    #  0x8C -> UNDEFINED
    '\ufffe'    #  0x8D -> UNDEFINED
    '&'         #  0x8E -> AMPERSAND
    '\ufffe'    #  0x8F -> UNDEFINED
    '\ufffe'    #  0x90 -> UNDEFINED
    '\ufffe'    #  0x91 -> UNDEFINED
    '\ufffe'    #  0x92 -> UNDEFINED
    '\ufffe'    #  0x93 -> UNDEFINED
    '\ufffe'    #  0x94 -> UNDEFINED
    '\ufffe'    #  0x95 -> UNDEFINED
    '\ufffe'    #  0x96 -> UNDEFINED
    '\ufffe'    #  0x97 -> UNDEFINED
    '\ufffe'    #  0x98 -> UNDEFINED
    '\ufffe'    #  0x99 -> UNDEFINED
    '\ufffe'    #  0x9A -> UNDEFINED
    '\ufffe'    #  0x9B -> UNDEFINED
    '\ufffe'    #  0x9C -> UNDEFINED
    '\ufffe'    #  0x9D -> UNDEFINED
    '\ufffe'    #  0x9E -> UNDEFINED
    '\ufffe'    #  0x9F -> UNDEFINED
    '\u2264'    #  0xA0 -> LESS-THAN OR EQUAL TO
    '\u2265'    #  0xA1 -> GREATER-THAN OR EQUAL TO
    '{'         #  0xA2 -> LEFT CURLY BRACKET
    '}'         #  0xA3 -> RIGHT CURLY BRACKET
    '#'         #  0xA4 -> NUMBER SIGN
    '\ufffe'    #  0xA5 -> UNDEFINED
    '\ufffe'    #  0xA6 -> UNDEFINED
    '\ufffe'    #  0xA7 -> UNDEFINED
    '\ufffe'    #  0xA8 -> UNDEFINED
    '\ufffe'    #  0xA9 -> UNDEFINED
    '\u2a2f'    #  0xAA -> VECTOR OR CROSS PRODUCT
    '\ufffe'    #  0xAB -> UNDEFINED
    '\ufffe'    #  0xAC -> UNDEFINED
    '\ufffe'    #  0xAD -> UNDEFINED
    '\ufffe'    #  0xAE -> UNDEFINED
    '\ufffe'    #  0xAF -> UNDEFINED
    '\ufffe'    #  0xB0 -> UNDEFINED
    '\ufffe'    #  0xB1 -> UNDEFINED
    '\ufffe'    #  0xB2 -> UNDEFINED
    '\ufffe'    #  0xB3 -> UNDEFINED
    '\ufffe'    #  0xB4 -> UNDEFINED
    '\ufffe'    #  0xB5 -> UNDEFINED
    '\ufffe'    #  0xB6 -> UNDEFINED
    '\ufffe'    #  0xB7 -> UNDEFINED
    '\ufffe'    #  0xB8 -> UNDEFINED
    '\ufffe'    #  0xB9 -> UNDEFINED
    '\ufffe'    #  0xBA -> UNDEFINED
    '\ufffe'    #  0xBB -> UNDEFINED
    '\ufffe'    #  0xBC -> UNDEFINED
    '\ufffe'    #  0xBD -> UNDEFINED
    '\ufffe'    #  0xBE -> UNDEFINED
    '\ufffe'    #  0xBF -> UNDEFINED
    '\ufffe'    #  0xC0 -> UNDEFINED
    '\u03b1'    #  0xC1 -> GREEK SMALL LETTER ALPHA
    '\u03b2'    #  0xC2 -> GREEK SMALL LETTER BETA
    '\u0327'    #  0xC3 -> COMBINING CEDILLA
    '\u03b4'    #  0xC4 -> GREEK SMALL LETTER DELTA
    '\u0301'    #  0xC5 -> COMBINING ACUTE ACCENT
    '\ufffe'    #  0xC6 -> UNDEFINED
    '\xe6'      #  0xC7 -> LATIN SMALL LETTER AE
    '\xf8'      #  0xC8 -> LATIN SMALL LETTER O WITH STROKE
    '\ufffe'    #  0xC9 -> UNDEFINED
    '\xe5'      #  0xCA -> LATIN SMALL LETTER A WITH RING ABOVE
    '\xe4'      #  0xCB -> LATIN SMALL LETTER A WITH DIAERESIS
    '\u03bb'    #  0xCC -> GREEK SMALL LETTER LAMBDA
    '\u03bc'    #  0xCD -> GREEK SMALL LETTER MU
    '\u0303'    #  0xCE -> COMBINING TILDE
    '\xb0'      #  0xCF -> DEGREE SIGN
    '\u03c0'    #  0xD0 -> GREEK SMALL LETTER PI
    '\u0300'    #  0xD1 -> COMBINING GRAVE ACCENT
    '\u03c1'    #  0xD2 -> GREEK SMALL LETTER RHO
    '\u03c3'    #  0xD3 -> GREEK SMALL LETTER SIGMA
    '\u0398'    #  0xD4 -> GREEK CAPITAL LETTER THETA
    '\u0308'    #  0xD5 -> COMBINING DIERESIS
    '\u030c'    #  0xD6 -> COMBINING CARON
    '\u03c9'    #  0xD7 -> GREEK SMALL LETTER OMEGA
    '\u0302'    #  0xD8 -> COMBINING CIRCUMFLEX ACCENT
    '\xf6'      #  0xD9 -> LATIN SMALL LETTER O WITH DIAERESIS
    '\ufffe'    #  0xDA -> UNDEFINED
    '\u2260'    #  0xDB -> NOT EQUAL TO
    '\u223c'    #  0xDC -> TILDE OPERATOR
    '\\'        #  0xDD -> REVERSE SOLIDUS
    '\ufffe'    #  0xDE -> UNDEFINED
    '\u2195'    #  0xDF -> UP DOWN ARROW
    '\ufffe'    #  0xE0 -> UNDEFINED
    '\u2190'    #  0xE1 -> LEFTWARDS ARROW
    '\ufffe'    #  0xE2 -> UNDEFINED
    '\xa9'      #  0xE3 -> COPYRIGHT SIGN
    '\u2192'    #  0xE4 -> RIGHTWARDS ARROW
    '\ufffe'    #  0xE5 -> UNDEFINED
    '\u2666'    #  0xE6 -> BLACK DIAMOND SUIT
    '\xc6'      #  0xE7 -> LATIN CAPITAL LETTER AE
    '\xd8'      #  0xE8 -> LATIN CAPITAL LETTER O WITH STROKE
    '|'         #  0xE9 -> VERTICAL LINE
    '\xc5'      #  0xEA -> LATIN CAPITAL LETTER A WITH RING ABOVE
    '\xc4'      #  0xEB -> LATIN CAPITAL LETTER A WITH DIAERESIS
    '\ufffe'    #  0xEC -> UNDEFINED
    '\ufffe'    #  0xED -> UNDEFINED
    '\ufffe'    #  0xEE -> UNDEFINED
    '\u25a1'    #  0xEF -> WHITE SQUARE
    '\ufffe'    #  0xF0 -> UNDEFINED
    '\ufffe'    #  0xF1 -> UNDEFINED
    '\ufffe'    #  0xF2 -> UNDEFINED
    '\ufffe'    #  0xF3 -> UNDEFINED
    '\ufffe'    #  0xF4 -> UNDEFINED
    '\ufffe'    #  0xF5 -> UNDEFINED
    '\ufffe'    #  0xF6 -> UNDEFINED
    '\u2191'    #  0xF7 -> UPWARDS ARROW
    '\u2193'    #  0xF8 -> DOWNWARDS ARROW
    '\xd6'      #  0xF9 -> LATIN CAPITAL LETTER O WITH DIAERESIS
    '\ufffe'    #  0xFA -> UNDEFINED
    '\u2261'    #  0xFB -> IDENTICAL TO
)

class PlatoKbCodec (codecs.Codec):

    def encode (self, input, errors = 'strict'):
        # Separate out the diacritical marks
        ninput = unicodedata.normalize ("NFD", input)
        result = [ ]
        for c in ninput:
            try:
                r = platokb_encoding_table[c]
                result.append (r)
            except KeyError:
                pass
        return b''.join (result), len (input)
    
class PlatoKbIncrementalEncoder (codecs.IncrementalEncoder):
    def encode (self, input, final = False):
        return PlatoKbCodec.encode (self, input, self.errors)[0]

class PlatoKbStreamWriter (PlatoKbCodec ,codecs.StreamWriter):
    pass

def keyframe (key):
    """Frame a key code for transmission to PLATO.
    """
    return struct.pack ("<BB", key >> 7, (0o200 | key & 0o177))

def unframe (key):
    """Reverse of keyframe ()
    """
    k1, k2 = struct.unpack ("<BB", key)
    if k2 & 0o200:
        return (k1 << 7) | (k2 & 0o177)
    raise ValueError

platokb_encoding_table = dict ()
for i, c in enumerate (platokb_decoding_table):
    keycodes = [ ]
    if i & 0x80:
        # Codes 80 and up represent MICRO codes (ACCESS codes)
        keycodes.append (keyframe (0o074))
        i &= 0x7f
    keycodes.append (keyframe (i))
    platokb_encoding_table[c] = b''.join (keycodes)
platokb_encoding_table['\r'] = platokb_encoding_table['\n']

codecs.register (plato_getregentry)

class Pterm (Connection, MyThread):
    """A connection to an NIU port, i.e., a "classic" type pterm.
    It includes a thread that collects data from DtCyber and updates
    local state to reflect what it hears.
    """
    # Keycodes
    NEXT  = 0o26
    ERASE = 0o23
    BACK  = 0o30
    STOP  = 0o32
    TAB   = 0o14
    ASSIGN = 0o15
    SUPER = 0o20
    SUB   = 0o21
    SQUARE = 0o34
    ANS   = 0o22
    COPY  = 0o33
    DATA  = 0o31
    EDIT  = 0o27
    MICRO = 0o24
    HELP  = 0o25
    LAB   = 0o35
    SHIFT = 0o40

    # A blank line
    line64 = 64 * b'\055'

    def __init__ (self, host = "localhost", port = 5004):
        """host is the host name to connect to, default is localhost.
        port is the pterm port number, default is 5004.
        Note that this must be an NIU ('classic' protocol) port;
        ASCII mode is not supported.
        """
        MyThread.__init__ (self)
        Connection.__init__ (self, host, port)
        self.settimeout (5)
        self.lines = [ ]
        self.lines = [ bytearray (64) for i in range (32) ]
        self.fserase ()
        self.station = None
        self.start ()
        
    def run (self):
        """Collect data from DtCyber.  Text mode output is copied
        into the lines list, which is an array of 32 lines corresponding
        to the 32 lines of the coarse grid layout.  The port number
        status message is also processed.  Echo commands are handled
        by sending back the matching echo key (so this terminal looks
        like a Plato IV terminal).  Non-text output is ignored.
        """
        while True:
            try:
                if self.stopnow:
                    self.close ()
                    break
                byte = self.read (1)[0]
                if byte & 0o200:
                    print ("output out of sync")
                    continue
                word = self.read (2)
                word = byte << 12 | \
                       ((word[0] & 0o77) << 6) | \
                       (word[1] & 0o77)
                if DEBUG:
                    print ("%07o" % word)
                if word & 0o1000000:
                    # Data word
                    if self.mode == 3:
                        # Character plotting mode
                        self.char (word >> 12)
                        self.char (word >> 6)
                        self.char (word)
                else:
                    # control word
                    cmd = word >> 15
                    if cmd == 0:
                        # NOP command
                        if (word & 0o77000) == 0o42000:
                            # Station number report code
                            self.station = word & 0o777
                    elif cmd == 1:
                        # Load Mode command
                        if word & 1:
                            self.fserase ()
                        mode = (word >> 1) & 0o37
                        self.wemode = mode & 3
                        self.mode = mode >> 2
                    elif cmd == 2:
                        # Load Coordinate command
                        coord = word & 0o777
                        if word & 0o1000:
                            self.y = coord
                        else:
                            self.x = coord
                    elif cmd == 3:
                        key = (word & 0o177) + 0o200
                        self.sendkey (key)
            except socket.timeout:
                pass
            except EOFError:
                break
        self.stopnow = True
        
    def fserase (self):
        self.mode = 3          # char
        self.wemode = 3        # rewrite
        self.mem = 0           # char memory number
        self.x = 0
        self.y = 496
        self.uncover = False
        self.arrow = False     # last char was not arrow (prompt)
        for i in range (32):
            self.lines[i][:] = self.line64
            
    def sendkey (self, key):
        key &= 0o1777
        #print "sending key", key
        key = keyframe (key)
        self.sendall (key)
        self.arrow = False

    def waitarrow (self, wait):
        """Wait the specified number of seconds for an arrow to appear.
        Returns True if there was an arrow, False if not.
        """
        delay = wait / 10.0
        if delay > 1:
            delay = 1
        while wait > 0:
            if self.arrow:
                return True
            time.sleep (delay)
            wait -= delay
        return False

    def sendstr (self, s, term = None, wait = None):
        """Send a text string, converting from ASCII as we go.
        There is no delay in here, so don't send too much.
        If term is specified, send that key after the string.
        If wait is specified, wait that long (in seconds) for
        an arrow to appear before sending the string.
        """
        if wait:
            self.waitarrow (wait)
        s = s.encode ("platokb", "ignore")
        self.sendall (s)
        if term:
            self.sendkey (term)

    def char (self, c):
        """Display a character at current x/y.
        """
        c &= 0o77
        if c == 0o77:
            # uncover
            self.uncover = not self.uncover
            return
        if self.uncover:
            # handle uncover codes
            self.uncover = False
            if c == 0o10:
                # backspace
                self.x = (self.x - 8) & 0o777
            elif c == 0o11:
                # "tab"
                self.x = (self.x + 8) & 0o777
            elif c == 0o12:
                # line feed
                self.y = (self.y - 16) & 0o777
            elif c == 0o13:
                # vertical tab
                self.y = (self.y + 16) & 0o777
            elif c == 0o14:
                # form feed
                self.x, self.y = 0, 496
            elif c == 0o15:
                # carriage return
                self.x = 0
                self.y = (self.y - 16) & 0o777
            elif c == 0o16:
                # super
                self.y = (self.y + 5) & 0o777
            elif c == 0o17:
                # sub
                self.y = (self.y - 5) & 0o777
            elif c >= 0o20 and c <= 0o27:
                self.mem = c - 0o20
        else:
            cx = self.x // 8
            # line 0 is the top line
            cy = 31 - (self.y // 16)
            if self.wemode & 1:
                # mode write or rewrite
                if self.mem > 1:
                    # Not rom char, clear arrow flag
                    self.arrow = False
                    c = 0o55
                else:
                    if c != 0o55:
                        c += self.mem * 0o100
                        # Set the arrow flag, unless this is a space.
                        self.arrow = (c == 0o76)
            else:
                # mode erase, write a space
                self.arrow = False
                c = 0o55
            # Note that there are two encodings for space (055 and 155,
            # since space is in both M0 and M1 ROM pages).  But the way
            # the flow above works, we only ever use 055.  That makes
            # it possible to do reliable matches against the PLATO-coded
            # data in the lines[] array.
            self.lines[cy][cx] = c
            self.x = (self.x + 8) & 0o777

    def __repr__ (self):
        """The representation of the object is the current contents
        of the screen, as a string of 32 lines each 64 characters long.
        """
        return b'\177'.join (self.lines).decode ("plato")

    def login (self, user, group, passwd = None, waitauthor = True):
        self.sendkey (self.NEXT)
        self.sendstr (user, self.NEXT, 10)
        self.sendstr (group, self.SHIFT + self.STOP, 10)
        if passwd:
            self.sendstr (passwd, self.NEXT, 10)
        # If requested, handle any extraneous pages before author mode, 
        # like a message page or a consultant signon page.
        if waitauthor:
            for i in range(4):
                if self.waitarrow (5):
                    break
                self.sendkey (self.NEXT)

    def logout (self):
        while "Press  NEXT  to begin" not in str (self):
            self.sendkey (self.SHIFT + self.STOP)
            self.waitarrow (1)
            
# Python Character Mapping Codec dd60 generated from './console.txt' with gencodec.py,
# then edited a fair amount.

import codecs

### Codec APIs

class Dd60Codec (codecs.Codec):

    def encode (self,input,errors='strict'):
        return codecs.charmap_encode (input,errors,dd60_encoding_table)

    def decode (self,input,errors='strict'):
        return codecs.charmap_decode (input,errors,dd60_decoding_table)

class Dd60IncrementalEncoder (codecs.IncrementalEncoder):
    def encode (self, input, final = False):
        return codecs.charmap_encode (input,self.errors,dd60_encoding_table)[0]

class Dd60IncrementalDecoder (codecs.IncrementalDecoder):
    def decode (self, input, final = False):
        return codecs.charmap_decode (input,self.errors,dd60_decoding_table)[0]

class Dd60StreamWriter (Dd60Codec, codecs.StreamWriter):
    pass

class Dd60StreamReader (Dd60Codec, codecs.StreamReader):
    pass

class Dd60KbCodec (Dd60Codec):

    def encode (self,input,errors='strict'):
        return codecs.charmap_encode (input,errors,dd60kb_encoding_table)

class Dd60KbIncrementalEncoder (Dd60IncrementalEncoder):
    def encode (self, input, final = False):
        return codecs.charmap_encode (input,self.errors,dd60_encoding_table)[0]

### encodings module API

def dd60_getregentry (name):
    if name == "dd60":
        return codecs.CodecInfo (
            name = 'dd60',
            encode = Dd60Codec ().encode,
            decode = Dd60Codec ().decode,
            incrementalencoder = Dd60IncrementalEncoder,
            incrementaldecoder = Dd60IncrementalDecoder,
            streamreader = Dd60StreamReader,
            streamwriter = Dd60StreamWriter,
        )
    elif name == "dd60kb":
        return codecs.CodecInfo (
            name = 'dd60kb',
            encode = Dd60KbCodec ().encode,
            decode = Dd60KbCodec ().decode,
            incrementalencoder = Dd60KbIncrementalEncoder,
            # These 3 are unchanged:
            incrementaldecoder = Dd60IncrementalDecoder,
            streamreader = Dd60StreamReader,
            streamwriter = Dd60StreamWriter,
        )


### Decoding Table

dd60_decoding_table = (
    ' '         #  0x00 -> SPACE
    'A'         #  0x01 -> LATIN CAPITAL LETTER A
    'B'         #  0x02 -> LATIN CAPITAL LETTER B
    'C'         #  0x03 -> LATIN CAPITAL LETTER C
    'D'         #  0x04 -> LATIN CAPITAL LETTER D
    'E'         #  0x05 -> LATIN CAPITAL LETTER E
    'F'         #  0x06 -> LATIN CAPITAL LETTER F
    'G'         #  0x07 -> LATIN CAPITAL LETTER G
    'H'         #  0x08 -> LATIN CAPITAL LETTER H
    'I'         #  0x09 -> LATIN CAPITAL LETTER I
    'J'         #  0x0A -> LATIN CAPITAL LETTER J
    'K'         #  0x0B -> LATIN CAPITAL LETTER K
    'L'         #  0x0C -> LATIN CAPITAL LETTER L
    'M'         #  0x0D -> LATIN CAPITAL LETTER M
    'N'         #  0x0E -> LATIN CAPITAL LETTER N
    'O'         #  0x0F -> LATIN CAPITAL LETTER O
    'P'         #  0x10 -> LATIN CAPITAL LETTER P
    'Q'         #  0x11 -> LATIN CAPITAL LETTER Q
    'R'         #  0x12 -> LATIN CAPITAL LETTER R
    'S'         #  0x13 -> LATIN CAPITAL LETTER S
    'T'         #  0x14 -> LATIN CAPITAL LETTER T
    'U'         #  0x15 -> LATIN CAPITAL LETTER U
    'V'         #  0x16 -> LATIN CAPITAL LETTER V
    'W'         #  0x17 -> LATIN CAPITAL LETTER W
    'X'         #  0x18 -> LATIN CAPITAL LETTER X
    'Y'         #  0x19 -> LATIN CAPITAL LETTER Y
    'Z'         #  0x1A -> LATIN CAPITAL LETTER Z
    '0'         #  0x1B -> DIGIT ZERO
    '1'         #  0x1C -> DIGIT ONE
    '2'         #  0x1D -> DIGIT TWO
    '3'         #  0x1E -> DIGIT THREE
    '4'         #  0x1F -> DIGIT FOUR
    '5'         #  0x20 -> DIGIT FIVE
    '6'         #  0x21 -> DIGIT SIX
    '7'         #  0x22 -> DIGIT SEVEN
    '8'         #  0x23 -> DIGIT EIGHT
    '9'         #  0x24 -> DIGIT NINE
    '+'         #  0x25 -> PLUS SIGN
    '-'         #  0x26 -> HYPHEN-MINUS
    '*'         #  0x27 -> ASTERISK
    '/'         #  0x28 -> SOLIDUS
    '('         #  0x29 -> LEFT PARENTHESIS
    ')'         #  0x2A -> RIGHT PARENTHESIS
    ' '         #  0x2B -> SPACE
    '='         #  0x2C -> EQUALS SIGN
    ' '         #  0x2D -> SPACE
    ','         #  0x2E -> COMMA
    '.'         #  0x2F -> FULL STOP
    '\n'        #  0x30 -> NEWLINE
)
# Decoding table must be 256 chars long
dd60_decoding_table += '\ufffe' * (256 - len (dd60_decoding_table))

### Encoding table
dd60_encoding_table = codecs.charmap_build (dd60_decoding_table)

# Some tweaks to the encoding to handle space, lowercase
dd60_encoding_table[ord (' ')] = 0o55
for c in range (ord ('A'), ord ('Z') + 1):
    dd60_encoding_table[c + 32] = dd60_encoding_table[c]

# A slightly different encoding table to encode keyboard data, which
# has a few different codes.
dd60kb_encoding_table = copy.copy (dd60_encoding_table)
dd60kb_encoding_table[0o010] = 0o61        # Backspace (erase)
dd60kb_encoding_table[0o177] = 0o61        # Backspace (erase)
dd60kb_encoding_table[ord (' ')] = 0o62    # Space
dd60kb_encoding_table[ord ('[')] = 0o53    # Left blank
dd60kb_encoding_table[ord (']')] = 0o55    # Right blank

codecs.register (dd60_getregentry)

class Dd60 (Connection, MyThread):
    """A connection to the DtCyber console (green tubes).  It includes
    a thread that collects data from DtCyber and updates local
    state to reflect what it hears.

    The last full screen worth of content is available via attribute
    'screen' which is the left and right screens as a tuple;  each
    screen is an array of 64 strings, each 64 characters long, containing
    the content of that screen.

    We use 64 lines (8 unit height) because that's the smallest line
    spacing found in use.  The actual line spacing varies -- 10 units
    in the A display, 12 in the B display, a mix of 8 and 9 in the E
    display, and so on.  This way, we may get some spurious blank lines
    in the screen image but won't miss anything.
    """
    # A blank line
    line64 = 64 * b'\055'

    # Mode codes
    LEFT = 0
    RIGHT = 4
    SMALL = 0
    MEDIUM = 1
    LARGE = 2
    DOT = 3

    # Command codes (output stream non-data codes).  Unlisted codes
    # are ignored.
    SETX = 0o200
    SETY = 0o210
    SETMODE = 0o240
    ENDBLOCK = 0o250
    
    def __init__ (self, port = 5007, interval = 3):
        """port is the console port number, default is 5007.
        interval is the update interval, default is 3 seconds.
        Note that the interval can be set quite short, but
        values shorter than a second or two are not recommended
        because of excessive overhead.
        """
        MyThread.__init__ (self)
        Connection.__init__ (self, "localhost", port)
        self.settimeout (5)
        self.interval = -1
        self.setinterval (interval)
        self.sendkey (0o70)                 # XON to start output flow
        self.mode = self.DOT + self.LEFT   # no display until mode set
        self.seq = 0
        self.x = 0
        self.y = 31
        self.lineheight = 8
        self.screen = [ bytearray (64) for i in range (64) ], \
                      [ bytearray (64) for i in range (64) ]
        self.erase (self.screen)
        self.pending = [ bytearray (64) for i in range (64) ], \
                       [ bytearray (64) for i in range (64) ]
        self.erase (self.pending)
        self.start ()

    def erase (self, screen):
        lines = 512 // self.lineheight
        left, right = screen
        for i in range (64):
            left[i][:] = self.line64
            right[i][:] = self.line64
    
    def __next__ (self):
        """Return next byte of Cyber output, as an integer.
        """
        return self.read (1)[0]
    
    def run (self):
        """Collect data from DtCyber, and store it away into the
        left or right screen arrays.
        """
        while True:
            try:
                if self.stopnow:
                    self.close ()
                    break
                ch = next(self)
                self.process_output (ch)
            except socket.timeout:
                pass
            except EOFError:
                break
        self.stopnow = True

    def sendkey (self, key):
        self.sendall (struct.pack ("B", key))
        
    def setinterval (self, interval):
        if self.interval != interval:
            self.interval = interval
            if interval > 63:
                interval = 63
            if interval < 1:
                self.sendkey (0o200 + int (interval * 50))
            else:
                self.sendkey (0o300 + int (interval))

    def sendstr (self, text):
        """Send a text string, converting from ASCII as we go.
        There is no delay in here, so don't send too much.
        """
        text = text.encode ("dd60kb", "ignore")
        self.sendall (text)

    def __repr__ (self):
        """The representation of the object is the current contents
        of the right screen.
        """
        return self.screentext (1)

    def screentext (self, n, nums = False):
        """Returns the current contents of the specified screen
        (0 or 1), as a string, 64 lines of 64 characters each.
        """
        if nums:
            ret = [ ]
            for i, l in enumerate (self.screen[n]):
                ret.append ("%d: %s" % (i, l.decode ("dd60", "ignore")))
            return '\n'.join (ret)
        return str (b'\060'.join (self.screen[n]).decode ("dd60", "ignore"))

    spaces = frozenset ((0, 0o55))
    def process_char (self, ch):
        """Process a data byte.  If not in dot mode, store it in the
        correct position of the current line.  Medium and large chars
        are handled by storing the char at the current X/Y and skipping
        one (medium) or three (large) character positions.
        """
        if self.mode & self.RIGHT:
            side = 1
        else:
            side = 0
        mode = self.mode & 3
        if mode != self.DOT:
            if ch not in self.spaces:
                try:
                    self.pending[side][self.y][self.x] = ch
                except IndexError:
                    pass
            x = self.x + 1
            if mode == self.MEDIUM:
                x += 1
            elif mode == self.LARGE:
                x += 3
            self.x = x & 0o77

    def process_output (self, ch):
        """Process Cyber output data.  Sort control codes (upper bit set)
        from data codes (upper bit clear). 
        """
        if ch & 0o200:
            # Control code
            action = ch & 0o270
            if action == self.SETX:
                self.x = (((ch << 8) + next(self)) & 0o770) // 8
            elif action == self.SETY:
                self.y = (512 // self.lineheight) - (((ch << 8) + next(self)) & 0o777) // self.lineheight
            elif action == self.SETMODE:
                self.mode = ch & 7
            elif action == self.ENDBLOCK:
                # Swap the two buffers
                self.screen, self.pending = self.pending, self.screen
                # then erase the current one
                self.erase (self.pending)
                self.seq += 1
        else:
            self.process_char (ch)

    def wait_update (self):
        """Wait until the screen has been updated twice.  This ensures
        that any keyboard input has been reflected in the currently
        visible screen text.
        """
        # Wait for any flurry of updates to settle
        time.sleep (self.interval)
        seq = self.seq
        while self.seq < seq + 2:
            time.sleep (self.interval)
        #print (self.screentext(0,1))
        #print (self.seq)

    o26 = "O26.".encode ("dd60")
    dis = "DIS ".encode ("dd60")
    cons = "NEXT    Z    X".encode ("dd60")
    
    def todsd (self):
        """Force us into DSD if we're in some other recognizable
        utilities.
        """
        while self.screen[0][2][:4] == self.o26:
            self.sendstr ("[xr.\n")
            self.wait_update ()
        while self.screen[0][2][:4] == self.dis:
            self.sendstr ("[drop.\n")
            self.wait_update ()
        while self.cons in self.screen[1][24]:
            self.sendstr ("[]x")
            self.wait_update ()
            
TERM   = Pterm.SHIFT + Pterm.ANS
conskeymap1 = (
    # Mapping of (unshifted) PLATO keyboard codes to CONSOLE input
    # This part is for codes 000 to 035
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "[m",    # Multiply
    "[d",    # Divide
    "[[",    # Tab
    "[a",    # Assign
    "+",
    "-",
    "[7",    # Super
    "[8",    # Sub
    "][9",   # Ans ("Shift-Term")
    "\010",  # Erase
    "[+",    # Micro
    "[h",    # Help
    "\n",    # Next
    "[*",    # Edit
    "[b",    # Back
    "[(",    # Data
    "[)",    # Stop
    "[=",    # Copy
    "[-",    # Box
    "[/"     # Lab
    )
conskeymap2 = (
    # Mapping of (unshifted) PLATO keyboard codes to CONSOLE input
    # This part is for codes 100 to 137
    " ",
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j",
    "k",
    "l",
    "m",
    "n",
    "o",
    "p",
    "q",
    "r",
    "s",
    "t",
    "u",
    "v",
    "w",
    "x",
    "y",
    "z",
    "=",
    "[,",    # semicolon
    "/",
    ".",
    ","
    )

# Key code for shifted characters in CONSOLE.
shiftcode = "]".encode ("dd60kb")

# conskeymap is a dictionary that maps from PLATO key codes to the
# corresponding CONSOLE key sequence
conskeymap = dict ()
for p, k in enumerate (conskeymap1):
    k = k.encode ("dd60kb")
    conskeymap[p] = k
    conskeymap[p ^ 0o040] = shiftcode + k
for p, k in enumerate (conskeymap2):
    k = k.encode ("dd60kb")
    conskeymap[p + 0o100] = k
    conskeymap[p + 0o140] = shiftcode + k
conskeymap[TERM] = "[9".encode ("dd60kb")

# Now build an encoding dictionary for encoding strings into
# CONSOLE key sequences.  This is done by starting from the
# platokb_encoding_table and replacing each value (the PLATO key
# code or sequence of codes) by the corresponding CONSOLE sequence.
consolekb_encoding_table = dict ()
for k, v in platokb_encoding_table.items ():
    v2 = [ ]
    for i in range (0, len (v), 2):
        # The value (encoded string) in the platokb encoding table
        # is in framed form, ready for transmission.  We need to
        # convert each framed key code back to the actual code that
        # represents.
        pk = unframe (v[i:i + 2])
        v2.append (conskeymap[pk])
    consolekb_encoding_table[k] = b''.join (v2)

class ConsoleKbCodec (codecs.Codec):

    def encode (self, input, errors = 'strict'):
        # Separate out the diacritical marks
        ninput = unicodedata.normalize ("NFD", input)
        result = [ ]
        for c in ninput:
            try:
                r = consolekb_encoding_table[c]
                result.append (r)
            except KeyError:
                pass
        return b''.join (result), len (input)
    
class ConsoleKbIncrementalEncoder (codecs.IncrementalEncoder):
    def encode (self, input, final = False):
        return ConsoleKbCodec.encode (self, input, self.errors)[0]

class ConsoleKbStreamWriter (ConsoleKbCodec ,codecs.StreamWriter):
    pass

def console_getregentry (name):
    if name == "consolekb":
        return codecs.CodecInfo (
            name = 'consolekb',
            encode = ConsoleKbCodec ().encode,
            decode = Dd60Codec ().decode,
            incrementalencoder = ConsoleKbIncrementalEncoder,
            incrementaldecoder = Dd60IncrementalDecoder,
            streamreader = Dd60StreamReader,
            streamwriter = ConsoleKbStreamWriter,
            )

codecs.register (console_getregentry)
    
class Console (Dd60, Pterm):
    """Subclass of Dd60 to add PLATO terminal functionality via
    the "console" program.
    """

    def sendstr (self, s):
        if self.plato:
            s = s.encode ("consolekb", "ignore")
            self.sendall (s)
        else:
            Dd60.sendstr (self, s)
        
    def __init__ (self, port = 5007, interval = 3):
        self.plato = False
        Dd60.__init__ (self, port, interval)
        
    def sendkey (self,key):
        """Convert PLATO key code to CONSOLE key sequence.
        """
        if not self.plato:
            Dd60.sendkey (self, key)
            return
        try:
            self.sendall (conskeymap[key])
        except KeyError:
            pass

    author = "AUTHOR MODE".encode ("dd60")
    lesdes = "LESSON DESIRED".encode ("dd60")
    
    def login (self, *args):
        """Override the pterm login.  We ignore arguments,
        just go to the author mode page.
        """
        self.plato = False
        self.wait_update ()
        
        if Dd60.cons not in self.screen[1][24]:
            # We're not already in console.  First get us
            # to DSD.
            self.todsd ()
            self.sendstr ("[xconsole.\n")
            self.wait_update ()
        if Dd60.cons not in self.screen[1][24]:
            print ("Failed to start Console")
            return
        
        # The console (station 0-0) starts in whatever
        # state things were left in.  So we need to get it
        # to "author mode" which may require a trip via
        # "press next to begin" and/or "lesson desired"
        while True:
            left = self.screen[0]
            #print ("author:", self.author in left[12] or self.author in left[13])
            #print ("lesson desired:", self.lesdes in left[14])
            if self.author in left[12] or \
                   self.author in left[13]:
                break
            elif self.lesdes in left[14]:
                # back
                self.sendstr ("[b")
            else:
                # shift-stop
                self.sendstr ("[])")
            self.wait_update ()
        self.plato = True
                
    def logout (self):
        self.plato = False
        self.wait_update ()
        while True:
            left = self.screen[0]
            if self.lesdes in left[14]:
                break
            else:
                # shift-stop
                self.sendstr ("[])")
                self.wait_update ()
        # exit console utility
        self.sendstr ("[]x")
