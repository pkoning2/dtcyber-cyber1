/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Paul Koning, Tom Hunter (see license.txt)
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
#define DEBUGE                   1

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
static FcStatus ddpFunc(PpWord funcCode);
static void ddpIo(void);
static void ddpActivate(void);
static void ddpDisconnect(void);

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
    
    dp = channelAttach(channelNo, DtDdp);

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
static FcStatus ddpFunc(PpWord funcCode)
{
    DdpContext *dc;

    dc = (DdpContext *) (activeDevice->context[activeDevice->selectedUnit]);

    switch (funcCode)
    {
    default:
        return (FcDeclined);

    case FcDdpReadECS:
    case FcDdpWriteECS:
    case FcDdpStatus:
        dc->abyte = dc->dbyte = 0;
        dc->addr = 0;
        activeDevice->fcode = funcCode;
        return (FcAccepted);

    case FcDdpMasterClear:
        activeDevice->fcode = 0;
        dc->stat = StDdpAccept;
        return (FcProcessed);
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void ddpIo(void)
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
        }
        break;
        
    case FcDdpReadECS:
    case FcDdpWriteECS:
        if (dc->abyte < 2)
        {
            // We need to get the address from the PPU
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
                // we'll delay a bit before we set channel full
                dc->endaddrcycle = cycles;
                // we have a complete address; if this is read, 
                // read the first word.  We need to do that
                // unconditionally in case it's a flag
                // register operation, because then the PPU
                // is probably not going to read the data.
                dc->stat = StDdpAccept;
                if (cpuEcsAccess (dc->addr, &dc->curword, FALSE))
                {
                    if (DEBUGE)
                        printf ("ddp read abort addr %08o\n", dc->addr);
                    activeChannel->discAfterInput = TRUE;
                    dc->stat = StDdpAbort;
                }
                dc->dbyte == 0;
            }
            break;
        }
        if (activeDevice->fcode == FcDdpReadECS)
        {
            if (!activeChannel->full &&
                cycles - dc->endaddrcycle > 20)
            {
                if (dc->dbyte == -1)
                {
                    dc->stat = StDdpAccept;
                    if (cpuEcsAccess (dc->addr, &dc->curword, FALSE))
                    {
                        if (DEBUGE)
                            printf ("ddp read abort addr %08o\n", dc->addr);
                        activeChannel->discAfterInput = TRUE;
                        dc->stat = StDdpAbort;
                    }
                    dc->dbyte = 0;
                }
                activeChannel->data = (dc->curword >> 48) & Mask12;
                if (DEBUG)
                    printf ("ddp read addr %08o data %04o byte %d\n",
                            dc->addr, activeChannel->data, dc->dbyte);
                activeChannel->full = TRUE;
                dc->curword <<= 12;
                if (++dc->dbyte == 5)
                {
                    if (dc->addr & (DdpAddrReadOne | DdpAddrFlagReg))
                        activeChannel->discAfterInput = TRUE;
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
                if (cpuEcsAccess (dc->addr, &dc->curword, TRUE))
                {
                    if (DEBUGE)
                        printf ("ddp write abort addr %08o\n", dc->addr);
                    activeChannel->active = FALSE;
                    dc->stat = StDdpAbort;
                    return;
                }
                if (dc->addr & DdpAddrFlagReg)
                    activeChannel->active = FALSE;
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
static void ddpActivate(void)
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
static void ddpDisconnect(void)
{
}

/*---------------------------  End Of File  ------------------------------*/
