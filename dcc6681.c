/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: dcc6681.c
**
**  Description:
**      Perform simulation of CDC 6681 or 6684 data channel converter.
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

/*
**  Function codes.
*/
#define Fc6681Select                02000
#define Fc6681DeSelect              02100
#define Fc6681ConnectMode2          01000
#define Fc6681FunctionMode2         01100
#define Fc6681DccStatusReq          01200
#define Fc6681DevStatusReq          01300
#define Fc6681MasterClear           01700
                                    
#define Fc6681FunctionMode1         00000
#define Fc6681Connect4Mode1         04000
#define Fc6681Connect5Mode1         05000
#define Fc6681Connect6Mode1         06000
#define Fc6681Connect7Mode1         07000
#define Fc6681ConnectEquipmentMask  07000
#define Fc6681ConnectUnitMask       00777
#define Fc6681ConnectFuncMask       00777
                                    
#define Fc6681InputToEor            01400
#define Fc6681Input                 01500
#define Fc6681Output                01600
#define Fc6681IoModeMask            07700
#define Fc6681IoIosMask             00070
#define Fc6681IoBcdMask             00001
                                    
/*                                  
**      Status reply                
*/                                  
#define StFc6681Ready               00000
#define StFc6681Reject              00001
#define StFc6681IntReject           00003

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
    DevSlot     *device3000[MaxEquipment];
    bool        interrupting[MaxEquipment];
    i8          connectedEquipment;
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
DevSlot *active3000Device;

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
**  Purpose:        Attach 3000 equipment to 6681 data channel converter.
**
**  Parameters:     Name        Description.
**                  channelNo   channel number the device is attached to
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
DevSlot *dcc6681Attach(u8 channelNo, u8 eqNo, u8 unitNo, u8 devType)
    {
    DevSlot *dp;
    DccControl *cp;
    DevSlot *device;
    
    dp = channelAttach(channelNo, 0, DtDcc6681);

    dp->activate = dcc6681Activate;
    dp->disconnect = dcc6681Disconnect;
    dp->func = dcc6681Func;
    dp->io = dcc6681Io;
    dp->load = dcc6681Load;
    
    /*
    **  Allocate converter context when first created.
    */
    if (dp->context[0] == NULL)
        {
        cp = (DccControl *)calloc(1, sizeof(DccControl));
        if (cp == NULL)
            {
            fprintf(stderr, "Failed to allocate dcc6681 context block\n");
            exit(1);
            }

        cp->selected = TRUE;
        cp->connectedEquipment = -1;
        dp->context[0] = (void *)cp;
        }
    else
        {
        cp = (DccControl *)dp->context[0];
        }

    /*
    **  Allocate 3000 series device control block.
    */
    if (cp->device3000[eqNo] == NULL)
        {
        device = calloc(1, sizeof(DevSlot));
        if (device == NULL)
            {
            fprintf(stderr, "Failed to allocate device control block for converter on channel %d\n", channelNo);
            exit(1);
            }

        cp->device3000[eqNo] = device;
        device->devType = devType;
        device->channel = channel + channelNo;
        device->eqNo = eqNo;
        }
    else
        {
        device = (DevSlot *)cp->device3000[eqNo];
        }

    /*
    **  Print a friendly message.
    */
    printf("Equipment %02o, Unit %02o attached to DCC6681 on channel %o\n", eqNo, unitNo, channelNo);

    /*
    ** Return the allocated 3000 series control block pointer
    */
    return(device);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Return device control block attached to a channel converter.
**
**  Parameters:     Name        Description.
**                  channelNo   channel number
**                  equipmentNo equipment number
**                  devType     device type
**
**  Returns:        Pointer to device slot.
**
**------------------------------------------------------------------------*/
DevSlot *dcc6681FindDevice(u8 channelNo, u8 equipmentNo, u8 devType)
    {
    DevSlot *dp;
    DccControl *cp;

    /*
    **  First find the channel converter.
    */
    dp = channelFindDevice(channelNo, DtDcc6681);
    if (dp == NULL)
        {
        return(NULL);
        }

    /*
    **  Locate channel converter context.
    */
    cp = (DccControl *)dp->context[0];
    if (cp == NULL)
        {
        return(NULL);
        }

    /*
    **  Lookup and verify equipment.
    */
    dp = cp->device3000[equipmentNo];
    if (dp == NULL || dp->devType != devType)
        {
        return(NULL);
        }

    /*
    **  Return series 3000 device control block.
    */
    return(dp);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Update interrupt status of current equipment.
**
**  Parameters:     Name        Description.
**                  dp          device pointer
**                  status      new interrupt status
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dcc6681InterruptDev(DevSlot *dp, bool status)
    {
    DccControl *mp = (DccControl *)dp->context[0];
    if (mp->connectedEquipment >= 0)
        {
        mp->interrupting[mp->connectedEquipment] = status;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Update interrupt status of current equipment.
**
**  Parameters:     Name        Description.
**                  status      new interrupt status
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dcc6681Interrupt(bool status)
    {
    dcc6681InterruptDev(activeDevice, status);
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
    DccControl *mp = (DccControl *)activeDevice->context[0];
    DevSlot *device;
    i8 u;
    i8 e;
    
    /*
    **  If not selected, we recognize only a select.
    */
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
        
    case Fc6681ConnectMode2:
    case Fc6681FunctionMode2:
    case Fc6681DccStatusReq:
        activeDevice->fcode = funcCode;
        return(FcAccepted);

    case Fc6681MasterClear:
        mp->status = StFc6681Ready;
        for (e = 0; e < MaxEquipment; e++)
            {
            mp->interrupting[e] = FALSE;
            active3000Device = mp->device3000[e];
            if (active3000Device != NULL)
                {
                active3000Device->selectedUnit = -1;
                (active3000Device->func)(funcCode);
                }
            }

        mp->connectedEquipment = -1;
        return(FcProcessed);
        }

    switch (funcCode & Fc6681IoModeMask)
        {
    case Fc6681DevStatusReq:
        funcCode &= Fc6681IoModeMask;
        e = mp->connectedEquipment;
        if (e < 0)
            {
            activeDevice->fcode = Fc6681DccStatusReq;
            mp->status = StFc6681IntReject;
            return(FcAccepted);
            }
        active3000Device = mp->device3000[e];
        activeDevice->fcode = funcCode;
        funcCode &= Fc6681IoModeMask;
        return((active3000Device->func)(funcCode));

    case Fc6681InputToEor:
    case Fc6681Input:
    case Fc6681Output:
        e = mp->connectedEquipment;
        if (e < 0)
            {
            mp->status = StFc6681IntReject;
            return(FcProcessed);
            }

        active3000Device = mp->device3000[e];
        activeDevice->fcode = funcCode;
        mp->ios = funcCode & Fc6681IoIosMask;
        mp->bcd = funcCode & Fc6681IoBcdMask;
        funcCode &= Fc6681IoModeMask;
        return((active3000Device->func)(funcCode));
        }

    switch (funcCode & Fc6681ConnectEquipmentMask)
        {
    case Fc6681Connect4Mode1:
    case Fc6681Connect5Mode1:
    case Fc6681Connect6Mode1:
    case Fc6681Connect7Mode1:
        e = (funcCode & Fc6681ConnectEquipmentMask) >> 9;
        u = funcCode & Fc6681ConnectUnitMask;
        device = mp->device3000[e];
        if (device == NULL || device->context[u] == NULL)
            {
            mp->connectedEquipment = -1;
            mp->status = StFc6681IntReject;
            return(FcProcessed);
            }

        mp->connectedEquipment = e;
        device->selectedUnit = u;
        mp->status = StFc6681Ready;
        return(FcProcessed);

    case Fc6681FunctionMode1:
        e = mp->connectedEquipment;
        if (e < 0)
            {
            mp->status = StFc6681IntReject;
            return(FcProcessed);
            }

        active3000Device = mp->device3000[e];
        funcCode &= Fc6681ConnectFuncMask;
        (active3000Device->func) (funcCode);
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
    DccControl *mp = (DccControl *)activeDevice->context[0];
    DevSlot *device;
    i8 u;
    i8 e;
    PpWord stat;
    
    switch (activeDevice->fcode)
        {
    default:
        return;

    case Fc6681Select:
    case Fc6681DeSelect:
    case Fc6681MasterClear:
    case Fc6681Connect4Mode1:
    case Fc6681Connect5Mode1:
    case Fc6681Connect6Mode1:
    case Fc6681Connect7Mode1:
        printf("unexpected IO for function %04o\n", activeDevice->fcode); 
        break;

    case Fc6681ConnectMode2:
        if (activeChannel->full)
            {
            activeChannel->full = FALSE;
            activeDevice->fcode = 0;
            e = (activeChannel->data & Fc6681ConnectEquipmentMask) >> 9;
            u = activeChannel->data & Fc6681ConnectUnitMask;
            device = mp->device3000[e];
            if (device == NULL || device->context[u] == NULL)
                {
                mp->connectedEquipment = -1;
                mp->status = StFc6681IntReject;
                break;
                }

            mp->connectedEquipment = e;
            device->selectedUnit = u;
            mp->status = StFc6681Ready;
            }
        break;

    case Fc6681FunctionMode2:
        if (activeChannel->full)
            {
            active3000Device = mp->device3000[mp->connectedEquipment];
            (active3000Device->func) (activeChannel->data);
            activeChannel->full = FALSE;
            activeDevice->fcode = 0;
            }
        break;

    case Fc6681InputToEor:
    case Fc6681Input:
    case Fc6681Output:
    case Fc6681DevStatusReq:
        active3000Device = mp->device3000[mp->connectedEquipment];
        (active3000Device->io)();
        break;
        
    case Fc6681DccStatusReq:
        if (!activeChannel->full)
            {
            stat = mp->status;
            
            /*
            **  Assemble interrupt status.
            */
            for (e = 0; e < MaxEquipment; e++)
                {
                if (   mp->device3000[e] != NULL
                    && mp->interrupting[e])
                    {
                    stat |= (010 << e);
                    }
                }

            /*
            **  Return status.
            */
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
static void dcc6681Load(DevSlot *dp, int eqNo, char *fn)
    {
    DccControl *mp = (DccControl *)dp->context[0];
    DevSlot *device;
    
    if (eqNo < 0 || eqNo >= MaxEquipment)
        {
        opSetMsg ("$INVALID EQUIPMENT NO");
        return;
        }
    device = mp->device3000[eqNo];

    if (device == NULL)
        {
        opSetMsg ("$EQ NOT ALLOCATED");
        return;
        }

    if (device->load == NULL)
        {
        opSetMsg ("$LOAD/UNLOAD NOT SUPPORTED ON EQUIPMENT");
        return;
        }
    active3000Device = device;
    (device->load)(device, eqNo, fn);
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
    DccControl *mp = (DccControl *)activeDevice->context[0];
    i8 e;

    e = mp->connectedEquipment;
    if (e < 0)
        {
        return;
        }

    active3000Device = mp->device3000[e];
    if (active3000Device == NULL)
        {
        return;
        }

    (active3000Device->activate)();
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
    DccControl *mp = (DccControl *)activeDevice->context[0];
    i8 e;

    e = mp->connectedEquipment;
    if (e < 0)
        {
        return;
        }

    active3000Device = mp->device3000[e];
    if (active3000Device == NULL)
        {
        return;
        }

    (active3000Device->disconnect)();
    }

/*---------------------------  End Of File  ------------------------------*/
