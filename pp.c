/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: pp.c
**
**  Description:
**      Perform simulation of CDC 6600 PPs
**
**--------------------------------------------------------------------------
*/

/*
**  -------------
**  Include Files
**  -------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define InterlockWords          11
#define StatusAndControlWords   22

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#define PpIncrement(word) (word) = (((word) + 1) & Mask12)
#define PpDecrement(word) (word) = (((word) - 1) & Mask12)

///// the program (PPU ?) stops when "from" is 000 or 077 a deadstart is necessary - see 6600 RM page 4-22 (UJN)
#define PpAddOffset(to, from)                                               \
    {                                                                       \
    (to) = ((to) - 1) & Mask12;                                             \
    if (from < 040)                                                         \
        (to) = ((to) + (from));                                             \
    else                                                                    \
        (to) = ((to) + (from) - 077);                                       \
    if (((to) & Overflow12) != 0)                                           \
        {                                                                   \
        (to) += 1;                                                          \
        (to) &= Mask12;                                                     \
        }                                                                   \
    }

#define IndexLocation                                                       \
    if (opD != 0)                                                           \
        {                                                                   \
        location = activePpu->mem[opD] + activePpu->mem[activePpu->regP];   \
        if ((location & Overflow12) != 0 || (location & Mask12) == 07777)   \
            {                                                               \
            location += 1;                                                  \
            }                                                               \
        location &= Mask12;                                                 \
        }                                                                   \
    else                                                                    \
        {                                                                   \
        location = activePpu->mem[activePpu->regP];                         \
        }                                                                   \
    PpIncrement(activePpu->regP);


/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void ppOpPSN(void);    // 00
static void ppOpLJM(void);    // 01
static void ppOpRJM(void);    // 02
static void ppOpUJN(void);    // 03
static void ppOpZJN(void);    // 04
static void ppOpNJN(void);    // 05
static void ppOpPJN(void);    // 06
static void ppOpMJN(void);    // 07
static void ppOpSHN(void);    // 10
static void ppOpLMN(void);    // 11
static void ppOpLPN(void);    // 12
static void ppOpSCN(void);    // 13
static void ppOpLDN(void);    // 14
static void ppOpLCN(void);    // 15
static void ppOpADN(void);    // 16
static void ppOpSBN(void);    // 17
static void ppOpLDC(void);    // 20
static void ppOpADC(void);    // 21
static void ppOpLPC(void);    // 22
static void ppOpLMC(void);    // 23
static void ppOpPSN(void);    // 24
static void ppOpPSN(void);    // 25
static void ppOpEXN(void);    // 26
static void ppOpRPN(void);    // 27
static void ppOpLDD(void);    // 30
static void ppOpADD(void);    // 31
static void ppOpSBD(void);    // 32
static void ppOpLMD(void);    // 33
static void ppOpSTD(void);    // 34
static void ppOpRAD(void);    // 35
static void ppOpAOD(void);    // 36
static void ppOpSOD(void);    // 37
static void ppOpLDI(void);    // 40
static void ppOpADI(void);    // 41
static void ppOpSBI(void);    // 42
static void ppOpLMI(void);    // 43
static void ppOpSTI(void);    // 44
static void ppOpRAI(void);    // 45
static void ppOpAOI(void);    // 46
static void ppOpSOI(void);    // 47
static void ppOpLDM(void);    // 50
static void ppOpADM(void);    // 51
static void ppOpSBM(void);    // 52
static void ppOpLMM(void);    // 53
static void ppOpSTM(void);    // 54
static void ppOpRAM(void);    // 55
static void ppOpAOM(void);    // 56
static void ppOpSOM(void);    // 57
static void ppOpCRD(void);    // 60
static void ppOpCRM(void);    // 61
static void ppOpCWD(void);    // 62
static void ppOpCWM(void);    // 63
static void ppOpAJM(void);    // 64
static void ppOpIJM(void);    // 65
static void ppOpFJM(void);    // 66
static void ppOpEJM(void);    // 67
static void ppOpIAN(void);    // 70
static void ppOpIAM(void);    // 71
static void ppOpOAN(void);    // 72
static void ppOpOAM(void);    // 73
static void ppOpACN(void);    // 74
static void ppOpDCN(void);    // 75
static void ppOpFAN(void);    // 76
static void ppOpFNC(void);    // 77

static INLINE u32 ppAdd18(u32 op1, u32 op2);
static INLINE u32 ppSubtract18(u32 op1, u32 op2);
static void ppInterlock(PpWord func);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
PpSlot *ppu;
PpSlot *activePpu;
u8 ppuCount;
FILE *devF;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static u8 pp = 0;
static PpWord opCode;
static PpByte opF;
static PpByte opD;
static PpWord location;
static u32 acc18;
static bool noHang;
static u32 cpuMemStart, cpuMemLen;  // for tracing CRD/CRM/CWD/CWM

static void (*decodePpuOpcode[])(void) = 
    {
    ppOpPSN,    // 00
    ppOpLJM,    // 01
    ppOpRJM,    // 02
    ppOpUJN,    // 03
    ppOpZJN,    // 04
    ppOpNJN,    // 05
    ppOpPJN,    // 06
    ppOpMJN,    // 07
    ppOpSHN,    // 10
    ppOpLMN,    // 11
    ppOpLPN,    // 12
    ppOpSCN,    // 13
    ppOpLDN,    // 14
    ppOpLCN,    // 15
    ppOpADN,    // 16
    ppOpSBN,    // 17
    ppOpLDC,    // 20
    ppOpADC,    // 21
    ppOpLPC,    // 22
    ppOpLMC,    // 23
    ppOpPSN,    // 24
    ppOpPSN,    // 25
    ppOpEXN,    // 26
    ppOpRPN,    // 27
    ppOpLDD,    // 30
    ppOpADD,    // 31
    ppOpSBD,    // 32
    ppOpLMD,    // 33
    ppOpSTD,    // 34
    ppOpRAD,    // 35
    ppOpAOD,    // 36
    ppOpSOD,    // 37
    ppOpLDI,    // 40
    ppOpADI,    // 41
    ppOpSBI,    // 42
    ppOpLMI,    // 43
    ppOpSTI,    // 44
    ppOpRAI,    // 45
    ppOpAOI,    // 46
    ppOpSOI,    // 47
    ppOpLDM,    // 50
    ppOpADM,    // 51
    ppOpSBM,    // 52
    ppOpLMM,    // 53
    ppOpSTM,    // 54
    ppOpRAM,    // 55
    ppOpAOM,    // 56
    ppOpSOM,    // 57
    ppOpCRD,    // 60
    ppOpCRM,    // 61
    ppOpCWD,    // 62
    ppOpCWM,    // 63
    ppOpAJM,    // 64
    ppOpIJM,    // 65
    ppOpFJM,    // 66
    ppOpEJM,    // 67
    ppOpIAN,    // 70
    ppOpIAM,    // 71
    ppOpOAN,    // 72
    ppOpOAM,    // 73
    ppOpACN,    // 74
    ppOpDCN,    // 75
    ppOpFAN,    // 76
    ppOpFNC     // 77
    };

/*
**--------------------------------------------------------------------------
**
**  Inline Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        18 bit ones-complement addition with subtractive adder
**
**  Parameters:     Name        Description.
**                  op1         18 bit operand1
**                  op2         18 bit operand2
**
**  Returns:        18 bit result.
**
**------------------------------------------------------------------------*/
static INLINE u32 ppAdd18(u32 op1, u32 op2)
    {
    acc18 = (op1 & Mask18) - (~op2 & Mask18);
    if ((acc18 & Overflow18) != 0)
        {
        acc18 -= 1;
        acc18 &= Mask18;
        }

    return(acc18);
    }

/*--------------------------------------------------------------------------
**  Purpose:        18 bit ones-complement subtraction
**
**  Parameters:     Name        Description.
**                  op1         18 bit operand1
**                  op2         18 bit operand2
**
**  Returns:        18 bit result.
**
**------------------------------------------------------------------------*/
static INLINE u32 ppSubtract18(u32 op1, u32 op2)
    {
    acc18 = (op1 & Mask18) - (op2 & Mask18);
    if ((acc18 & Overflow18) != 0)
        {
        acc18 -= 1;
        acc18 &= Mask18;
        }

    return(acc18);
    }


/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialise PP subsystem.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ppInit(u8 count)
    {
    /*
    **  Allocate ppu structures.
    */
    ppuCount = count;
    ppu = calloc(count, sizeof(PpSlot));
    if (ppu == NULL)
        {
        fprintf(stderr, "Failed to allocate ppu control blocks\n");
        exit(1);
        }

    /*
    **  Initialise all ppus.
    */
    for (pp = 0; pp < ppuCount; pp++)
        {
        ppu[pp].id = pp;
        }

    pp = 0;

    /*
    **  Print a friendly message.
    */
    printf("PPs initialised (number of PPUs %o)\n", ppuCount);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute one instruction in an active PPU.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ppStep(void)
    {
    u8 i;

    /*
    **  Locate a PPU which is active (ie not waiting for I/O)
    */
    for (i = 0; i < ppuCount; i++)
        {
        /*
        **  Advance to next PPU.
        */
        pp = (pp + 1) % ppuCount;
        activePpu = ppu + pp;

        /*
        **  Check if this one is active.
        */
        if (!activePpu->stopped)
            {
            /*
            **  Extract next PPU instruction.
            */
            opCode = activePpu->mem[activePpu->regP];
            opF = (opCode >> 6) & 077;
            opD = opCode & 077;

#if CcDebug == 1
            /*
            **  If the previous instruction was an I/O, then we do
            **  the last few trace steps (registers after, etc.) here
            **  rather than right after instruction execution, so that 
            **  the information displayed reflects the completion of any
            **  I/O that was done.  In particular, for I/O instructions
            **  the A register is updated after the instruction
            **  is "finished", so to show A correctly we display it now.
            */
            if (activePpu->ioFlag)
                {
                /*
                **  Trace result.
                */
                traceRegisters();

                /*
                **  Trace new channel status.
                */
                traceChannel(opD);

                traceEnd();
                /*
                **  Trace memory touched by IAM/OAM, and if necessary,
                **  A register after IAM/IAN/OAM/OAN
                */
                if (activePpu->ppMemLen > 0)
                    {
                    tracePM ();
                    activePpu->ppMemLen = 0;
                    }
                }
            
            /*
            **  Trace instructions.
            */
            traceSequence();
            traceRegisters();
            traceOpcode();
#endif

            /*
            **  Increment register P.
            */
            PpIncrement(activePpu->regP);

            /*
            **  Execute PPU instruction.
            */
            activePpu->ioFlag = FALSE;
            decodePpuOpcode[opF]();

#if CcDebug == 1
            /*
            **  If the current instruction is an I/O, then we do
            **  the last few trace steps (registers after, etc.) later
            **  rather than right after instruction execution, so that 
            **  the information displayed reflects the completion of any
            **  I/O that was done.  
            */
            if (!activePpu->ioFlag)
                {
                /*
                **  Trace result.
                */
                traceRegisters();

                /*
                **  Trace new channel status.
                */
                if (opF >= 064)
                    {
                    traceChannel(opD);
                    }

                traceEnd();
                /*
                **  Trace memory touched by central read/write
                */
                if (cpuMemLen > 0)
                    {
                    traceCM (cpuMemStart, cpuMemLen);
                    cpuMemLen = 0;
                    }
                }
            
#endif
            }

        if ((activePpu->ioWaitType & WaitMany) != 0 && !activePpu->channel->active)
            {
            /*
            **  Channel with IAM or OAM has been deactivated.
            */
            activeChannel = activePpu->channel;
            activeChannel->ioDevice = NULL;
            activeChannel->full = FALSE;

            /*
            **  Handle PPU to PPU channel disconnection.
            */
            if (activePpu->ioWaitType == WaitInMany)
                {
                activePpu->mem[activePpu->regP] = 0;
                }

            activePpu->ioWaitType = WaitNone;
            activePpu->stopped = FALSE;
            activePpu->regP = activePpu->mem[0];
            PpIncrement(activePpu->regP);
            }

        if (activePpu->ioWaitType == WaitInOne && !activePpu->channel->active && activePpu->channel->id != ChClock)
            {
            /*
            **  Channel with IAN has been deactivated (ignore RTC channel - it may be active or not - nobody cares).
            */
            activeChannel = activePpu->channel;
            activeChannel->ioDevice = NULL;
            activeChannel->full = FALSE;
            activePpu->ioWaitType = WaitNone;
            activePpu->stopped = FALSE;
            activePpu->regA = 0;
            }

        if (activePpu->ioWaitType == WaitOutOne && !activePpu->channel->active)
            {
            activeChannel = activePpu->channel;
            activeChannel->ioDevice = NULL;
            activeChannel->full = FALSE;
            activePpu->ioWaitType = WaitNone;
            activePpu->stopped = FALSE;
            }

        if ((activePpu->ioWaitType & WaitIn) != 0)
            {
            activeChannel = activePpu->channel;


            if (!activeChannel->full)
                {
                /*
                **  Handle possible input.
                */
                channelIo();
                }

            if (activeChannel->full || activeChannel->id == 014)
                {
                /*
                **  Handle input.
                */
                switch (activePpu->ioWaitType)
                    {
                case WaitInOne:
                    activePpu->regA = activeChannel->data & Mask12;
                    activePpu->ioWaitType = WaitNone;
                    activePpu->stopped = FALSE;
                    activeChannel->full = FALSE;
                    if (activeChannel->discAfterInput)
                        {
                        activeChannel->discAfterInput = FALSE;
                        activeChannel->active = FALSE;
                        activeChannel->ioDevice = NULL;
                        }
                    break;

                case WaitInMany:
                    activePpu->mem[activePpu->regP] = activeChannel->data & Mask12;
                    activePpu->regP = (activePpu->regP + 1) & Mask12;
                    activePpu->regA = (activePpu->regA - 1) & Mask18;
                    activePpu->ppMemLen++;
                    
                    if (activeChannel->discAfterInput)
                        {
                        activeChannel->discAfterInput = FALSE;
                        activeChannel->active = FALSE;
                        activeChannel->ioDevice = NULL;
                        if (activePpu->regA != 0)
                            {
                            activePpu->mem[activePpu->regP] = 0;
                            }
                        activePpu->ioWaitType = WaitNone;
                        activePpu->stopped = FALSE;
                        activePpu->regP = activePpu->mem[0];
                        PpIncrement(activePpu->regP);
                        }
                    else if (activePpu->regA == 0)
                        {
                        activePpu->ioWaitType = WaitNone;
                        activePpu->stopped = FALSE;
                        activePpu->regP = activePpu->mem[0];
                        PpIncrement(activePpu->regP);
                        }

                    activeChannel->full = FALSE;
                    break;
                    }
                }
            }

        if ((activePpu->ioWaitType & WaitOut) != 0)
            {
            /*
            **  Handle output.
            */
            activeChannel = activePpu->channel;
            if (!activeChannel->full)
                {
                switch (activePpu->ioWaitType)
                    {
                case WaitOutOne:
                    activeChannel->data = (PpWord)activePpu->regA & Mask12;
                    activePpu->ioWaitType = WaitNone;
                    activePpu->stopped = FALSE;
                    activeChannel->full = TRUE;
                    break;

                case WaitOutMany:
                    activeChannel->data = activePpu->mem[activePpu->regP] & Mask12;
                    activePpu->regP = (activePpu->regP + 1) & Mask12;
                    activePpu->regA = (activePpu->regA - 1) & Mask18;
                    activePpu->ppMemLen++;
                    activeChannel->full = TRUE;

                    if (activePpu->regA == 0)
                        {
                        activePpu->ioWaitType = WaitNone;
                        activePpu->stopped = FALSE;
                        activePpu->regP = activePpu->mem[0];
                        PpIncrement(activePpu->regP);
                        break;
                        }

                    break;
                    }
                }

            if (activeChannel->full)
                {
                if (activeChannel->id > 014 && activeChannel->id < 020)
                    {
                    activeChannel->full = FALSE;
                    } 

                /*
                **  Handle possible output.
                */
                channelIo();
                }
            }
        }
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Status and control channel function processor
**
**  Parameters:     Name        Description.
**                  func        function word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void ppStatusAndControl(PpWord func)
    {
    static PpWord ppStatusAndControlRegister[StatusAndControlWords] = {0};
    u8 code;
    u8 designator;
    u8 word;
    u8 bit;

    code = (func >> 9) & 7;
    designator = func & 0377;

#if 0
    {
    static char *codeString[] =
        {
        "read word",
        "test bit",
        "clear bit",
        "test & clear bit",
        "set bit",
        "test & set bit",
        "clear all",
        "test all"
        };

    fprintf(devF, "S&C Reg: bit %03o %s\n", designator, codeString[code]);
    }
#endif

    /*
    **  Set a few dynamic bits.
    */
    if (cpuStopped)
        {
        ppStatusAndControlRegister[020] |= 1;
        }
    else
        {
        ppStatusAndControlRegister[020] &= ~1;
        }

    if (cpu.monitorMode)
        {
        ppStatusAndControlRegister[020] |= 010;
        }
    else
        {
        ppStatusAndControlRegister[020] &= ~010;
        }

    switch (code)
        {
    case 0:
        /*
        **  Read word.
        */
        if (designator < StatusAndControlWords)
            {
            activeChannel->data = ppStatusAndControlRegister[designator] & Mask12;
            }
        else
            {
            activeChannel->data = 0;
            }

        activeChannel->full = TRUE;
        break;

    case 1:
        /*
        **  Test bit.
        */
        word = designator / 12;
        bit = designator % 12;

        if (word < StatusAndControlWords)
            {
            activeChannel->data = (ppStatusAndControlRegister[word] & (1 << bit)) != 0 ? 1 : 0;
            }
        else
            {
            activeChannel->data = 0;
            }

        activeChannel->full = TRUE;
        break;

    case 2:
        /*
        **  Clear bit.
        */
        word = designator / 12;
        bit = designator % 12;

        if (word < StatusAndControlWords)
            {
            ppStatusAndControlRegister[word] &= ~(1 << bit);
            }

        activeChannel->data = 0;
        activeChannel->full = TRUE;
        break;

    case 3:
        /*
        **  Test bit and leave clear.
        */
        word = designator / 12;
        bit = designator % 12;

        if (word < StatusAndControlWords)
            {
            activeChannel->data = (ppStatusAndControlRegister[word] & (1 << bit)) != 0 ? 1 : 0;
            ppStatusAndControlRegister[word] &= ~(1 << bit);
            }
        else
            {
            activeChannel->data = 0;
            }

        activeChannel->full = TRUE;
        break;

    case 4:
        /*
        **  Set bit.
        */
        word = designator / 12;
        bit = designator % 12;

        if (word < StatusAndControlWords)
            {
            ppStatusAndControlRegister[word] |= (1 << bit);
            }

        activeChannel->data = 0;
        activeChannel->full = TRUE;
        break;

    case 5:
        /*
        **  Test bit and leave set.
        */
        word = designator / 12;
        bit = designator % 12;

        if (word < StatusAndControlWords)
            {
            activeChannel->data = (ppStatusAndControlRegister[word] & (1 << bit)) != 0 ? 1 : 0;
            ppStatusAndControlRegister[word] |= (1 << bit);
            }
        else
            {
            activeChannel->data = 0;
            }

        activeChannel->full = TRUE;
        break;

    case 6:
        /*
        **  Clear all bits.
        */
        for (word = 0; word < StatusAndControlWords; word++)
            {
            ppStatusAndControlRegister[word] = 0;
            }

        activeChannel->data = 0;
        activeChannel->full = TRUE;
        break;

    case 7:
        /*
        **  Test all error bits and return one if any set.
        */
        activeChannel->data = 0;
        for (word = 0; word < 4; word++)
            {
            if (word == 3)
                {
                if ((ppStatusAndControlRegister[word] & 017) != 0)
                    {
                    activeChannel->data = 1;
                    break;
                    }
                }
            else if (ppStatusAndControlRegister[word] != 0)
                {
                activeChannel->data = 1;
                break;
                }
            }

        activeChannel->full = TRUE;
        break;
        }
    }
    
/*--------------------------------------------------------------------------
**  Purpose:        Interlock function processor
**
**  Parameters:     Name        Description.
**                  func        function word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void ppInterlock(PpWord func)
    {
    static PpWord ppInterlockRegister[InterlockWords] = {0};
    u8 code;
    u8 designator;
    u8 word;
    u8 bit;

    code = (func >> 9) & 7;
    designator = func & 0177;

    switch (code)
        {
    case 0:
        /*
        **  Read word.
        */
        if (designator < InterlockWords)
            {
            activeChannel->data = ppInterlockRegister[designator] & Mask12;
            }
        else
            {
            activeChannel->data = 0;
            }

        activeChannel->full = TRUE;
        break;

    case 1:
        /*
        **  Test bit.
        */
        word = designator / 12;
        bit = designator % 12;

        if (word < InterlockWords)
            {
            activeChannel->data = (ppInterlockRegister[word] & (1 << bit)) != 0 ? 1 : 0;
            }
        else
            {
            activeChannel->data = 0;
            }

        activeChannel->full = TRUE;
        break;

    case 2:
        /*
        **  Clear bit.
        */
        word = designator / 12;
        bit = designator % 12;

        if (word < InterlockWords)
            {
            ppInterlockRegister[word] &= ~(1 << bit);
            }

        activeChannel->data = 0;
        activeChannel->full = TRUE;
        break;

    case 3:
        /*
        **  Test bit and leave clear.
        */
        word = designator / 12;
        bit = designator % 12;

        if (word < InterlockWords)
            {
            activeChannel->data = (ppInterlockRegister[word] & (1 << bit)) != 0 ? 1 : 0;
            ppInterlockRegister[word] &= ~(1 << bit);
            }
        else
            {
            activeChannel->data = 0;
            }

        activeChannel->full = TRUE;
        break;

    case 4:
        /*
        **  Set bit.
        */
        word = designator / 12;
        bit = designator % 12;

        if (word < InterlockWords)
            {
            ppInterlockRegister[word] |= (1 << bit);
            }

        activeChannel->data = 0;
        activeChannel->full = TRUE;
        break;

    case 5:
        /*
        **  Test bit and leave set.
        */
        word = designator / 12;
        bit = designator % 12;

        if (word < InterlockWords)
            {
            activeChannel->data = (ppInterlockRegister[word] & (1 << bit)) != 0 ? 1 : 0;
            ppInterlockRegister[word] |= (1 << bit);
            }
        else
            {
            activeChannel->data = 0;
            }

        activeChannel->full = TRUE;
        break;

    case 6:
        /*
        **  Clear all bits.
        */
        for (word = 0; word < InterlockWords; word++)
            {
            ppInterlockRegister[word] = 0;
            }

        activeChannel->data = 0;
        activeChannel->full = TRUE;
        break;

    case 7:
        /*
        **  Test all bits and return one if any set.
        */
        activeChannel->data = 0;
        for (word = 0; word < InterlockWords; word++)
            {
            if (ppInterlockRegister[word] != 0)
                {
                activeChannel->data = 1;
                break;
                }
            }

        activeChannel->full = TRUE;
        break;
        }
    }
    
/*--------------------------------------------------------------------------
**  Purpose:        Functions to implement all opcodes
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void ppOpPSN(void)     // 00
    {
    /*
    **  Do nothing.
    */
#if CcDebug == 1
    /*
    **  For tracing under program control, a PSN opcode of 2542 
    **  turns on tracing for this PPU, and 2576 turns it off.
    **  Note that 25xx is normally an undefined opcode, but it's handled
    **  as a PSN.
    */
    if (opCode == 02542)
        {
        traceMask |= (1 << activePpu->id);
        traceClearMask |= (1 << activePpu->id);
        }
    else if (opCode == 02576)
        {
        traceMask &= ~(1 << activePpu->id);
        traceClearMask &= ~(1 << activePpu->id);
        }
#endif
    }

static void ppOpLJM(void)     // 01
    {
    IndexLocation;
    activePpu->regP = location;
    }

static void ppOpRJM(void)     // 02
    {
    IndexLocation;
    activePpu->mem[location] = activePpu->regP;
    PpIncrement(location);
    activePpu->regP = location;
    }

static void ppOpUJN(void)     // 03
    {
    PpAddOffset(activePpu->regP, opD);
    }

static void ppOpZJN(void)     // 04
    {
    if (activePpu->regA == 0)
        {
        PpAddOffset(activePpu->regP, opD);
        }
    }

static void ppOpNJN(void)     // 05
    {
    if (activePpu->regA != 0)
        {
        PpAddOffset(activePpu->regP, opD);
        }
    }

static void ppOpPJN(void)     // 06
    {
    if (activePpu->regA < 0400000)
        {
        PpAddOffset(activePpu->regP, opD);
        }
    }

static void ppOpMJN(void)     // 07
    {
    if (activePpu->regA > 0377777)
        {
        PpAddOffset(activePpu->regP, opD);
        }
    }

static void ppOpSHN(void)     // 10
    {
    u64 acc;

    if (opD < 040)
        {
        opD = opD % 18;
        acc = activePpu->regA & Mask18;
        acc <<= opD;
        activePpu->regA = (u32)((acc & Mask18) | (acc >> 18));
        }
    else if (opD > 037)
        {
        opD = 077 - opD;
        activePpu->regA >>= opD;
        }
    }

static void ppOpLMN(void)     // 11
    {
    activePpu->regA ^= opD;
    }

static void ppOpLPN(void)     // 12
    {
    activePpu->regA &= opD;
    }

static void ppOpSCN(void)     // 13
    {
    activePpu->regA &= ~(opD & 077);
    }

static void ppOpLDN(void)     // 14
    {
    activePpu->regA = opD;
    }

static void ppOpLCN(void)     // 15
    {
    activePpu->regA = ~opD & Mask18;
    }

static void ppOpADN(void)     // 16
    {
    activePpu->regA = ppAdd18(activePpu->regA, opD);
    }

static void ppOpSBN(void)     // 17
    {
    activePpu->regA = ppSubtract18(activePpu->regA, opD);
    }

static void ppOpLDC(void)     // 20
    {
    activePpu->regA = (opD << 12) | (activePpu->mem[activePpu->regP] & Mask12);
    PpIncrement(activePpu->regP);
    }

static void ppOpADC(void)     // 21
    {
    activePpu->regA = ppAdd18(activePpu->regA, (opD << 12) | (activePpu->mem[activePpu->regP] & Mask12));
    PpIncrement(activePpu->regP);
    }

static void ppOpLPC(void)     // 22
    {
    activePpu->regA &= (opD << 12) | (activePpu->mem[activePpu->regP] & Mask12);
    PpIncrement(activePpu->regP);
    }

static void ppOpLMC(void)     // 23
    {
    activePpu->regA ^= (opD << 12) | (activePpu->mem[activePpu->regP] & Mask12);
    PpIncrement(activePpu->regP);
    }

static void ppOpEXN(void)     // 26
    {
    u32 exchangeAddress;

    if ((opD & 070) == 0)
        {
        exchangeAddress = activePpu->regA & Mask18;
        }
    else
        {
        if (cpu.monitorMode)
            {
            return;
            }

        cpu.monitorMode = TRUE;

        if ((opD & 070) == 010)
            {
            /*
            **  MXN.
            */
            exchangeAddress = activePpu->regA & Mask18;
            }
        else if ((opD & 070) == 020)
            {
            /*
            **  MAN.
            */
            exchangeAddress = cpu.regMa & Mask18;
            }
        }

    while (!cpuExchangeJump(exchangeAddress))
        {
        cpuStep();
        }
    }

static void ppOpRPN(void)     // 27
    {
    activePpu->regA = cpuGetP();
    }

static void ppOpLDD(void)     // 30
    {
    activePpu->regA = activePpu->mem[opD] & Mask12;
    activePpu->regA &= Mask18;
    }

static void ppOpADD(void)     // 31
    {
    activePpu->regA = ppAdd18(activePpu->regA, activePpu->mem[opD] & Mask12);
    }

static void ppOpSBD(void)     // 32
    {
    activePpu->regA = ppSubtract18(activePpu->regA, activePpu->mem[opD] & Mask12);
    }

static void ppOpLMD(void)     // 33
    {
    activePpu->regA ^= activePpu->mem[opD] & Mask12;
    activePpu->regA &= Mask18;
    }

static void ppOpSTD(void)     // 34
    {
    activePpu->mem[opD] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpRAD(void)     // 35
    {
    activePpu->regA = ppAdd18(activePpu->regA, activePpu->mem[opD] & Mask12);
    activePpu->mem[opD] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpAOD(void)     // 36
    {
    activePpu->regA = ppAdd18(activePpu->mem[opD] & Mask12, 1);
    activePpu->mem[opD] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpSOD(void)     // 37
    {
    activePpu->regA = ppSubtract18(activePpu->mem[opD] & Mask12, 1);
    activePpu->mem[opD] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpLDI(void)     // 40
    {
    location = activePpu->mem[opD] & Mask12;
    activePpu->regA = activePpu->mem[location] & Mask12;
    }

static void ppOpADI(void)     // 41
    {
    location = activePpu->mem[opD] & Mask12;
    activePpu->regA = ppAdd18(activePpu->regA, activePpu->mem[location] & Mask12);
    }

static void ppOpSBI(void)     // 42
    {
    location = activePpu->mem[opD] & Mask12;
    activePpu->regA = ppSubtract18(activePpu->regA, activePpu->mem[location] & Mask12);
    }

static void ppOpLMI(void)     // 43
    {
    location = activePpu->mem[opD] & Mask12;
    activePpu->regA ^= activePpu->mem[location] & Mask12;
    activePpu->regA &= Mask18;
    }

static void ppOpSTI(void)     // 44
    {
    location = activePpu->mem[opD] & Mask12;
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpRAI(void)     // 45
    {
    location = activePpu->mem[opD] & Mask12;
    activePpu->regA = ppAdd18(activePpu->regA, activePpu->mem[location] & Mask12);
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpAOI(void)     // 46
    {
    location = activePpu->mem[opD] & Mask12;
    activePpu->regA = ppAdd18(activePpu->mem[location] & Mask12, 1);
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpSOI(void)     // 47
    {
    location = activePpu->mem[opD] & Mask12;
    activePpu->regA = ppSubtract18(activePpu->mem[location] & Mask12, 1);
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpLDM(void)     // 50
    {
    IndexLocation;
    activePpu->regA = activePpu->mem[location] & Mask12;
    }

static void ppOpADM(void)     // 51
    {
    IndexLocation;
    activePpu->regA = ppAdd18(activePpu->regA, activePpu->mem[location] & Mask12);
    }

static void ppOpSBM(void)     // 52
    {
    IndexLocation;
    activePpu->regA = ppSubtract18(activePpu->regA, activePpu->mem[location] & Mask12);
    }

static void ppOpLMM(void)     // 53
    {
    IndexLocation;
    activePpu->regA ^= activePpu->mem[location] & Mask12;
    }

static void ppOpSTM(void)     // 54
    {
    IndexLocation;
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpRAM(void)     // 55
    {
    IndexLocation;
    activePpu->regA = ppAdd18(activePpu->regA, activePpu->mem[location] & Mask12);
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpAOM(void)     // 56
    {
    IndexLocation;
    activePpu->regA = ppAdd18(activePpu->mem[location] & Mask12, 1);
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpSOM(void)     // 57
    {
    IndexLocation;
    activePpu->regA = ppSubtract18(activePpu->mem[location] & Mask12, 1);
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpCRD(void)     // 60
    {
    CpWord data;

    cpuMemStart = activePpu->regA & Mask18;
    cpuMemLen = 1;
    if (cpuPpReadMem(activePpu->regA & Mask18, &data))
        {
        activePpu->mem[opD++ & Mask12] = (PpWord)((data >> 48) & Mask12);
        activePpu->mem[opD++ & Mask12] = (PpWord)((data >> 36) & Mask12);
        activePpu->mem[opD++ & Mask12] = (PpWord)((data >> 24) & Mask12);
        activePpu->mem[opD++ & Mask12] = (PpWord)((data >> 12) & Mask12);
        activePpu->mem[opD   & Mask12] = (PpWord)((data      ) & Mask12);
        }
    }

static void ppOpCRM(void)     // 61
    {
    CpWord data;
    PpWord length;

    activePpu->mem[0] = activePpu->regP;
    location = activePpu->mem[activePpu->regP] & Mask12;
    cpuMemStart = activePpu->regA & Mask18;
    cpuMemLen = length = activePpu->mem[opD] & Mask12;

    while (length--)
        {
        if (cpuPpReadMem(activePpu->regA & Mask18, &data))
            {
            activePpu->mem[location++ & Mask12] = (PpWord)((data >> 48) & Mask12);
            activePpu->mem[location++ & Mask12] = (PpWord)((data >> 36) & Mask12);
            activePpu->mem[location++ & Mask12] = (PpWord)((data >> 24) & Mask12);
            activePpu->mem[location++ & Mask12] = (PpWord)((data >> 12) & Mask12);
            activePpu->mem[location++ & Mask12] = (PpWord)((data      ) & Mask12);
            }

        activePpu->regA += 1;
        activePpu->regA &= Mask18;
        }                                                             

    activePpu->regP = activePpu->mem[0];
    PpIncrement(activePpu->regP);
    }

static void ppOpCWD(void)     // 62
    {
    CpWord data;

    data = activePpu->mem[opD++ & Mask12] & Mask12;
    data <<= 12;

    data |= activePpu->mem[opD++ & Mask12] & Mask12;
    data <<= 12;

    data |= activePpu->mem[opD++ & Mask12] & Mask12;
    data <<= 12;

    data |= activePpu->mem[opD++ & Mask12] & Mask12;
    data <<= 12;

    data |= activePpu->mem[opD   & Mask12] & Mask12;

    cpuMemStart = activePpu->regA & Mask18;
    cpuMemLen = 1;
    cpuPpWriteMem(activePpu->regA & Mask18, data); 
    }

static void ppOpCWM(void)     // 63
    {
    CpWord data;
    PpWord length;

    activePpu->mem[0] = activePpu->regP;
    location = activePpu->mem[activePpu->regP] & Mask12;
    cpuMemStart = activePpu->regA & Mask18;
    cpuMemLen = length = activePpu->mem[opD] & Mask12;

    while (length--)
        {
        data = activePpu->mem[location++ & Mask12] & Mask12;
        data <<= 12;

        data |= activePpu->mem[location++ & Mask12] & Mask12;
        data <<= 12;

        data |= activePpu->mem[location++ & Mask12] & Mask12;
        data <<= 12;

        data |= activePpu->mem[location++ & Mask12] & Mask12;
        data <<= 12;

        data |= activePpu->mem[location++ & Mask12] & Mask12;

        cpuPpWriteMem(activePpu->regA & Mask18, data); 

        activePpu->regA += 1;
        activePpu->regA &= Mask18;
        }

    PpIncrement(activePpu->regP);
    }

static void ppOpAJM(void)     // 64
    {
    opD &= 037;
    location = activePpu->mem[activePpu->regP];
    location &= Mask12;
    PpIncrement(activePpu->regP);

    if (opD < channelCount && channel[opD].active)
        {
        activePpu->regP = location;
        }
    }

static void ppOpIJM(void)     // 65
    {
    opD &= 037;
    location = activePpu->mem[activePpu->regP];
    location &= Mask12;
    PpIncrement(activePpu->regP);

    if (opD >= channelCount || !channel[opD].active)
        {
        activePpu->regP = location;
        }
    }

static void ppOpFJM(void)     // 66
    {
    location = activePpu->mem[activePpu->regP];
    location &= Mask12;
    PpIncrement(activePpu->regP);

    opD &= 037;
    activeChannel = channel + opD;
    activePpu->channel = activeChannel;
    channelIo();

    if (opD < channelCount && activeChannel->full)
        {
        activePpu->regP = location;
        }
    }

static void ppOpEJM(void)     // 67
    {
    location = activePpu->mem[activePpu->regP];
    location &= Mask12;
    PpIncrement(activePpu->regP);

    opD &= 037;
    activeChannel = channel + opD;
    activePpu->channel = activeChannel;
    channelIo();

    if (opD >= channelCount || !activeChannel->full)
        {
        activePpu->regP = location;
        }
    }

static void ppOpIAN(void)     // 70
    {
    noHang = (opD & 040) != 0;
    opD &= 037;
    activeChannel = channel + opD;
    activePpu->channel = activeChannel;

    if (!activeChannel->active && opD != 014)
        {
        if (!noHang)
            {
            PpDecrement(activePpu->regP);
            }
        return;
        }

    activePpu->ioWaitType = WaitInOne;
    activePpu->stopped = TRUE;
    activePpu->ioFlag = TRUE;
    }

static void ppOpIAM(void)     // 71
    {
    opD &= 037;
    activeChannel = channel + opD;
    activePpu->channel = activeChannel;
    activePpu->ppMemStart = location = activePpu->mem[activePpu->regP] & Mask12;
    activePpu->ioFlag = TRUE;

    if (!activeChannel->active)
        {
        activePpu->mem[location] = 0;
        activePpu->mem[0] = activePpu->regP;
        PpIncrement(activePpu->regP);
        }
    else
        {
        activePpu->mem[0] = activePpu->regP;
        activePpu->regP = location;
        activePpu->ioWaitType = WaitInMany;
        activePpu->stopped = TRUE;
        }
    }

static void ppOpOAN(void)     // 72
    {
    noHang = (opD & 040) != 0;
    opD &= 037;
    activeChannel = channel + opD;
    activePpu->channel = activeChannel;

    /*
    **  Interlock register function.
    */
    if (opD == ChStatusAndControl)
        {
        ppStatusAndControl((PpWord)(activePpu->regA & Mask12));
        return;
        }

    /*
    **  S/C register function.
    */
    if (opD == ChInterlock)
        {
        ppInterlock((PpWord)(activePpu->regA & Mask12));
        return;
        }

    if (!activeChannel->active)
        {
        if (!noHang)
            {
            PpDecrement(activePpu->regP);
            }
        return;
        }

    activePpu->ioWaitType = WaitOutOne;
    activePpu->stopped = TRUE;
    activePpu->ioFlag = TRUE;
    }

static void ppOpOAM(void)     // 73
    {
    opD &= 037;
    activeChannel = channel + opD;
    activePpu->channel = activeChannel;
    activePpu->mem[0] = activePpu->regP;
    activePpu->ppMemStart = activePpu->regP = activePpu->mem[activePpu->regP] & Mask12;
    activePpu->ioWaitType = WaitOutMany;
    activePpu->stopped = TRUE;
    activePpu->ioFlag = TRUE;
    }

static void ppOpACN(void)     // 74
    {
    noHang = (opD & 040) != 0;
    opD &= 037;
    /*
    **  Interlock and S/C register channels need not be activated.
    */
    if (opD == ChInterlock || opD == ChStatusAndControl)
        {
        return;
        }

    activeChannel = channel + opD;
    activePpu->channel = activeChannel;

    if (activeChannel->active)
        {
        if (!noHang)
            {
            PpDecrement(activePpu->regP);
            }
        return;
        }

    channelActivate();
    }

static void ppOpDCN(void)     // 75
    {
    noHang = (opD & 040) != 0;
    opD &= 037;

    /*
    **  Interlock and S/C register channel can not be deactivated.
    */
    if (opD == ChInterlock || opD == ChStatusAndControl)
        {
        return;
        }

    activeChannel = channel + opD;
    activePpu->channel = NULL;

    if (!activeChannel->active)
        {
        if (!noHang)
            {
            PpDecrement(activePpu->regP);
            }
        return;
        }

    channelDisconnect();
    }

static void ppOpFAN(void)     // 76
    {
    noHang = (opD & 040) != 0;
    opD &= 037;
    activeChannel = channel + opD;

    /*
    **  Interlock and S/C register channels ignore functions.
    */
    if (opD == ChInterlock || opD == ChStatusAndControl)
        {
        return;
        }

    if (activeChannel->active)
        {
        if (!noHang)
            {
            PpDecrement(activePpu->regP);
            }
        return;
        }

    channelFunction((PpWord)(activePpu->regA & Mask12));
    activePpu->ioFlag = TRUE;
    }

static void ppOpFNC(void)     // 77
    {
    noHang = (opD & 040) != 0;
    opD &= 037;
    activeChannel = channel + opD;

    /*
    **  Interlock and S/C register channels ignore functions.
    */
    if (opD == ChInterlock || opD == ChStatusAndControl)
        {
        return;
        }

    if (activeChannel->active)
        {
        if (!noHang)
            {
            PpDecrement(activePpu->regP);
            }
        return;
        }

    channelFunction((PpWord)(activePpu->mem[activePpu->regP] & Mask12));
    PpIncrement(activePpu->regP);
    activePpu->ioFlag = TRUE;
    }

/*---------------------------  End Of File  ------------------------------*/
