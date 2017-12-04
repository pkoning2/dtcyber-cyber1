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


Preface:
-------

   The main functions in this file are LoadSet, ResetProc, and main8080a. 
LoadSet loads a set of ROM files from the disk into the 8080a's RAM.  ResetProc
zeroes out all of the registers and 8080a memory, other then the ROM.
main8080a is the main simulation core.  This is where the fetch/decode/execute
simulation phase occurs.

*******************************************************************************/

//#define DEBUG_8080

//#define Z80

#define Level2Pause 0x68d9
#define Level2Xplato 0x602c

/*******************************************************************************
* Files:
* -----
*
*   8080a.h - This file contains the function definitions for main8080a,
*   LoadSet, and ResetProc.
*
*   8080avar.h - This file contains the data type definitions for the 8080a's
*   registers, memory, and interrupts.
*
*   opcodes.h - This file contains the definitions for the 8080a's opcodes.
*
*******************************************************************************/
#include "8080a.h"
#include "8080avar.h"
#include "opcodes.h"
#include "ppt.h"
//#include <SDL_hints.h>

/*******************************************************************************

ResetProc:
---------

   This function resets the processor.  This is done by resetting the program
counter (PC), resetting the processor status word (PSW), enabling interrupts,
disabling the non-maskable interrupt, resettting the stack pointer (SP),
enabling interrupts, and resetting the cycle counter. 



*******************************************************************************/
void emul8080::ResetProc(void)
{
    /***********************************************************************
    * Initializations:
    * ---------------
    *
    *   INTERRUPT - This variable is used to test to see if an interrupt
    *   has occured.  This variable is checked after the number of
    *   cycles reaches INTHIGH.
    *
    *   NMI - This variable is used to test to see if a non-maskable
    *   interrupt has occured.  This variable is checked after the
    *   number of cycles reaches INTHIGH.
    *
    *   INTDISABLE - This variable is used to test to see if interrupts are
    *   disabled.  If set, an interrupt cannot occur.
    *
    *   COUNTER - This variable counts the number of cycles per
    *   instruction.  Once this number reaches INTHIGH, an interrupt
    *   is set to occur.
    *
    *
    * 16-bit registers:
    * ----------------
    *
    *   SP - This variable is the stack pointer.  It is reset to SPSET,
    *   which is the beginning of the 8080a's stack.
    *
    *   PC - This variable is the program counter.  It is reset to PCSET,
    *   which points to the start of the 8080a's ROM
    *
    *
    * 8-bit registers:
    * ---------------
    *
    *   PSW - This variable is the processor status word.  It is reset to
    *   PSWSET, which by default sets all flags (except 0x02) to off.
    *
    *   A - This variable is the accumulator.  It is reset to 0.
    *
    *   BC.pair - This variable is the registers B and C.  Both B and C are
    *   reset to 0.
    *
    *   DE.pair - This variable is the registers D and E.  Both D and E are
    *   reset to 0.
    *
    *   HL.pair - This variable is the registers H and L.  Both H and L are
    *   reset to 0.
    ***********************************************************************/
    INTERRUPT   = 1;
    NMI         = 0;
    INTDISABLE  = 0;
    COUNTER     = 0;
    SP          = SPSET;
    PC          = PCSET;
    PSW         = PSWSET;
    A           = 0;
    BC.pair     = 0;
    DE.pair     = 0;
    HL.pair     = 0;

    /***********************************************************************
     *
     *  Zero ROM and Plant RET instructions for all interrupt RST locations
     *
     **********************************************************************/

    for (int i = STARTROM1; i < MEMSIZE; i++)   // zero rom/ram
    {
        RAM[i] = 0;
    }

    RAM[0x0] = RET;
    RAM[0x8] = RET;
    RAM[0x10] = RET;
    RAM[0x18] = RET;
    RAM[0x20] = RET;
    RAM[0x28] = RET;
    RAM[0x30] = RET;
    RAM[0x38] = RET;
    //RAM[0x3d] = RET;
    //RAM[0x40] = RET;

    RAM[ROMSIZE - 1] = RET;  // for safety

    ReturnOn8080Ret = 0;
}

#ifdef DEBUG_8080

// addresses of 8080 breakpoints
UINT16 breakpts[] =
{
    //0x4000,
    //0x5315,     // v.reloc
    //0x5320,
    //0x5325,
    //0x5337,

    //0x655c,     // exec overlay
    //0x5893,
    //0x5105,
    //0x5125,
    //0x5105 + 0x0051,

    //0x5187,

    0x64d5,

    0xffff,  // these can be overridden to add/remove 
    0xffff,  // break points while running.
    0xffff,  // use the immediate window.
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff  // must end with 0xffff
};

#endif

/*******************************************************************************

main8080a:
---------

   This function is the core of the 8080a simulator.  Before this function is
called, the ROMs are loaded into memory (either by the LoadRom function, or by
directly writing to the RAM.  When this function is called, it resets the
processor by calling ResetProc and enters the instruction fetch and decode 
loop (the for(;;) loop below).  With every pass of the loop, this function
fetches an instruction from RAM, increments the cycles depending on what
opcode was fetched, then decodes the opcode, executes the opcode (adding
extra cycles if the instruction is conditional), and finally it tests to see
if an interrupt has occured.

*******************************************************************************/
void emul8080::main8080a (void)
{
    /***********************************************************************
    * Declarations:
    * ------------
    *
    *   CARRYOVER - This integer is used in instructions that rely on
    * operating on the CARRY flag.
    *
    *   OPCODE - This integer stores the current instruction after it has
    * been fetched from RAM.
    ***********************************************************************/
    Uint8 CARRYOVER;
    Uint8 OPCODE = 0;
    //long int update = 0;
#ifdef DEBUG_8080
    bool step = false;
#endif

    if (m_mtutorPatch)
    {
        if (m_mtPLevel == 2)
        {
            // Call resident and wxWidgets for brief pause
            RAM[Level2Pause] = CALLa;
            RAM[Level2Pause + 1] = R_WAIT16;
            RAM[Level2Pause + 2] = 0;

            // remove off-line check for calling r.exec - only safe place to
            // give up control..
            RAM[Level2Xplato] = 0;
            RAM[Level2Xplato + 1] = 0;
            RAM[Level2Xplato + 2] = 0;

            // attempt to move unit buffer...
            //RAM[0x586c] = 0xff;  // top of unit buffer
            //RAM[0x586d] = 0x00;  //  "

            //RAM[0x5a89] = 0x80;  // bottom of unit buffer.
        }
    }

    /***********************************************************************
    *   The 8080a's main instruction loop.
    *
    *   Keep executing so long as the PC points into the RAM area.  When
    * the code jumps to ROM in any way, we exit, leaving it up to the
    * caller to decide what to do next.
    *
    *   The reason for this is that the PPT ROM routines are not handled
    * by emulating the 8080 code, but rather by emulating the semantics
    * of the routine.
    ***********************************************************************/
    while ((PC >= WORKRAM) || (PC < 0x40))
    {
        if (m_giveup8080)
        {
            m_giveup8080 = false;
            // give the resident time to process keys and update display
            return;
        }

    /***********************************************************************
    *   If INTERRUPT is not equal to 2, then it is safe to fetch an opcode.
    * The opcode that is fetched is the opcode at location PC in the
    * 8080a's ROM area.
    *
    *   If INTERRUPT is equal to 2, then an interrupt has occured, so this
    * function disables interrupts because a new interrupt cannot occur
    * while an interrupt is in progress.  Interrupts can only be reenabled
    * by executing the EI intruction.
    ***********************************************************************/
        if (INTERRUPT != 2)
        {

    /***********************************************************************
            *   This assignment fetches an opcode from the 8080a's ROM RAM area.
    * The opcode is fetched from the PC location in RAM. The program
    * counter, named PC, is incremented by 1.
    ***********************************************************************/
            OPCODE  = ReadRAM(PC++);

        }
        else if (INTERRUPT == 2)
        {


    /***********************************************************************
    *   This assignment disables interrupts.  Interrupts can only be
    * reenabled by the EI instruction.
    ***********************************************************************/
            INTERRUPT = 0;

        }


    /***********************************************************************
    *   If INTDISABLE is equal to 2, then the EI instruction was executed.
    * The EI instruction requires that interrupts be enabled following the
    * instruction after the EI instruction.  Setting INTDISABLE to 0
    * reenables interrupts, following the execution of the next opcode.
    ***********************************************************************/
        if (INTDISABLE == 2)
        {
            INTDISABLE = 0;
        }


    /***********************************************************************
    *   This increments the number of cycles.  The number of cycles are
    * received from the table named CYCLES.
    ***********************************************************************/
        COUNTER += CYCLES[OPCODE];

        /*  This code is used for quick debugging. */
#ifdef DEBUG_8080
        if (m_MtTrace) // && ((PC > 0x7aee)  || ((PC < 0x4000 )) && (PC > 0x90)))  // overlay regions
        {
            //printf("%s\n", MNEMONICS[OPCODE]);

            //printf("OPCODE-[0x%X]\tPC-[0x%X]\tSP-[0x%X]\t"
            //    "PSW-[0x%X]\n", OPCODE, PC, SP, PSW);
            //printf("A-[0x%X] \tB/C-[0x%X/0x%X]\tD/E-[0x%X/0x%X]\t"
            //    "H/L-[0x%X/0x%X]\n", A, BC.reg.B, BC.reg.C, DE.reg.D,
            //    DE.reg.E, HL.reg.H, HL.reg.L);
            //printf("BC-[0x%X]\tDE-[0x%X]\t"
            //    "HL-[0x%X]\tSP-[0x%X/0x%X]\n", BC.pair, DE.pair,
            //    HL.pair, ReadRAM(SP), ReadRAM(SP + 1));
            //printf("RAM-[0x%X/0x%X/0x%X/0x%X]\n\n", ReadRAM(PC - 1), ReadRAM(PC),
            //    ReadRAM(PC + 1), ReadRAM(PC + 2));

            //printf("OPCODE-[0x%X]\tPC-[0x%X]\tSP-[0x%X]\t"
            //    "PSW-[]\n", OPCODE, PC - 1, SP);
            //printf("AF-[0x%X]\tBC-[0x%X]\tDE-[0x%X]\t"
            //    "HL-[0x%X]\n", A, BC.pair, DE.pair, HL.pair);

            //for (int i = 0x2400; i >= SP; i -= 2)
            //    printf("RAMW[0x%X]=0x%X\t", i, ReadRAMW(i));
            //printf("\nRAMW[SP]=0x%X\n\n", ReadRAMW(SP));

            /* begin simple 8080 degugger -  works well with MSVS
             * 
             * Add 8080 breakpoints to breakpts table below.
             * Set MSVS break points at two locations indicated.
             * 
             * Load watch window with Registers, PC, SP, RAM[...] etc.
             * 
             * Use immediate window to turn single step on/off - true/false
             */


            if (step)       // use immediate window to turn step on/off - true/false
            {
                PC = PC;    // <<<<< set break point here for step
            }

            UINT16 * bp = breakpts;
            while (*bp != 0xffff)
            {
                if (PC-1 == *bp )
                {
                    PC = PC;    // <<<<<< set break point here for break point
                    break;
                }
                bp++;
            }

            // end 8080 debugger
        }
#endif


    /***********************************************************************
    *   This switch statement executes the current opcode contained in
    * OPCODE.  Opcodes are defined in opcodes.h.
    ***********************************************************************/
        switch (OPCODE)
        {


    /***********************************************************************
    * ACIn - Add immediate with carry.
    * ----
    *
    *   If the value of the variable OPCODE is equal to ACIn, this routine
    * adds the content of the 8080a's memory, RAM[PC] and the content of
    * the CARRY flag, found in the PSW, to the content of the accumulator.
    * The resulting value is stored in the accumulator.  The processor
    * status word, named PSW, is set according to the result of the
    * operation.  The program counter, named PC, is incremented by one.
    ***********************************************************************/
            case ACIn:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) + (ReadRAM(PC)&0xF) + (PSW&CARRY)
                       > 0xF
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A + ReadRAM(PC) + (PSW&CARRY) > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;

                A += ReadRAM(PC++) + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;




    /***********************************************************************
    * ADCr - Add register with carry.
    * ----
    *
    *   If the value of the variable OPCODE is equal to ADCr, where r is an
    * 8-bit 8080a register, this routine adds the content of the register,
    * r, and the content of the CARRY flag, found in the PSW, to the
    * content of the accumulator.  The resulting value is stored in the
    * accumulator.  The processor status word, named PSW, is set according
    * to the result of the operation.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include ADCA, ADCB, ADCC, ADCD, ADCE, ADCH, and ADCL.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADCA, this routine
    * adds the contents of: A, the CARRY flag, and the accumulator to the
    * accumulator.  The PSW reflects the result of this addition.
    ***********************************************************************/
            case ADCA:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) + (A&0xF) + (PSW&CARRY) > 0xF
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A + A + (PSW&CARRY) > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;

                A += A + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADCB, this routine
    * adds the contents of: B, the CARRY flag, and the accumulator to the
    * accumulator.  The PSW reflects the result of this addition.
    ***********************************************************************/
            case ADCB:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) + (BC.reg.B&0xF) +
                      (PSW&CARRY) > 0xF
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A + BC.reg.B + (PSW&CARRY) > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;

                A += BC.reg.B + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADCC, this routine
    * adds the contents of: C, the CARRY flag, and the accumulator to the
    * accumulator.  The PSW reflects the result of this addition.
    ***********************************************************************/
            case ADCC:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) + (BC.reg.C&0xF) +
                      (PSW&CARRY) > 0xF
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A + BC.reg.C + (PSW&CARRY) > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;

                A += BC.reg.C + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADCD, this routine
    * adds the contents of: D, the CARRY flag, and the accumulator to the
    * accumulator.  The PSW reflects the result of this addition.
    ***********************************************************************/
            case ADCD:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) + (DE.reg.D&0xF) +
                      (PSW&CARRY) > 0xF
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A + DE.reg.D + (PSW&CARRY) > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;

                A += DE.reg.D + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADCE, this routine
    * adds the contents of: E, the CARRY flag, and the accumulator to the
    * accumulator.  The PSW reflects the result of this addition.
    ***********************************************************************/
            case ADCE:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) + (DE.reg.E&0xF) +
                      (PSW&CARRY) > 0xF
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A + DE.reg.E + (PSW&CARRY) > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;

                A += DE.reg.E + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADCH, this routine
    * adds the contents of: H, the CARRY flag, and the accumulator to the
    * accumulator.  The PSW reflects the result of this addition.
    ***********************************************************************/
            case ADCH:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) + (HL.reg.H&0xF) +
                      (PSW&CARRY) > 0xF
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A + HL.reg.H + (PSW&CARRY) > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;

                A += HL.reg.H + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADCL, this routine
    * adds the contents of: L, the CARRY flag, and the accumulator to the
    * accumulator.  The PSW reflects the result of this addition.
    ***********************************************************************/
            case ADCL:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) + (HL.reg.L&0xF) +
                      (PSW&CARRY) > 0xF
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A + HL.reg.L + (PSW&CARRY) > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;

                A += HL.reg.L + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * ADCM - Add memory with carry.
    * ----
    *
    *   If the value of the variable OPCODE is equal to ADCM, this routine
    * adds the content of the 8080a's memory, RAM[HL], and the content
    * of the CARRY flag (found in the PSW) to the content of the
    * accumulator.  The resulting value is stored in the accumulator.
    * The processor status word, named PSW, is set according to the result
    * of the operation.
    ***********************************************************************/
            case ADCM:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) + (ReadRAM(HL.pair)&0xF) +
                       (PSW&CARRY) > 0xF
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A + ReadRAM(HL.pair) + (PSW&CARRY) > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;

                A += ReadRAM(HL.pair) + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * ADDr - Add register.
    * ----
    *
    *   If the value of the variable OPCODE is equal to ADDr, where r is an
    * 8-bit 8080a register, this routine adds the content of the register,
    * r, to the content of the accumulator.  The resulting value is stored
    * in the accumulator.  The processor status word, named PSW, is set
    * according to the result of the operation.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include ADDA, ADDB, ADDC, ADDD, ADDE, ADDH, and ADDL.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADDA, this routine
    * adds the content of A and the accumulator to the accumulator.  The
    * PSW reflects the result of this addition.
    ***********************************************************************/
            case ADDA:
                PSW = A + A > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) + (A&0xF) > 0xF
                       ? PSW|AUX     : PSW&NAUX;

                A += A;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADDB, this routine
    * adds the content of B and the accumulator to the accumulator.  The
    * PSW reflects the result of this addition.
    ***********************************************************************/
            case ADDB:
                PSW = A + BC.reg.B > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) + (BC.reg.B&0xF) > 0xF
                       ? PSW|AUX     : PSW&NAUX;

                A += BC.reg.B;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADDC, this routine
    * adds the content of C and the accumulator to the accumulator.  The
    * PSW reflects the result of this addition.
    ***********************************************************************/
            case ADDC:
                PSW = A + BC.reg.C > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) + (BC.reg.C&0xF) > 0xF
                       ? PSW|AUX     : PSW&NAUX;

                A += BC.reg.C;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADDD, this routine
    * adds the content of D and the accumulator to the accumulator.  The
    * PSW reflects the result of this addition.
    ***********************************************************************/
            case ADDD:
                PSW = A + DE.reg.D > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) + (DE.reg.D&0xF) > 0xF
                       ? PSW|AUX     : PSW&NAUX;

                A += DE.reg.D;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADDE, this routine
    * adds the content of E and the accumulator to the accumulator.  The
    * PSW reflects the result of this addition.
    ***********************************************************************/
            case ADDE:
                PSW = A + DE.reg.E > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) + (DE.reg.E&0xF) > 0xF
                       ? PSW|AUX     : PSW&NAUX;
                A += DE.reg.E;
                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADDH, this routine
    * adds the content of H and the accumulator to the accumulator.  The
    * PSW reflects the result of this addition.
    ***********************************************************************/
            case ADDH:
                PSW = A + HL.reg.H > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) + (HL.reg.H&0xF) > 0xF
                       ? PSW|AUX     : PSW&NAUX;

                A += HL.reg.H;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ADDL, this routine
    * adds the content of L and the accumulator to the accumulator.  The
    * PSW reflects the result of this addition.
    ***********************************************************************/
            case ADDL:
                PSW = A + HL.reg.L > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) + (HL.reg.L&0xF) > 0xF
                       ? PSW|AUX     : PSW&NAUX;

                A += HL.reg.L;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * ADDM - Add memory.
    * ----
    *
    *   If the value of the variable OPCODE is equal to ADCM, this routine
    * adds the content of the 8080a's memory, RAM[HL], to the content
    * of the accumulator.  The resulting value is stored in the
    * accumulator.  The processor status word, named PSW, is set according
    * to the result of the operation.
    ***********************************************************************/
            case ADDM:
                PSW = A + ReadRAM(HL.pair) > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) + (ReadRAM(HL.pair)&0xF) > 0xF
                       ? PSW|AUX     : PSW&NAUX;

                A += ReadRAM(HL.pair);

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * ADIn - Add immediate.
    * ----
    *
    *   If the value of the variable OPCODE is equal to ADIn, this routine
    * adds the content of the 8080a's memory, RAM[PC] to the content of the
    * accumulator.  The resulting value is stored in the accumulator.  The
    * processor status word, named PSW, is set according to the result of
    * the operation.  The program counter, named PC, is incremented by one.
    ***********************************************************************/
            case ADIn:
                PSW = A + ReadRAM(PC) > 0xFF
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) + (ReadRAM(PC)&0xF) > 0xF
                       ? PSW|AUX     : PSW&NAUX;

                A += ReadRAM(PC++);

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * ANAr - AND register.
    * ----
    *
    *   If the value of the variable OPCODE is equal to ANAr, where r is an
    * 8-bit 8080a register, this routine ANDs the content of the register,
    * r, with the content of the accumulator.  The resulting value is
    * stored in the accumulator.  The processor status word, named PSW, is
    * set according to the result of the operation.  The CARRY and AUX
    * flags are cleared in the PSW.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include ANAA, ANAB, ANAC, ANAD, ANAE, ANAH, and ANAL.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ANAA, this routine
    * ANDs the content of A with the accumulator to the accumulator.  The
    * PSW reflects the result of this AND operation.  The CARRY and AUX are
    * flags cleared.
    ***********************************************************************/
            case ANAA:
                A = A&A;

                PSW = PSW&NCARRY&NAUX;
                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ANAB, this routine
    * ANDs the content of B with the accumulator to the accumulator.  The
    * PSW reflects the result of this AND operation.  The CARRY and AUX are
    * flags cleared.
    ***********************************************************************/
            case ANAB:
                A = A&BC.reg.B;

                PSW = PSW&NCARRY&NAUX;
                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ANAC, this routine
    * ANDs the content of C with the accumulator to the accumulator.  The
    * PSW reflects the result of this AND operation.  The CARRY and AUX are
    * flags cleared.
    ***********************************************************************/
            case ANAC:
                A = A&BC.reg.C;

                PSW = PSW&NCARRY&NAUX;
                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ANAD, this routine
    * ANDs the content of D with the accumulator to the accumulator.  The
    * PSW reflects the result of this AND operation.  The CARRY and AUX are
    * flags cleared.
    ***********************************************************************/
            case ANAD:
                A = A&DE.reg.D;

                PSW = PSW&NCARRY&NAUX;
                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ANAE, this routine
    * ANDs the content of E with the accumulator to the accumulator.  The
    * PSW reflects the result of this AND operation.  The CARRY and AUX are
    * flags cleared.
    ***********************************************************************/
            case ANAE:
                A = A&DE.reg.E;

                PSW = PSW&NCARRY&NAUX;
                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ANAH, this routine
    * ANDs the content of H with the accumulator to the accumulator.  The
    * PSW reflects the result of this AND operation.  The CARRY and AUX are
    * flags cleared.
    ***********************************************************************/
            case ANAH:
                A = A&HL.reg.H;

                PSW = PSW&NCARRY&NAUX;
                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ANAL, this routine
    * ANDs the content of L with the accumulator to the accumulator.  The
    * PSW reflects the result of this AND operation.  The CARRY and AUX are
    * flags cleared.
    ***********************************************************************/
            case ANAL:
                A = A&HL.reg.L;

                PSW = PSW&NCARRY&NAUX;
                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * ANAM - AND memory.
    * ----
    *
    *   If the value of the variable OPCODE is equal to ANAM, this routine
    * ANDs the content of the 8080a's memory, RAM[HL], with the content
    * of the accumulator.  The resulting value is stored in the
    * accumulator.  The processor status word, named PSW, is set according
    * to the result of the operation.  The CARRY and AUX flags are cleared
    * in the PSW.
    ***********************************************************************/
            case ANAM:
                A = A&ReadRAM(HL.pair);

                PSW = PSW&NCARRY&NAUX;
                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * ANIn - AND immediate.
    * ----
    *
    *   If the value of the variable OPCODE is equal to ANIn, this routine
    * ANDs the content of the 8080a's memory, RAM[PC] with the content of
    * the accumulator.  The resulting value is stored in the accumulator.
    * The processor status word, named PSW, is set according to the result
    * of the operation.  The CARRY and AUX flags are cleared in the PSW.
    * The program counter, named PC, is incremented by one.
    ***********************************************************************/
            case ANIn:
                A = A&ReadRAM(PC++);

                PSW = PSW&NCARRY&NAUX;
                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * CALLa - Call.
    * -----
    *
    *   If the value of the variable OPCODE is equal to CALLa, this routine
    * moves the high and low order bits of the next instruction address,
    * which is the value currently stored in the program counter, named PC,
    * to one minus and two minus the stack pointer register's location in
    * memory, RAM[SP - 1] and RAM[SP - 2], respectively.  The content of
    * the stack pointer register is decremented by two.  The program
    * counter is reset to the contents of byte three and byte two,
    * RAM[PC + 1] and RAM[PC], of the current instruction, respectively.
    ***********************************************************************/
            case CALLa:
                WriteRAM(SP - 1, (PC + 2)>>8);
                WriteRAM(SP - 2, (PC + 2)&0xFF);

                SP -= 2;

                PC = (ReadRAM(PC + 1)<<8) | ReadRAM(PC);
                switch (check_pc8080a ())
                {
                case 1: goto doret;
                case 2: return;
                default: break;
                }
                break;



    /***********************************************************************
    * C-condition-a - Conditional call.
    * -------------
    *
    *   If the value of the variable OPCODE is equal to CALLa, and the
    * condition is met, this routine moves the high and low order bits of
    * the next instruction address, which is the value currently stored in
    * the program counter, named PC, to one minus and two minus the stack
    * pointer register's location in memory, RAM[SP - 1] and RAM[SP - 2],
    * respectively.  The content of the stack pointer register is
    * decremented by two.  The program counter is reset to the contents
    * of byte three and byte two, RAM[PC + 1] and RAM[PC], of the current
    * instruction, respectively.  The number of cycles for this instruction
    * are incremented accordingly.
    *   If the condition is not met, the program counter, named PC, is
    * incremented by 2.  The program continues sequentially from this
    * point.
    *   The condition that must be met is based on the value of a flag set
    * in the processor status word.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include CCa, CMa, CNCa, CNZa, CPa, CPEa, CPOa, and CZa.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CCa, and the CARRY
    * flag is set in the PSW, this routine moves the location of the next
    * instruction to the top of the stack.  The program counter is set to
    * the address stored in the two bytes after the opcode in the 8080a's
    * ROM, with respect to the current program counter.  The number of
    * cycles is also adjusted.  If the CARRY flag is not set, the program
    * continues sequentially.
    ***********************************************************************/
            case CCa:
                if (PSW & CARRY)
                {
                    WriteRAM(SP - 1, (PC + 2)>>8);
                    WriteRAM(SP - 2, (PC + 2)&0xFF);

                    SP -= 2;

                    COUNTER += 6;

                    PC = (ReadRAM(PC + 1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                } else
                    PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CMa, and the SIGN
    * flag is set in the PSW, this routine moves the location of the next
    * instruction to the top of the stack.  The program counter is set to
    * the address stored in the two bytes after the opcode in the 8080a's
    * ROM, with respect to the current program counter.  The number of
    * cycles is also adjusted.  If the SIGN flag is not set, the program
    * continues sequentially.
    ***********************************************************************/
            case CMa:
                if (PSW & SIGN)
                {
                    WriteRAM(SP - 1, (PC + 2)>>8);
                    WriteRAM(SP - 2, (PC + 2)&0xFF);

                    SP -= 2;

                    COUNTER += 6;

                    PC = (ReadRAM(PC + 1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                } else
                    PC += 2;
                break;



    /***********************************************************************
    * CMA - Complement accumulator.
    * ---
    *
    *   If the value of the variable OPCODE is equal to CMA, this routine
    * complements the content of the accumulator.  The resulting value is
    * stored in the accumulator.
    ***********************************************************************/
            case CMA:
                A = ~A;
                break;



    /***********************************************************************
    * CMC - Complement carry.
    * ---
    *
    *   If the value of the variable OPCODE is equal to CMC, this routine
    * complements the flag CARRY in the program status word, named PSW.
    * The PSWs CARRY flag is set according to this operation.
    ***********************************************************************/
            case CMC:
                PSW = PSW&CARRY ? PSW&NCARRY  : PSW|CARRY;
                break;



    /***********************************************************************
    * CMPr - Compare register.
    * ----
    *
    *   If the value of the variable OPCODE is equal to CMPr, where r is an
    * 8-bit 8080a register, this routine compares the content of the
    * register, r, with the content of the accumulator.  The comparison is
    * performed by subtracting the accumulator from r.  The processor
    * status word, named PSW, is set according to the result of the
    * operation.  This operation is treated like a subtraction.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include ANAA, ANAB, ANAC, ANAD, ANAE, ANAH, and ANAL.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CMPA, this routine
    * resets the PSW to PSWSET, and sets the ZERO and PARITY flags.  This
    * is because the result of A - A is 0.
    ***********************************************************************/
            case CMPA:
                PSW = ZERO|PARITY|PSWSET;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CMPB, this routine
    * subtracts the accumulator from the B register.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case CMPB:
                PSW = A < BC.reg.B ? PSW|CARRY : PSW&NCARRY;
                PSW = (A&0xF) < (BC.reg.B&0xF)
                           ? PSW|AUX : PSW&NAUX;
                PSW = (A-BC.reg.B)==0
                           ? PSW|ZERO : PSW&NZERO;
                PSW = (A-BC.reg.B)>>7
                           ? PSW|SIGN : PSW&NSIGN;
                PSW = Parity8((A-BC.reg.B))
                           ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CMPC, this routine
    * subtracts the accumulator from the C register.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case CMPC:
                PSW = A < BC.reg.C ? PSW|CARRY : PSW&NCARRY;
                PSW = (A&0xF) < (BC.reg.C&0xF)
                           ? PSW|AUX : PSW&NAUX;
                PSW = (A-BC.reg.C)==0
                           ? PSW|ZERO : PSW&NZERO;
                PSW = (A-BC.reg.C)>>7
                           ? PSW|SIGN : PSW&NSIGN;
                PSW = Parity8((A-BC.reg.C))
                           ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CMPD, this routine
    * subtracts the accumulator from the D register.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case CMPD:
                PSW = A < DE.reg.D ? PSW|CARRY : PSW&NCARRY;
                PSW = (A&0xF) < (DE.reg.D&0xF)
                           ? PSW|AUX : PSW&NAUX;
                PSW = (A-DE.reg.D)==0
                           ? PSW|ZERO    : PSW&NZERO;
                PSW = (A-DE.reg.D)>>7
                           ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8((A-DE.reg.D))
                           ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CMPE, this routine
    * subtracts the accumulator from the E register.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case CMPE:
                PSW = A < DE.reg.E ? PSW|CARRY : PSW&NCARRY;
                PSW = (A&0xF) < (DE.reg.E&0xF)
                           ? PSW|AUX : PSW&NAUX;
                PSW = (A-DE.reg.E)==0
                           ? PSW|ZERO : PSW&NZERO;
                PSW = (A-DE.reg.E)>>7
                           ? PSW|SIGN : PSW&NSIGN;
                PSW = Parity8((A-DE.reg.E))
                           ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CMPH, this routine
    * subtracts the accumulator from the H register.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case CMPH:
                PSW = A < HL.reg.H ? PSW|CARRY : PSW&NCARRY;
                PSW = (A&0xF) < (HL.reg.H&0xF)
                           ? PSW|AUX : PSW&NAUX;
                PSW = (A-HL.reg.H)==0
                           ? PSW|ZERO    : PSW&NZERO;
                PSW = (A-HL.reg.H)>>7
                           ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8((A-HL.reg.H))
                           ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CMPL, this routine
    * subtracts the accumulator from the L register.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case CMPL:
                PSW = A < HL.reg.L ? PSW|CARRY : PSW&NCARRY;
                PSW = (A&0xF) < (HL.reg.L&0xF)
                           ? PSW|AUX : PSW&NAUX;
                PSW = (A-HL.reg.L)==0
                           ? PSW|ZERO : PSW&NZERO;
                PSW = (A-HL.reg.L)>>7
                           ? PSW|SIGN : PSW&NSIGN;
                PSW = Parity8((A-HL.reg.L))
                           ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * CMPM - Compare memory.
    * ----
    *
    *   If the value of the variable OPCODE is equal to CMPM, this routine
    * compares the content of the 8080a's memory, RAM[HL], with the
    * content of the accumulator.  The comparison is performed by
    * subtracting the accumulator from RAM[HL].  The processor status word,
    * named PSW, is set according to the result of the operation.  This
    * operation is treated like subtraction.
    ***********************************************************************/
            case CMPM:
                PSW = A < ReadRAM(HL.pair)
                        ? PSW|CARRY  : PSW&NCARRY;
                PSW = (A&0xF) < (ReadRAM(HL.pair)&0xF)
                        ? PSW|AUX    : PSW&NAUX;
                PSW = (A-ReadRAM(HL.pair))==0
                        ? PSW|ZERO   : PSW&NZERO;
                PSW = (A-ReadRAM(HL.pair))>>7
                        ? PSW|SIGN   : PSW&NSIGN;
                PSW = Parity8((A-ReadRAM(HL.pair)))
                        ? PSW|PARITY: PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CNCa, and the CARRY
    * flag is not set in the PSW, this routine moves the location of the
    * next instruction to the top of the stack.  The program counter is set
    * to the address stored in the two bytes after the opcode in the
    * 8080a's ROM, with respect to the current program counter.  The number
    * of cycles is also adjusted. If the SIGN flag is set, the program
    * continues sequentially.
    ***********************************************************************/
            case CNCa:
                if ((~PSW) & CARRY)
                {
                    WriteRAM(SP - 1, (PC + 2)>>8);
                    WriteRAM(SP - 2, (PC + 2)&0xFF);

                    SP -= 2;

                    PC = (ReadRAM(PC + 1)<<8) | ReadRAM(PC);

                    COUNTER += 6;
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                } else
                    PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CNZa, and the ZERO
    * flag is not set in the PSW, this routine moves the location of the
    * next instruction to the top of the stack.  The program counter is set
    * to the address stored in the two bytes after the opcode in the
    * 8080a's ROM, with respect to the current program counter.  The number
    * of cycles is also adjusted. If the ZERO flag is set, the program
    * continues sequentially.
    ***********************************************************************/
            case CNZa:
                if ((~PSW) & ZERO)
                {
                    WriteRAM(SP - 1, (PC + 2)>>8);
                    WriteRAM(SP - 2, (PC + 2)&0xFF);

                    SP -= 2;

                    COUNTER += 6;

                    PC = (ReadRAM(PC + 1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                } else
                    PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CPa, and the SIGN
    * flag is not set in the PSW, this routine moves the location of the
    * next instruction to the top of the stack.  The program counter is set
    * to the address stored in the two bytes after the opcode in the
    * 8080a's ROM, with respect to the current program counter.  The number
    * of cycles is also adjusted. If the SIGN flag is set, the program
    * continues sequentially.
    ***********************************************************************/
            case CPa:
                if ((~PSW) & SIGN)
                {
                    WriteRAM(SP - 1, (PC + 2)>>8);
                    WriteRAM(SP - 2, (PC + 2)&0xFF);

                    SP -= 2;

                    COUNTER += 6;

                    PC = (ReadRAM(PC + 1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                } else
                    PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CPEa, and the
    * PARITY flag is set in the PSW, this routine moves the location of the
    * next instruction to the top of the stack.  The program counter is set
    * to the address stored in the two bytes after the opcode in the
    * 8080a's ROM, with respect to the current program counter.  The number
    * of cycles is also adjusted. If the PARITY flag is not set, the
    * program continues sequentially.
    ***********************************************************************/
            case CPEa:
                if (PSW & PARITY)
                {
                    WriteRAM(SP - 1, (PC + 2)>>8);
                    WriteRAM(SP - 2, (PC + 2)&0xFF);

                    SP -= 2;

                    COUNTER += 6;

                    PC = (ReadRAM(PC + 1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                } else
                    PC += 2;
                break;



    /***********************************************************************
    * CPIn - Compare immediate.
    * ----
    *
    *   If the value of the variable OPCODE is equal to CMPM, this routine
    * compares the content of the 8080a's memory, RAM[PC], with the
    * content of the accumulator.  This routine performs the comparison 
    * by subtracting the accumulator from RAM[PC].  The processor status
    * word, named PSW, is set according to the result of the operation. 
    * This operation is treated like subtraction.
    ***********************************************************************/
            case CPIn:
                PSW = A < ReadRAM(PC) ? PSW|CARRY: PSW&NCARRY;
                PSW = (A&0xF) < (ReadRAM(PC)&0xF)
                        ? PSW|AUX    : PSW&NAUX;
                PSW = A == ReadRAM(PC)
                        ? PSW|ZERO   : PSW&NZERO;
                PSW = (A-ReadRAM(PC))>>7
                        ? PSW|SIGN   : PSW&NSIGN;
                PSW = Parity8((A-ReadRAM(PC)))
                        ? PSW|PARITY: PSW&NPARITY;
                PC ++;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CPOa, and the
    * PARITY flag is not set in the PSW, this routine moves the location of
    * the next instruction to the top of the stack.  The program counter is
    * set to the address stored in the two bytes after the opcode in the
    * 8080a's ROM, with respect to the current program counter.  The number
    * of cycles is also adjusted. If the PARITY flag is set, the program
    * continues sequentially.
    ***********************************************************************/
            case CPOa:
                if ((~PSW) & PARITY)
                {
                    WriteRAM(SP - 1, (PC + 2)>>8);
                    WriteRAM(SP - 2, (PC + 2)&0xFF);

                    SP -= 2;

                    COUNTER += 6;

                    PC = (ReadRAM(PC + 1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                } else
                    PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to CZa, and the ZERO
    * flag is set in the PSW, this routine moves the location of the
    * next instruction to the top of the stack.  The program counter is set
    * to the address stored in the two bytes after the opcode in the
    * 8080a's ROM, with respect to the current program counter.  The number
    * of cycles is also adjusted. If the ZERO flag is not set, the program
    * continues sequentially.
    ***********************************************************************/
            case CZa:
                if (PSW & ZERO)
                {
                    WriteRAM(SP - 1, (PC + 2)>>8);
                    WriteRAM(SP - 2, (PC + 2)&0xFF);

                    SP -= 2;

                    COUNTER += 6;

                    PC = (ReadRAM(PC + 1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                } else
                    PC += 2;
                break;



    /***********************************************************************
    * DAA - Decimal adjust accumulator.
    * ---
    *
    *   If the value of the variable OPCODE is equal to DAA, this routine
    * uses the content of the accumulator and creates a two digit decimal
    * number, where the higher order decimal digit is stored in the most
    * significant four bits of the accumulator and the lower order decimal
    * digit is stored in least significant four bits of the accumulator.
    * To obtain the lower order decimal number, the value of the four least
    * sifnificant bits must be tested.  If the value is above 0x09 or the
    * AUX flag is set in the PSW, the bits must be adjusted.  To adjust the
    * lower order bits, this routine adds 0x06 to the accumulator. To
    * obtain the higher order decimal number, the value of the four most
    * significant bits must be tested.  If the value is above 0x90, or the
    * CARRY flag is set in the PSW, the bits must be adjusted.  To adjust
    * the higher order bits, this routine adds 0x60 to the accumulator. 
    * The processor status word, named PSW, is set according to the result
    * of these operations.
    ***********************************************************************/
            case DAA:
                if (((A&0x0F) > 0x09) || (PSW&AUX))
                {
                    PSW = (A&0xF) + (0x06) > 0xF
                        ? PSW|AUX: PSW&NAUX;

                    A += 0x06;
                }

                if (((A&0xF0) > 0x90) || (PSW&CARRY))
                {
                    PSW = A + (0x60) > 0xFF
                        ? PSW|CARRY: PSW&NCARRY;

                    A += 0x60;
                }

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * DADrp - Add register pair to HL.
    * -----
    *
    *   If the value of the variable OPCODE is equal to DADrp, where rp is
    * a 16-bit 8080a register, this routine adds the content of the
    * register, rp, to the content of the HL register.  The resulting value
    * is stored in the HL register.  The processor status word's CARRY flag
    * is set if a 16-bit overflow occurs during the addition.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include DADB, DADD, DADH, and DADSP.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DADB, this routine
    * adds the contents of BC and HL to the HL register.  The PSW's CARRY
    * flag is set if an overflow occurs.
    ***********************************************************************/
            case DADB:
                PSW = HL.pair + BC.pair > 0xFFFF
                       ? PSW|CARRY   : PSW&NCARRY;

                HL.pair = (HL.pair + BC.pair)&0xFFFF;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DADD, this routine
    * adds the contents of DE and HL to the HL register.  The PSW's CARRY
    * flag is set if an overflow occurs.
    ***********************************************************************/
            case DADD:
                PSW = HL.pair + DE.pair > 0xFFFF
                       ? PSW|CARRY   : PSW&NCARRY;

                HL.pair = (HL.pair + DE.pair)&0xFFFF;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DADH, this routine
    * adds the contents of HL and HL to the HL register.  The PSW's CARRY
    * flag is set if an overflow occurs.
    ***********************************************************************/
            case DADH:
                PSW = HL.pair + HL.pair > 0xFFFF
                       ? PSW|CARRY   : PSW&NCARRY;

                HL.pair = (HL.pair + HL.pair)&0xFFFF;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DADSP, this routine
    * adds the contents of stack pointer, SP, and HL to the HL register.
    * The PSW's CARRY flag is set if an overflow occurs.
    ***********************************************************************/
            case DADSP:
                PSW = HL.pair + SP > 0xFFFF
                       ? PSW|CARRY   : PSW&NCARRY;

                HL.pair = (HL.pair + SP)&0xFFFF;
                break;



    /***********************************************************************
    * DCRr - Decrement register.
    * ----
    *
    *   If the value of the variable OPCODE is equal to DCRr, where r is an
    * 8-bit 8080a register, this routine decrements the content of the
    * register, r, by one.  The resulting value is stored in the register
    * r.  The processor status word, named PSW, is set according to the
    * result of the operation.  The CARRY flag is not affected.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include DCRA, DCRB, DCRC, DCRD, DCRE, DCRH, and DCRL.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DCRA, this routine
    * decrements the content of A and stores the result in the register A.
    * The PSW reflects the result of this decrement.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case DCRA:
                PSW = A&0xF ? PSW&NAUX   : PSW|AUX;

                A--;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DCRB, this routine
    * decrements the content of B and stores the result in the register B.
    * The PSW reflects the result of this decrement.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case DCRB:
                PSW = BC.reg.B&0xF ? PSW&NAUX   : PSW|AUX;

                BC.reg.B--;

                PSW = BC.reg.B==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = BC.reg.B>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(BC.reg.B) ? PSW|PARITY
                                : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DCRC, this routine
    * decrements the content of C and stores the result in the register C.
    * The PSW reflects the result of this decrement.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case DCRC:
                PSW = BC.reg.C&0xF ? PSW&NAUX   : PSW|AUX;

                BC.reg.C--;

                PSW = BC.reg.C==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = BC.reg.C>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(BC.reg.C) ? PSW|PARITY
                                : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DCRD, this routine
    * decrements the content of D and stores the result in the register D.
    * The PSW reflects the result of this decrement.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case DCRD:
                PSW = DE.reg.D&0xF ? PSW&NAUX   : PSW|AUX;

                DE.reg.D--;

                PSW = DE.reg.D==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = DE.reg.D>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(DE.reg.D) ? PSW|PARITY
                                : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DCRE, this routine
    * decrements the content of E and stores the result in the register E.
    * The PSW reflects the result of this decrement.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case DCRE:
                PSW = DE.reg.E&0xF ? PSW&NAUX   : PSW|AUX;

                DE.reg.E--;

                PSW = DE.reg.E==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = DE.reg.E>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(DE.reg.E) ? PSW|PARITY
                                : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DCRH, this routine
    * decrements the contents of H and stores the result in the register H.
    * The PSW reflects the result of this decrement.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case DCRH:
                PSW = HL.reg.H&0xF ? PSW&NAUX   : PSW|AUX;

                HL.reg.H--;

                PSW = HL.reg.H==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = HL.reg.H>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(HL.reg.H) ? PSW|PARITY
                                : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DCRL, this routine
    * decrements the content of L and stores the result in the register L.
    * The PSW reflects the result of this decrement.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case DCRL:
                PSW = HL.reg.L&0xF ? PSW&NAUX   : PSW|AUX;

                HL.reg.L--;

                PSW = HL.reg.L==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = HL.reg.L>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(HL.reg.L) ? PSW|PARITY
                                : PSW&NPARITY;
                break;



    /***********************************************************************
    * DCRM - Decrement memory.
    * ----
    *
    *   If the value of the variable OPCODE is equal to DCRM, this routine
    * decrements the content of the 8080a's memory, RAM[HL], by one.  The
    * resulting value is stored in the the 8080a's memory, RAM[HL].  The
    * processor status word, named PSW, is set according to the result of
    * the operation.  The CARRY flag is not affected.
    ***********************************************************************/
            case DCRM:
                PSW = ReadRAM(HL.pair)&0xF ? PSW&NAUX  :
                                 PSW|AUX;

                WriteRAM(HL.pair, ReadRAM(HL.pair) - 1);

                PSW = ReadRAM(HL.pair)==0 ? PSW|ZERO   :
                                PSW&NZERO;
                PSW = ReadRAM(HL.pair)>>7 ? PSW|SIGN   :
                                PSW&NSIGN;
                PSW = Parity8(ReadRAM(HL.pair)) ? PSW|PARITY:
                                PSW&NPARITY;
                break;



    /***********************************************************************
    * DCRrp - Decrement register pair.
    * -----
    *
    *   If the value of the variable OPCODE is equal to DCXrp, where rp is
    * a 16-bit 8080a register, this routine decrements the content of the
    * register, rp, by one.  The resulting value is stored in the register
    * rp.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include DCXB, DCXD, DCXH, and DCXSP.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DCXB, this routine
    * decrements the content of BC and stores the result in the register
    * BC.
    ***********************************************************************/
            case DCXB:
                BC.pair--;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DCXD, this routine
    * decrements the content of DE and stores the result in the register
    * DE.
    ***********************************************************************/
            case DCXD:
                DE.pair--;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DCXH, this routine
    * decrements the content of HL and stores the result in the register
    * HL.
    ***********************************************************************/
            case DCXH:
                HL.pair--;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to DCXSP, this routine
    * decrements the content of the stack pointer, SP, and stores the
    * result in the stack pointer, SP.
    ***********************************************************************/
            case DCXSP:
                SP--;
                break;



    /***********************************************************************
    * DI - Disable interrupts.
    * --
    *
    *   If the value of the variable OPCODE is equal to DI, interrupts are
    * disabled.  To perform this operation, INTERRUPT is set to 0, which
    * sets the current interrupt state to none.  It also sets INTDISABLE to
    * 0, which disables all timer based interrupts. The only way an
    * interrupt can occur after this opcode is executed is if an EI is
    * executed.
    ***********************************************************************/
            case DI:
                INTERRUPT = 0;

                INTDISABLE = 1;
                break;



    /***********************************************************************
    * EI - Enable interrupts.
    * --
    *
    *   If the value of the variable OPCODE is equal to EI, interrupts are
    * enabled.  To perform this operation, INTERRUPT is set to 1, which
    * sets the current interrupt state to on.  It also sets INTDISABLE to
    * 2, which enables all timer based interrupts after the instruction
    * following this instruction.
    ***********************************************************************/
            case EI:
                INTERRUPT = 1;

                INTDISABLE = 2;
                break;



    /***********************************************************************
    * HLT - Halt.
    * ----
    *
    *   If the value of the variable OPCODE is equal to HLT, the 8080a
    * processor "halts".  This operation is treated like a NMI.
    ***********************************************************************/
            case HLT:
            #ifdef VERBOSE
                printf("<*> HALT <*>\n");
            #endif

                INTERRUPT = 2;
                COUNTER = 0;

                OPCODE = RST2;
                break;



    /***********************************************************************
    * INp - Input.
    * ---
    *
    *   If the value of the variable OPCODE is equal to INp, where p is
    * the content of the 8080a's memory RAM[PC], this routine retrieves
    * data from the input device requested.  The program counter, named PC,
    * is incremented by one.
    ***********************************************************************/
            case INp:
                A = input8080a(ReadRAM(PC++));
                break;



    /***********************************************************************
    * INRr - Increment register.
    * ----
    *
    *   If the value of the variable OPCODE is equal to INRr, where r is an
    * 8-bit 8080a register, this routine increments the content of the
    * register, r, by one.  The resulting value is stored in the register
    * r.  The processor status word, named PSW, is set according to the
    * result of the operation.  The CARRY flag is not affected.  
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include INRA, INRB, INRC, INRD, INRE, INRH, and INRL.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to INRA, this routine
    * increments the content of A and stores the result in the register A.
    * The PSW reflects the result of this increment.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case INRA:
                A ++;

                PSW = A&0xF ? PSW&NAUX   : PSW|AUX;
                PSW = A==0  ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7  ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A)  ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to INRB, this routine
    * increments the content of B and stores the result in the register B.
    * The PSW reflects the result of this increment.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case INRB:
                BC.reg.B ++;

                PSW = BC.reg.B&0xF ? PSW&NAUX   : PSW|AUX;
                PSW = BC.reg.B==0  ? PSW|ZERO   : PSW&NZERO;
                PSW = BC.reg.B>>7  ? PSW|SIGN   : PSW&NSIGN;
                PSW = Parity8(BC.reg.B)  ? PSW|PARITY
                                : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to INRC, this routine
    * increments the content of C and stores the result in the register C.
    * The PSW reflects the result of this increment.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case INRC:
                BC.reg.C ++;

                PSW = BC.reg.C&0xF ? PSW&NAUX   : PSW|AUX;
                PSW = BC.reg.C==0  ? PSW|ZERO   : PSW&NZERO;
                PSW = BC.reg.C>>7  ? PSW|SIGN   : PSW&NSIGN;
                PSW = Parity8(BC.reg.C)  ? PSW|PARITY
                                : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to INRD, this routine
    * increments the content of D and stores the result in the register D.
    * The PSW reflects the result of this increment.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case INRD:
                DE.reg.D ++;

                PSW = DE.reg.D&0xF ? PSW&NAUX   : PSW|AUX;
                PSW = DE.reg.D==0  ? PSW|ZERO   : PSW&NZERO;
                PSW = DE.reg.D>>7  ? PSW|SIGN   : PSW&NSIGN;
                PSW = Parity8(DE.reg.D)  ? PSW|PARITY
                                : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to INRE, this routine
    * increments the content of E and stores the result in the register E.
    * The PSW reflects the result of this increment.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case INRE:
                DE.reg.E ++;

                PSW = DE.reg.E&0xF ? PSW&NAUX   : PSW|AUX;
                PSW = DE.reg.E==0  ? PSW|ZERO   : PSW&NZERO;
                PSW = DE.reg.E>>7  ? PSW|SIGN   : PSW&NSIGN;
                PSW = Parity8(DE.reg.E)  ? PSW|PARITY
                                : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to INRH, this routine
    * increments the content of H and stores the result in the register H.
    * The PSW reflects the result of this increment.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case INRH:
                HL.reg.H ++;

                PSW = HL.reg.H&0xF ? PSW&NAUX   : PSW|AUX;
                PSW = HL.reg.H==0  ? PSW|ZERO   : PSW&NZERO;
                PSW = HL.reg.H>>7  ? PSW|SIGN   : PSW&NSIGN;
                PSW = Parity8(HL.reg.H)  ? PSW|PARITY
                                : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to INRL, this routine
    * increments the content of L and stores the result in the register L.
    * The PSW reflects the result of this increment.  The CARRY flag is not
    * affected.
    ***********************************************************************/
            case INRL:
                HL.reg.L ++;

                PSW = HL.reg.L&0xF ? PSW&NAUX   : PSW|AUX;
                PSW = HL.reg.L==0  ? PSW|ZERO   : PSW&NZERO;
                PSW = HL.reg.L>>7  ? PSW|SIGN   : PSW&NSIGN;
                PSW = Parity8(HL.reg.L)  ? PSW|PARITY
                                : PSW&NPARITY;
                break;



    /***********************************************************************
    * INRM - Increment memory.
    * ----
    *
    *   If the value of the variable OPCODE is equal to INRM, this routine
    * increments the content of the 8080a's memory, RAM[HL], by one.  The
    * resulting value is stored in the the 8080a's memory, RAM[HL].  The
    * processor status word, named PSW, is set according to the result of
    * the operation.  The CARRY flag is not affected.
    ***********************************************************************/
            case INRM:
                WriteRAM(HL.pair, ReadRAM(HL.pair) + 1);

                PSW = ReadRAM(HL.pair)&0xF ? PSW&NAUX :
                                 PSW|AUX;
                PSW = ReadRAM(HL.pair)==0  ? PSW|ZERO :
                                 PSW&NZERO;
                PSW = ReadRAM(HL.pair)>>7  ? PSW|SIGN :
                                 PSW&NSIGN;
                PSW = Parity8(ReadRAM(HL.pair))
                        ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * INXrp - Increment register pair.
    * -----
    *
    *   If the value of the variable OPCODE is equal to INXrp, where rp is
    * a 16-bit 8080a register, this routine increments the content of the
    * register, rp, by one.  The resulting value is stored in the register
    * rp.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include INXB, INXD, INXH, and INXSP.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to INXB, this routine
    * increments the content of BC and stores the result in the register
    * BC.
    ***********************************************************************/
            case INXB:
                BC.pair++;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to INXD, this routine
    * increments the content of DE and stores the result in the register
    * DE.
    ***********************************************************************/
            case INXD:
                DE.pair++;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to INXH, this routine
    * increments the content of HL and stores the result in the register
    * HL.
    ***********************************************************************/
            case INXH:
                HL.pair++;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to INXSP, this routine
    * increments the content of stack pointer, SP, and stores the result in
    * the stack pointer, SP.
    ***********************************************************************/
            case INXSP:
                SP++;
                break;



    /***********************************************************************
    * JMPa - Jump.
    * ----
    *
    *   If the value of the variable OPCODE is equal to JUMPa, this routine
    * sets the program counter to the contents of byte three and byte two,
    * RAM[PC + 1] and RAM[PC], of the current instruction.
    ***********************************************************************/
            case JMPa:
                PC = (ReadRAM(PC+1)<<8) | ReadRAM(PC);
                switch (check_pc8080a ())
                {
                case 1: goto doret;
                case 2: return;
                default: break;
                }
                break;



    /***********************************************************************
    * J-condition-a - Conditional jump.
    * -------------
    *
    *   If the value of the variable OPCODE is equal to JUMPa, and the
    * condition is met, this routine sets the program counter to the
    * contents of byte three and byte two, RAM[PC + 1] and RAM[PC], of the
    * current instruction. If the condition is not met, the program
    * counter, named PC, is incremented by 2.  The program continues
    * sequentially from this point. The condition that must be met is based
    * on the value of a flag set in the processor status word.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include JCa, JMa, JNCa, JNZa, JPa, JPEa, JPOa, and JZa.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to JCa, and the CARRY
    * flag is set in the PSW, this routine sets the program counter to the
    * address stored in the two bytes after the opcode in the 8080a's ROM,
    * with respect to the current program counter.  If the CARRY flag is
    * not set, the program continues sequentially.
    ***********************************************************************/
            case JCa:
                if (PSW & CARRY)
                {
                    PC = (ReadRAM(PC+1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }
                else
                    PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to JMa, and the MINUS
    * flag is set in the PSW, this routine sets the program counter to the
    * address stored in the two bytes after the opcode in the 8080a's ROM,
    * with respect to the current program counter.  If the MINUS flag is
    * not set, the program continues sequentially.
    ***********************************************************************/
            case JMa:
                if (PSW & SIGN)
                {
                    PC = (ReadRAM(PC+1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }
                else
                    PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to JNCa, and the CARRY
    * flag is not set in the PSW, this routine sets the program counter to
    * the address stored in the two bytes after the opcode in the 8080a's
    * ROM, with respect to the current program counter.  If the CARRY flag
    * is set, the program continues sequentially.
    ***********************************************************************/
            case JNCa:
                if ((~PSW) & CARRY)
                {
                    PC = (ReadRAM(PC+1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }
                else
                    PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to JNZa, and the ZERO
    * flag is not set in the PSW, this routine sets the program counter to
    * the address stored in the two bytes after the opcode in the 8080a's
    * ROM, with respect to the current program counter.  If the ZERO flag
    * is set, the program continues sequentially.
    ***********************************************************************/
            case JNZa:
                if ((~PSW) & ZERO)
                {
                    PC = (ReadRAM(PC+1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }
                else
                    PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to JPa, and the SIGN
    * flag is not set in the PSW, this routine sets the program counter to
    * the address stored in the two bytes after the opcode in the 8080a's
    * ROM, with respect to the current program counter.  If the SIGN flag
    * is set, the program continues sequentially.
    ***********************************************************************/
            case JPa:
                if ((~PSW) & SIGN)
                {
                    PC = (ReadRAM(PC+1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }
                else
                    PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to JPEa, and the
    * PARITY flag is set in the PSW, this routine sets the program counter
    * to the address stored in the two bytes after the opcode in the
    * 8080a's ROM, with respect to the current program counter.  If the
    * CARRY flag is not set, the program continues sequentially.
    ***********************************************************************/
            case JPEa:
                if (PSW & PARITY)
                {
                    PC = (ReadRAM(PC+1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }
                else
                    PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to JPOa, and the
    * PARITY flag is not set in the PSW, this routine sets the program
    * counter to the address stored in the two bytes after the opcode in
    * the 8080a's ROM, with respect to the current program counter.  If the
    * CARRY flag is set, the program continues sequentially.
    ***********************************************************************/
            case JPOa:
                if ((~PSW) & PARITY)
                {
                    PC = (ReadRAM(PC+1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }
                else
                    PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to JZa, and the ZERO
    * flag is set in the PSW, this routine sets the program counter to
    * the address stored in the two bytes after the opcode in the 8080a's
    * ROM, with respect to the current program counter.  If the CARRY flag
    * is not set, the program continues sequentially.
    ***********************************************************************/
            case JZa:
                if (PSW & ZERO)
                {
                    PC = (ReadRAM(PC+1)<<8) | ReadRAM(PC);
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }
                else
                    PC += 2;
                break;



    /***********************************************************************
    * LDAa - Load accumulator direct.
    * ----
    *
    *   If the value of the variable OPCODE is equal to LDAa, this routine
    * sets the accumulator to the content of the 8080a's memory at the
    * location where the high order byte is RAM[PC + 1] and the low order
    * byte is RAM[PC].  The program counter, named PC, is incremented by 2.
    ***********************************************************************/
            case LDAa:
                A = ReadRAM((ReadRAM(PC + 1)<<8) | ReadRAM(PC));
                PC += 2;
                break;



    /***********************************************************************
    * LDAXrp - Load accumulator indirect.
    * ------
    *
    *   If the value of the variable OPCODE is equal to LDAXrp, where rp is
    * a 16-bit 8080a register, this routine sets the accumulator equal to
    * the content of the 8080a's memory at BC.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include LDAXB and LDAXD.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to LDAXB, this routine
    * sets the accumulator to the 8080a's RAM at BC.
    ***********************************************************************/
            case LDAXB:
                A = ReadRAM(BC.pair);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to LDAXB, this routine
    * sets the accumulator to the 8080a's RAM at BC.
    ***********************************************************************/
            case LDAXD:
                A = ReadRAM(DE.pair);
                break;



    /***********************************************************************
    * LHLDa - Load HL direct.
    * -----
    *
    *   If the value of the variable OPCODE is equal to LHLDa, this routine
    * sets the 8-bit H register to the content of the 8080a's memory at the
    * location one plus the high order byte and low order byte, RAM[PC + 1]
    * and RAM[PC], respectively.  It also sets the L register to the
    * content of the 8080a's memory at the location where the high order
    * byte is RAM[PC + 1] and low order byte is RAM[PC].  The program
    * counter, named PC, is incremented by 2.
    ***********************************************************************/
            case LHLDa:
                HL.reg.H = ReadRAM(((ReadRAM(PC + 1) << 8) |
                        ReadRAM(PC)) + 1);
                HL.reg.L = ReadRAM((ReadRAM(PC + 1) << 8)  |
                        ReadRAM(PC));

                PC += 2;
                break;



    /***********************************************************************
    * LXI-rp-nn - Load register pair immediate.
    * ---------
    *
    *   If the value of the variable OPCODE is equal to LXIrpnn, where rp
    * is a 16-bit 8080a register and nn is a 16-bit address, this routine
    * moves the contents of RAM[PC+1] and RAM[PC] into the register pair,
    * rp, where RAM[PC+1] is the high order bit and RAM[PC] is the low
    * order bit.  The content of the program counter, named PC, is
    * incremented by 2.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include LXIBnn, LXIDnn, LXIHnn, and LXISPnn.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to LXIBnn, this
    * routine moves the contents of RAM[PC+1] and RAM[PC] into the register
    * BC.  PC is incremented by 2.
    ***********************************************************************/
            case LXIBnn:
                BC.reg.B = ReadRAM(PC + 1);
                BC.reg.C = ReadRAM(PC);

                PC += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to LXIDnn, this
    * routine moves the contents of RAM[PC+1] and RAM[PC] into the register
    * DE.  PC is incremented by 2.
    ***********************************************************************/
            case LXIDnn:
                DE.reg.D = ReadRAM(PC + 1);
                DE.reg.E = ReadRAM(PC);

                PC += 2; 
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to LXIHnn, this
    * routine moves the contents of RAM[PC+1] and RAM[PC] into the register
    * HL.  PC is incremented by 2.
    ***********************************************************************/
            case LXIHnn:
                HL.reg.H = ReadRAM(PC + 1);
                HL.reg.L = ReadRAM(PC);

                PC += 2; 
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to LXISPnn, this
    * routine moves the contents of RAM[PC+1] and RAM[PC] into the stack
    * pointer, SP.  PC is incremented by 2.
    ***********************************************************************/
            case LXISPnn:
                SP = (ReadRAM(PC + 1) << 8) | ReadRAM(PC);

                PC += 2;
                break;



    /***********************************************************************
    * MOV-r1-r2 - Move register.
    * ---------
    *
    *   If the value of the variable OPCODE is equal to MOVr1r2, where r1
    * and r2 are 8-bit registers, this routine moves the content of
    * register r2 into register r1.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include MOVAA, MOVAB, MOVAC, MOVAD, MOVAE, MOVAH, MOVAL,
    * MOVBA, MOVBB, MOVBC, MOVBD, MOVBE, MOVBH, MOVBL, MOVCA, MOVCB, MOVCC,
    * MOVCD, MOVCE, MOVCH, MOVCL, MOVDA, MOVDB, MOVDC, MOVDD, MOVDE, MOVDH,
    * MOVDL, MOVEA, MOVEB, MOVEC, MOVED, MOVEE, MOVEH, MOVEL, MOVHA, MOVHB,
    * MOVHC, MOVHD, MOVHE, MOVHH, MOVHL, MOVLA, MOVLB, MOVLC, MOVLD, MOVLE,
    * MOVLH, and MOVLL.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVAA, the content
    * of register A is moved into register A.
    ***********************************************************************/
            case MOVAA:
                //A = A;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVAB, the content
    * of register B is moved into register A.
    ***********************************************************************/
            case MOVAB:
                A = BC.reg.B;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVAC, the content
    * of register C is moved into register A.
    ***********************************************************************/
            case MOVAC:
                A = BC.reg.C;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVAD, the content
    * of register D is moved into register A.
    ***********************************************************************/
            case MOVAD:
                A = DE.reg.D;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVAE, the content
    * of register E is moved into register A.
    ***********************************************************************/
            case MOVAE:
                A = DE.reg.E;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVAH, the content
    * of register H is moved into register A.
    ***********************************************************************/
            case MOVAH:
                A = HL.reg.H;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVAL, the content
    * of register L is moved into register A.
    ***********************************************************************/
            case MOVAL:
                A = HL.reg.L;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVBA, the content
    * of register A is moved into register B.
    ***********************************************************************/
            case MOVBA:
                BC.reg.B = A;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVBB, the content
    * of register B is moved into register B.
    ***********************************************************************/
            case MOVBB:
                BC.reg.B = BC.reg.B;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVBC, the content
    * of register C is moved into register B.
    ***********************************************************************/
            case MOVBC:
                BC.reg.B = BC.reg.C;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVBD, the content
    * of register D is moved into register B.
    ***********************************************************************/
            case MOVBD:
                BC.reg.B = DE.reg.D;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVBE, the content
    * of register E is moved into register B.
    ***********************************************************************/
            case MOVBE:
                BC.reg.B = DE.reg.E;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVBH, the content
    * of register H is moved into register B.
    ***********************************************************************/
            case MOVBH:
                BC.reg.B = HL.reg.H;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVBL, the content
    * of register L is moved into register B.
    ***********************************************************************/
            case MOVBL:
                BC.reg.B = HL.reg.L;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVCA, the content
    * of register A is moved into register C.
    ***********************************************************************/
            case MOVCA:
                BC.reg.C = A;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVCB, the content
    * of register B is moved into register C.
    ***********************************************************************/
            case MOVCB:
                BC.reg.C = BC.reg.B;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVCC, the content
    * of register C is moved into register C.
    ***********************************************************************/
            case MOVCC:
                BC.reg.C = BC.reg.C;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVCD, the content
    * of register D is moved into register C.
    ***********************************************************************/
            case MOVCD:
                BC.reg.C = DE.reg.D;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVCE, the content
    * of register E is moved into register C.
    ***********************************************************************/
            case MOVCE:
                BC.reg.C = DE.reg.E;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVCH, the content
    * of register H is moved into register C.
    ***********************************************************************/
            case MOVCH:
                BC.reg.C = HL.reg.H;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVCL, the content
    * of register L is moved into register C.
    ***********************************************************************/
            case MOVCL:
                BC.reg.C = HL.reg.L;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVDA, the content
    * of register A is moved into register D.
    ***********************************************************************/
            case MOVDA:
                DE.reg.D = A;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVDB, the content
    * of register B is moved into register D.
    ***********************************************************************/
            case MOVDB:
                DE.reg.D = BC.reg.B;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVDC, the content
    * of register C is moved into register D.
    ***********************************************************************/
            case MOVDC:
                DE.reg.D = BC.reg.C;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVDD, the content
    * of register D is moved into register D.
    ***********************************************************************/
            case MOVDD:
                DE.reg.D = DE.reg.D;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVDE, the content
    * of register E is moved into register D.
    ***********************************************************************/
            case MOVDE:
                DE.reg.D = DE.reg.E;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVDH, the content
    * of register H is moved into register D.
    ***********************************************************************/
            case MOVDH:
                DE.reg.D = HL.reg.H;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVDL, the content
    * of register L is moved into register D.
    ***********************************************************************/
            case MOVDL:
                DE.reg.D = HL.reg.L;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVEA, the content
    * of register A is moved into register E.
    ***********************************************************************/
            case MOVEA:
                DE.reg.E = A;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVEB, the content
    * of register B is moved into register E.
    ***********************************************************************/
            case MOVEB:
                DE.reg.E = BC.reg.B;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVEC, the content
    * of register C is moved into register E.
    ***********************************************************************/
            case MOVEC:
                DE.reg.E = BC.reg.C;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVED, the content
    * of register D is moved into register E.
    ***********************************************************************/
            case MOVED:
                DE.reg.E = DE.reg.D;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVEE, the content
    * of register E is moved into register E.
    ***********************************************************************/
            case MOVEE:
                DE.reg.E = DE.reg.E;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVEH, the content
    * of register H is moved into register E.
    ***********************************************************************/
            case MOVEH:
                DE.reg.E = HL.reg.H;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVEL, the content
    * of register L is moved into register E.
    ***********************************************************************/
            case MOVEL:
                DE.reg.E = HL.reg.L;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVHA, the content
    * of register A is moved into register H.
    ***********************************************************************/
            case MOVHA:
                HL.reg.H = A;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVHB, the content
    * of register B is moved into register H.
    ***********************************************************************/
            case MOVHB:
                HL.reg.H = BC.reg.B;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVHC, the content
    * of register C is moved into register H.
    ***********************************************************************/
            case MOVHC:
                HL.reg.H = BC.reg.C;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVHD, the content
    * of register D is moved into register H.
    ***********************************************************************/
            case MOVHD:
                HL.reg.H = DE.reg.D;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVHE, the content
    * of register E is moved into register H.
    ***********************************************************************/
            case MOVHE:
                HL.reg.H = DE.reg.E;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVHH, the content
    * of register H is moved into register H.
    ***********************************************************************/
            case MOVHH:
                HL.reg.H = HL.reg.H;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVHL, the content
    * of register L is moved into register H.
    ***********************************************************************/
            case MOVHL:
                HL.reg.H = HL.reg.L;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVLA, the content
    * of register A is moved into register L.
    ***********************************************************************/
            case MOVLA:
                HL.reg.L = A;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVLB, the content
    * of register B is moved into register L.
    ***********************************************************************/
            case MOVLB:
                HL.reg.L = BC.reg.B;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVLC, the content
    * of register C is moved into register L.
    ***********************************************************************/
            case MOVLC:
                HL.reg.L = BC.reg.C;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVLD, the content
    * of register D is moved into register L.
    ***********************************************************************/
            case MOVLD:
                HL.reg.L = DE.reg.D;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVLE, the content
    * of register E is moved into register L.
    ***********************************************************************/
            case MOVLE:
                HL.reg.L = DE.reg.E;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVLH, the content
    * of register H is moved into register L.
    ***********************************************************************/
            case MOVLH:
                HL.reg.L = HL.reg.H;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVLL, the content
    * of register L is moved into register L.
    ***********************************************************************/
            case MOVLL:
                HL.reg.L = HL.reg.L;
                break;



    /***********************************************************************
    * MOVMr - Move register to memory.
    * -----
    *
    *   If the value of the variable OPCODE is equal to MOVMr, where r
    * is an 8-bit register, this routine moves the content of register r to
    * the 8080a's memory at RAM[HL].
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include MOVMA, MOVMB, MOVMC, MOVMD, MOVME, MOVMH, and
    * MOVML.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVMA, the content
    * of register A is moved to RAM[HL].
    ***********************************************************************/
            case MOVMA:
                WriteRAM(HL.pair, A);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVMB, the content
    * of register B is moved to RAM[HL].
    ***********************************************************************/
            case MOVMB:
                WriteRAM(HL.pair, BC.reg.B);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVMC, the content
    * of register C is moved to RAM[HL].
    ***********************************************************************/
            case MOVMC:
                WriteRAM(HL.pair, BC.reg.C);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVMD, the content
    * of register D is moved to RAM[HL].
    ***********************************************************************/
            case MOVMD:
                WriteRAM(HL.pair, DE.reg.D);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVME, the content
    * of register E is moved to RAM[HL].
    ***********************************************************************/
            case MOVME:
                WriteRAM(HL.pair, DE.reg.E);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVMH, the content
    * of register G is moved to RAM[HL].
    ***********************************************************************/
            case MOVMH:
                WriteRAM(HL.pair, HL.reg.H);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVML, the content
    * of register L is moved to RAM[HL].
    ***********************************************************************/
            case MOVML:
                WriteRAM(HL.pair, HL.reg.L);
                break;



    /***********************************************************************
    * MOVrM - Move memory to register.
    * -----
    *
    *   If the value of the variable OPCODE is equal to MOVrM, where r
    * is an 8-bit register, this routine moves the content of the 8080a's
    * memory at RAM[HL] to register r.  
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include MOVAM, MOVBM, MOVCM, MOVDM, MOVEM, MOVHM, and
    * MOVLM.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVAM, the content
    * of RAM[HL] is moved to register A.
    ***********************************************************************/
            case MOVAM:
                A = ReadRAM(HL.pair);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVBM, the content
    * of RAM[HL] is moved to register B.
    ***********************************************************************/
            case MOVBM:
                BC.reg.B = ReadRAM(HL.pair);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVCM, the content
    * of RAM[HL] is moved to register C.
    ***********************************************************************/
            case MOVCM:
                BC.reg.C = ReadRAM(HL.pair);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVDM, the content
    * of RAM[HL] is moved to register D.
    ***********************************************************************/
            case MOVDM:
                DE.reg.D = ReadRAM(HL.pair);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVEM, the content
    * of RAM[HL] is moved to register E.
    ***********************************************************************/
            case MOVEM:
                DE.reg.E = ReadRAM(HL.pair);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVHM, the content
    * of RAM[HL] is moved to register H.
    ***********************************************************************/
            case MOVHM:
                HL.reg.H = ReadRAM(HL.pair);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MOVLM, the content
    * of RAM[HL] is moved to register L.
    ***********************************************************************/
            case MOVLM:
                HL.reg.L = ReadRAM(HL.pair);
                break;



    /***********************************************************************
    * MOVrn - Move immediate to register.
    * -----
    *
    *   If the value of the variable OPCODE is equal to MOVrn, where r
    * is an 8-bit register, this routine moves the content of the 8080a's
    * memory at RAM[PC] to register r.  The program counter, named PC, is
    * incremented by one. 
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include MVIAn, MVIBn, MVICn, MVIDn, MVIEn, MVIHn, and
    * MVILn.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MVIAn, the content
    * of RAM[PC] is moved to register A.  PC is incremented by one.
    ***********************************************************************/
            case MVIAn:
                A = ReadRAM(PC++);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MVIBn, the content
    * of RAM[PC] is moved to register B.  PC is incremented by one.
    ***********************************************************************/
            case MVIBn:
                BC.reg.B = ReadRAM(PC++);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MVICn, the content
    * of RAM[PC] is moved to register C.  PC is incremented by one.
    ***********************************************************************/
            case MVICn:
                BC.reg.C = ReadRAM(PC++);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MVIDn, the content
    * of RAM[PC] is moved to register D.  PC is incremented by one.
    ***********************************************************************/
            case MVIDn:
                DE.reg.D = ReadRAM(PC++);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MVIEn, the content
    * of RAM[PC] is moved to register E.  PC is incremented by one.
    ***********************************************************************/
            case MVIEn:
                DE.reg.E = ReadRAM(PC++);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MVIHn, the content
    * of RAM[PC] is moved to register H.  PC is incremented by one.
    ***********************************************************************/
            case MVIHn:
                HL.reg.H = ReadRAM(PC++);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to MVILn, the content
    * of RAM[PC] is moved to register L.  PC is incremented by one.
    ***********************************************************************/
            case MVILn:
                HL.reg.L = ReadRAM(PC++);
                break;



    /***********************************************************************
    * MOVMn - Move immediate to memory.
    * -----
    *
    *   If the value of the variable OPCODE is equal to MOVMn, this routine
    * moves the content of the 8080a's memory, RAM[PC], to the content of
    * the 8080a's mmemory at RAM[HL].  The program counter, named PC, is
    * incremented by one.
    ***********************************************************************/
            case MVIMn:
                WriteRAM(HL.pair, ReadRAM(PC++));
                break;



    /***********************************************************************
    * NOP - No operation.
    * ---
    *
    *   If the value of the variable OPCODE is equal to NOP, this routine
    * does nothing.  There is no operation performed.
    ***********************************************************************/
            case NOP:
                /* NO OPERATION */
                break;



    /***********************************************************************
    * ORAr - OR register.
    * ----
    *
    *   If the value of the variable OPCODE is equal to ORAr, where r is an
    * 8-bit 8080a register, this routine ORs the content of the register,
    * r, with the content of the accumulator.  The resulting value is
    * stored in the accumulator.  The processor status word, named PSW, is
    * set according to the result of the operation.  The CARRY and AUX
    * flags are cleared in the PSW.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include ORAA, ORAB, ORAC, ORAD, ORAE, ORAH, and ORAL.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ORAA, this routine
    * ORs the content of A with the accumulator to the accumulator.  The
    * PSW reflects the result of this OR operation.  The CARRY and AUX are
    * flags cleared.
    ***********************************************************************/
            case ORAA:
                PSW = PSW&NCARRY&NAUX;

                A = A | A;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ORAB, this routine
    * ORs the content of B with the accumulator to the accumulator.  The
    * PSW reflects the result of this OR operation.  The CARRY and AUX
    * flags are cleared.
    ***********************************************************************/
            case ORAB:
                PSW = PSW&NCARRY&NAUX;
                A = A | BC.reg.B;
                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ORAC, this routine
    * ORs the content of C with the accumulator to the accumulator.  The
    * PSW reflects the result of this OR operation.  The CARRY and AUX
    * flags are cleared.
    ***********************************************************************/
            case ORAC:
                PSW = PSW&NCARRY&NAUX;

                A = A | BC.reg.C;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ORAD, this routine
    * ORs the content of D with the accumulator to the accumulator.  The
    * PSW reflects the result of this OR operation.  The CARRY and AUX
    * flags are cleared.
    ***********************************************************************/
            case ORAD:
                PSW = PSW&NCARRY&NAUX;

                A = A | DE.reg.D;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ORAE, this routine
    * ORs the content of E with the accumulator to the accumulator.  The
    * PSW reflects the result of this OR operation.  The CARRY and AUX
    * flags are cleared.
    ***********************************************************************/
            case ORAE:
                PSW = PSW&NCARRY&NAUX;

                A = A | DE.reg.E;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ORAH, this routine
    * ORs the content of H with the accumulator to the accumulator.  The
    * PSW reflects the result of this OR operation.  The CARRY and AUX
    * flags are cleared.
    ***********************************************************************/
            case ORAH:
                PSW = PSW&NCARRY&NAUX;

                A = A | HL.reg.H;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to ORAL, this routine
    * ORs the content of L with the accumulator to the accumulator.  The
    * PSW reflects the result of this OR operation.  The CARRY and AUX
    * flags are cleared.
    ***********************************************************************/
            case ORAL:
                PSW = PSW&NCARRY&NAUX;

                A = A | HL.reg.L;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * ORAM - OR memory.
    * ----
    *
    *   If the value of the variable OPCODE is equal to ORAM, this routine
    * ORs the content of the 8080a's memory, RAM[HL], with the content
    * of the accumulator.  The resulting value is stored in the
    * accumulator.  The processor status word, named PSW, is set according
    * to the result of the operation.  The CARRY and AUX flags are cleared
    * in the PSW.
    ***********************************************************************/
            case ORAM:
                PSW = PSW&NCARRY&NAUX;

                A = A | ReadRAM(HL.pair);

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * ORIn - OR immediate.
    * ----
    *
    *   If the value of the variable OPCODE is equal to ORIn, this routine
    * ORs the content of the 8080a's memory, RAM[PC] with the content of
    * the accumulator.  The resulting value is stored in the accumulator.
    * The processor status word, named PSW, is set according to the result
    * of the operation.  The CARRY and AUX flags are cleared in the PSW.
    * The program counter, named PC, is incremented by one.
    ***********************************************************************/
            case ORIn:
                PSW = PSW&NCARRY&NAUX;

                A = A | ReadRAM(PC++);

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * OUTp - Output.
    * ----
    *
    *   If the value of the variable OPCODE is equal to OUTp, where p is
    * the content of the 8080a's memory RAM[PC], this routine sends data
    * to the output device requested.  The program counter, named PC, is
    * incremented by one.
    ***********************************************************************/
            case OUTp:
                output8080a (ReadRAM(PC++), A);
                break;



    /***********************************************************************
    * PCHL - Move HL to PC.
    * ----
    *
    *   If the value of the variable OPCODE is equal to PCHL, this routine
    * moves the content of the HL register to the program counter, named
    * PC.
    ***********************************************************************/
            case PCHL:
                PC = HL.pair;
                switch (check_pc8080a ())
                {
                case 1: goto doret;
                case 2: return;
                default: break;
                }
                break;



    /***********************************************************************
    * POPrp - Pop register pair.
    * -----
    *
    *   If the value of the variable OPCODE is equal to POPrp, where rp is
    * a 16-bit 8080a register, this routine moves the content of the
    * top of the stack to register rp.  The content of the stack pointer, 
    * named SP, is incremented by 2. 
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include POPB, POPD, and POPH.
    *   The second register in the register pair is on the top of the
    * stack, followed by the first register of the register pair.  Example:
    * C is on the top of the stack, if C is popped off, B is on the top of
    * the stack.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to POPB, this routine
    * moves the contents of the 8080a's stack at SP and SP+1 to registers
    * C and B, respectively.  The stack pointer is incremented by 2.
    ***********************************************************************/
            case POPB:
                BC.reg.C = ReadRAM(SP);
                BC.reg.B = ReadRAM(SP + 1);

                SP += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to POPD, this routine
    * moves the contents of the 8080a's stack at SP and SP+1 to registers
    * E and D, respectively.  The stack pointer is incremented by 2.
    ***********************************************************************/
            case POPD:
                DE.reg.E = ReadRAM(SP);
                DE.reg.D = ReadRAM(SP + 1);

                SP += 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to POPH, this routine
    * moves the contents of the 8080a's stack at SP and SP+1 to registers
    * L and H, respectively.  The stack pointer is incremented by 2.
    ***********************************************************************/
            case POPH:
                HL.reg.L = ReadRAM(SP);
                HL.reg.H = ReadRAM(SP + 1);

                SP += 2;
                break;



    /***********************************************************************
    * POPPSW - Pop processor status word and accumulator.
    * ------
    *
    *   If the value of the variable OPCODE is equal to POPPSW, this
    * routine takes the content on the top of the stack and copies the
    * important information to the processor status word, named PSW.  Next,
    * this routine moves the content of the next stack byte to the
    * accumulator.  The content of the stack pointer, named SP, is
    * incremented by 2.
    ***********************************************************************/
            case POPPSW:
                PSW =   (ReadRAM(SP)&CARRY)  |
                    (ReadRAM(SP)&PARITY) |
                    (ReadRAM(SP)&AUX)    |
                    (ReadRAM(SP)&ZERO)   |
                    (ReadRAM(SP)&SIGN)   | PSWSET;

                A = ReadRAM(SP + 1);

                SP += 2;
                break;



    /***********************************************************************
    * PUSHrp - Push register pair.
    * ------
    *
    *   If the value of the variable OPCODE is equal to PUSHrp, where rp is
    * a 16-bit 8080a register, this routine moves the content of the
    * register pair to the top of the stack to register rp.  The content of
    * the stack pointer, named SP, is decremented by 2.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include POPB, POPD, and POPH.
    *   The second register in the register pair is on the top of the
    * stack, followed by the first register of the register pair.  Example:
    * C is on the top of the stack, if C is popped off, B is on the top of
    * the stack.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to PUSHB, this routine
    * moves the contents of the 8080a's C and B registers to the top of the
    * stack at SP-1 and SP-2, respectively.  The stack pointer is
    * incremented by 2.
    ***********************************************************************/
            case PUSHB:
                WriteRAM(SP - 1, BC.reg.B);
                WriteRAM(SP - 2, BC.reg.C);

                SP -= 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to PUSHD, this routine
    * moves the contents of the 8080a's D and E registers to the top of the
    * stack at SP-1 and SP-2, respectively.  The stack pointer is
    * incremented by 2.
    ***********************************************************************/
            case PUSHD:
                WriteRAM(SP - 1, DE.reg.D);
                WriteRAM(SP - 2, DE.reg.E);

                SP -= 2;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to PUSHH, this routine
    * moves the contents of the 8080a's H and L registers to the top of the
    * stack at SP-1 and SP-2, respectively.  The stack pointer is
    * incremented by 2.
    ***********************************************************************/
            case PUSHH:
                WriteRAM(SP - 1, HL.reg.H);
                WriteRAM(SP - 2, HL.reg.L);

                SP -= 2;
                break;



    /***********************************************************************
    * PUSHPSW - Push processor status word and accumulator.
    * -------
    *
    *   If the value of the variable OPCODE is equal to PUSHPSW, this
    * routine takes the content of the accumulator and moves it to the top
    * of the stack.  Next, this routine moves the content of the program
    * status word, named PSW, to the top of the stack, on top of the
    * accumulator that was just pushed.  The content of the stack
    * pointer, named SP, is decremented by 2.
    ***********************************************************************/
            case PUSHPSW:
                WriteRAM(SP - 1, A);
                WriteRAM(SP - 2, PSW);

                SP -= 2;
                break;



    /***********************************************************************
    * RAL - Rotate accumulator left with carry.
    * ---
    *
    *   If the value of the variable OPCODE is equal to RAL, this routine
    * takes the content of the accumulator and shifts the bits to the left.
    * If the most significant bit of the accumulator is set before the
    * shift, the carry flag is set in the program status word, named PSW.
    * If the carry flag is set in the PSW before the shift, the least
    * significant bit of the accumulator is set.
    ***********************************************************************/
            case RAL:
                CARRYOVER = PSW&CARRY;

                PSW = A&0x80 ? PSW|CARRY : PSW&NCARRY;

                A = (A << 1) | CARRYOVER;
                break;



    /***********************************************************************
    * RAR - Rotate accumulator right with carry.
    * ---
    *
    *   If the value of the variable OPCODE is equal to RAR, this routine
    * takes the content of the accumulator and shifts the bits to the
    * right. If the least significant bit of the accumulator is set before
    * the shift, the carry flag is set in the program status word, named
    * PSW. If the carry flag is set in the PSW before the shift, the most
    * significant bit of the accumulator is set.
    ***********************************************************************/
            case RAR:
                CARRYOVER = PSW&CARRY;

                PSW = A%2 ? PSW|CARRY : PSW&NCARRY;

                A = (A >> 1) | (CARRYOVER << 7);
                break;



    /***********************************************************************
    * RET - Return.
    * ---
    *
    *   If the value of the variable OPCODE is equal to RET, this routine
    * sets the program counter to the contents of the top two bytes, where
    * the top most byte of the stack is the the high order byte of the
    * program counter, named PC, and the second byte on the stack is the
    * low order byte of the PC.  The content of the stack pointer, is
    * decremented by 2.
    ***********************************************************************/
            case RET:
                if (ReturnOn8080Ret == 1)
                    return;

                doret:
                PC = (ReadRAM(SP + 1) << 8) | ReadRAM(SP);
                SP += 2;
                switch (check_pc8080a())
                {
                case 1: goto doret;
                case 2: return;
                default: break;
                }
                break;



    /***********************************************************************
    * Rcondition - Conditional return.
    * ----------
    *
    *   If the value of the variable OPCODE is equal to RET and the
    * condition is met, this routine sets the program counter to the
    * contents of the top two bytes of the stack, where the top most byte
    * of the stack is the high order byte of the program counter, named
    * PC, and the second byte on the stack is the low order byte of the PC.
    * The content of the stack pointer, is decremented by 2.  The cycle
    * counter is incremented by 6.
    *   If the condition is not met, the program continues sequentially
    * from this point.
    *   The condition that must be met is based on the value of a flag set
    * in the processor status word.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include RC, RM, RNC, RNZ, RP, RPE, RPO, and RZ.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RC, and the CARRY
    * flag is set in the PSW, this routine sets the program counter to the
    * address stored in the two bytes on top of the stack.  If the CARRY
    * flag is not set, the program continues sequentially.
    ***********************************************************************/
            case RC:
                if (PSW & CARRY)
                {
                    if (ReturnOn8080Ret == 1)
                        return;

                    PC = (ReadRAM(SP+1)<<8) | ReadRAM(SP);

                    COUNTER += 6;

                    SP += 2;
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RM, and the MINUS
    * flag is set in the PSW, this routine sets the program counter to the
    * address stored in the two bytes on top of the stack.  If the MINUS
    * flag is not set, the program continues sequentially.
    ***********************************************************************/
            case RM:
                if (PSW & SIGN)
                {
                    if (ReturnOn8080Ret == 1)
                        return;

                    PC = (ReadRAM(SP+1)<<8) | ReadRAM(SP);

                    COUNTER += 6;

                    SP += 2;
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }               
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RNC, and the CARRY
    * flag is not set in the PSW, this routine sets the program counter to
    * the address stored in the two bytes on top of the stack.  If the
    * CARRY flag is set, the program continues sequentially.
    ***********************************************************************/
            case RNC:
                if ((~PSW) & CARRY)
                {
                    if (ReturnOn8080Ret == 1)
                        return;

                    PC = (ReadRAM(SP+1)<<8) | ReadRAM(SP);

                    COUNTER += 6;

                    SP += 2;
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }               
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RNZ, and the ZERO
    * flag is not set in the PSW, this routine sets the program counter to
    * the address stored in the two bytes on top of the stack.  If the
    * ZERO flag is set, the program continues sequentially.
    ***********************************************************************/
            case RNZ:
                if ((~PSW) & ZERO)
                {
                    if (ReturnOn8080Ret == 1)
                        return;

                    PC = (ReadRAM(SP+1)<<8) | ReadRAM(SP);

                    COUNTER += 6;

                    SP += 2;
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RP, and the SIGN
    * flag is not set in the PSW, this routine sets the program counter to
    * the address stored in the two bytes on top of the stack.  If the
    * SIGN flag is set, the program continues sequentially.
    ***********************************************************************/
            case RP:
                if ((~PSW) & SIGN)
                {
                    if (ReturnOn8080Ret == 1)
                        return;

                    PC = (ReadRAM(SP+1)<<8) | ReadRAM(SP);

                    COUNTER += 6;

                    SP += 2;
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }               
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RPE, and the PARITY
    * flag is set in the PSW, this routine sets the program counter to the
    * address stored in the two bytes on top of the stack.  If the PARITY
    * flag is not set, the program continues sequentially.
    ***********************************************************************/
            case RPE:
                if (PSW & PARITY)
                {
                    if (ReturnOn8080Ret == 1)
                        return;

                    PC = (ReadRAM(SP+1)<<8) | ReadRAM(SP);

                    COUNTER += 6;

                    SP += 2;
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }   
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RPO, and the PARITY
    * flag is not set in the PSW, this routine sets the program counter to
    * the address stored in the two bytes on top of the stack.  If the
    * PARITY flag is set, the program continues sequentially.
    ***********************************************************************/
            case RPO:
                if ((~PSW) & PARITY)
                {
                    if (ReturnOn8080Ret == 1)
                        return;

                    PC = (ReadRAM(SP+1)<<8) | ReadRAM(SP);

                    COUNTER += 6;

                    SP += 2;
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RZ, and the ZERO
    * flag is set in the PSW, this routine sets the program counter to the
    * address stored in the two bytes on top of the stack.  If the ZERO
    * flag is not set, the program continues sequentially.
    ***********************************************************************/
            case RZ:
                if (PSW & ZERO)
                {
                    if (ReturnOn8080Ret == 1)
                        return;

                    PC = (ReadRAM(SP+1)<<8) | ReadRAM(SP);

                    COUNTER += 6;

                    SP += 2;  
                    switch (check_pc8080a ())
                    {
                    case 1: goto doret;
                    case 2: return;
                    default: break;
                    }
                }               
                break;



    /***********************************************************************
    * RLC - Rotate accumulator left.
    * ---
    *
    *   If the value of the variable OPCODE is equal to RLC, this routine
    * takes the content of the accumulator and shifts the bits to the left.
    * If the most significant bit of the accumulator is set before the
    * shift, the carry flag is set in the program status word, named PSW.
    * If the most significant bit of the accumulator is set before the
    * shift, the least significant bit of the accumulator is set after
    * the shift.
    ***********************************************************************/
            case RLC:
                PSW = A&0x80 ? PSW|CARRY : PSW&NCARRY;

                A = (A << 1) | (PSW&CARRY);
                break;



    /***********************************************************************
    * RRC - Rotate accumulator right.
    * ---
    *
    *   If the value of the variable OPCODE is equal to RRC, this routine
    * takes the content of the accumulator and shifts the bits to the
    * right. If the least significant bit of the accumulator is set before
    * the shift, the carry flag is set in the program status word, named
    * PSW, and the most significant bit of the accumulator is set after
    * the shift.
    ***********************************************************************/
            case RRC:
                PSW = A%2 ? PSW|CARRY : PSW&NCARRY;

                A = (A >> 1) | ((PSW&CARRY) << 7);
                break;



    /***********************************************************************
    * RSTn - Restart.
    * ----
    *
    *   If the value of the variable OPCODE is equal to RSTn, where n is a
    * four bit integer, this routine sets the top two bytes of the stack to
    * the content of the program counter.  The content of the high order
    * byte of the program counter is moved to the top of the stack.  The
    * content of the low order byte of the program counter is moved to the
    * byte on top of the previously moved data. The content of the stack
    * pointer is decremented by 2. The program counter is reset to 0x08*n.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include RST0, RST1, RST2, RST3, RST4, RST5, RST6, and RST7.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RST0, this routine
    * moves the high and low order bytes to the top of the stack.  The
    * program counter is reset to 0x00.
    ***********************************************************************/
            case RST0:
                WriteRAM(SP-1, PC >> 8);
                WriteRAM(SP-2, PC&0xFF);

                SP -= 2;

                PC = 0x00;
                break;

                //return;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RST1, this routine
    * moves the high and low order bytes to the top of the stack.  The
    * program counter is reset to 0x08.
    ***********************************************************************/
            case RST1:
                WriteRAM(SP-1, PC >> 8);
                WriteRAM(SP-2, PC&0xFF);

                SP -= 2;

                PC = 0x08;
                break;
                //return;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RST2, this routine
    * moves the high and low order bytes to the top of the stack.  The
    * program counter is reset to 0x10.
    ***********************************************************************/
            case RST2:
                WriteRAM(SP-1, PC >> 8);
                WriteRAM(SP-2, PC&0xFF);

                SP -= 2;

                PC = 0x10;
                break;
                //return;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RST3, this routine
    * moves the high and low order bytes to the top of the stack.  The 
    * program counter is reset to 0x18.
    ***********************************************************************/
            case RST3:
                WriteRAM(SP-1, PC >> 8);
                WriteRAM(SP-2, PC&0xFF);

                SP -= 2;

                PC = 0x18;
                break;
                //return;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RST4, this routine
    * moves the high and low order bytes to the top of the stack.  The
    * program counter is reset to 0x20.
    ***********************************************************************/
            case RST4:
                WriteRAM(SP-1, PC >> 8);
                WriteRAM(SP-2, PC&0xFF);

                SP -= 2;

                PC = 0x20;
                break;
                //return;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RST5, this routine
    * moves the high and low order bytes to the top of the stack.  The
    * program counter is reset to 0x28.
    ***********************************************************************/
            case RST5:
                WriteRAM(SP-1, PC >> 8);
                WriteRAM(SP-2, PC&0xFF);

                SP -= 2;

                PC = 0x28;
                break;
                //return;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RST6, this routine
    * moves the high and low order bytes to the top of the stack.  The
    * program counter is reset to 0x30.
    ***********************************************************************/
            case RST6:
                WriteRAM(SP-1, PC >> 8);
                WriteRAM(SP-2, PC&0xFF);

                SP -= 2;

                PC = 0x30;
                break;
                //return;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to RST7, this routine
    * moves the high and low order bytes to the top of the stack.  The
    * program counter is reset to 0x38.
    ***********************************************************************/
            case RST7:
                WriteRAM(SP-1, PC >> 8);
                WriteRAM(SP-2, PC&0xFF);

                SP -= 2;

                PC = 0x38;
                break;
                //return;



    /***********************************************************************
    * SBBr - Subtract register with borrow.
    * ----
    *
    *   If the value of the variable OPCODE is equal to SBBr, where r is an
    * 8-bit 8080a register, this routine subtracts the content of the
    * register, r, and the content of the CARRY flag, found in the PSW,
    * from the content of the accumulator.  The resulting value is stored
    * in the accumulator.  The processor status word, named PSW, is set
    * according to the result of the operation.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include SBBA, SBBB, SBBC, SBBD, SBBE, SBBH, and SBBL.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SBBA, this routine
    * subtracts the content of A and the CARRY flag from the accumulator
    * and stores the result in the accumulator.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case SBBA:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) - (A&0xF) - (PSW&CARRY) < 0x00
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A - A - (PSW&CARRY) < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;

                A -= A + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SBBB, this routine
    * subtracts the content of B and the CARRY flag from the accumulator
    * and stores the result in the accumulator.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case SBBB:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) - (BC.reg.B&0xF) -
                    (PSW&CARRY) < 0x00
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A - BC.reg.B - (PSW&CARRY) < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;

                A -= BC.reg.B + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SBBC, this routine
    * subtracts the content of C and the CARRY flag from the accumulator
    * and stores the result in the accumulator.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case SBBC:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) - (BC.reg.C&0xF) -
                    (PSW&CARRY) < 0x00
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A - BC.reg.C - (PSW&CARRY) < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;

                A -= BC.reg.C + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SBBD, this routine
    * subtracts the content of D and the CARRY flag from the accumulator
    * and stores the result in the accumulator.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case SBBD:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) - (DE.reg.D&0xF) -
                    (PSW&CARRY) < 0x00
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A - DE.reg.D - (PSW&CARRY) < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;

                A -= DE.reg.D + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SBBE, this routine
    * subtracts the content of E and the CARRY flag from the accumulator
    * and stores the result in the accumulator.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case SBBE:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) - (DE.reg.E&0xF) -
                    (PSW&CARRY) < 0x00
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A - DE.reg.E - (PSW&CARRY) < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;

                A -= DE.reg.E + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SBBH, this routine
    * subtracts the content of H and the CARRY flag from the accumulator
    * and stores the result in the accumulator.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case SBBH:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) - (HL.reg.H&0xF) -
                    (PSW&CARRY) < 0x00
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A - HL.reg.H - (PSW&CARRY) < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;

                A -= HL.reg.H + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SBBL, this routine
    * subtracts the content of L and the CARRY flag from the accumulator
    * and stores the result in the accumulator.  The PSW reflects the
    * result of this subtraction.
    ***********************************************************************/
            case SBBL:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) - (HL.reg.L&0xF) -
                    (PSW&CARRY) < 0x00
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A - HL.reg.L - (PSW&CARRY) < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;

                A -= HL.reg.L + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * SBBM - Subtract memory with borrow.
    * ----
    *
    *   If the value of the variable OPCODE is equal to SBBM, this routine
    * subtracts the content of the 8080a's memory, RAM[HL], and the content
    * of the CARRY flag (found in the PSW) from the content of the
    * accumulator.  The resulting value is stored in the accumulator.
    * The processor status word, named PSW, is set according to the result
    * of the operation.
    ***********************************************************************/
            case SBBM:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) - (ReadRAM(HL.pair)&0xF) -
                       (PSW&CARRY) < 0x00
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A - ReadRAM(HL.pair) - (PSW&CARRY) < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;

                A -= ReadRAM(HL.pair) + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * SBIn - Subtract immediate with borrow.
    * ----
    *
    *   If the value of the variable OPCODE is equal to SBIn, this routine
    * subtracts the content of the 8080a's memory, RAM[PC] and the content
    * of the CARRY flag, found in the PSW, from the content of the
    * accumulator. The resulting value is stored in the accumulator.  The
    * processor status word, named PSW, is set according to the result of
    * the operation.  The program counter, named PC, is incremented by one.
    ***********************************************************************/
            case SBIn:
                CARRYOVER = PSW&CARRY;

                PSW = (A&0xF) - (ReadRAM(PC)&0xF) -
                       (PSW&CARRY) < 0x00
                       ? PSW|AUX     : PSW&NAUX;
                PSW = A - ReadRAM(PC) - (PSW&CARRY) < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;

                A -= ReadRAM(PC++) + CARRYOVER;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * STLDa - Store HL direct.
    * -----
    *
    *   If the value of the variable OPCODE is equal to STLDa, this routine
    * moves the content of the 8-bit H register to the 8080a's memory at
    * the location one plus the high order byte, RAM[PC + 1], and low order
    * byte, RAM[PC].  It also moves the content of the L register to the
    * 8080a's memory at the location where the high order byte is
    * RAM[PC + 1] and low order byte is RAM[PC].  The program counter,
    * named PC, is incremented by 2.
    ***********************************************************************/
            case SHLDa:
                WriteRAM(((ReadRAM(PC+1) << 8) | ReadRAM(PC))
                        + 1, HL.reg.H);
                WriteRAM(((ReadRAM(PC+1) << 8) | ReadRAM(PC)),
                        HL.reg.L);

                PC += 2;
                break;



    /***********************************************************************
    * SPHL - Move HL to SP.
    * ----
    *
    *   If the value of the variable OPCODE is equal to SPHL, this routine
    * moves the content of the HL register to the stack pointer, named
    * SP.
    ***********************************************************************/
            case SPHL:
                SP = HL.pair;
                break;



    /***********************************************************************
    * STAa - Store accumulator direct.
    * ----
    *
    *   If the value of the variable OPCODE is equal to STAa, this routine
    * moves the content of the accumulator to the 8080a's memory at the
    * location one plus the high and low order bytes, RAM[PC + 1] and
    * RAM[PC], respectively.  The program counter, named PC, is incremented
    * by 2.
    ***********************************************************************/
            case STAa:
                WriteRAM(((ReadRAM(PC + 1)<<8) | ReadRAM(PC)),
                        A);

                PC += 2;
                break;



    /***********************************************************************
    * STAXrp - Store accumulator indirect.
    * ------
    *
    *   If the value of the variable OPCODE is equal to STAXrp, where rp
    * is a 16-bit 8080a register pair, this routine moves the content of
    * the accumulator to the 8080a's memory at RAM[rp].
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include STAXB and STAXD.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to STAXB, this routine
    * moves the content of the accumulator to RAM[BC].
    ***********************************************************************/
            case STAXB:
                WriteRAM(BC.pair, A);
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to STAXD, this routine
    * moves the content of the accumulator to RAM[DE].
    ***********************************************************************/
            case STAXD:
                WriteRAM(DE.pair, A);
                break;



    /***********************************************************************
    * STC - Set carry
    * ---
    *
    *   If the value of the variable OPCODE is equal to STC, this routine
    * sets the CARRY flag in the program status word, named PSW.
    ***********************************************************************/
            case STC:
                PSW = PSW|CARRY;
                break;



    /***********************************************************************
    * SUBr - Subtract register.
    * ----
    *
    *   If the value of the variable OPCODE is equal to SUBr, where r is an
    * 8-bit 8080a register, this routine subtracts the content of the
    * register, r, from the content of the accumulator.  The resulting
    * value is stored in the accumulator.  The processor status word,
    * named PSW, is set according to the result of the operation.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include SUBA, SUBB, SUBC, SUBD, SUBE, SUBH, and SUBL.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SUBA, this routine
    * subtracts the content of A from the accumulator and stores the result
    * in the accumulator.  The PSW reflects the result of this subtraction.
    ***********************************************************************/
            case SUBA:
                PSW = A - A < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) - (A&0xF) < 0x00
                       ? PSW|AUX     : PSW&NAUX;

                A -= A;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SUBB, this routine
    * subtracts the content of B from the accumulator and stores the result
    * in the accumulator.  The PSW reflects the result of this subtraction.
    ***********************************************************************/
            case SUBB:
                PSW = A - BC.reg.B < 0x00
                    ? PSW | CARRY : PSW&NCARRY;
                PSW = (A&0xF) - (BC.reg.B&0xF) < 0x00
                       ? PSW|AUX     : PSW&NAUX;

                A -= BC.reg.B;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SUBC, this routine
    * subtracts the content of C from the accumulator and stores the result
    * in the accumulator.  The PSW reflects the result of this subtraction.
    ***********************************************************************/
            case SUBC:
                PSW = A - BC.reg.C < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) - (BC.reg.C&0xF) < 0x00
                       ? PSW|AUX     : PSW&NAUX;

                A -= BC.reg.C;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SUBD, this routine
    * subtracts the content of D from the accumulator and stores the result
    * in the accumulator.  The PSW reflects the result of this subtraction.
    ***********************************************************************/
            case SUBD:
                PSW = A - DE.reg.D < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) - (DE.reg.D&0xF) < 0x00
                       ? PSW|AUX     : PSW&NAUX;

                A -= DE.reg.D;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SUBE, this routine
    * subtracts the content of E from the accumulator and stores the result
    * in the accumulator.  The PSW reflects the result of this subtraction.
    ***********************************************************************/
            case SUBE:
                PSW = A - DE.reg.E < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) - (DE.reg.E&0xF) < 0x00
                       ? PSW|AUX     : PSW&NAUX;

                A -= DE.reg.E;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SUBH, this routine
    * subtracts the content of H from the accumulator and stores the result
    * in the accumulator.  The PSW reflects the result of this subtraction.
    ***********************************************************************/
            case SUBH:
                PSW = A - HL.reg.H < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) - (HL.reg.H&0xF) < 0x00
                       ? PSW|AUX     : PSW&NAUX;

                A -= HL.reg.H;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to SUBL, this routine
    * subtracts the content of L from the accumulator and stores the result
    * in the accumulator.  The PSW reflects the result of this subtraction.
    ***********************************************************************/
            case SUBL:
                PSW = A - HL.reg.L < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) - (HL.reg.L&0xF) < 0x00
                       ? PSW|AUX     : PSW&NAUX;

                A -= HL.reg.L;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * SUBM - Subtract memory.
    * ----
    *
    *   If the value of the variable OPCODE is equal to SBBM, this routine
    * subtracts the content of the 8080a's memory, RAM[HL], from the
    * content of the accumulator.  The resulting value is stored in the
    * accumulator. The processor status word, named PSW, is set according
    * to the result of the operation.
    ***********************************************************************/
            case SUBM:
                PSW = A - ReadRAM(HL.pair) < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) - (ReadRAM(HL.pair)&0xF) < 0x00
                       ? PSW|AUX     : PSW&NAUX;

                A -= ReadRAM(HL.pair);

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * SUIn - Subtract immediate.
    * ----
    *
    *   If the value of the variable OPCODE is equal to SUIn, this routine
    * subtracts the content of the 8080a's memory, RAM[PC], and from the
    * content of the accumulator. The resulting value is stored in the
    * accumulator.  The processor status word, named PSW, is set according
    * to the result of the operation.  The program counter, named PC, is
    * incremented by one.
    ***********************************************************************/
            case SUIn:
                PSW = A - ReadRAM(PC) < 0x00
                       ? PSW|CARRY   : PSW&NCARRY;
                PSW = (A&0xF) - (ReadRAM(PC)&0xF) < 0x00
                       ? PSW|AUX     : PSW&NAUX;

                A -= ReadRAM(PC++);

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * XCHG - Exchange HL and DE registers.
    * ----
    *
    *   If the value of the variable OPCODE is equal to XCHG, this routine
    * moves the content of the 16-bit register, DE, to HL and the content
    * of the 16-bit register, HL, to DE.
    ***********************************************************************/
            case XCHG:
                DE.pair ^= HL.pair;
                HL.pair ^= DE.pair;
                DE.pair ^= HL.pair;
                break;



    /***********************************************************************
    * XRAr - Exclusive OR register.
    * ----
    *
    *   If the value of the variable OPCODE is equal to XRAr, where r is an
    * 8-bit 8080a register, this routine XORs the content of the register,
    * r, with the content of the accumulator.  The resulting value is
    * stored in the accumulator.  The processor status word, named PSW, is
    * set according to the result of the operation.  The CARRY and AUX
    * flags are cleared in the PSW.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include XRAA, XRAB, XRAC, XRAD, XRAE, XRAH, and XRAL.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to XRAA, this routine
    * XORs the content of A with the accumulator and stored the result in
    * the accumulator.  The PSW reflects the result of this XOR operation.
    * The CARRY and AUX flags are cleared.
    ***********************************************************************/
            case XRAA:
                PSW = PSW&NCARRY&NAUX;

                A = A ^ A;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to XRAB, this routine
    * XORs the content of B with the accumulator and stored the result in
    * the accumulator.  The PSW reflects the result of this XOR operation.
    * The CARRY and AUX flags are cleared.
    ***********************************************************************/
            case XRAB:
                PSW = PSW&NCARRY&NAUX;

                A = A ^ BC.reg.B;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to XRAC, this routine
    * XORs the content of C with the accumulator and stored the result in
    * the accumulator.  The PSW reflects the result of this XOR operation.
    * The CARRY and AUX flags are cleared.
    ***********************************************************************/
            case XRAC:
                PSW = PSW&NCARRY&NAUX;

                A = A ^ BC.reg.C;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to XRAD, this routine
    * XORs the content of D with the accumulator and stored the result in
    * the accumulator.  The PSW reflects the result of this XOR operation.
    * The CARRY and AUX flags are cleared.
    ***********************************************************************/
            case XRAD:
                PSW = PSW&NCARRY&NAUX;

                A = A ^ DE.reg.D;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to XRAE, this routine
    * XORs the content of E with the accumulator and stored the result in
    * the accumulator.  The PSW reflects the result of this XOR operation.
    * The CARRY and AUX flags are cleared.
    ***********************************************************************/
            case XRAE:
                PSW = PSW&NCARRY&NAUX;

                A = A ^ DE.reg.E;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to XRAH, this routine
    * XORs the content of H with the accumulator and stored the result in
    * the accumulator.  The PSW reflects the result of this XOR operation.
    * The CARRY and AUX flags are cleared.
    ***********************************************************************/
            case XRAH:
                PSW = PSW&NCARRY&NAUX;

                A = A ^ HL.reg.H;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to XRAL, this routine
    * XORs the content of L with the accumulator and stored the result in
    * the accumulator.  The PSW reflects the result of this XOR operation.
    * The CARRY and AUX flags are cleared.
    ***********************************************************************/
            case XRAL:
                PSW = PSW&NCARRY&NAUX;

                A = A ^ HL.reg.L;

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * XRAM - Exclusive OR memory.
    * ----
    *
    *   If the value of the variable OPCODE is equal to XRAM, this routine
    * XORs the content of the 8080a's memory, RAM[HL], with the content
    * of the accumulator.  The resulting value is stored in the
    * accumulator.  The processor status word, named PSW, is set according
    * to the result of the operation.  The CARRY and AUX flags are cleared
    * in the PSW.
    ***********************************************************************/
            case XRAM:
                PSW = PSW&NCARRY&NAUX;

                A = A ^ ReadRAM(HL.pair);

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;



    /***********************************************************************
    * XRIn - Exclusive OR immediate.
    * ----
    *
    *   If the value of the variable OPCODE is equal to XRIn, this routine
    * XORs the content of the 8080a's memory, RAM[PC] with the content of
    * the accumulator.  The resulting value is stored in the accumulator.
    * The processor status word, named PSW, is set according to the result
    * of the operation.  The CARRY and AUX flags are cleared in the PSW.
    * The program counter, named PC, is incremented by one.
    ***********************************************************************/
            case XRIn:
                PSW = PSW&NCARRY&NAUX;

                A = A ^ ReadRAM(PC++);

                PSW = A==0 ? PSW|ZERO    : PSW&NZERO;
                PSW = A>>7 ? PSW|SIGN    : PSW&NSIGN;
                PSW = Parity8(A) ? PSW|PARITY : PSW&NPARITY;
                break;


    /***********************************************************************
    * XTHL - Exchange top of stack with HL register.
    * ----
    *
    *   If the value of the variable OPCODE is equal to XTHL, this routine
    * moves the content of the two top bytes of the stack to the H and L, 
    * registers and the content of the 16-bit register, HL, to the top
    * two bytes of the stack. 
    ***********************************************************************/
            case XTHL:
                CARRYOVER = HL.reg.H;
                HL.reg.H = ReadRAM(SP + 1);
                WriteRAM(SP + 1, CARRYOVER);

                CARRYOVER = HL.reg.L;
                HL.reg.L = ReadRAM(SP);
                WriteRAM(SP, CARRYOVER);
                break;

  /*******************************************************************************
  * LDIR and LDDR z80 transfer instructions for micro tutor -moveb- routine
  *******************************************************************************/

            case Z80T:

                if (RAM[PC] == 0xB0)    // LDIR
                {
                    while (--BC.pair != 0xffff)
                    {
                        RAM[DE.pair++] = RAM[HL.pair++];
                    }
                }
                else if (RAM[PC] == 0xB8)   // LDDR
                {
                    while (--BC.pair != 0xffff)
                    {
                        RAM[DE.pair--] = RAM[HL.pair--];
                    }
                }
                else if (RAM[PC] == 0x53)   // SHLD (word) -< DE
                {
                    WriteRAMW(ReadRAMW(PC + 1), DE.pair);
                }
                else
                {
                    printf("Unimplemented z80 0xED OPCODE "
                        "not supported. %02x\n", RAM[PC]);
                }

                PC++;
                break;

    /***********************************************************************
    * z80 CODE
    * --------
    *
    * Psuedo code:
    *   --- JR    index     18index  PC <- PC + index
    *   --- JR    NZ,index  20index  If NZ, PC <- PC + index
    *   --- JR    Z, index  28index  If Z, PC <- PC + index
    *   --- JR    NC,index  30index  If NC, PC <- PC + index
    *   --- JR    C, index  38index  If C, PC <- PC + index
    *   --- DJNZ  index     10index  B <- B - 1;
    *       while B > 0, PC <- PC + index
    *
    *   Some 8080a programs actually use these "z80" instructions.  These
    * instructions "complete" the 8080a set.  Unfortunately, these
    * instructions are not documented, so actual z80 instructions were
    * used in their place.  These instructions may have unknown side-
    * effects that may cause the user's program to fail.
    *
    *
    * Note:
    * ----
    *
    *   The macro Z80 must be defined when compiling to enable support
    * for these instructions.  See the Makefile.
    ***********************************************************************/
        #ifdef Z80



    /***********************************************************************
    * EXAF - Exchange A and PSW registers.
    * ----
    *
    *   If the value of the variable OPCODE is equal to 0x08, this routine
    * exchanges the accumulator with the program status word.
    ***********************************************************************/
            case 0x08:
                PSW = (~PSW)&PSWSET;
                break;



    /***********************************************************************
    * DJNZn - Decrement register B, jump on non zero.
    * -----
    *
    *   If the value of the variable OPCODE is equal to 0x10, this routine
    * decrements the content of B, then tests the value of B.  If B is not
    * equal to zero, the program counter, named PC, is incremented by the
    * signed byte after this instruction and incremented 2.  If B is equal
    * to zero, the PC is incremented by 1.  The program continues
    * sequentially from this point.  Register BC changes, to reflect the
    * new state of B. This instruction is not a documented part of the
    * 8080a instruction set.
    ***********************************************************************/
            case 0x10:
                BC.reg.B--;

                if (BC.reg.B != 0)
                    PC += (Sint8)ReadRAM(PC) + 2;
                else
                    PC ++;
                break;



    /***********************************************************************
    * JRn - Jump immediate.
    * ---
    *
    *   If the value of the variable OPCODE is equal to 0x18, this routine
    * increments the program counter, named PC, by the signed byte after
    * this instruction.
    ***********************************************************************/
            case 0x18:
                PC += ReadRAM(PC);
                break;



    /***********************************************************************
    * JR-condition-n - Conditional jump immediate.
    * --------------
    *
    *   If the value of the variable OPCODE is equal to 0x20 and the
    * condition is met, this routine increments the program counter, named
    * PC, by the signed byte after this instruction.  If the condition is
    * not met, the program counter, named PC, is incremented by 1.  The
    * program continues sequentially from this point. The condition that
    * must be met is based on the value of a flag set in the processor
    * status word.  These instructions are not a documented part of the
    * 8080a instruction set.
    *
    *
    * Note:
    * ----
    *
    *   OPCODEs include JRNZn,  JRZn,   JRCn,   and JRNCn.
    *   OPCODE values   0x20,   0x28,   0x30,   and 0x38.
    ***********************************************************************/

    /***********************************************************************
    *   If the value of the variable OPCODE is equal to 0x20, and the ZERO
    * flag is not set in the PSW, this routine increments the PC based on
    * the value of the next signed byte.  If the ZERO flag is set, the
    * program continues sequentially.
    *
    *   This opcode's name is JRNZn.  This instruction is not a documented
    * part of the 8080a instruction set.
    ***********************************************************************/
            case 0x20:
                if ((~PSW) & ZERO)
                    PC += ReadRAM(PC);
                else
                    PC ++;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to 0x28, and the ZERO
    * flag is set in the PSW, this routine increments the PC based on
    * the value of the next signed byte.  If the ZERO flag is not set, the
    * program continues sequentially.
    *
    *   This opcode's name is JRZn.  This instruction is not a documented
    * part of the 8080a instruction set.
    ***********************************************************************/
            case 0x28:
                if (PSW & ZERO)
                    PC += ReadRAM(PC);
                else
                    PC ++;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to 0x30, and the CARRY
    * flag is not set in the PSW, this routine increments the PC based on
    * the value of the next signed byte.  If the CARRY flag is set, the
    * program continues sequentially.
    *
    *   This opcode's name is JRNCn.  This instruction is not a documented
    * part of the 8080a instruction set.
    ***********************************************************************/
            case 0x30:
                if ((~PSW) & CARRY)
                    PC += ReadRAM(PC);
                else
                    PC ++;
                break;


    /***********************************************************************
    *   If the value of the variable OPCODE is equal to 0x38, and the CARRY
    * flag is set in the PSW, this routine increments the PC based on
    * the value of the next signed byte.  If the CARRY flag is not set, the
    * program continues sequentially.
    *
    *   This opcode's name is JRCn.  This instruction is not a documented
    * part of the 8080a instruction set.
    ***********************************************************************/
            case 0x38:
                if (PSW & CARRY)
                    PC += ReadRAM(PC);
                else
                    PC ++;
                break;


        #else
    /***********************************************************************
    * z80 CODE
    * --------
    *
    *   If the Z80 macro is not defined when compiling this file, an error
    * is printed to STDOUT.  Debug information follows the error.
    ***********************************************************************/


            case 0x10:
            case 0x20:
            case 0x8:
            case 0x18:
            case 0x28:
            case 0x38:
            case 0x30:
                printf( "Undocumented 8080a OPCODE "
                    "not supported.\n");
        #endif



    /***********************************************************************
    * Default Case
    * ------------
    *
    *   If OPCODE is unrecognized and the VERBOSE macro is set, debug
    * information is printed to STDOUT. Information includes: the 8-bit
    * hex value of the opcode, the program counter, named PC, the stack
    * pointer, named SP, and the program status word, named PSW.  It also
    * includes the following registers and register pairs: A, B, C, D, E,
    * H, L, BC, DE, and HL as well as the top 2 bytes on the stack:
    * RAM[SP] and RAM[SP+1], and the current 3 bytes from the ROM: RAM[PC],
    * RAM[PC+1], and RAM[PC+2].  If the VERBOSE macro is not set, "Unknown
    * OPCODE" is printed to STDOUT with the value of OPCODE.
    * In both cases, After debugging information is printed, this routine
    * exits.
    ***********************************************************************/
            default:
        #ifdef VERBOSE
                printf( "OPCODE-[0x%X]\tPC-[0x%X]\tSP-[0x%X]\t"
                    "PSW-[0x%X]\n",
                    OPCODE, PC, SP,
                    PSW);

                printf( "A-[0x%X] \tB/C-[0x%X/0x%X]\t"
                    "D/E-[0x%X/0x%X]\tH/L-[0x%X/0x%X]\n",
                    A, BC.reg.B, BC.reg.C, DE.reg.D,
                    DE.reg.E, HL.reg.H, HL.reg.L);

                printf( "BC-[0x%X]\tDE-[0x%X]\t"
                    "HL-[0x%X]\tTop of stack-[0x%X/0x%X]\n",
                    BC.pair, DE.pair,
                    HL.pair, ReadRAM(SP), ReadRAM(SP + 1));

                printf( "RAM-[0x%X/0x%X/0x%X]\n",
                    ReadRAM(PC), ReadRAM(PC+1),
                    ReadRAM(PC+2));

        #else

                printf("Unknown OPCODE 0x%X\n", OPCODE);

        #endif

                break;
        }



    /***********************************************************************
    * Interrupt handling
    * ------------------
    *
    *   If enough cycles have passed (the cycle counter is greater than the
    * number cycles needed for an interrupt) and interrupts are not
    * disabled, then an interrupt can be processed.  If the variable
    * INTERRUPT is equal to 1, it is safe to have an interrupt.  If NMI is
    * equal to 0, a normal interrupt occured.  If NMI is equal to 1, a
    * non-maskable interrupt occured.  The screen is updated during a
    * non-maskable interrupt.
    ***********************************************************************/
        if ((COUNTER >= INTHIGH) && !INTDISABLE)
        {


    /***********************************************************************
    *   If the variable INTERRUPT is equal to 1, it is safe to have an
    * interrupt.
    ***********************************************************************/
            if (INTERRUPT == 1)
            {


    /***********************************************************************
    *   This resets the counter to almost 0.  The reset point depends on
    * the number of cycles the last instruction used.
    ***********************************************************************/
                COUNTER -= INTHIGH;


    /***********************************************************************
    *   If NMI is equal to 0, a normal interrupt occured.  During a normal
    * interrupt, the OPCODE is set to RST2.  RST2 is the next instruction
    * that will be executed.  If NMI is equal to 1, a non-maskable
    * interrupt occured.  The screen is updated during a non-maskable
    * interrupt and the OPCODE is set to RST1.  RST1 is the next
    * instruction that will be executed.
    ***********************************************************************/
                NMI ++;
                if (NMI%2 == 0)
                {

    /***********************************************************************
    *   This assignment sets the next opcode to be executed during the
    * interrupt to RST2.
    ***********************************************************************/
                    OPCODE = RST2;
                } else
                {

    /***********************************************************************
    *   This assignment sets the next opcode to be executed during the
    * interrupt to RST1.
    ***********************************************************************/
                    OPCODE = RST1;
                }


    /***********************************************************************
    *   These assignments set INTERUPT to 2 and INTDISABLE to 1.  Setting
    * INTERRUPT to 2 implies that an interrupt has just occured and to
    * not fetch the next instruction.  Setting INTDISABLE to 1 disables
    * interrupts.
    ***********************************************************************/
                INTERRUPT = 2;
                INTDISABLE = 1;
            }
        }
    }
}

Uint8 emul8080::input8080a (Uint8 data)
{
    return 0;
}

void emul8080::output8080a (Uint8 data, Uint8 acc)
{
}

int emul8080::check_pc8080a (void)
{
    return 0;
}
