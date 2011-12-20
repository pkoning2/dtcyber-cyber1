/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Paul Koning, Tom Hunter (see license.txt)
**
**  Name: ddp.c
**
**  Description:
**      Perform simulation of CDC Distributive Data Path
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

#define DEBUG                    0
#define DEBUGE                   0

/*
**  DDP function and status codes.
*/
#define FcDdpReadECS             05001
#define FcDdpWriteECS            05002
#define FcDdpStatus              05004
#define FcDdpMasterClear         05010

/*
**      Status reply flags
**
**      0001 = ECS abort
**      0002 = ECS accept
**      0004 = ECS parity error
**      0010 = ECS write pending
**      0020 = Channel parity error
**      0040 = 6640 parity error
*/
#define StDdpAbort               00001
#define StDdpAccept              00002
#define StDdpParErr              00004
#define StDdpWrite               00010
#define StDdpChParErr            00020
#define StDdp6640ParErr          00040

/*
**  DDP magical ECS address bits
*/
#define DdpAddrReadOne           (01 << 21)
#define DdpAddrMaint             (02 << 21)
#define DdpAddrFlagReg           (04 << 21)

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/

typedef struct
    {
    CpWord  curword;
    u32     addr;
    int     dbyte;
    int     abyte;
    int     endaddrcycle;
    PpWord  stat;
    } DdpContext;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus ddpFunc(ChSlot *activeChannel, DevSlot *activeDevice,
                        PpWord funcCode);
static void ddpIo(ChSlot *activeChannel, DevSlot *activeDevice);
static void ddpActivate(ChSlot *activeChannel, DevSlot *activeDevice);
static void ddpDisconnect(ChSlot *activeChannel, DevSlot *activeDevice);

/*
**  ----------------
**  Public Variables
**  ----------------
*/

/*
**  -----------------
**  Private Variables
**  -----------------
*/

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialise DDP.
**
**  Parameters:     Name        Description.
**                  model       Cyber model number
**                  increment   clock increment per iteration.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ddpInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    DevSlot *dp;
    DdpContext *dc;
    
    (void)eqNo;
    (void)unitNo;
    (void)deviceName;

    if (ecsMaxMemory == 0)
        {
        fprintf (stderr, "Cannot configure DDP, no ECS configured\n");
        exit (1);
        }
    
    dp = channelAttach(channelNo, eqNo, DtDdp);

    dp->activate = ddpActivate;
    dp->disconnect = ddpDisconnect;
    dp->func = ddpFunc;
    dp->io = ddpIo;

    dc = calloc (1, sizeof (DdpContext));
    if (dc == NULL)
        {
        fprintf (stderr, "Failed to allocate DDP context block\n");
        exit (1);
        }

    dp->context[unitNo] = dc;
    dc->stat = StDdpAccept;

    /*
    **  Print a friendly message.
    */
    printf ("DDP initialised on channel %o\n", channelNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on DDP device.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static FcStatus ddpFunc(ChSlot *activeChannel, DevSlot *activeDevice,
                        PpWord funcCode)
    {
    DdpContext *dc;

    dc = (DdpContext *) (activeDevice->context[activeDevice->selectedUnit]);

    switch (funcCode)
        {
    default:
        break;

    case FcDdpReadECS:
    case FcDdpWriteECS:
    case FcDdpStatus:
        dc->abyte = 0;
        dc->dbyte = 0;
        dc->addr = 0;
        activeDevice->fcode = funcCode;
        return(FcAccepted);

    case FcDdpMasterClear:
        activeDevice->fcode = 0;
        dc->stat = StDdpAccept;
        return(FcProcessed);
        }

    return(FcDeclined);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void ddpIo(ChSlot *activeChannel, DevSlot *activeDevice)
    {
    DdpContext *dc;

    dc = (DdpContext *) (activeDevice->context[activeDevice->selectedUnit]);

    switch (activeDevice->fcode)
        {
    default:
        return;
        
    case FcDdpStatus:
        if (!activeChannel->full)
            {
            if (DEBUG)
                printf ("ddp status %04o\n", dc->stat);
            activeChannel->data = dc->stat;
            activeChannel->full = TRUE;
            activeDevice->fcode = 0;
            // ? activeChannel->discAfterInput = TRUE;
            }
        break;
        
    case FcDdpReadECS:
    case FcDdpWriteECS:
        if (dc->abyte < 2)
            {
            /*
            **  We need to get two address bytes from the PPU.
            */
            if (activeChannel->full)
                {
                dc->addr <<= 12;
                dc->addr += activeChannel->data;
                if (DEBUG)
                    printf ("address %08o byte %d\n", dc->addr, dc->abyte);
                dc->abyte++;
                activeChannel->full = FALSE;
                }

            if (dc->abyte == 2 && activeDevice->fcode == FcDdpReadECS)
                {
                /*
                **  Delay a bit before we set channel full.
                */
                dc->endaddrcycle = cycles;

                /*
                **  A flag register reference occurs when bit 23 is set address.
                */
                if ((dc->addr & DdpAddrFlagReg) != 0)
                    {
                    if (cpuEcsFlagRegister(dc->addr))
                        {
                        dc->stat = StDdpAccept;
                        }
                    else
                        {
                        activeChannel->discAfterInput = TRUE;
                        dc->stat = StDdpAbort;
                        }

                    dc->dbyte = 0;
                    dc->curword = 0;
                    }
                else
                    {
                    dc->dbyte = -1;
                    }
                }

            break;
            }

        if (activeDevice->fcode == FcDdpReadECS)
            {
            if (!activeChannel->full && cycles - dc->endaddrcycle > 20)
                {
                if (dc->dbyte == -1)
                    {
                    /*
                    **  Fetch next 60 bits from ECS.
                    */
                    if (cpuDdpTransfer(dc->addr, &dc->curword, FALSE))
                        {
                        dc->stat = StDdpAccept;
                        }
                    else
                        {
                        if (DEBUGE)
                            printf ("ddp read abort addr %08o\n", dc->addr);
                        activeChannel->discAfterInput = TRUE;
                        dc->stat = StDdpAbort;
                        }

                    dc->dbyte = 0;
                    }

                /*
                **  Return next byte to PPU.
                */
                activeChannel->data = (PpWord)((dc->curword >> 48) & Mask12);
                activeChannel->full = TRUE;

                /*
                **  Update admin stuff.
                */
                dc->curword <<= 12;
                if (++dc->dbyte == 5)
                    {
                    if (dc->addr & (DdpAddrReadOne | DdpAddrFlagReg))
                        {
                        activeChannel->discAfterInput = TRUE;
                        }

                    dc->dbyte = -1;
                    dc->addr++;
                    }
                }
            }
        else if (activeChannel->full)
            {
            dc->stat = StDdpAccept;
            dc->curword <<= 12;
            dc->curword += activeChannel->data;
            activeChannel->full = FALSE;
            if (DEBUG)
                printf ("ddp write addr %08o data %04o byte %d\n",
                        dc->addr, activeChannel->data, dc->dbyte);
            if (++dc->dbyte == 5)
                {
                /*
                **  Write next 60 bit to ECS.
                */
                if (!cpuDdpTransfer(dc->addr, &dc->curword, TRUE))
                    {
                    if (DEBUGE)
                        printf ("ddp write abort addr %08o\n", dc->addr);
                    activeChannel->active = FALSE;
                    dc->stat = StDdpAbort;
                    return;
                    }

                dc->dbyte = 0;
                dc->addr++;
                }
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle channel activation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void ddpActivate(ChSlot *activeChannel, DevSlot *activeDevice)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle disconnecting of channel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void ddpDisconnect(ChSlot *activeChannel, DevSlot *activeDevice)
    {
#if 0
    DdpContext *dc;

    dc = (DdpContext *) (activeDevice->context[activeDevice->selectedUnit]);

    if (activeDevice->fcode == FcDdpWriteECS && dc->dbyte > 0)
    {
        printf ("*** ddp: write disconnect on partial word boundary\n");
    }
#endif
    }

/*---------------------------  End Of File  ------------------------------*/
