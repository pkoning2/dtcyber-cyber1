#define CcDebug 1
/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
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

#define DebugOps 1

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
    PpWord location;                                                        \
    if (opD != 0)                                                           \
        {                                                                   \
        location = activePpu->mem[opD] + activePpu->mem[activePpu->regP];   \
        }                                                                   \
    else                                                                    \
        {                                                                   \
        location = activePpu->mem[activePpu->regP];                         \
        }                                                                   \
    if ((location & Overflow12) != 0 || (location & Mask12) == 07777)       \
        {                                                                   \
        location += 1;                                                      \
        }                                                                   \
    location &= Mask12;                                                     \
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
static void ppOpPSN (PpSlot *activePpu, PpByte opD);    // 00
static void ppOpLJM (PpSlot *activePpu, PpByte opD);    // 01
static void ppOpRJM (PpSlot *activePpu, PpByte opD);    // 02
static void ppOpUJN (PpSlot *activePpu, PpByte opD);    // 03
static void ppOpZJN (PpSlot *activePpu, PpByte opD);    // 04
static void ppOpNJN (PpSlot *activePpu, PpByte opD);    // 05
static void ppOpPJN (PpSlot *activePpu, PpByte opD);    // 06
static void ppOpMJN (PpSlot *activePpu, PpByte opD);    // 07
static void ppOpSHN (PpSlot *activePpu, PpByte opD);    // 10
static void ppOpLMN (PpSlot *activePpu, PpByte opD);    // 11
static void ppOpLPN (PpSlot *activePpu, PpByte opD);    // 12
static void ppOpSCN (PpSlot *activePpu, PpByte opD);    // 13
static void ppOpLDN (PpSlot *activePpu, PpByte opD);    // 14
static void ppOpLCN (PpSlot *activePpu, PpByte opD);    // 15
static void ppOpADN (PpSlot *activePpu, PpByte opD);    // 16
static void ppOpSBN (PpSlot *activePpu, PpByte opD);    // 17
static void ppOpLDC (PpSlot *activePpu, PpByte opD);    // 20
static void ppOpADC (PpSlot *activePpu, PpByte opD);    // 21
static void ppOpLPC (PpSlot *activePpu, PpByte opD);    // 22
static void ppOpLMC (PpSlot *activePpu, PpByte opD);    // 23
static void ppOpPSN (PpSlot *activePpu, PpByte opD);    // 24
static void ppOpPSN (PpSlot *activePpu, PpByte opD);    // 25
static void ppOpEXN (PpSlot *activePpu, PpByte opD);    // 26
static void ppOpRPN (PpSlot *activePpu, PpByte opD);    // 27
static void ppOpLDD (PpSlot *activePpu, PpByte opD);    // 30
static void ppOpADD (PpSlot *activePpu, PpByte opD);    // 31
static void ppOpSBD (PpSlot *activePpu, PpByte opD);    // 32
static void ppOpLMD (PpSlot *activePpu, PpByte opD);    // 33
static void ppOpSTD (PpSlot *activePpu, PpByte opD);    // 34
static void ppOpRAD (PpSlot *activePpu, PpByte opD);    // 35
static void ppOpAOD (PpSlot *activePpu, PpByte opD);    // 36
static void ppOpSOD (PpSlot *activePpu, PpByte opD);    // 37
static void ppOpLDI (PpSlot *activePpu, PpByte opD);    // 40
static void ppOpADI (PpSlot *activePpu, PpByte opD);    // 41
static void ppOpSBI (PpSlot *activePpu, PpByte opD);    // 42
static void ppOpLMI (PpSlot *activePpu, PpByte opD);    // 43
static void ppOpSTI (PpSlot *activePpu, PpByte opD);    // 44
static void ppOpRAI (PpSlot *activePpu, PpByte opD);    // 45
static void ppOpAOI (PpSlot *activePpu, PpByte opD);    // 46
static void ppOpSOI (PpSlot *activePpu, PpByte opD);    // 47
static void ppOpLDM (PpSlot *activePpu, PpByte opD);    // 50
static void ppOpADM (PpSlot *activePpu, PpByte opD);    // 51
static void ppOpSBM (PpSlot *activePpu, PpByte opD);    // 52
static void ppOpLMM (PpSlot *activePpu, PpByte opD);    // 53
static void ppOpSTM (PpSlot *activePpu, PpByte opD);    // 54
static void ppOpRAM (PpSlot *activePpu, PpByte opD);    // 55
static void ppOpAOM (PpSlot *activePpu, PpByte opD);    // 56
static void ppOpSOM (PpSlot *activePpu, PpByte opD);    // 57
static void ppOpCRD (PpSlot *activePpu, PpByte opD);    // 60
static void ppOpCRM (PpSlot *activePpu, PpByte opD);    // 61
static void ppOpCWD (PpSlot *activePpu, PpByte opD);    // 62
static void ppOpCWM (PpSlot *activePpu, PpByte opD);    // 63
static void ppOpAJM (PpSlot *activePpu, PpByte opD);    // 64
static void ppOpIJM (PpSlot *activePpu, PpByte opD);    // 65
static void ppOpFJM (PpSlot *activePpu, PpByte opD);    // 66
static void ppOpEJM (PpSlot *activePpu, PpByte opD);    // 67
static void ppOpIAN (PpSlot *activePpu, PpByte opD);    // 70
static void ppOpIAM (PpSlot *activePpu, PpByte opD);    // 71
static void ppOpOAN (PpSlot *activePpu, PpByte opD);    // 72
static void ppOpOAM (PpSlot *activePpu, PpByte opD);    // 73
static void ppOpACN (PpSlot *activePpu, PpByte opD);    // 74
static void ppOpDCN (PpSlot *activePpu, PpByte opD);    // 75
static void ppOpFAN (PpSlot *activePpu, PpByte opD);    // 76
static void ppOpFNC (PpSlot *activePpu, PpByte opD);    // 77

static void ppStep(PpSlot *activePpu);
static INLINE u32 ppAdd18(u32 op1, u32 op2);
static INLINE u32 ppSubtract18(u32 op1, u32 op2);
static void ppInterlock(PpWord func);
#ifdef USE_THREADS
static void ppCreateThread(int ppNum);
static ThreadFunRet ppThread(void *param);
#endif
/*
**  ----------------
**  Public Variables
**  ----------------
*/
PpSlot *ppu;
u8 ppuCount;
FILE *devF;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
/* Pointers used in the idle detection machinery.  */
static PpWord ia = -1;
static PpWord ir = -1;
static u32 pprl = -1;

static void (*decodePpuOpcode[]) (PpSlot *activePpu, PpByte opD) = 
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
    u32 acc18;
    
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
    u32 acc18;
    
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
    int pp;

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
        ppu[pp].state = ' ';
#ifdef USE_THREADS
        pthread_cond_init (&ppu[pp].cond, NULL);
        pthread_mutex_init (&ppu[pp].mutex, NULL);
#endif
        }

    /*
    **  Set the pointers for idle detection, if enabled.
    */
    switch (idleMode)
        {
    case 287:
        ia = 075;
        ir = 050;
        pprl = 051;
        break;
    case 0:
        ia = ir = pprl = -1;
        }
    
    /*
    **  Print a friendly message.
    */
    printf("PPs initialised (number of PPUs %o)\n", ppuCount);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Terminate PP subsystem.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ppTerminate(void)
    {
    free(ppu);
    }

void ppStart (void)
    {
    int pp;
    
    for (pp = 0; pp < ppuCount; pp++)
        {
        /*
        **  Initialise PPU.
        */
        memset(ppu + pp, 0, sizeof(ppu[0]));
        ppu[pp].id = pp;

        /*
        **  Assign PPs to the corresponding channels.
        */
        if (pp < 012)
            {
            ppu[pp].channel = channel + pp;
            }
        else
            {
            ppu[pp].channel = channel + (pp - 012 + 020);
            }

        /*
        **  Set all PPs to INPUT (71) instruction.
        */
        ppu[pp].ioWaitType = WaitInMany;
        ppu[pp].stopped = TRUE;
        ppu[pp].state = 'R';
        ppu[pp].ioFlag = TRUE;
        ppu[pp].ppMemLen = 0;
        ppu[pp].ppMemStart = 0;
#if CcDebug == 1
        traceSequence(&ppu[pp]);
#endif
        /*
        **  Clear P registers and location zero of each PP.
        */
        ppu[pp].regP   = 0;
        ppu[pp].mem[0] = 0;

        /*
        **  Set all A registers to an input word count of 10000.
        */
        ppu[pp].regA = 010000;

#ifdef USE_THREADS
        /*
        **  Start thread for PPU
        */
        ppCreateThread (pp);
#endif
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Create a thread for a PP
**
**  Parameters:     Name        Description.
**                  ppNum       Pp number to set up
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
#ifdef USE_THREADS
static void ppCreateThread(int ppNum)
    {
#if defined(_WIN32)
#error "Windows multithread PP support TBS"
#else
    int rc;

    /*
    **  Create POSIX thread with default attributes.
    */
    rc = pthread_create(&ppu[ppNum].thread, NULL, ppThread, ppu + ppNum);
    if (rc < 0)
        {
        fprintf(stderr, "Failed to create pp %d thread\n", ppNum);
        exit(1);
        }
    printf ("pp %d thread running\n", ppNum);
#endif
    }

/*--------------------------------------------------------------------------
**  Purpose:        Thread execution function for a PP
**
**  Parameters:     Name        Description.
**                  param       pointer to PP context
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static ThreadFunRet ppThread(void *param)
    {
    PpSlot *activePpu = (PpSlot *) param;

    for (;;)
        {
        ppStep (activePpu);
        }
    }
#endif /* USE_THREADS */

#ifndef USE_THREADS
/*--------------------------------------------------------------------------
**  Purpose:        Execute one instruction in all PPUs.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ppStepAll (void)
    {
    u8 i;

    /*
    **  Locate a PPU which is active (ie not waiting for I/O)
    */
    for (i = 0; i < ppuCount; i++)
        {
        /*
        **  Step one PPU.
        */
        ppStep (ppu + i);
        }
    }
#endif /* USE_THREADS */

/*--------------------------------------------------------------------------
**  Purpose:        Execute one instruction in a PPU.
**
**  Parameters:     Name        Description.
**                  activePpu   Pointer to PPU slot state
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void ppStep(PpSlot *activePpu)
    {
    ChSlot *activeChannel;
    PpWord opCode;
    PpByte opF, opD;
    
    /*
    **  Check if this one is active.
    */
    if (!activePpu->stopped)
        {
        /*
        **  If we need to make this PP wait a bit, do so.
        **  (We do that for CWM, not as much as the real
        **  hardware would but enough that some delay
        **  sensitive crud like 1MT/1LT interaction works.)
        */
        if (activePpu->delay)
            {
            --activePpu->delay;
            return;
            }
            
        /*
        **  Extract next PPU instruction.
        */
        opCode = activePpu->mem[activePpu->regP];
        opF = (opCode >> 6) & 077;
        opD = opCode & 077;

#if CcDebug == 1
        if (traceMask != 0)
            {
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
                traceRegisters(activePpu);

                /*
                **  Trace new channel status.
                */
                traceChannel(activePpu, opD);

                traceEnd(activePpu);
                /*
                **  Trace memory touched by IAM/OAM, and if necessary,
                **  A register after IAM/IAN/OAM/OAN
                */
                if (activePpu->ppMemLen > 0)
                    {
                    tracePM (activePpu);
                    activePpu->ppMemLen = 0;
                    }
                }
            
            /*
            **  Trace instructions.
            */
            traceSequence(activePpu);
            traceRegisters(activePpu);
            traceOpcode(activePpu);
            }
#endif

        /*
        **  Increment register P.
        */
        PpIncrement(activePpu->regP);

        /*
        **  Execute PPU instruction.
        */
        activePpu->ioFlag = FALSE;
        decodePpuOpcode[opF](activePpu, opD);

#if CcDebug == 1
        /*
        **  If the current instruction is an I/O, then we do
        **  the last few trace steps (registers after, etc.) later
        **  rather than right after instruction execution, so that 
        **  the information displayed reflects the completion of any
        **  I/O that was done.  
        */
        if (traceMask != 0 && !activePpu->ioFlag)
            {
            /*
            **  Trace result.
            */
            traceRegisters(activePpu);

            /*
            **  Trace new channel status.
            */
            if (opF >= 064)
                {
                traceChannel(activePpu, opD);
                }

            traceEnd(activePpu);
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
        activePpu->state = ' ';
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
        activePpu->state = ' ';
        activePpu->regA = 0;
        }

    if (activePpu->ioWaitType == WaitOutOne && !activePpu->channel->active)
        {
        activeChannel = activePpu->channel;
        activeChannel->ioDevice = NULL;
        activeChannel->full = FALSE;
        activePpu->ioWaitType = WaitNone;
        activePpu->stopped = FALSE;
        activePpu->state = ' ';
        }

    if ((activePpu->ioWaitType & WaitIn) != 0)
        {
        activeChannel = activePpu->channel;

        while ((activePpu->ioWaitType & WaitIn) != 0)
            {
            /*
            **  Loop so long as we want input and have some
            */
            if (!activeChannel->full)
                {
                /*
                **  Handle possible input.
                */
                channelIo(activePpu, activeChannel);
                }

            if (activeChannel->full || activeChannel->id == ChClock)
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
                    activePpu->state = ' ';
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
                    activeChannel->full = FALSE;
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
                        activePpu->state = ' ';
                        activePpu->regP = activePpu->mem[0];
                        PpIncrement(activePpu->regP);
                        }
                    else if (activePpu->regA == 0)
                        {
                        activePpu->ioWaitType = WaitNone;
                        activePpu->stopped = FALSE;
                        activePpu->state = ' ';
                        activePpu->regP = activePpu->mem[0];
                        PpIncrement(activePpu->regP);
                        }

                    break;
                    }
                }
            else
                {
                /*
                **  No more data right now, look again next cycle.
                */
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
        while ((activePpu->ioWaitType & WaitOut) != 0)
            {
            /*
            **  Loop so long as we want more output and the channel can take it.
            */

            if (!activeChannel->full)
                {
                switch (activePpu->ioWaitType)
                    {
                case WaitOutOne:
                    activeChannel->data = (PpWord)activePpu->regA & Mask12;
                    activePpu->ioWaitType = WaitNone;
                    activePpu->stopped = FALSE;
                    activePpu->state = ' ';
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
                        activePpu->state = ' ';
                        activePpu->regP = activePpu->mem[0];
                        PpIncrement(activePpu->regP);
                        break;
                        }

                    break;
                    }
                }
            else
                {
                /*
                **  Channel is not ready for more data, stop looping
                */
                break;
                }
                

            if (activeChannel->full)
                {
                /*
                **  Discard data for channels we don't yet know how to deal with.
                */
                if (activeChannel->id > 014 && activeChannel->id < 020)
                    {
                    activeChannel->full = FALSE;
                    } 

                /*
                **  Handle possible output.
                */
                channelIo(activePpu, activeChannel);
                }
            else
                {
                /*
                **  We have no more data, stop looping
                */
                break;
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
    ChSlot *activeChannel = channel + ChStatusAndControl;
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
    if (cpu[0].cpuStopped)
        {
        ppStatusAndControlRegister[020] |= 1;
        }
    else
        {
        ppStatusAndControlRegister[020] &= ~1;
        }
    if (cpu[1].cpuStopped)
        {
        ppStatusAndControlRegister[020] |= 2;
        }
    else
        {
        ppStatusAndControlRegister[020] &= ~2;
        }

    if (monitorCpu >= 0)
        {
        ppStatusAndControlRegister[020] |= 010 << monitorCpu;
        }
    else
        {
        ppStatusAndControlRegister[020] &= ~030;
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
    ChSlot *activeChannel = channel + ChInterlock;
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
static void ppOpPSN (PpSlot *activePpu, PpByte opD)     // 00
    {
    /*
    **  Do nothing.
    */
#if DebugOps == 1
    PpWord opCode;

    opCode = activePpu->mem[activePpu->regP];
    
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

static void ppOpLJM (PpSlot *activePpu, PpByte opD)     // 01
    {
    IndexLocation;
    activePpu->regP = location;
    }

static void ppOpRJM (PpSlot *activePpu, PpByte opD)     // 02
    {
    IndexLocation;
    activePpu->mem[location] = activePpu->regP;
    PpIncrement(location);
    activePpu->regP = location;
    }

static void ppOpUJN (PpSlot *activePpu, PpByte opD)     // 03
    {
    PpAddOffset(activePpu->regP, opD);
    }

static void ppOpZJN (PpSlot *activePpu, PpByte opD)     // 04
    {
    if (activePpu->regA == 0)
        {
        PpAddOffset(activePpu->regP, opD);
        }
    }

static void ppOpNJN (PpSlot *activePpu, PpByte opD)     // 05
    {
    if (activePpu->regA != 0)
        {
        PpAddOffset(activePpu->regP, opD);
        }
    }

static void ppOpPJN (PpSlot *activePpu, PpByte opD)     // 06
    {
    if (activePpu->regA < 0400000)
        {
        PpAddOffset(activePpu->regP, opD);
        }
    }

static void ppOpMJN (PpSlot *activePpu, PpByte opD)     // 07
    {
    if (activePpu->regA > 0377777)
        {
        PpAddOffset(activePpu->regP, opD);
        }
    }

static void ppOpSHN (PpSlot *activePpu, PpByte opD)     // 10
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

static void ppOpLMN (PpSlot *activePpu, PpByte opD)     // 11
    {
    activePpu->regA ^= opD;
    }

static void ppOpLPN (PpSlot *activePpu, PpByte opD)     // 12
    {
    activePpu->regA &= opD;
    }

static void ppOpSCN (PpSlot *activePpu, PpByte opD)     // 13
    {
    activePpu->regA &= ~(opD & 077);
    }

static void ppOpLDN (PpSlot *activePpu, PpByte opD)     // 14
    {
    activePpu->regA = opD;
    }

static void ppOpLCN (PpSlot *activePpu, PpByte opD)     // 15
    {
    activePpu->regA = ~opD & Mask18;
    }

static void ppOpADN (PpSlot *activePpu, PpByte opD)     // 16
    {
    activePpu->regA = ppAdd18(activePpu->regA, opD);
    }

static void ppOpSBN (PpSlot *activePpu, PpByte opD)     // 17
    {
    activePpu->regA = ppSubtract18(activePpu->regA, opD);
    }

static void ppOpLDC (PpSlot *activePpu, PpByte opD)     // 20
    {
    activePpu->regA = (opD << 12) | (activePpu->mem[activePpu->regP] & Mask12);
    PpIncrement(activePpu->regP);
    }

static void ppOpADC (PpSlot *activePpu, PpByte opD)     // 21
    {
    activePpu->regA = ppAdd18(activePpu->regA, (opD << 12) | (activePpu->mem[activePpu->regP] & Mask12));
    PpIncrement(activePpu->regP);
    }

static void ppOpLPC (PpSlot *activePpu, PpByte opD)     // 22
    {
    activePpu->regA &= (opD << 12) | (activePpu->mem[activePpu->regP] & Mask12);
    PpIncrement(activePpu->regP);
    }

static void ppOpLMC (PpSlot *activePpu, PpByte opD)     // 23
    {
    activePpu->regA ^= (opD << 12) | (activePpu->mem[activePpu->regP] & Mask12);
    PpIncrement(activePpu->regP);
    }

static void ppOpEXN (PpSlot *activePpu, PpByte opD)     // 26
    {
    u32 exchangeAddress = 0;
    int cpnum = opD & 007;
    int monitor = -1;
    int ret;

    if (cpnum >= cpuCount)
    {
    cpnum = 0;
    }

    if ((opD & 070) == 0)
        {
        exchangeAddress = activePpu->regA & Mask18;
        monitor = -1;       // no change to monitor mode flag
        }
    else
        {
        if ((opD & 070) == 010)
            {
            /*
            **  MXN.
            */
            exchangeAddress = activePpu->regA & Mask18;
            monitor = 1;    // switch this CPU to monitor mode
            }
        else if ((opD & 070) == 020)
            {
            /*
            **  MAN.
            */
            exchangeAddress = 0;
            monitor = 2;    // switch this CPU to monitor mode at MA
            }
        }

    if ((ret = cpuIssueExchange (cpnum, exchangeAddress, monitor)) != 0)
        {
        /*
        **  Come here if exchange was not accepted.  There are two
        **  possible reasons: 
        **  1. An exchange is currently pending.
        **  2. A CPU is in monitor mode and monitor mode was requested.
        */
	  if (ret == 2)
	    return;

        // Must be exchange busy, retry the instruction
        PpDecrement(activePpu->regP);
        return;
        }
#ifdef USE_THREADS
	if (cpnum != 0) 
        {
	    while (exchangeCpu != -1) ;
        }
#endif
    }

static void ppOpRPN (PpSlot *activePpu, PpByte opD)     // 27
    {
    activePpu->regA = cpuGetP(opD);
    }

static void ppOpLDD (PpSlot *activePpu, PpByte opD)     // 30
    {
    activePpu->regA = activePpu->mem[opD] & Mask12;
    activePpu->regA &= Mask18;
    }

static void ppOpADD (PpSlot *activePpu, PpByte opD)     // 31
    {
    activePpu->regA = ppAdd18(activePpu->regA, activePpu->mem[opD] & Mask12);
    }

static void ppOpSBD (PpSlot *activePpu, PpByte opD)     // 32
    {
    activePpu->regA = ppSubtract18(activePpu->regA, activePpu->mem[opD] & Mask12);
    }

static void ppOpLMD (PpSlot *activePpu, PpByte opD)     // 33
    {
    activePpu->regA ^= activePpu->mem[opD] & Mask12;
    activePpu->regA &= Mask18;
    }

static void ppOpSTD (PpSlot *activePpu, PpByte opD)     // 34
    {
    activePpu->mem[opD] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpRAD (PpSlot *activePpu, PpByte opD)     // 35
    {
    activePpu->regA = ppAdd18(activePpu->regA, activePpu->mem[opD] & Mask12);
    activePpu->mem[opD] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpAOD (PpSlot *activePpu, PpByte opD)     // 36
    {
    activePpu->regA = ppAdd18(activePpu->mem[opD] & Mask12, 1);
    activePpu->mem[opD] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpSOD (PpSlot *activePpu, PpByte opD)     // 37
    {
    activePpu->regA = ppSubtract18(activePpu->mem[opD] & Mask12, 1);
    activePpu->mem[opD] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpLDI (PpSlot *activePpu, PpByte opD)     // 40
    {
    PpWord location;
    
    location = activePpu->mem[opD] & Mask12;
    activePpu->regA = activePpu->mem[location] & Mask12;
    }

static void ppOpADI (PpSlot *activePpu, PpByte opD)     // 41
    {
    PpWord location;

    location = activePpu->mem[opD] & Mask12;
    activePpu->regA = ppAdd18(activePpu->regA, activePpu->mem[location] & Mask12);
    }

static void ppOpSBI (PpSlot *activePpu, PpByte opD)     // 42
    {
    PpWord location;

    location = activePpu->mem[opD] & Mask12;
    activePpu->regA = ppSubtract18(activePpu->regA, activePpu->mem[location] & Mask12);
    }

static void ppOpLMI (PpSlot *activePpu, PpByte opD)     // 43
    {
    PpWord location;

    location = activePpu->mem[opD] & Mask12;
    activePpu->regA ^= activePpu->mem[location] & Mask12;
    activePpu->regA &= Mask18;
    }

static void ppOpSTI (PpSlot *activePpu, PpByte opD)     // 44
    {
    PpWord location;

    location = activePpu->mem[opD] & Mask12;
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpRAI (PpSlot *activePpu, PpByte opD)     // 45
    {
    PpWord location;

    location = activePpu->mem[opD] & Mask12;
    activePpu->regA = ppAdd18(activePpu->regA, activePpu->mem[location] & Mask12);
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpAOI (PpSlot *activePpu, PpByte opD)     // 46
    {
    PpWord location;

    location = activePpu->mem[opD] & Mask12;
    activePpu->regA = ppAdd18(activePpu->mem[location] & Mask12, 1);
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpSOI (PpSlot *activePpu, PpByte opD)     // 47
    {
    PpWord location;

    location = activePpu->mem[opD] & Mask12;
    activePpu->regA = ppSubtract18(activePpu->mem[location] & Mask12, 1);
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpLDM (PpSlot *activePpu, PpByte opD)     // 50
    {
    IndexLocation;
    activePpu->regA = activePpu->mem[location] & Mask12;
    }

static void ppOpADM (PpSlot *activePpu, PpByte opD)     // 51
    {
    IndexLocation;
    activePpu->regA = ppAdd18(activePpu->regA, activePpu->mem[location] & Mask12);
    }

static void ppOpSBM (PpSlot *activePpu, PpByte opD)     // 52
    {
    IndexLocation;
    activePpu->regA = ppSubtract18(activePpu->regA, activePpu->mem[location] & Mask12);
    }

static void ppOpLMM (PpSlot *activePpu, PpByte opD)     // 53
    {
    IndexLocation;
    activePpu->regA ^= activePpu->mem[location] & Mask12;
    }

static void ppOpSTM (PpSlot *activePpu, PpByte opD)     // 54
    {
    IndexLocation;
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpRAM (PpSlot *activePpu, PpByte opD)     // 55
    {
    IndexLocation;
    activePpu->regA = ppAdd18(activePpu->regA, activePpu->mem[location] & Mask12);
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpAOM (PpSlot *activePpu, PpByte opD)     // 56
    {
    IndexLocation;
    activePpu->regA = ppAdd18(activePpu->mem[location] & Mask12, 1);
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpSOM (PpSlot *activePpu, PpByte opD)     // 57
    {
    IndexLocation;
    activePpu->regA = ppSubtract18(activePpu->mem[location] & Mask12, 1);
    activePpu->mem[location] = (PpWord)activePpu->regA & Mask12;
    }

static void ppOpCRD (PpSlot *activePpu, PpByte opD)     // 60
    {
    CpWord data;
    u32 cpuMemStart;

    cpuMemStart = activePpu->regA & Mask18;
    if (cpuPpReadMem(cpuMemStart, &data))
        {
        if (((opD & Mask12) == ir && 
             cpuMemStart == activePpu->mem[ia]))
            {
            /* Input register read.  See if we're idle. */
            if (data == 0)
                {
                activePpu->state = 'X';
                }
            else
                {
                activePpu->state = ' ';
                }
            }
        activePpu->mem[opD++ & Mask12] = (PpWord)((data >> 48) & Mask12);
        activePpu->mem[opD++ & Mask12] = (PpWord)((data >> 36) & Mask12);
        activePpu->mem[opD++ & Mask12] = (PpWord)((data >> 24) & Mask12);
        activePpu->mem[opD++ & Mask12] = (PpWord)((data >> 12) & Mask12);
        activePpu->mem[opD   & Mask12] = (PpWord)((data      ) & Mask12);
        }

    /*
    **  Trace memory touched by central read/write
    */
    traceCM (activePpu, cpuMemStart, 1);
    }

static void ppOpCRM (PpSlot *activePpu, PpByte opD)     // 61
    {
    CpWord data;
    PpWord length;
    u32 cpuMemStart, cpuMemLen;
    PpWord location;

    location = activePpu->mem[activePpu->regP] & Mask12;
    cpuMemStart = activePpu->regA & Mask18;
    cpuMemLen = length = activePpu->mem[opD] & Mask12;
    activePpu->mem[0] = activePpu->regP;

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

    /*
    **  Trace memory touched by central read/write
    */
    traceCM (activePpu, cpuMemStart, cpuMemLen);

    activePpu->regP = activePpu->mem[0];
    PpIncrement(activePpu->regP);
    }

static void ppOpCWD (PpSlot *activePpu, PpByte opD)     // 62
    {
    CpWord data;
    u32 cpuMemStart;

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
    cpuPpWriteMem(cpuMemStart, data); 
    
    /* If this is an MTR request, mark PP0 not idle. */
    if (cpuMemStart == pprl && activePpu->id != 0)
        {
        ppu[0].state = ' ';
        }

    /*
    **  Trace memory touched by central read/write
    */
    traceCM (activePpu, cpuMemStart, 1);
    }

static void ppOpCWM (PpSlot *activePpu, PpByte opD)     // 63
    {
    CpWord data;
    PpWord length;
    u32 cpuMemStart, cpuMemLen;
    PpWord location;
    
    location = activePpu->mem[activePpu->regP] & Mask12;
    cpuMemStart = activePpu->regA & Mask18;
    cpuMemLen = length = activePpu->mem[opD] & Mask12;
    activePpu->mem[0] = activePpu->regP;

    /*
    **  Delay one major cycle per cm word.
    */
//    activePpu->delay = length;
    activePpu->delay = 0;
    
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

    /*
    **  Trace memory touched by central read/write
    */
    traceCM (activePpu, cpuMemStart, cpuMemLen);

    PpIncrement(activePpu->regP);
    }

static void ppOpAJM (PpSlot *activePpu, PpByte opD)     // 64
    {
    ChSlot *activeChannel;
    PpWord location;

    opD &= 037;
    location = activePpu->mem[activePpu->regP];
    location &= Mask12;
    PpIncrement(activePpu->regP);

    if (opD < channelCount)
        {
        activeChannel = channel + opD;
#ifdef USE_THREADS
        pthread_mutex_lock (&activeChannel->mutex);
#endif
        if (activeChannel->active)
            {
            activePpu->regP = location;
            }
#ifdef USE_THREADS
        pthread_mutex_unlock (&activeChannel->mutex);
#endif
        }
    }

static void ppOpIJM (PpSlot *activePpu, PpByte opD)     // 65
    {
    ChSlot *activeChannel;
    PpWord location;

    opD &= 037;
    location = activePpu->mem[activePpu->regP];
    location &= Mask12;
    PpIncrement(activePpu->regP);

    if (opD < channelCount)
        {
        activeChannel = channel + opD;
#ifdef USE_THREADS
        pthread_mutex_lock (&activeChannel->mutex);
#endif
        if (!(activeChannel->active))
            {
            activePpu->regP = location;
            }
#ifdef USE_THREADS
        pthread_mutex_unlock (&activeChannel->mutex);
#endif
        }
    else
        {
        activePpu->regP = location;
        }
    }

static void ppOpFJM (PpSlot *activePpu, PpByte opD)     // 66
    {
    ChSlot *activeChannel;
    PpWord location;

    location = activePpu->mem[activePpu->regP];
    location &= Mask12;
    PpIncrement(activePpu->regP);

    opD &= 037;
    if (opD < channelCount)
        {
        activeChannel = channel + opD;
#ifdef USE_THREADS
        pthread_mutex_lock (&activeChannel->mutex);
#endif
        channelProbe(activePpu, activeChannel);
        if (activeChannel->full)
            {
            activePpu->regP = location;
            }
#ifdef USE_THREADS
        pthread_mutex_unlock (&activeChannel->mutex);
#endif
        }
    }

static void ppOpEJM (PpSlot *activePpu, PpByte opD)     // 67
    {
    ChSlot *activeChannel;
    PpWord location;

    location = activePpu->mem[activePpu->regP];
    location &= Mask12;
    PpIncrement(activePpu->regP);

    opD &= 037;
    if (opD < channelCount)
        {
        activeChannel = channel + opD;
#ifdef USE_THREADS
        pthread_mutex_lock (&activeChannel->mutex);
#endif
        channelProbe(activePpu, activeChannel);
        if (!(activeChannel->full))
            {
            activePpu->regP = location;
            }
#ifdef USE_THREADS
        pthread_mutex_unlock (&activeChannel->mutex);
#endif
        }
    }

static void ppOpIAN (PpSlot *activePpu, PpByte opD)     // 70
    {
    ChSlot *activeChannel;
    bool noHang;

    noHang = (opD & 040) != 0;
    opD &= 037;
    activeChannel = channel + opD;
    activePpu->channel = activeChannel;

#ifdef USE_THREADS
    pthread_mutex_lock (&activeChannel->mutex);
#endif
    if (!activeChannel->active && opD != ChClock)
        {
        if (!noHang)
            {
            PpDecrement(activePpu->regP);
            activePpu->state = 'H';
            }
#ifdef USE_THREADS
        pthread_mutex_unlock (&activeChannel->mutex);
#endif
        return;
        }

    activeChannel->delayStatus = 0;
    activePpu->ioWaitType = WaitInOne;
    activePpu->stopped = TRUE;
    activePpu->ioFlag = TRUE;
    activePpu->state = 'R';
#ifdef USE_THREADS
    pthread_mutex_unlock (&activeChannel->mutex);
#endif
    }

static void ppOpIAM (PpSlot *activePpu, PpByte opD)     // 71
    {
    ChSlot *activeChannel;
    PpWord location;

    opD &= 037;
    activeChannel = channel + opD;
    activePpu->channel = activeChannel;
    activePpu->ppMemStart = location = activePpu->mem[activePpu->regP] & Mask12;
    activePpu->ioFlag = TRUE;

#ifdef USE_THREADS
    pthread_mutex_lock (&activeChannel->mutex);
#endif
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
        activeChannel->delayStatus = 0;
        activePpu->ppMemLen = 0;
        activePpu->ioWaitType = WaitInMany;
        activePpu->stopped = TRUE;
        activePpu->state = 'R';
        }
#ifdef USE_THREADS
    pthread_mutex_unlock (&activeChannel->mutex);
#endif
    }

static void ppOpOAN (PpSlot *activePpu, PpByte opD)     // 72
    {
    ChSlot *activeChannel;
    bool noHang;

    noHang = (opD & 040) != 0;
    opD &= 037;
    activeChannel = channel + opD;
    activePpu->channel = activeChannel;

    /*
    **  S/C register function.
    */
    if (opD == ChStatusAndControl)
        {
        ppStatusAndControl((PpWord)(activePpu->regA & Mask12));
        return;
        }

    /*
    **  Interlock register function.
    */
    if (opD == ChInterlock)
        {
        ppInterlock((PpWord)(activePpu->regA & Mask12));
        return;
        }

#ifdef USE_THREADS
    pthread_mutex_lock (&activeChannel->mutex);
#endif
    if (!activeChannel->active)
        {
        if (!noHang)
            {
            PpDecrement(activePpu->regP);
            activePpu->state = 'H';
            }
#ifdef USE_THREADS
        pthread_mutex_unlock (&activeChannel->mutex);
#endif
        return;
        }

    activeChannel->delayStatus = 0;
    activePpu->ioWaitType = WaitOutOne;
    activePpu->stopped = TRUE;
    activePpu->ioFlag = TRUE;
    activePpu->state = 'W';
#ifdef USE_THREADS
    pthread_mutex_unlock (&activeChannel->mutex);
#endif
    }

static void ppOpOAM (PpSlot *activePpu, PpByte opD)     // 73
    {
    ChSlot *activeChannel;

    opD &= 037;
    activeChannel = channel + opD;
    activePpu->channel = activeChannel;
#ifdef USE_THREADS
    pthread_mutex_lock (&activeChannel->mutex);
#endif
    activePpu->mem[0] = activePpu->regP;
    activePpu->ppMemStart = activePpu->regP = activePpu->mem[activePpu->regP] & Mask12;
    activeChannel->delayStatus = 0;
    activePpu->ioWaitType = WaitOutMany;
    activePpu->ppMemLen = 0;
    activePpu->stopped = TRUE;
    activePpu->ioFlag = TRUE;
    activePpu->state = 'W';
#ifdef USE_THREADS
    pthread_mutex_unlock (&activeChannel->mutex);
#endif
    }

static void ppOpACN (PpSlot *activePpu, PpByte opD)     // 74
    {
    ChSlot *activeChannel;
    bool noHang;

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

#ifdef USE_THREADS
    pthread_mutex_lock (&activeChannel->mutex);
#endif
    if (activeChannel->active)
        {
        if (!noHang)
            {
            PpDecrement(activePpu->regP);
            activePpu->state = 'H';
            }
#ifdef USE_THREADS
        pthread_mutex_unlock (&activeChannel->mutex);
#endif
        return;
        }

    channelActivate(activePpu, activeChannel);
#ifdef USE_THREADS
    pthread_mutex_unlock (&activeChannel->mutex);
#endif
    }

static void ppOpDCN (PpSlot *activePpu, PpByte opD)     // 75
    {
    ChSlot *activeChannel;
    bool noHang;

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

#ifdef USE_THREADS
    pthread_mutex_lock (&activeChannel->mutex);
#endif
    if (!activeChannel->active)
        {
        if (!noHang)
            {
            PpDecrement(activePpu->regP);
            activePpu->state = 'H';
            }
#ifdef USE_THREADS
        pthread_mutex_unlock (&activeChannel->mutex);
#endif
        return;
        }

    channelDisconnect(activePpu, activeChannel);
#ifdef USE_THREADS
    pthread_mutex_unlock (&activeChannel->mutex);
#endif
    }

static void ppOpFAN (PpSlot *activePpu, PpByte opD)     // 76
    {
    ChSlot *activeChannel;
    bool noHang;

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

#ifdef USE_THREADS
    pthread_mutex_lock (&activeChannel->mutex);
#endif
    if (activeChannel->active)
        {
        if (!noHang)
            {
            PpDecrement(activePpu->regP);
            activePpu->state = 'H';
            }
#ifdef USE_THREADS
        pthread_mutex_unlock (&activeChannel->mutex);
#endif
        return;
        }

    channelFunction(activePpu, activeChannel, activePpu->regA & Mask12);
    activePpu->ioFlag = TRUE;
#ifdef USE_THREADS
    pthread_mutex_unlock (&activeChannel->mutex);
#endif
    }

static void ppOpFNC (PpSlot *activePpu, PpByte opD)     // 77
    {
    ChSlot *activeChannel;
    bool noHang;

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

#ifdef USE_THREADS
    pthread_mutex_lock (&activeChannel->mutex);
#endif
    if (activeChannel->active)
        {
        if (!noHang)
            {
            PpDecrement(activePpu->regP);
            activePpu->state = 'H';
            }
#ifdef USE_THREADS
        pthread_mutex_unlock (&activeChannel->mutex);
#endif
        return;
        }

    channelFunction(activePpu, activeChannel,
                    activePpu->mem[activePpu->regP] & Mask12);
    PpIncrement(activePpu->regP);
    activePpu->ioFlag = TRUE;
#ifdef USE_THREADS
    pthread_mutex_unlock (&activeChannel->mutex);
#endif
    }

/*---------------------------  End Of File  ------------------------------*/
