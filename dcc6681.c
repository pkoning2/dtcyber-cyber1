/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: dcc6681.c
**
**  Description:
**      Perform simulation of CDC 6681 data channel converter.
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
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#ifndef DEBUG
#define DEBUG 0
#endif

/*
**  Function codes.
*/
#define Fc6681Select            02000
#define Fc6681DeSelect          02100
#define Fc6681Connect1          01000
#define Fc6681Function          01100
#define Fc6681StatusReq         01200
#define Fc6681DevStatusReq      01300
#define Fc6681MasterClear       01700

#define Fc6681Function1         00000
#define Fc6681Connect4          04000
#define Fc6681Connect5          05000
#define Fc6681Connect6          06000
#define Fc6681Connect7          07000
#define Fc6681ConnectUnitMask   07000
#define Fc6681ConnectFuncMask   00777

#define Fc6681InputToEor        01400
#define Fc6681Input             01500
#define Fc6681Output            01600
#define Fc6681IoModeMask        07700
#define Fc6681IoIosMask         00070
#define Fc6681IoBcdMask         00001

/*
**      Status reply
*/
#define StFc6681Ready           00000
#define StFc6681Reject          00001
#define StFc6681IntReject       00003

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
typedef struct dccControl
    {
    bool        configured;
    bool        selected;
    PpWord      ios;
    PpWord      bcd;
    int 	    status;
    } DccControl;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus dcc6681Func(PpWord funcCode);
static void dcc6681Io(void);
static void dcc6681Load(DevSlot *, int, char *);
static void dcc6681Activate(void);
static void dcc6681Disconnect(void);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
Dev3kSlot *activeUnit;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static DccControl dccMap[MaxChannels];

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Attach 3000 equipment to 6681 data channel converter.
**
**  Parameters:     Name        Description.
**                  channelNo   channel number the device is attached to
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
Dev3kSlot * dcc6681Attach(u8 channelNo, u8 unitNo, u8 devType)
    {
    DevSlot *dp;
    Dev3kSlot *up;
    
    dp = channelAttach(channelNo, DtDdc6681);

    dp->activate = dcc6681Activate;
    dp->disconnect = dcc6681Disconnect;
    dp->func = dcc6681Func;
    dp->io = dcc6681Io;
    dp->load = dcc6681Load;
    dp->selectedUnit = -1;
    
    dccMap[channelNo].configured = TRUE;
    dccMap[channelNo].selected = TRUE;

    if (dp->context[unitNo] != NULL)
        {
        fprintf(stderr, "Unit %02o already allocated on channel %02o\n",
                unitNo, channelNo);
        exit(1);
        }
    /*
    ** Allocate a 3000 series unit block
    */
    up = (Dev3kSlot *) calloc(1, sizeof(Dev3kSlot));
    if (up == NULL)
        {
        fprintf(stderr, "Failed to allocate control block for Channel %02o unit %02o\n", channelNo, unitNo);
        exit(1);
        }
    dp->context[unitNo] = up;
    up->conv = dp;
    up->devType = devType;
    up->id = unitNo;
    
    /*
    **  Print a friendly message.
    */
    printf("Unit %02o attached to DCC6681 channel %o\n",
           unitNo, channelNo);

    /*
    ** Return the allocated 3000 series control block pointer
    */
    return(up);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 6681 data channel converter.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus dcc6681Func(PpWord funcCode)
    {
    DccControl *mp = dccMap + activeChannel->id;
    i8 u;
    
    if (!mp->configured)
        {
        return(FcDeclined);
        }

    // If not selected, we recognize only a select
    if (!mp->selected && funcCode != Fc6681Select)
        {
        return(FcDeclined);
        }
    
    switch (funcCode)
        {
    case Fc6681Select:
        mp->selected = TRUE;
        mp->status = StFc6681Ready;
        return(FcProcessed);
        
    case Fc6681DeSelect:
        mp->selected = FALSE;
        mp->status = StFc6681Ready;
        return(FcProcessed);
        
    case Fc6681Connect1:
    case Fc6681Function:
    case Fc6681StatusReq:
        activeDevice->fcode = funcCode;
        return(FcAccepted);

    case Fc6681MasterClear:
        mp->status = StFc6681Ready;
        for (u = 0; u < MaxUnits; u++)
            {
            activeUnit = (Dev3kSlot *) (activeDevice->context[u]);
            if (activeUnit != NULL)
                {
                activeUnit->intr = FALSE;
                (activeUnit->func) (funcCode);
                }
            }
        activeDevice->selectedUnit = -1;        // nothing connected
        return(FcProcessed);
        }

    switch (funcCode & Fc6681IoModeMask)
        {
    case Fc6681InputToEor:
    case Fc6681Input:
    case Fc6681Output:
    case Fc6681DevStatusReq:
        u = activeDevice->selectedUnit;
        if (u < 0)
            {
            mp->status = StFc6681IntReject;
            return(FcProcessed);
            }
        activeDevice->fcode = funcCode;
        mp->ios = funcCode & Fc6681IoIosMask;
        mp->bcd = funcCode & Fc6681IoBcdMask;
        funcCode &= Fc6681IoModeMask;
        activeUnit = (Dev3kSlot *) (activeDevice->context[u]);
        return((activeUnit->func) (funcCode));
        }

    switch (funcCode & Fc6681ConnectUnitMask)
        {
    case Fc6681Connect4:
    case Fc6681Connect5:
    case Fc6681Connect6:
    case Fc6681Connect7:
        u = (funcCode & Fc6681ConnectUnitMask) >> 9;
        activeUnit = (Dev3kSlot *) (activeDevice->context[u]);
        if (activeUnit == NULL)
            {
            mp->status = StFc6681IntReject;
            return(FcProcessed);
            }
        activeDevice->selectedUnit = u;
        if (DEBUG)
            {
            printf("Fc6681ConnectX %04o %o\n",funcCode, activeDevice->selectedUnit);
            }
        mp->status = StFc6681Ready;
        return(FcProcessed);

    case Fc6681Function1:
        u = activeDevice->selectedUnit;
        if (u < 0)
            {
            mp->status = StFc6681IntReject;
            return(FcProcessed);
            }
        activeUnit = (Dev3kSlot *) (activeDevice->context[u]);
        funcCode &= Fc6681ConnectFuncMask;
        (activeUnit->func) (funcCode);
        mp->status = StFc6681Ready;
        return(FcProcessed);

        }

    mp->status = StFc6681IntReject;
    return(FcProcessed);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 6681 data channel converter.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void dcc6681Io(void)
    {
    DccControl *mp = dccMap + activeChannel->id;
    u8 u;
    Dev3kSlot *up;
    PpWord stat;
    
    switch (activeDevice->fcode)
        {
    default:
        return;

    case Fc6681Select:
    case Fc6681DeSelect:
    case Fc6681MasterClear:
    case Fc6681Connect4:
    case Fc6681Connect5:
    case Fc6681Connect6:
    case Fc6681Connect7:
        printf("unexpected IO for function %04o\n", activeDevice->fcode); 
        break;

    case Fc6681Connect1:
        if (activeChannel->full)
            {
            u = (activeChannel->data & Fc6681ConnectUnitMask) >> 9;
            activeUnit = (Dev3kSlot *) (activeDevice->context[u]);
            if (activeUnit == NULL)
                {
                activeChannel->full = FALSE;
                mp->status = StFc6681IntReject;
                break;
                }
            activeDevice->selectedUnit = u;
            if (DEBUG)
                {
                printf("Fc6681Connect1 %04o\n", activeChannel->data);
                }
            activeChannel->full = FALSE;
            activeDevice->fcode = 0;
            mp->status = StFc6681Ready;
            }
        break;

    case Fc6681Function:
        activeUnit = (Dev3kSlot *) (activeDevice->context[activeDevice->selectedUnit]);
        if (activeChannel->full)
            {
            if (DEBUG)
                {
                printf("Fc6681Function %04o\n", activeChannel->data);
                }
            (activeUnit->func) (activeChannel->data);
            activeChannel->full = FALSE;
            activeDevice->fcode = 0;
            }
        break;

    case Fc6681InputToEor:
    case Fc6681Input:
    case Fc6681Output:
    case Fc6681DevStatusReq:
        activeUnit = (Dev3kSlot *) (activeDevice->context[activeDevice->selectedUnit]);
        (activeUnit->io) ();
        break;
        
    case Fc6681StatusReq:
        if (!activeChannel->full)
            {
            stat = mp->status;
            // Gather up interrupt flags from the attached units
            for (u = 0; u < MaxUnits; u++)
                {
                up = (Dev3kSlot *) (activeDevice->context[u]);
                if (up != NULL && up->intr)
                    {
                    stat |= 010 << u;
                    }
                }
            activeChannel->data = stat;
            activeChannel->full = TRUE;
            activeDevice->fcode = 0;
            }
        break;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform load/unload on 3000 series equipment.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dcc6681Load(DevSlot *dp, int unitNo, char *fn)
    {
    Dev3kSlot *up;
    
    if (unitNo < 0 || unitNo >= MaxUnits)
        {
        opSetMsg ("$INVALID UNIT NO");
        return;
        }
    up = (Dev3kSlot *)dp->context[unitNo];

    if (up == NULL)
        {
        opSetMsg ("$UNIT NOT ALLOCATED");
        return;
        }

    if (up->load == NULL)
        {
        opSetMsg ("$LOAD/UNLOAD NOT SUPPORTED ON UNIT");
        return;
        }
    (up->load)(up, fn);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle channel activation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dcc6681Activate(void)
    {
    i8 u;

    u = activeDevice->selectedUnit;
    if (u >= 0)
        {
        activeUnit = (Dev3kSlot *) (activeDevice->context[u]);
        (activeUnit->activate) ();
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle disconnecting of channel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dcc6681Disconnect(void)
    {
    i8 u;

    u = activeDevice->selectedUnit;
    if (u >= 0)
        {
        activeUnit = (Dev3kSlot *) (activeDevice->context[u]);
        (activeUnit->disconnect) ();
        }
    }

/*---------------------------  End Of File  ------------------------------*/
