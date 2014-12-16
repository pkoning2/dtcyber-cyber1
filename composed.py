#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unicodedata
import sys

rom = ":abcdefg" \
      "hijklmno" \
      "pqrstuvw" \
      "xyz01234" \
      "56789+-*" \
      "/()$= ,." \
      "÷[]%×⇦'\"" \
      "!;<>_?⪢ " \
      "#ABCDEFG" \
      "HIJKLMNO" \
      "PQRSTUVW" \
      "XYZ˜¨^´`" \
      "↑→↓←~ΣΔ∪" \
      "∩{}&≠ |°" \
      "≣αβδλμπρ" \
      "σω≤≥Θ@\\ " \
      "⦓⦔©▫◆✕ˇ↕" \
      "○¸"

acc = "`´^˜¨ˇ¸"
cacc = "\u0300\u0301\u0302\u0303\u0308\u030c\u0327"

nonacc = ''.join ([ c for c in rom if c not in acc ])

# ASCII (0..127 codes) to PLATO key code mapping.  If a single value
# appears, that is the key code.  If a tuple appears, that sequence of
# key codes is used.  "None" means this ASCII code doesn't correspond to
# a PLATO keycode.
asc2plato = [
	None,	0o022,	0o030,	0o033,
	0o031,	0o027,	0o064,	0o013,
	0o025,	None,	None,	None,
	0o035,	0o024,	0o026,	None,
	0o020,	0o034,	0o023,	0o032,
	0o062,	None,	None,	None,
	0o012,	0o021,	None,	None,
	None,	None,	None,	None,
	None,	0o176,	0o177,	(0o74, 0o044),
	0o044,	0o045,	(0o74, 0o016),	0o047,
	0o051,	0o173,	0o050,	0o016,
        0o137,	0o017,	0o136,	0o135,
	0o000,	0o001,	0o002,	0o003,
	0o004,	0o005,	0o006,	0o007,
	0o010,	0o011,	0o174,	0o134,
	0o040,	0o133,	0o041,	0o175,
	(0o74, 0o005),	0o141,	0o142,	0o143,
	0o144,	0o145,	0o146,	0o147,
	0o150,	0o151,	0o152,	0o153,
	0o154,	0o155,	0o156,	0o157,
	0o160,	0o161,	0o162,	0o163,
	0o164,	0o165,	0o166,	0o167,
	0o170,	0o171,	0o172,	0o042,
	(0o74, 0o135),	0o043,	(0o100, 0o74, 0o130),	0o046,
	(0o100, 0o74, 0o121),	0o101,	0o102,	0o103,
	0o104,	0o105,	0o106,	0o107,
	0o110,	0o111,	0o112,	0o113,
	0o114,	0o115,	0o116,	0o117,
	0o120,	0o121,	0o122,	0o123,
	0o124,	0o125,	0o126,	0o127,
	0o130,	0o131,	0o132,	(0o74, 0o042),
	(0o74, 0o151),	(0o74, 0o043),	(0o100, 0o74, 0o116),	None,
]

pastedasc2plato = list (asc2plato)
for i in range (32):
    pastedasc2plato[i] = None
pastedasc2plato[ord ("\t")] = 0o014   # TAB
pastedasc2plato[ord ("\n")] = 0o026   # NEXT
pastedasc2plato[ord (" ")] = 0o100    # Space

# Dictionary to map non-ASCII Unicode values to PLATO key codes.  The
# representation of the PLATO key codes it the same as above.
u2plato = {
    "¨" : (0o100, 0o74, 0o125),
    "©" : (0o74, 0o143),
    "´" : (0o100, 0o74, 0o105),
    "¸" : (0o100, 0o74, 0o103),
    "¹" : (0o74, 0o117),
    "Å" : (0o74, 0o152),
    "Æ" : (0o74, 0o147),
    "×" : 0o012,
    "Ø" : (0o74, 0o150),
    "å" : (0o74, 0o112),
    "æ" : (0o74, 0o107),
    "÷" : 0o013,
    "ø" : (0o74, 0o110),
    "ˇ" : (0o100, 0o74, 0o126),
    "˜" : (0o100, 0o74, 0o116),
    "\u0300" : (0o74, 0o121),    # combining grave
    "\u0301" : (0o74, 0o105),    # combining acute
    "\u0302" : (0o74, 0o130),    # combining circumflex (caret)
    "\u0303" : (0o74, 0o116),    # combining tilde
    "\u0308" : (0o74, 0o125),    # combining dieresis (unlaut)
    "\u030c" : (0o74, 0o126),    # combining hacek (caron)
    "\u0327" : (0o74, 0o103),    # combining cedilla
    "Δ" : 0o057,
    "Θ" : (0o74, 0o124),
    "Σ" : 0o056,
    "α" : (0o74, 0o101),
    "β" : (0o74, 0o102),
    "δ" : (0o74, 0o104),
    "λ" : (0o74, 0o114),
    "μ" : (0o74, 0o115),
    "π" : (0o74, 0o120),
    "ρ" : (0o74, 0o122),
    "σ" : (0o74, 0o123),
    "ω" : (0o74, 0o127),
    "←" : (0o74, 0o141),
    "↑" : (0o74, 0o167),
    "→" : (0o74, 0o144),
    "↓" : (0o74, 0o170),
    "↕" : (0o74, 0o137),
    "⇦" : 0o015,
    "∩" : 0o053,
    "∪" : 0o052,
    "≠" : (0o74, 0o133),
    "≣" : (0o74, 0o173),
    "≤" : (0o74, 0o040),
    "≥" : (0o74, 0o041),
    "▫" : (0o74, 0o157),
    "◆" : (0o74, 0o146),
    "○" : (0o74, 0o012),
    "✕" : (0o74, 0o052),
    "⦓" : (0o74, 0o000),
    "⦔" : (0o74, 0o001),
    "⪢" : (0o74, 0o006),
    }

# Mapping from separate representation to combined.  This is only for things
# that aren't made up from combining diacriticals -- separated to combined
# accented letter mappings are added in "addcombined".
sep2comb = {
    "__c" : "©",
    "A°" : "Å",
    "AE" : "Æ",
    "O/" : "Ø",
    "a°" : "å",
    "ae" : "æ",
    "o/" : "ø",
    "↑↓" : "↕",
    "∩∪" : "▫",
    "↑←↓" : "◆",
    "/\\" : "✕",
    "<(" : "⦓",
    ">)" : "⦔",
    }
def addcombined ():
    """For any accented characters that have a precomposed form
    (according to the Unicode normalization machinery), add an entry
    for the precomposed code to two tables: the unicode to PLATO keycode
    table, and the autobackspace mapping table.
    """
    for c in cacc:
        for n in nonacc:
            dc = n + c
            pc = unicodedata.normalize ("NFC", dc)
            if len (pc) == 1:
                o = ord (n)
                if o < len (asc2plato):
                    k = asc2plato[o]
                else:
                    k = u2plato[n]
                if not isinstance (k, tuple):
                    k = (k, )
                k2 = u2plato[c]
                k += k2
                u2plato[pc] = k
                sep2comb[dc] = pc
                
def g (t, i):
    try:
        return "%04o" % t[i]
    except IndexError:
        return "None"
    
def emitkey (k):
    if isinstance (k, tuple):
        kc = ", ".join ([g (k, i) for i in range (4)])
        return "KEY (%s)" % kc
    if k is None:
        return "None"
    return "KEY (%04o, None, None, None)" % k

def escape (s):
    return s.encode ("unicode_escape").decode ()

def emitstr (s):
    return 'L"%s"' % escape (s)

def emitchr (s):
    return "L'%s'" % escape (s)
    
def main ():
    addcombined ()
    sys.stdout = open ("ptermkeytabs.h", "wt")

    print ("// Generated by composed.py\n")

    print ("const u32 asciiToPlato[128] = {")
    for i in range (0, len (asc2plato), 2):
        l = ", ".join ([emitkey (asc2plato[i + j]) for j in range (2)])
        if i != len (asc2plato) - 2:
            l += ","
        print ("  ", l)
    print ("};\n")

    print ("const u32 pastedAsciiToPlato[128] = {")
    for i in range (0, len (pastedasc2plato), 2):
        l = ", ".join ([emitkey (pastedasc2plato[i + j]) for j in range (2)])
        if i != len (pastedasc2plato) - 2:
            l += ","
        print ("  ", l)
    print ("};\n")

    print ("const ukey unicodeToPlato[] = {")
    for k in sorted (u2plato):
        print ("  { %s, %s }," % (emitchr (k), emitkey (u2plato[k])))
    print ("};\n")

    print ("const autobsentry autobsmap[] = {")
    for s, c in sorted (zip (acc, cacc)):
        print ("    { %s, %s }," % (emitstr (" " + c), emitchr (s)))
    for s, c in sorted (sep2comb.items ()):
        print ("    { %s, %s }," % (emitstr (s), emitchr (c)))
    print ("};\n")

    print ("const wxString accent (%s);" % emitstr (acc))
    print ("const wxString combining_accent (%s);" % emitstr (cacc))

    sys.stdout = sys.__stdout__

if __name__ == "__main__":
    main ()
    
