/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter and Gerard van der Grinten
**  (see license.txt)
**
**  Name: dd8xx.c
**
**  Description:
**      Perform simulation of CDC 844 and 885 disk drives.
**
**--------------------------------------------------------------------------
*/

#define DEBUG 0

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
**  CDC 844 and 885 disk drive function and status codes.
*/
#define Fc8xxConnect                00000
#define Fc8xxSeekFull               00001
#define Fc8xxSeekHalf               00002
#define Fc8xxIoLength               00003
#define Fc8xxRead                   00004
#define Fc8xxWrite                  00005
#define Fc8xxWriteVerify            00006
#define Fc8xxReadCheckword          00007
#define Fc8xxOpComplete             00010
#define Fc8xxDisableReserve         00011
#define Fc8xxGeneralStatus          00012
#define Fc8xxDetailedStatus         00013
#define Fc8xxContinue               00014
#define Fc8xxDropSeeks              00015
#define Fc8xxFormatPack             00016
#define Fc8xxOnSectorStatus         00017
#define Fc8xxDriveRelease           00020
#define Fc8xxReturnCylAddr          00021
#define Fc8xxSetClearFlaw           00022
#define Fc8xxDetailedStatus2        00023
#define Fc8xxGapRead                00024
#define Fc8xxGapWrite               00025
#define Fc8xxGapWriteVerify         00026
#define Fc8xxGapReadCheckword       00027
#define Fc8xxReadFactoryData        00030
#define Fc8xxReadUtilityMap         00031
#define Fc8xxReadFlawedSector       00034
#define Fc8xxWriteLastSector        00035
#define Fc8xxWriteFlawedSector      00037
#define Fc8xxManipulateProcessor    00062
#define Fc8xxDeadstart              00300
#define Fc8xxStartMemLoad           00414

/*
**  
**  Status Reply:
**
**  4000    Abnormal termination
**  2000    Dual access coupler reserved
**  1000    Nonrecoverable error
**  0400    Recovery in progress
**  0200    Checkword error
**  0100    Correctable address error
**  0040    Correctable data error
**  0020    DSU malfunction
**  0010    DSU reserved
**  0004    Miscellaneous error
**  0002    Busy
**  0001    Noncorrectable data error
*/
#define St844Abnormal           04000
#define St844NonRecoverable     01000
#define St844Busy               00002

/*
**  Physical dimensions of 844 disks.
**  322 12-bit bytes per sector (64 cm wds + 2 bytes).  1st
**      byte is unused. 2nd byte contains byte count of data.                                              
**   24 sectors/track                                     
**   19 tracks/cylinder                                    
**  411 cylinders/unit on 844-2  and 844-21                          
**  823 cylinders/unit on 844-41 and 844-44                         
*/
#define MaxCylinders844_2       410
#define MaxCylinders844_4       822
#define MaxTracks844            19
#define MaxSectors844           24
#define SectorSize              322
#define SectorBytes             512

/*
**  Physical dimensions of disk 885.
**  322 12-bit bytes per sector (64 cm wds + 2 bytes).  1st
**      byte is unused. 2nd byte contains byte count of data.                                              
**   32 sectors/track                                     
**   40 tracks/cylinder                                    
**  886 cylinders/unit on 885-2
** 1772 cylinders/unit on 885-4
*/
#define MaxCylinders885_2       841
#define MaxCylinders885_4       1682
#define MaxTracks885            40  /* spec says 20  1MV goes to 40 */
#define MaxSectors885           32

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
typedef struct diskSize
    {
    i32         maxCylinders;
    i32         maxTracks;
    i32         maxSectors;
    } DiskSize;

typedef struct diskParam
    {
    u8          *bp;
    i32         sector;
    i32         track;
    i32         cylinder;
    DiskSize    size;
    u16         sectorSize;
    u8          sec[SectorSize * 2];
    bool        oldFormat;
    i8          interlace;
    bool        seekNeeded;
    u8          lastFunc;
    bool        even;
    } DiskParam;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void dd8xxInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName, DiskSize *size);
static FcStatus dd8xxFunc(PpWord funcCode);
static void dd8xxIo(void);
static void dd8xxActivate(void);
static void dd8xxDisconnect(void);
static void dd8xxLoad(DevSlot *, int, char *);
static void dd8xxSeek(FILE *fcb, DiskParam *dp, u8 func);
static void dd8xxSeekNextSector(DiskParam *dp);
static void dd8xxDump(PpWord data);
static void dd8xxFlush(void);
static void dd8xxSetClearFlaw(DiskParam *dp, u32 flaw, PpWord flawState);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
FILE *devF;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static PpWord mySector[SectorSize];

static DiskSize sizeDd844_2 = {MaxCylinders844_2, MaxTracks844, MaxSectors844};
static DiskSize sizeDd844_4 = {MaxCylinders844_4, MaxTracks844, MaxSectors844};
static DiskSize sizeDd885_2 = {MaxCylinders885_2, MaxTracks885, MaxSectors885};
static DiskSize sizeDd885_4 = {MaxCylinders885_4, MaxTracks885, MaxSectors885};

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialise disk drive types (844-2, 844-21, 844-41 ,
**                  844-42, 885-2 and 885-4).
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
void dd844Init_2(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    dd8xxInit(eqNo, unitNo, channelNo, deviceName, &sizeDd844_2);
    }

void dd844Init_4(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    dd8xxInit(eqNo, unitNo, channelNo, deviceName, &sizeDd844_4);
    }

void dd885Init_2(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    dd8xxInit(eqNo, unitNo, channelNo, deviceName, &sizeDd885_2);
    }

void dd885Init_4(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    dd8xxInit(eqNo, unitNo, channelNo, deviceName, &sizeDd885_4);
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialise specified disk drive.
**
**  Parameters:     Name        Description.
**                  eqNo        equipment number
**                  unitNo      unit number
**                  channelNo   channel number the device is attached to
**                  deviceName  optional device file name
**                  size        pointer to disk size structure
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dd8xxInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName, DiskSize *size)
    {
    DevSlot *ds;
    FILE *fcb;
    char fname[80];
    DiskParam *dp;
    char *opt;

    (void)eqNo;

    /*
    **  Setup channel functions.
    */
    ds = channelAttach(channelNo, eqNo, DtDd8xx);
    activeDevice = ds;
    ds->activate = dd8xxActivate;
    ds->disconnect = dd8xxDisconnect;
    ds->func = dd8xxFunc;
    ds->io = dd8xxIo;
    ds->load = dd8xxLoad;

    /*
    **  Save disk parameters.
    */
    ds->selectedUnit = unitNo;
    dp = (DiskParam *)calloc(1, sizeof(DiskParam));
    if (dp == NULL)
        {
        fprintf(stderr, "Failed to allocate dd8xx context block\n");
        exit(1);
        }
    dp->seekNeeded = TRUE;
    dp->size = *size;

    ds->context[unitNo] = dp;

    /*
    **  Open or create disk image.
    */
    if (deviceName == NULL)
        {
        sprintf(fname, "DD844_C%02ou%1o", channelNo, unitNo);
        }
    else
        {
        opt = strchr (deviceName, ',');
        if (opt != NULL)
            {
            *opt++ = '\0';
            }
        strcpy(fname, deviceName);
        }

    fcb = fopen(fname, "r+b");
    if (fcb == NULL)
        {
        fcb = fopen(fname, "w+b");
        if (fcb == NULL)
            {
            fprintf(stderr, "Failed to open %s\n", fname);
            exit(1);
            }

        /*
        **  Initialise factory data
        */
        dp->cylinder = size->maxCylinders;
        dp->track = 0;
        dp->sector = 0;

        memset(mySector, 0, SectorSize * 2);

        /*
        **  Assemble serial number.
        */
        if (size->maxTracks == MaxTracks844)
            {
            mySector[0] = 8 << 8 | 4 << 4 | 4;
            }
        else
            {
            mySector[0] = 8 << 8 | 8 << 4 | 5;
            }

        mySector[1] = channelNo << 8 | eqNo << 4 | unitNo;
        mySector[2] = 4 << 8 | 8 << 4 | 0;
        mySector[3] = 1 << 8 | 1 << 4 | 6;

        dd8xxSeek(fcb, dp, Fc8xxWrite);
        fwrite(&mySector, 2, SectorSize, fcb);

        dp->sector += 1;
        dd8xxSeek(fcb, dp, Fc8xxWrite);
        memset(mySector, 0, SectorSize * 2);
        fwrite(&mySector, 2, SectorSize, fcb);

        /*
        **  Initialize utility map
        */
        dp->sector += 1;
        dd8xxSeek(fcb, dp, Fc8xxWrite);
        fwrite(&mySector, 2, SectorSize, fcb);
        }

    ds->fcb[unitNo] = fcb;

    /*
    **  Reset disk seek position.
    */
    dp->cylinder = 0;
    dp->track = 0;
    dp->sector = 0;
    dp->interlace = 1;
    dp->seekNeeded = TRUE;

    /*
    **  Parse option, if any.
    */
    if (opt != NULL)
        {
        if (strcmp (opt, "old") == 0)
            {
            dp->oldFormat = TRUE;
            }
        else if (strcmp (opt, "new") != 0 &&
                 strcmp (opt, "packed") != 0)
            {
            fprintf (stderr, "Unrecognized option name %s\n", opt);
            exit (1);
            }
        }
    dp->sectorSize = (dp->oldFormat) ? SectorSize * 2 : SectorBytes;

    /*
    **  Print a friendly message.
    */
    printf("DD844 with %d cylinders initialised on channel %o unit %o, %s sector format\n",
           dp->size.maxCylinders, channelNo, unitNo,
           (dp->oldFormat) ? "old (622 byte)" : "packed (512 byte)");
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 844 disk drive.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus dd8xxFunc(PpWord funcCode)
    {
    u8 unitNo;
    FILE *fcb;
    DiskParam *dp;
    PpWord dsSecSize;

    /*
    **  Deal with deadstart function.
    */
    if ((funcCode & 0700) == Fc8xxDeadstart )
        {
        activeDevice->selectedUnit = funcCode & 077;
        funcCode = Fc8xxDeadstart;
        }

    unitNo = activeDevice->selectedUnit;
    fcb = activeDevice->fcb[unitNo];
    dp = (DiskParam *)activeDevice->context[unitNo];

    /*
    **  Process function request.
    */
    switch (funcCode)
        {
    default:
        return(FcDeclined);

    case Fc8xxConnect:
        /*
        **  Expect drive number.
        */
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 1;     
        break;

    case Fc8xxSeekFull:
        /*
        **  Expect drive number, cylinder, track and sector.
        */
        dp->interlace = 1;
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 4;
        break;

    case Fc8xxSeekHalf:
        /*
        **  Expect drive number, cylinder, track and sector.
        */
        dp->interlace = 2;
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 4;
        break;

    case Fc8xxRead:
    case Fc8xxReadFlawedSector:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = SectorSize;
        dd8xxSeek(fcb, dp, Fc8xxRead);
        fread(dp->sec, 1, dp->sectorSize, fcb);
        dp->bp = dp->sec;
        dp->even = TRUE;
        break;

    case Fc8xxWrite:
    case Fc8xxWriteFlawedSector:
    case Fc8xxWriteLastSector:
    case Fc8xxWriteVerify:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = SectorSize;
        dd8xxSeek(fcb, dp, Fc8xxWrite);
        dp->bp = dp->sec;
        dp->even = TRUE;
        break;

    case Fc8xxReadCheckword:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 2;
        break;

    case Fc8xxOpComplete:
    case Fc8xxDropSeeks:
        break;

    case Fc8xxGeneralStatus:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 1;
        break;
 
    case Fc8xxDetailedStatus:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 12;
        break;
 
    case Fc8xxDetailedStatus2:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 20;
        break;
 
    case Fc8xxStartMemLoad:
        activeDevice->fcode = funcCode;
        break;

    case Fc8xxReadUtilityMap:
    case Fc8xxReadFactoryData:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = SectorSize;
        break;

    case Fc8xxDriveRelease:
        /*
        **  Silently ignore.
        */
        break;

    case Fc8xxDeadstart:
        activeDevice->fcode = funcCode;
        dp->cylinder = dp->size.maxCylinders;
        dp->track = 0;
        dp->sector = 3;
        dp->seekNeeded = TRUE;
        dd8xxSeek(fcb, dp, Fc8xxRead);
        fread(dp->sec, 1, dp->sectorSize, fcb);
        dp->bp = dp->sec;
        dp->even = TRUE;
        activeDevice->fcode = funcCode;
        /* the first word in the sector contains data length */
        if (dp->oldFormat)
            {
            dsSecSize = *(PpWord *) (dp->bp);
            }
        else
            {
            dsSecSize = (dp->sec[0] << 4) + (dp->sec[1] >> 4);
            }
        activeDevice->recordLength = dsSecSize;
        break;

    case Fc8xxSetClearFlaw:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 1;
        break;

    case Fc8xxFormatPack:
        activeDevice->fcode = funcCode;
        if (dp->size.maxTracks == MaxTracks844)
            {
            activeDevice->recordLength = 7;
            }
        else
            {
            activeDevice->recordLength = 18;
            }
        break;

    case Fc8xxManipulateProcessor:
        activeDevice->fcode = funcCode;
        break;

    case Fc8xxIoLength:
    case Fc8xxDisableReserve:
    case Fc8xxContinue:
    case Fc8xxOnSectorStatus:
    case Fc8xxReturnCylAddr:
    case Fc8xxGapRead:
    case Fc8xxGapWrite:
    case Fc8xxGapWriteVerify:
    case Fc8xxGapReadCheckword:
        logError(LogErrorLocation, "ch %o, function %o not implemented\n", activeChannel->id, funcCode);
        break;
        }

#if DEBUG
    fprintf(devF, "pp %o ch %o, dd8xx u%o - function %04o issued\n",
            activePpu->id, activeChannel->id, unitNo, funcCode);
#endif
    return(FcAccepted);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 844 disk drive.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dd8xxIo(void)
    {
    u8 unitNo;
    FILE *fcb;
    DiskParam *dp;
    u32 flaw;
    i32 pos;

    unitNo = activeDevice->selectedUnit;
    fcb = activeDevice->fcb[unitNo];
    dp = (DiskParam *)activeDevice->context[unitNo];

    switch (activeDevice->fcode)
        {
    default:
        logError(LogErrorLocation, "channel %02o - invalid function code: %4.4o\n", activeChannel->id, (u32)activeDevice->fcode);
        break;

    case Fc8xxConnect:
        if (activeChannel->full)
            {
            activeDevice->selectedUnit = activeChannel->data & 07;
            if (activeDevice->fcb[activeDevice->selectedUnit] == NULL)
                {
                logError(LogErrorLocation, "channel %02o - invalid select: %4.4o", activeChannel->id, (u32)activeDevice->fcode);
                }

            activeChannel->full = FALSE;
            }
        break;

    case Fc8xxSeekFull:
    case Fc8xxSeekHalf:
        if (activeChannel->full)
            {
            switch (activeDevice->recordLength--)
                {
            case 4:
                activeDevice->selectedUnit = activeChannel->data & 07;
                unitNo = activeDevice->selectedUnit;
                if (activeDevice->fcb[activeDevice->selectedUnit] == NULL)
                    {
                    logError(LogErrorLocation, "channel %02o - invalid select: %4.4o", activeChannel->id, (u32)activeDevice->fcode);
                    }

                fcb = activeDevice->fcb[unitNo];
                dp = activeDevice->context[unitNo];
                break;

            case 3:
                if (dp->cylinder != activeChannel->data)
                    {
                    dp->cylinder = activeChannel->data;
                    dp->seekNeeded = TRUE;
                    }
                break;

            case 2:
                if (dp->track != activeChannel->data)
                    {
                    dp->track = activeChannel->data;
                    dp->seekNeeded = TRUE;
                    }
                break;

            case 1:
                if (dp->sector != activeChannel->data)
                    {
                    dp->sector = activeChannel->data;
                    dp->seekNeeded = TRUE;
                    }
                dd8xxSeek(fcb, dp, Fc8xxRead);
                break;

            default:
                activeDevice->recordLength = 0;
                break;
                }

            activeChannel->full = FALSE;
            }
        break;

    case Fc8xxDeadstart:
    case Fc8xxRead:
    case Fc8xxReadFlawedSector:
        if (!activeChannel->full)
            {
            if (dp->oldFormat)
                {
                activeChannel->data = *(PpWord *) (dp->bp) & Mask12;
                dp->bp += sizeof (PpWord);
                }
            else
                {
                if (dp->even)
                    {
                    activeChannel->data = (*(dp->bp) << 4) + (*(dp->bp + 1) >> 4);
                    dp->bp++;
                    }
                else
                    {
                    activeChannel->data = (*(dp->bp) << 8) + *(dp->bp + 1);
                    dp->bp += 2;
                    }
                dp->even = !dp->even;
                }
            activeChannel->full = TRUE;

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                dd8xxSeekNextSector(dp);
                }
            }
        break;

    case Fc8xxWrite:
    case Fc8xxWriteFlawedSector:
    case Fc8xxWriteLastSector:
    case Fc8xxWriteVerify:
        if (activeChannel->full)
            {
            if (dp->oldFormat)
                {
                *(PpWord *) (dp->bp) = activeChannel->data & Mask12;
                dp->bp += sizeof (PpWord);
                }
            else
                {
                if (dp->even)
                    {
                    *dp->bp++ = activeChannel->data >> 4;
                    *dp->bp = activeChannel->data << 4;
                    }
                else
                    {
                    *dp->bp++ |= activeChannel->data >> 8;
                    *dp->bp++ = activeChannel->data;
                    }
                dp->even = !dp->even;
                }
            activeChannel->full = FALSE;

            if (--activeDevice->recordLength == 0)
                {
                fwrite(dp->sec, 1, dp->sectorSize, fcb);
                dd8xxSeekNextSector(dp);
                }
            }
        break;

    case Fc8xxGeneralStatus:
        if (!activeChannel->full)
            {
//            activeChannel->data = activeDevice->status;   // <<<<<< needs more thought >>>>>>>>
            activeChannel->data = 0;
            activeChannel->full = TRUE;

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;

    case Fc8xxReadCheckword:
    case Fc8xxDetailedStatus:
    case Fc8xxDetailedStatus2:
        if (!activeChannel->full)
            {
            activeChannel->data = 0;
            activeChannel->full = TRUE;

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;

    case Fc8xxReadFactoryData:
    case Fc8xxReadUtilityMap:
        if (!activeChannel->full)
            {
            fread(&activeChannel->data, 2, 1, fcb);
            activeChannel->full = TRUE;

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;

    case Fc8xxSetClearFlaw:
        if (activeChannel->full)
            {
            flaw = (dp->cylinder << 12) | (dp->track <<6) | dp->sector;
            dd8xxSetClearFlaw(dp, flaw, activeChannel->data);
            activeChannel->full = FALSE;
            }
        break;

    case Fc8xxStartMemLoad:
    case Fc8xxFormatPack:
    case Fc8xxManipulateProcessor:
    case Fc8xxIoLength:
    case Fc8xxOpComplete:
    case Fc8xxDisableReserve:
    case Fc8xxContinue:
    case Fc8xxDropSeeks:
    case Fc8xxOnSectorStatus:
    case Fc8xxDriveRelease:
    case Fc8xxReturnCylAddr:
    case Fc8xxGapRead:
    case Fc8xxGapWrite:
    case Fc8xxGapWriteVerify:
    case Fc8xxGapReadCheckword:
        activeChannel->full = FALSE;
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
static void dd8xxActivate(void)
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
static void dd8xxDisconnect(void)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        Seek to the currently set cyl/track/sector
**
**  Parameters:     Name        Description.
**                  fcb         pointer to FILE struct.
**                  dp          pointer to DiskParam struct.
**                  func        function code (read or write).
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
static void dd8xxSeek(FILE *fcb, DiskParam *dp, u8 func)
    {
    i32 result;

#if DEBUG
    fprintf(devF, "pp %o ch %o - seek c:%d t:%d S:%d\n",
            activePpu->id, activeChannel->id,
            dp->cylinder,
            dp->track,
            dp->sector);
#endif
    activeDevice->status = 0;

    if (dp->cylinder > dp->size.maxCylinders)
        {
        logError(LogErrorLocation, "ch %o, cylinder %o invalid\n", activeChannel->id, dp->cylinder);
//        activeDevice->status = St844NonRecoverable;
        activeDevice->status = St844NonRecoverable | St844Abnormal;
        return;
        }

    if (dp->track >= dp->size.maxTracks)
        {
        logError(LogErrorLocation, "ch %o, track %o invalid\n", activeChannel->id, dp->track);
//        activeDevice->status = St844NonRecoverable;
        activeDevice->status = St844NonRecoverable | St844Abnormal;
        return;
        }

    if (dp->sector >= dp->size.maxSectors)
        {
        logError(LogErrorLocation, "ch %o, sector %o invalid\n", activeChannel->id, dp->sector);
//        activeDevice->status = St844NonRecoverable;
        activeDevice->status = St844NonRecoverable | St844Abnormal;
        return;
        }
    
    if (dp->seekNeeded || dp->lastFunc != func)
        {
        result  = dp->cylinder * dp->size.maxTracks * dp->size.maxSectors;
        result += dp->track * dp->size.maxSectors;
        result += dp->sector;
        result *= dp->sectorSize;
        fseek(fcb, result, SEEK_SET);
        dp->seekNeeded = FALSE;
        dp->lastFunc = func;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Position to next sector taking into account interlace.
**
**  Parameters:     Name        Description.
**                  dp          Disk parameters (context).
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
static void dd8xxSeekNextSector(DiskParam *dp)
    {
    dp->sector += dp->interlace;

    if (dp->interlace == 1)
        {
        if (dp->sector == dp->size.maxSectors)
            {
            dp->sector = 0;
            dp->track += 1;
            }
        }
    else
        {
        dp->seekNeeded = TRUE;
        if (dp->sector == dp->size.maxSectors)
            {
            dp->sector = 0;
            dp->track += 1;
            if (dp->track == dp->size.maxTracks)
                {
                dp->track = 0;
                dp->sector = 1;
                }
            }
        else if (dp->sector == dp->size.maxSectors + 1)
            {
            dp->sector = 1;
            dp->track += 1;
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform load/unload on disk unit.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dd8xxLoad(DevSlot *dp, int unitNo, char *fn)
    {
    FILE *fcb;

    if (fn != NULL)
        {
        opSetMsg ("$LOAD NOT SUPPORTED ON DD8xx");
        return;
        }
    
    if (unitNo < 0 || unitNo >= MaxUnits)
        {
        opSetMsg ("$INVALID UNIT NO");
        return;
        }
    fcb = dp->fcb[unitNo];

    if (fcb == NULL)
        {
        opSetMsg ("$UNIT NOT ALLOCATED");
        return;
        }

    fflush (fcb);
    opSetMsg ("BUFFERS FLUSHED");
    }

/*--------------------------------------------------------------------------
**  Purpose:        Manipulate sector flaw map.
**
**  Parameters:     Name        Description.
**                  dp          Disk parameters (context).
**                  flaw
**                  flawState
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dd8xxSetClearFlaw(DiskParam *dp, u32 flaw, PpWord flawState)
    {
    u8 unitNo;
    FILE *fcb;
    int index;
    PpWord word0;
    PpWord word1;

    unitNo = activeDevice->selectedUnit;
    fcb = activeDevice->fcb[unitNo];

    index = 23 - (flawState & 01);
    word0 = (PpWord)(flaw >> 12) | (1 << 11) - (flawState & 01);
    word1 = (PpWord)flaw & 07777;

    dp->cylinder = dp->size.maxCylinders;
    dp->track = 0;
    dp->sector = 0;
    dp->seekNeeded = TRUE;
    dd8xxSeek(fcb, dp, Fc8xxWrite);
    fread(&mySector, 2, SectorSize, fcb);

    if (flawState & 02)
        {  /* set part */
        index = 4;
        while (mySector[index] != 0 ) 
            {
            index += 2;
            if (index > (SectorSize - 2))
                break;
            }
        if (index < SectorSize)
            {
#if DEBUG
            fprintf(devF,"DD844 set flaw , sector = %08o %04o %04o - %04o at %d\n",
                flaw, word0, word1, flawState, index);
#endif
            mySector[index] = word0;
            index += 1;
            mySector[index] = word1;
            }
        }
    else
        {
        index = 4;
        while (mySector[index] != 0 ) 
            {
            if (mySector[index] == word0 &&
                mySector[index + 1] == word1)
               break; 
            index += 2;
            if (index > (SectorSize - 2))
                break;
            }
        if (index < SectorSize)
            {
#if DEBUG
            fprintf(devF,"DD844 clear flaw , sector = %08o %04o %04o - %04o at %d\n",
                flaw, word0, word1, flawState, index);
#endif
            mySector[index] = 0;
            index += 1;
            mySector[index] = 0;
            }
        }
    dd8xxSeek(fcb, dp, Fc8xxWrite);
    fwrite(&mySector, 2, SectorSize, fcb);
    }

/*---------------------------  End Of File  ------------------------------*/
