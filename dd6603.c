/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: disk.c
**
**  Description:
**      Perform simulation of CDC 6603 disk drives.
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

/*
**  CDC 6603 disk drive function and status codes.
**  
**  10xx    Read sector xx (00-77)
**  11xx    Read sector xx (100-177)
**  12xx    Write sector xx (00-77)
**  13xx    Write sector xx (100-177)
**  14xx    Select tracks xx (00-77)
**  15xx    Select tracks xx (100-177)
**  16xy    Select head group y (x is read sampling time - ignore)
**  1700    Status request
*/
#define Fc6603CodeMask          07600
#define Fc6603SectMask          0177
#define Fc6603TrackMask         0177
#define Fc6603HeadMask          07

#define Fc6603ReadSector        01000
#define Fc6603WriteSector       01200
#define Fc6603SelectTrack       01400
#define Fc6603SelectHead        01600
#define Fc6603StatusReq         01700

/*
**  
**  Status Reply:
**
**  0xysSS
**  x=0     Ready
**  x=1     Not ready
**  y=0     No parity error
**  Y=1     Parity error
**  sSS     Sector number (bits 6-0)
*/
#define St6603StatusMask        07000
#define St6603StatusValue       00000
#define St6603SectMask          0177
#define St6603ParityErrorMask   0200
#define St6603ReadyMask         0400

/*
**  Physical dimensions of disk.
*/
#define MaxTracks               0200
#define MaxHeads                8
#define MaxOuterSectors         128
#define MaxInnerSectors         100
#define SectorSize              (322 + 16)


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
typedef struct diskParam
    {
    u32         sector;
    u32         track;
    u32         head;
    } DiskParam;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus dd6603Func(PpWord funcCode);
static void dd6603Io(void);
static void dd6603Activate(void);
static void dd6603Disconnect(void);
static u32 dd6603Seek(u32 track, u32 head, u32 sector);

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
static u8 logColumn;
static PpWord dmp[8];

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialise 6603 disk drive.
**
**  Parameters:     Name        Description.
**                  eqNo        equipment number
**                  unitNo      unit number
**                  channelNo   channel number the device is attached to
**                  deviceName  optional device file name
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dd6603Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    DevSlot *dp;
    FILE *fcb;
    char fname[80];

    (void)eqNo;
    (void)unitNo;
    (void)deviceName;

    dp = channelAttach(channelNo, DtDd6603);

    dp->activate = dd6603Activate;
    dp->disconnect = dd6603Disconnect;
    dp->func = dd6603Func;
    dp->io = dd6603Io;
    dp->selectedUnit = unitNo;

    dp->context[unitNo] = calloc(1, sizeof(DiskParam));
    if (dp->context[unitNo] == NULL)
        {
        fprintf(stderr, "Failed to allocate dd6603 context block\n");
        exit(1);
        }

    sprintf(fname, "DD6603_C%02oU%1o", channelNo,unitNo);
    fcb = fopen(fname, "r+b");
    if (fcb == NULL)
        {
        fcb = fopen(fname, "w+b");
        if (fcb == NULL)
            {
            fprintf(stderr, "Failed to open %s\n", fname);
            exit(1);
            }
        }

    dp->fcb[unitNo] = fcb;

    sprintf(fname, "DD6603_C%02oU%1o.log", channelNo, unitNo);
    fcb = fopen(fname, "wt");
    if (fcb == NULL)
        {
        fprintf(stderr, "Failed to open %s\n", fname);
        exit(1);
        }

    dp->log[unitNo] = fcb;

    /*
    **  Print a friendly message.
    */
    printf("DD6603 initialised on channel %o unit %o \n", channelNo, unitNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 6603 disk drive.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus dd6603Func(PpWord funcCode)
    {
    FILE *fcb = activeDevice->fcb[activeDevice->selectedUnit];
    FILE *log = activeDevice->log[activeDevice->selectedUnit];
    DiskParam *dp = (DiskParam *)activeDevice->context[activeDevice->selectedUnit];

    switch (funcCode & Fc6603CodeMask)
        {
    default:
        return(FcDeclined);

    case Fc6603ReadSector:
        activeDevice->fcode = funcCode;
        dp->sector = funcCode & Fc6603SectMask;
        fseek(fcb, dd6603Seek(dp->track, dp->head, dp->sector), SEEK_SET);
        fprintf(log, "\nFC: %04o - read  h: %o t: %03o s %03o\n", funcCode, dp->head, dp->track, dp->sector);
        logColumn = 0;
        break;

    case Fc6603WriteSector:
        activeDevice->fcode = funcCode;
        dp->sector = funcCode & Fc6603SectMask;
        fseek(fcb, dd6603Seek(dp->track, dp->head, dp->sector), SEEK_SET);
        fprintf(log, "\nFC: %04o - write h: %o t: %03o s %03o\n", funcCode, dp->head, dp->track, dp->sector);
        logColumn = 0;
        break;

    case Fc6603SelectTrack:
        dp->track = funcCode & Fc6603TrackMask;
        fprintf(log, "\nFC: %04o - select track %03o\n", funcCode, dp->track);
        break;

    case Fc6603SelectHead:
        if (funcCode == Fc6603StatusReq)
            {
            activeDevice->fcode = funcCode;
            activeChannel->status = (u16)dp->sector;
            fprintf(log, "\nFC: %04o - request status\n", funcCode);

            /*
            **  Simulate the moving disk - seems strange but is required.
            */
            dp->sector = (dp->sector + 1) & 0177;
            }
        else
            {
            /*
            **  Select head.
            */
            dp->head = funcCode & Fc6603HeadMask;
            fprintf(log, "\nFC: %04o - select head %o\n", funcCode, dp->head);
            }
        break;
        }

    return(FcAccepted);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 6603 disk drive.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dd6603Io(void)
    {
    FILE *fcb = activeDevice->fcb[activeDevice->selectedUnit];

    switch (activeDevice->fcode & Fc6603CodeMask)
        {
    default:
        ppAbort((stderr, "channel %02o - invalid function code: %4.4o", activeChannel->id, (u32)activeDevice->fcode));
        break;

    case Fc6603ReadSector:
        if (!activeChannel->full)
            {
            fread(&activeChannel->data, 2, 1, fcb);
            activeChannel->full = TRUE;
            }
        break;

    case Fc6603WriteSector:
        if (activeChannel->full)
            {
            fwrite(&activeChannel->data, 2, 1, fcb);
            activeChannel->full = FALSE;
            }
        break;

    case Fc6603SelectTrack:
        break;

    case Fc6603SelectHead:
        if (activeDevice->fcode == Fc6603StatusReq)
            {
            activeChannel->data = activeChannel->status;
            activeChannel->full = TRUE;
            activeChannel->status = 0;
            activeDevice->fcode = 0;
            }
        break;
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
static void dd6603Activate(void)
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
static void dd6603Disconnect(void)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        Work out seek offset.
**
**  Parameters:     Name        Description.
**                  track       Track number.
**                  head        Head group.
**                  sector      Sector number.
**
**  Returns:        Byte offset (not word!)
**
**------------------------------------------------------------------------*/
static u32 dd6603Seek(u32 track, u32 head, u32 sector)
    {
    u32 result;
    u32 sectorsPerTrack = MaxOuterSectors;

    if (track >= MaxTracks)
        {
        ppAbort((stderr, "ch %o, track %o invalid", activeChannel->id, track));
        }

    if (head >= MaxHeads)
        {
        ppAbort((stderr, "ch %o, head %o invalid", activeChannel->id, head));
        }

    if (sector >= sectorsPerTrack)
        {
        ppAbort((stderr, "ch %o, sector %o invalid", activeChannel->id, sector));
        }

    result  = track * MaxHeads * sectorsPerTrack;
    result += head * sectorsPerTrack;
    result += sector;
    result *= SectorSize * 2;

    return(result);
    }

/*---------------------------  End Of File  ------------------------------*/
