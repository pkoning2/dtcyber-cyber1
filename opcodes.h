/*******************************************************************************

8080A simulator
Copyright (C) 2002  Anoakie Ray Turner

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
USA.

        Contact:  Anoakie Turner
                  Anoakie.Turner@asu.edu

                  13240 N. 94th Pl.
                  Scottsdale, AZ 85260

Note:
----
  This file assumes that the user can access the source file for details about
the instructions listed below.  If this assumption becomes untrue, then this
file should contain a copy of the appropriate documentation for the
instructions.


Contents:
--------
  1.	This file contains all of the opcode values, cycles per opcode, and
  	opcode mnemonics.

*******************************************************************************/


#ifndef __OPCODES_H__
#define __OPCODES_H__


/*******************************************************************************
Opcodes:
-------

   Opcodes are listed below.  For more detailed information on the
instructions, please read 8080a.c

Guide:
-----
   Captial Letters - Instruction name
   	Example - RRC - Rotate Right Circular
	Note - Capital letters at the end of the instruction usually signify
		registers, such as DCXB decrements the BC register.
   Lowercase Letters - Addressing
   	a - Direct addressing - 16 bit address.
	p - Direct addressing - 8 bit I/O port.
	z - Indirect addressing - 8 bit vector. (0x00h, 0x08h, ..., 0x38h)
	n - Immediate addressing - 8 bit data.
	nn - Immediate addressing - 16 bit data.
	r - Register addressing - 8080 Register. (A, B, C, D, E, H, L, M)
   Register Definitions
	M - Indirect addressing - 8 bit address. (ROM[HL])
	A - Accumulator - 8 bit register.
   Other
	sign - Minus is set in the PSW

*******************************************************************************/



/*******************************************************************************
ACIn:	Immediate addressing
----	Add immediate and carry to accumulator
*******************************************************************************/
#define ACIn	0xCE


/*******************************************************************************
ADCr:	Register addressing
----	Add register and carry to accumulator
*******************************************************************************/
/*	#define ADCr	0x8F	*/
#define ADCA	0x8F
#define ADCB	0x88
#define ADCC	0x89
#define ADCD	0x8A
#define ADCE	0x8B
#define ADCH	0x8C
#define ADCL	0x8D
#define ADCM	0x8E


/*******************************************************************************
ADDr:	Register addressing
----	Add register to accumulator
*******************************************************************************/
/*	#define ADDr	0x87	*/
#define ADDA	0x87
#define ADDB	0x80
#define ADDC	0x81
#define ADDD	0x82
#define ADDE	0x83
#define ADDH	0x84
#define ADDL	0x85
#define ADDM	0x86


/*******************************************************************************
ADIn:	Immediate addressing
----	Add immediate to accumulator
*******************************************************************************/
#define ADIn	0xC6


/*******************************************************************************
ANAr:	Register addressing
----	AND register to accumulator
*******************************************************************************/
/*	#define ANAr	0xA7	*/
#define ANAA	0xA7
#define ANAB	0xA0
#define ANAC	0xA1
#define ANAD	0xA2
#define ANAE	0xA3
#define ANAH	0xA4
#define ANAL	0xA5
#define ANAM	0xA6


/*******************************************************************************
ANIn:	Immediate addressing
----	AND immediate to accumulator
*******************************************************************************/
#define ANIn	0xE6


/*******************************************************************************
CALLa:	Direct addressing
-----	Call unconditional
*******************************************************************************/
#define CALLa	0xCD


/*******************************************************************************
CCa:	Direct addressing
---	Call on carry
*******************************************************************************/
#define CCa	0xDC


/*******************************************************************************
CCa:	Direct addressing
---	Call on sign
*******************************************************************************/
#define CMa	0xFC


/*******************************************************************************
CMA:	Register addressing
---	Compliment accumulator
*******************************************************************************/
#define CMA	0x2F


/*******************************************************************************
CMC:	Register addressing
---	Compliment carry
*******************************************************************************/
#define CMC	0x3F


/*******************************************************************************
CMPr:	Register addressing
----	Compare with accumulator
*******************************************************************************/
/*	#define CMPr	0xBF	*/
#define CMPA	0xBF
#define CMPB	0xB8
#define CMPC	0xB9
#define CMPD	0xBA
#define CMPE	0xBB
#define CMPH	0xBC
#define CMPL	0xBD
#define CMPM	0xBE


/*******************************************************************************
CNCa:	Direct addressing
----	Call on no carry
*******************************************************************************/
#define CNCa	0xD4


/*******************************************************************************
CNZa:	Direct addressing
----	Call on no zero
*******************************************************************************/
#define CNZa	0xC4


/*******************************************************************************
CPa:	Direct addressing
---	Call on no sign
*******************************************************************************/
#define CPa	0xF4


/*******************************************************************************
CPEa:	Direct addressing
----	Call on even parity
*******************************************************************************/
#define CPEa	0xEC


/*******************************************************************************
CPIn:	Immediate addressing
----	Compare accululator with immediate
*******************************************************************************/
#define CPIn	0xFE


/*******************************************************************************
CPOa:	Direct addressing
----	Call on odd parity
*******************************************************************************/
#define CPOa	0xE4


/*******************************************************************************
CZa:	Direct addressing
---	Call on zero
*******************************************************************************/
#define CZa	0xCC


/*******************************************************************************
DAA:	Register addressing
---	Decimal adjust accumulator
*******************************************************************************/
#define DAA	0x27


/*******************************************************************************
DADB:	Register addressing
----	Add BC to HL
*******************************************************************************/
#define DADB	0x09


/*******************************************************************************
DADD:	Register addressing
----	Add DE to HL
*******************************************************************************/
#define DADD	0x19


/*******************************************************************************
DADH:	Register addressing
----	Add HL to HL
*******************************************************************************/
#define DADH	0x29


/*******************************************************************************
DADSP:	Register addressing
----	Add stack pointer to HL
*******************************************************************************/
#define DADSP	0x39


/*******************************************************************************
DCRr:	Register addressing
----	Decrement register
*******************************************************************************/
/*	#define DCRr	0x3D	*/
#define DCRA	0x3D
#define DCRB	0x05
#define DCRC	0x0D
#define DCRD	0x15
#define DCRE	0x1D
#define DCRH	0x25
#define DCRL	0x2D
#define DCRM	0x35


/*******************************************************************************
DCXB:	Register addressing
----	Decrement BC
*******************************************************************************/
#define DCXB	0x0B


/*******************************************************************************
DCXD:	Register addressing
----	Decrement DE
*******************************************************************************/
#define DCXD	0x1B


/*******************************************************************************
DCXH:	Register addressing
----	Decrement HL
*******************************************************************************/
#define DCXH	0x2B


/*******************************************************************************
DCRSP:	Register addressing
----	Decrement stack pointer
*******************************************************************************/
#define DCXSP	0x3B


/*******************************************************************************
DI:	
--	Disable interrupts
*******************************************************************************/
#define DI	0xF3


/*******************************************************************************
EI:	
--	Enable interrupts
*******************************************************************************/
#define EI	0xFB


/*******************************************************************************
HLT:	
---	Halt
*******************************************************************************/
#define HLT	0x76


/*******************************************************************************
INp:	Direct addressing
---	Input to accumulator
*******************************************************************************/
#define INp	0xDB


/*******************************************************************************
INRr:	Register addressing
----	Increment register
*******************************************************************************/
/*	#define INRr	0x3C	*/
#define INRA	0x3C
#define INRB	0x04
#define INRC	0x0C
#define INRD	0x14
#define INRE	0x1C
#define INRH	0x24
#define INRL	0x2C
#define INRM	0x34

/*******************************************************************************
INXB:	Register addressing
----	Increment BC
*******************************************************************************/
#define INXB	0x03


/*******************************************************************************
INXD:	Register addressing
----	Increment DE
*******************************************************************************/
#define INXD	0x13


/*******************************************************************************
INXH:	Register addressing
----	Increment HL
*******************************************************************************/
#define INXH	0x23


/*******************************************************************************
INXSP:	Register addressing
----	Increment stack pointer
*******************************************************************************/
#define INXSP	0x33


/*******************************************************************************
JMPa:	Direct addressing
----	Jump unconditional
*******************************************************************************/
#define JMPa	0xC3


/*******************************************************************************
JCa:	Direct addressing
---	Jump on carry
*******************************************************************************/
#define JCa	0xDA


/*******************************************************************************
JMa:	Direct addressing
---	Jump on sign
*******************************************************************************/
#define JMa	0xFA


/*******************************************************************************
JNCa:	Direct addressing
----	Jump on no carry
*******************************************************************************/
#define JNCa	0xD2


/*******************************************************************************
JNZa:	Direct addressing
----	Jump on no zero
*******************************************************************************/
#define JNZa	0xC2


/*******************************************************************************
JPa:	Direct addressing
---	Jump on no sign
*******************************************************************************/
#define JPa	0xF2


/*******************************************************************************
JPEa:	Direct addressing
----	Jump on even parity
*******************************************************************************/
#define JPEa	0xEA


/*******************************************************************************
JPOa:	Direct addressing
----	Jump on odd parity
*******************************************************************************/
#define JPOa	0xE2


/*******************************************************************************
JZa:	Direct addressing
---	Jump on zero
*******************************************************************************/
#define JZa	0xCA


/*******************************************************************************
LDAa:	Direct addressing
----	Load accumulator
*******************************************************************************/
#define LDAa	0x3A


/*******************************************************************************
LDAXB:	Register addressing
-----	Load accumulator from ROM[BC]
*******************************************************************************/
#define LDAXB	0x0A


/*******************************************************************************
LDAXD:	Register addressing
-----	Load accumulator from ROM[DE]
*******************************************************************************/
#define LDAXD	0x1A


/*******************************************************************************
LHLD:	Register addressing
----	Load HL
*******************************************************************************/
#define LHLDa	0x2A


/*******************************************************************************
LXIBnn:	Immediate addressing
------	Load BC
*******************************************************************************/
#define LXIBnn	0x01


/*******************************************************************************
LXIDnn:	Immediate addressing
------	Load DE
*******************************************************************************/
#define LXIDnn	0x11


/*******************************************************************************
LXIHnn:	Immediate addressing
------	Load HL
*******************************************************************************/
#define LXIHnn	0x21


/*******************************************************************************
LXISPnn:Immediate addressing
-------	Load stack pointer
*******************************************************************************/
#define LXISPnn	0x31


/*******************************************************************************
MOVrr:	Register addressing
-----	Move register to register
*******************************************************************************/
/*	#define MOVr1r2 0x7F	*/
#define MOVAA	0x7F
#define MOVAB	0x78
#define MOVAC	0x79
#define MOVAD	0x7A
#define MOVAE	0x7B
#define MOVAH	0x7C
#define MOVAL	0x7D
#define MOVBA	0x47
#define MOVBB	0x40
#define MOVBC	0x41
#define MOVBD	0x42
#define MOVBE	0x43
#define MOVBH	0x44
#define MOVBL	0x45
#define MOVCA	0x4F
#define MOVCB	0x48
#define MOVCC	0x49
#define MOVCD	0x4A
#define MOVCE	0x4B
#define MOVCH	0x4C
#define MOVCL	0x4D
#define MOVDA	0x57
#define MOVDB	0x50
#define MOVDC	0x51
#define MOVDD	0x52
#define MOVDE	0x53
#define MOVDH	0x54
#define MOVDL	0x55
#define MOVEA	0x5F
#define MOVEB	0x58
#define MOVEC	0x59
#define MOVED	0x5A
#define MOVEE	0x5B
#define MOVEH	0x5C
#define MOVEL	0x5D
#define MOVHA	0x67
#define MOVHB	0x60
#define MOVHC	0x61
#define MOVHD	0x62
#define MOVHE	0x63
#define MOVHH	0x64
#define MOVHL	0x65
#define MOVLA	0x6F
#define MOVLB	0x68
#define MOVLC	0x69
#define MOVLD	0x6A
#define MOVLE	0x6B
#define MOVLH	0x6C
#define MOVLL	0x6D


/*******************************************************************************
MOVMr:	Register addressing
-----	Move memory to register
*******************************************************************************/
/*	#define MOVMr	0x77	*/
#define MOVMA	0x77
#define MOVMB	0x70
#define MOVMC	0x71
#define MOVMD	0x72
#define MOVME	0x73
#define MOVMH	0x74
#define MOVML	0x75


/*******************************************************************************
MOVrM:	Register addressing
-----	Move register to memory
*******************************************************************************/
/*	#define MOVrM	0x7E	*/
#define MOVAM	0x7E
#define MOVBM	0x46
#define MOVCM	0x4E
#define MOVDM	0x56
#define MOVEM	0x5E
#define MOVHM	0x66
#define MOVLM	0x6E


/*******************************************************************************
MVIrn:	Immediate addressing
-----	Move immediate to register
*******************************************************************************/
/*	#define MVIrn	0x3E	*/
#define MVIAn	0x3E
#define MVIBn	0x06
#define MVICn	0x0E
#define MVIDn	0x16
#define MVIEn	0x1E
#define MVIHn	0x26
#define MVILn	0x2E
#define MVIMn	0x36


/*******************************************************************************
NOP:
---	No operation
*******************************************************************************/
#define NOP	0x00


/*******************************************************************************
ORAr:	Register addressing
----	Inclusive OR register to accumulator
*******************************************************************************/
/*	#define ORAr	0xB7	*/
#define ORAA	0xB7
#define ORAB	0xB0
#define ORAC	0xB1
#define ORAD	0xB2
#define ORAE	0xB3
#define ORAH	0xB4
#define ORAL	0xB5
#define ORAM	0xB6


/*******************************************************************************
ORIn:	Immediate addressing
----	Inclusive OR immediate to accumulator
*******************************************************************************/
#define ORIn	0xF6


/*******************************************************************************
OUTp:	Direct addressing
----	Output
*******************************************************************************/
#define OUTp	0xD3


/*******************************************************************************
PCHL:	Register addressing
----	Jump to ROM[HL]
*******************************************************************************/
#define PCHL	0xE9


/*******************************************************************************
POPB:	Register addressing
----	Pop BC
*******************************************************************************/
#define POPB	0xC1


/*******************************************************************************
POPD:	Register addressing
----	Pop DE
*******************************************************************************/
#define POPD	0xD1


/*******************************************************************************
POPH:	Register addressing
----	Pop HL
*******************************************************************************/
#define POPH	0xE1


/*******************************************************************************
POPPSW:	Register addressing
------	Pop processor status word
*******************************************************************************/
#define POPPSW	0xF1


/*******************************************************************************
PUSHB:	Register addressing
-----	Push BC
*******************************************************************************/
#define PUSHB	0xC5


/*******************************************************************************
PUSHD:	Register addressing
-----	Push DE
*******************************************************************************/
#define PUSHD	0xD5


/*******************************************************************************
PUSHH:	Register addressing
-----	Push HL
*******************************************************************************/
#define PUSHH	0xE5


/*******************************************************************************
PUSHPSW:Register addressing
-------	Push processor status word
*******************************************************************************/
#define PUSHPSW	0xF5


/*******************************************************************************
RAL:	Register addressing
---	Rotate left to accumulator
*******************************************************************************/
#define RAL	0x17


/*******************************************************************************
RAR:	Register addressing
---	Rotate right to accumulator
*******************************************************************************/
#define RAR	0x1F


/*******************************************************************************
RET:	Register addressing
---	Return
*******************************************************************************/
#define RET	0xC9


/*******************************************************************************
RC:	Register addressing
--	Return on carry
*******************************************************************************/
#define RC	0xD8


/*******************************************************************************
RM:	Register addressing
--	Return on sign
*******************************************************************************/
#define RM	0xF8


/*******************************************************************************
RNC:	Register addressing
---	Return on no carry
*******************************************************************************/
#define RNC	0xD0


/*******************************************************************************
RNZ:	Register addressing
---	Return on no zero
*******************************************************************************/
#define RNZ	0xC0


/*******************************************************************************
RP:	Register addressing
--	Return on no sign
*******************************************************************************/
#define RP	0xF0


/*******************************************************************************
RPE:	Register addressing
---	Return on parity even
*******************************************************************************/
#define RPE	0xE8


/*******************************************************************************
RPO:	Register addressing
---	Return on parity odd
*******************************************************************************/
#define RPO	0xE0


/*******************************************************************************
RZ:	Register addressing
--	Return on zero
*******************************************************************************/
#define RZ	0xC8


/*******************************************************************************
RLC:	Register addressing
---	Rotate left circular to accumulator
*******************************************************************************/
#define RLC	0x07


/*******************************************************************************
RRC:	Register addressing
---	Rotate right circular to accumulator
*******************************************************************************/
#define RRC	0x0F


/*******************************************************************************
RSTz:	Indirect addressing
----	Restart
*******************************************************************************/
/*	#define RSTz	0xC7	*/
#define RST0	0xC7
#define RST1	0xCF
#define RST2	0xD7
#define RST3	0xDF
#define RST4	0xE7
#define RST5	0xEF
#define RST6	0xF7
#define RST7	0xFF


/*******************************************************************************
SBBr:	Register addressing
----	Subtract with borrow from register to accumulator
*******************************************************************************/
/*	#define SBBr	0x9F	*/
#define SBBA	0x9F
#define SBBB	0x98
#define SBBC	0x99
#define SBBD	0x9A
#define SBBE	0x9B
#define SBBH	0x9C
#define SBBL	0x9D
#define SBBM	0x9E


/*******************************************************************************
SBIn:	Immediate addressing
----	Subtract with borrow from immediate to accumulator
*******************************************************************************/
#define SBIn	0xDE


/*******************************************************************************
SHLDa:	Direct addressing
-----	Store HL direct
*******************************************************************************/
#define SHLDa	0x22


/*******************************************************************************
SPHL:	Register addressing
----	Move HL to stack pointer
*******************************************************************************/
#define SPHL	0xF9


/*******************************************************************************
STAa:	Direct addressing
----	Store accumulator
*******************************************************************************/
#define STAa	0x32


/*******************************************************************************
STAXB:	Register addressing
-----	Store accumulator to ROM[BC]
*******************************************************************************/
#define STAXB	0x02


/*******************************************************************************
STAXD:	Register addressing
-----	Store accumulator to ROM[DE]
*******************************************************************************/
#define STAXD	0x12


/*******************************************************************************
STC:	Register addressing
---	Set carry
*******************************************************************************/
#define STC	0x37


/*******************************************************************************
SUBr:	Register addressing
----	Subtract from register to accumulator
*******************************************************************************/
/*	#define SUBr	0x97	*/
#define SUBA	0x97
#define SUBB	0x90
#define SUBC	0x91
#define SUBD	0x92
#define SUBE	0x93
#define SUBH	0x94
#define SUBL	0x95
#define SUBM	0x96


/*******************************************************************************
SUIn:	Immediate addressing
----	Subtract from immediate to accumulator
*******************************************************************************/
#define SUIn	0xD6


/*******************************************************************************
XCHG:	Register addressing
----	Exchange HL with DE
*******************************************************************************/
#define XCHG	0xEB


/*******************************************************************************
XRAr:	Register addressing
----	Exclusive OR register to accumulator
*******************************************************************************/
/*	#define XRAr	0xAF	*/
#define XRAA	0xAF
#define XRAB	0xA8
#define XRAC	0xA9
#define XRAD	0xAA
#define XRAE	0xAB
#define XRAH	0xAC
#define XRAL	0xAD
#define XRAM	0xAE


/*******************************************************************************
XRIn:	Immediate addressing
----	Exclusive OR register to accumulator
*******************************************************************************/
#define XRIn	0xEE


/*******************************************************************************
XTHL:	Register addressing
----	Exchange ROM[stack pointer] with HL
*******************************************************************************/
#define XTHL	0xE3




/*******************************************************************************

Cycles per instruction:
----------------------

   This table is a list of the number of cycles each instruction takes to
complete.  Conditional instructions are adjusted based on the determined truth
values and adjusted in the C file itself (8080a.c)

*******************************************************************************/
const char CYCLES[256] =
{
/**0, *1, *2, *3, *4, *5, *6, *7, *8, *9, *A, *B, *C, *D, *E, *F      */
  04, 10, 07, 05, 05, 05, 07, 04, 00, 10, 07, 05, 05, 05, 07, 04,  /*0*/
  00, 10, 07, 05, 05, 05, 07, 04, 00, 10, 07, 05, 05, 05, 07, 04,  /*1*/
  00, 10, 16, 05, 05, 05, 07, 04, 00, 10, 16, 05, 05, 05, 07, 04,  /*2*/
  00, 10, 13, 05, 05, 10, 10, 04, 00, 10, 13, 05, 05, 05, 07, 04,  /*3*/
  05, 05, 05, 05, 05, 05, 07, 05, 05, 05, 05, 05, 05, 05, 07, 05,  /*4*/
  05, 05, 05, 05, 05, 05, 07, 05, 05, 05, 05, 05, 05, 05, 07, 05,  /*5*/
  05, 05, 05, 05, 05, 05, 07, 05, 05, 05, 05, 05, 05, 05, 07, 05,  /*6*/
  07, 07, 07, 07, 07, 07, 07, 07, 05, 05, 05, 05, 05, 05, 07, 05,  /*7*/
  04, 04, 04, 04, 04, 04, 07, 04, 04, 04, 04, 04, 04, 04, 07, 04,  /*8*/
  04, 04, 04, 04, 04, 04, 07, 04, 04, 04, 04, 04, 04, 04, 07, 04,  /*9*/
  04, 04, 04, 04, 04, 04, 07, 04, 04, 04, 04, 04, 04, 04, 07, 04,  /*A*/
  04, 04, 04, 04, 04, 04, 07, 04, 04, 04, 04, 04, 04, 04, 07, 04,  /*B*/
  05, 10, 10, 11, 11, 11, 07, 11, 05, 10, 10, 00, 11, 17, 07, 11,  /*C*/
  05, 10, 10, 11, 11, 11, 07, 11, 05, 00, 10, 10, 11, 00, 07, 11,  /*D*/
  05, 10, 10, 11, 11, 11, 07, 11, 05, 05, 10, 04, 11, 00, 07, 11,  /*E*/
  05, 10, 10, 11, 11, 11, 07, 11, 05, 05, 10, 04, 11, 00, 07, 11   /*F*/
};



#ifdef DEBUG

/*******************************************************************************

Mnemonics:
---------

   This table is a list of the nmemonics for each instruction.  These
mnemonics are used so the programmer can see the name of the instruction that
is currently executing instead of seeing a hexidecimal number.

*******************************************************************************/
const char MNEMONICS[][256] = 
{

/*00       01       02      03      04     05     06      07
  08       09       0A      0B      0C     0D     0E      0F  */
"NOP",  "LXIBnn", "STAXB","INXB", "INRB","DCRB","MVIBn","RLC",   /*00*/
"0x08", "DADB",   "LDAXB","DCXB", "INRC","DCRC","MVICn","RRC",
"0x10", "LXIDnn", "STAXD","INXD", "INRD","DCRD","MVIDn","RAL",   /*10*/
"0x18", "DADD",   "LDAXD","DCXD", "INRE","DCRE","MVIEn","RAR",
"0x20", "LXIHnn", "SHLDa","INXH", "INRH","DCRH","MVIHn","DAA",   /*20*/
"0x28", "DADH",   "LDAXH","DCXH", "INRL","DCRL","MVILn","CMA",
"0x30", "LXISPnn","STAa", "INXSP","INRM","DCRM","MVIMn","STC",   /*30*/
"0x38", "DADSP",  "LDAa", "DCXSP","INRA","DCRA","MVIAn","CMC",
"MOVBB","MOVBC","MOVBD","MOVBE","MOVBH","MOVBL","MOVBM","MOVBA", /*40*/
"MOVCB","MOVCC","MOVCD","MOVCE","MOVCH","MOVCL","MOVCM","MOVCA",
"MOVDB","MOVDC","MOVDD","MOVDE","MOVDH","MOVDL","MOVDM","MOVDA", /*50*/
"MOVEB","MOVEC","MOVED","MOVEE","MOVEH","MOVEL","MOVEM","MOVEA",
"MOVHB","MOVHC","MOVHD","MOVHE","MOVHH","MOVHL","MOVHM","MOVHA", /*60*/
"MOVLB","MOVLC","MOVLD","MOVLE","MOVLH","MOVLL","MOVLM","MOVLA",
"MOVMB","MOVMC","MOVMD","MOVME","MOVMH","MOVML","HLT",  "MOVMA", /*70*/
"MOVAB","MOVAC","MOVAD","MOVAE","MOVAH","MOVAL","MOVAM","MOVAA",
"ADDB", "ADDC", "ADDD", "ADDE", "ADDH", "ADDL", "ADDM","ADDA",   /*80*/
"ADCB", "ADCC", "ADCD", "ADCE", "ADCH", "ADCL", "ADCM","ADCA",
"SUBB", "SUBC", "SUBD", "SUBE", "SUBH", "SUBL", "SUBM","SUBA",   /*90*/
"SBBB", "SBBC", "SBBD", "SBBE", "SBBH", "SBBL", "SBBM","SBBA",
"ANAB", "ANAC", "ANAD", "ANAE", "ANAH", "ANAL", "ANAM","ANAA",   /*A0*/
"XRAB", "XRAC", "XRAD", "XRAE", "XRAH", "XRAL", "XRAM","XRAA",
"ORAB", "ORAC", "ORAD", "ORAE", "ORAH", "ORAL", "ORAM","ORAA",   /*B0*/
"CMPB", "CMPC", "CMPD", "CMPE", "CMPH", "CMPL", "CMPM","CMPA",
"RNZ","POPB",  "JNZa","JMPa","CNZa","PUSHB",  "ADIn","RST0",     /*C0*/
"RZ", "RET",   "JZa", "0xCB","CZa", "CALLa",  "ACIn","RST1",
"RNC","POPD",  "JNCa","OUTp","CNCa","PUSHD",  "SUIn","RST2",     /*D0*/
"RC", "0xD9",  "JCa", "INp", "CCa", "0xDD",   "SBIn","RST3",
"RPO","POPH",  "JPOa","XTHL","CPOa","PUSHH",  "ANIn","RST4",     /*E0*/
"RPE","PCHL",  "JPEa","XCHG","CPEa","0xED",   "XRIn","RST5",
"RP", "POPPSW","JPa", "DI",  "CPa", "PUSHPSW","ORIn","RST6",     /*F0*/
"RM", "SPHL",  "JMa", "EI",  "CMa", "0xFD",   "CPIn","RST7"


};

#endif	// for #ifdef DEBUG


#endif
