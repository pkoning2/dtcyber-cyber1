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


Contents:
--------
  1.	This file contains the 8080a's registers, the 8080a's interrupt
	information, the 8080a's default processor values (when reset),
	definitions for the PSW, memory map information, and the memory itself.

*******************************************************************************/


#ifndef __8080AVAR_H__
#define __8080AVAR_H__

//#define SELFMOD

/* This endian stuff is lifted from SDL_byteorder.h.  It's here rather
 * than handled by #include because newer versions of SDL bring in too
 * much extraneous stuff. 
 */

/* The two types of endianness */
#define SDL_LIL_ENDIAN	1234
#define SDL_BIG_ENDIAN	4321


#ifdef __linux__
#include <endian.h>
#define SDL_BYTEORDER __BYTE_ORDER
#else

/* Pardon the mess, I'm trying to determine the endianness of this host.
   I'm doing it by preprocessor defines rather than some sort of configure
   script so that application code can use this too.  The "right" way would
   be to dynamically generate this file on install, but that's a lot of work.
 */
#if (defined(__i386__) || defined(__i386)) || \
     defined(__ia64__) || defined(WIN32) || \
    (defined(__alpha__) || defined(__alpha)) || \
     defined(__arm__) || \
    (defined(__mips__) && defined(__MIPSEL__)) || \
     defined(__SYMBIAN32__) || \
     defined(__x86_64__) || \
     defined(__LITTLE_ENDIAN__)
#define SDL_BYTEORDER	SDL_LIL_ENDIAN
#else
#define SDL_BYTEORDER	SDL_BIG_ENDIAN
#endif

#endif /* __linux __ */


/*******************************************************************************
* Conflicting Endian:
* ------------------
* 
*   If an endian type is already defined in the Makefile and it does not match
* the endian type that is autodetected, a warning is printed to the screen
* informing the user that a possible error has occured in endian selection.
* 
*******************************************************************************/
#if !defined(__WIN32__) & \
     (((SDL_BYTEORDER == SDL_LIL_ENDIAN) && BIGENDIAN) || \
     ((SDL_BYTEORDER == SDL_BIG_ENDIAN) && LITTLEENDIAN))
	#warning CONFLICTING ENDIAN TYPES DETECTED!
	#warning Edit the Makefile if this is incorrect.
#endif


/*******************************************************************************
* Variables:
* ---------
* 
*   INTDISABLE - This variable is used when the DI instruction is called.  It
*	signifies that interrupts are disabled.
*
*   INTERRUPT - This variable is used to determine if an interrupt occured,
*	and if an interrupt did occur, what type of interrupt occured.
*
*   NMI - This variable is used to determine if a non-maskable interrupt
*	occured.
*
*   COUNTER - This variable counts the number of cycles per instruction in an
*	attempt to get accurate interrupt timing.
*
*   RAM is the 8080a's RAM.  This is really RAM and ROM put together.  The
* 	simulator does not discriminate with read/write permissions, the whole
* 	RAM is writable.  Although this allows for self modifying code, access
* 	violations are not caught.  Rom size is 64k (0x10000 hex/65536 bytes).
*
*
* 16-bit registers:
* ----------------
*
*   PC is the Program Counter, which is a 16 bit register
*
*   SP is the Stack Pointer, which is a 16 bit register
*
*
* 8-bit registers:
* ---------------
*
*   A - This variable is the 8-bit accumulator.
*
*   PSW - This variable is the processor status word, which is also an 8-bit
*	register.
*
*   BC, DE, and HL are 16-bit paired registers.  Each register pair contains
*	2 8-bit registers.  BC contains B and C, DE contains D and E, and HL
*	contains H and L. Each pair is part of a union struct, where .pair
*	is the value of the pair and .reg. is the value of a single register.
*
*******************************************************************************/


/*******************************************************************************
* Memory Map:
* ----------
* 
*   This simplifies ROM set loading and 8080a memory access.
* 
* 
* Assumptions:
* -----------
*   Roms are split into four 800 hex (2048 bytes) files and are loaded
* sequentially.  Space Invaders, which follows the above assumption, is the
* main ROM set that is loaded.
* 
* 
* Definitions:
* -----------
* 
*   STARTROM1 - This is the memory address where the first ROM should be
*	loaded.
*
*   ROMSIZE - This is the size of each ROM.  2000hex (8192 bytes).
*
*   WORKRAM - This is where the processor's WORK RAM begins. 
*
*   UNUSED - This is where the processor's UNUSED memory area begins.  This
*	definition is only true for Space Invaders.
* 
*
* Diagram:
* -------
*
*  The 8080a's	
*  memory map	 Address ranges
* +-----------+
* |           |
* |  PROGROM  | 0x0000h-0x1FFFh
* +-----------+
* |           |
* |  WORKRAM  | 0x2000h-0xfFFFh
* +-----------+
* 
*
* Note:
* ----
* 
*    The above information may be PPT specific.
*
*******************************************************************************/
/*  Memory Map  */
#define STARTROM1 0x00000
#define ROMSIZE   0x02000
#define PROGROM   0x00000
#define WORKRAM   0x02000
#define MEMSIZE   0x0ffff
class emul8080
{
public:
    emul8080 () 
    {
        ResetProc ();
    }
    virtual ~emul8080 () {};
    
    void main8080a (void);
    void ResetProc (void);

    void WriteRAM(Uint16 offset, Uint8 data)
    {
#ifndef SELFMOD
        if ( (offset) >= WORKRAM )
        {
            RAM[(offset)] = data;
        }
        else
        { 
            printf ( "!!! Attempted write at offset %04x with data "
                     "%02x failed.  PC = %04x\n", offset, data, PC);
        }
#else
        RAM[(offset)] = data;
#endif
    }

	void WriteRAMW(Uint16 offset, Uint16 data)
	{
#ifndef SELFMOD
		if ((offset) >= WORKRAM)
		{
			RAM[(offset)] = data & 0xff;
			RAM[(offset + 1)] = (data << 8) & 0xff;
		}
		else
		{
			printf("!!! Attempted write at offset %04x with data "
				"%04x failed.  PC = %04x\n", offset, data, PC);
		}
#else
		RAM[(offset)] = data & 0xff;
		RAM[(offset + 1)] = (data << 8) & 0xff;
#endif
	}

    Uint8 ReadRAM(Uint16 offset) const 
    {
        return RAM[offset];
    }
    Uint16 ReadRAMW(Uint16 offset) const 
    {
        return RAM[offset] + (RAM[offset + 1] << 8);
    }

protected:
    virtual Uint8 input8080a (Uint8 data);
    virtual void output8080a (Uint8 data, Uint8 acc);

    // This emulates the "ROM resident".  Return values:
    // 0: PC is not special (not in resident), proceed normally.
    // 1: PC is ROM function entry point, it has been emulated,
    //    do a RET now.
    // 2: PC is either the 8080 emulation exit magic value, or R_INIT,
    //    or an invalid resident value.  Exit 8080 emulation.
    virtual int check_pc8080a (void);

    // this is a kludge for processing mode 6 data in MTutor
    Uint8 ReturnOn8080Ret;

    /*  Interrupt information  */
    Uint8 INTDISABLE;
    Uint8 INTERRUPT;
    Uint8 NMI;
    Uint16 COUNTER;

    /*  Registers  */
    Uint8 A, PSW;
    Uint16 SP, PC;


    /*  Register Pairs  */
    union
    {
        Uint16 pair;

        struct
        {
#if BIGENDIAN
			Uint8 B, C;
#else
			Uint8 C, B;
#endif
        } reg;
    } BC;


    union
    {
        Uint16 pair;

        struct
        {
#if BIGENDIAN
			Uint8 D, E;
#else
			Uint8 E, D;
#endif
        } reg;
    } DE;


    union
    {
        Uint16 pair;

        struct
        {
#if BIGENDIAN
			Uint8 H, L;
#else
			Uint8 L, H;
#endif
        } reg;
    } HL;

public:
    /*  8080a's RAM  */
    Uint8 RAM[MEMSIZE];
};


/*******************************************************************************
* Interrupt timings:
* -----------------
* 
* Processor	= 2048000 hz
*
* Refresh Rate	= 60 hz
* Interrupt	= 2048000 hz/60 hz ~= 34133/2 ~= 17066 cycles per interrupt
*
* Refresh Rate	= 50 hz
* Interrupt	= 2048000 hz/50 hz ~= 40960/2 ~= 20480 cycles per interrupt
* 
*
*    Although this number is inaccurate, it is still accurate enough to ensure
* proper timing and simulation.  After 33333 cycles (number of cycles may be
* higher, instruction and hertz rate depending), the program calls an
* interrupt.
* 
* 
* Resets:
* ------
* 
*   RST0 is a full restart.
*   RST1 is for vsync.  This is usually called during an non-maskable
*	interrupt.
*   RST2 is checking for input.  This opcode is called during a normal
*	interrupt.
* 
*******************************************************************************/

#ifndef PROCHZ
#define PROCHZ 2048000	// 2 Mhz
#endif

#ifndef REFRESH
#define REFRESH 60	// 60 hertz
#endif

#define INTHIGH	(PROCHZ/REFRESH)/2



/*******************************************************************************
* Default processor values:
* ------------------------
* 
*   The stack pointer starts at 0x2400.
*   The program counter starts at 0x0000.
*   The program status word value starts at 0x02
* 
*******************************************************************************/
/*  Defaults  */
#define SPSET	0xffff      /* To make microTutor not scribble on the stack */
#define PCSET	0x0000
#define PSWSET	0x02


/*******************************************************************************
* Processor status word:
* ---------------------
* 
*   The PSW is an 8 bit register that stores the 8080a's current instruction
* status.
* 
* 
* Map:
* ---
* 
* bit	7654 3210
* 	SZ_A P_1C
*
* S = Sign (negative), Z = Zero, A = Auxiliary carry,
* P = Parity, C = Carry, _ = Unknown, 1 = Always on.
* 
*******************************************************************************/
/*  PSW  */
#define SIGN	0x80
#define ZERO	0x40
#define AUX	0x10
	// Even parity
#define PARITY	0x04
#define CARRY	0x01
#define NSIGN	(0xFF - SIGN)
#undef  NZERO
#define NZERO	(0xFF - ZERO)
#define NAUX	(0xFF - AUX)
	// Odd parity
#define NPARITY	(0xFF - PARITY)
#define NCARRY	(0xFF - CARRY)



#endif
