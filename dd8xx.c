/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2007, Tom Hunter, Gerard van der Grinten,
**  and Paul Koning.
**  (see license.txt)
**
**  Name: dd8xx.c
**
**  Description:
**      Perform emulation of CDC 844 and 885 disk drives.
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
#include <time.h>
//#include <unistd.h>
#include <errno.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/

#define MAXDDUNITS 64

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
#define Fc8xxWriteVerifyLastSector  00036
#define Fc8xxWriteFlawedSector      00037
#define Fc8xxManipulateProcessor    00062
#define Fc8xxDeadstart              00300
#define Fc8xxStartMemLoad           00414

/*
**  
**  General status bits.
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
#define St8xxAbnormal           04000
#define St8xxOppositeReserved   02000
#define St8xxNonRecoverable     01000
#define St8xxRecovering         00400
#define St8xxCheckwordError     00200
#define St8xxCorrectableAddress 00100
#define St8xxCorrectableData    00040
#define St8xxDSUmalfunction     00020
#define St8xxDSUreserved        00010
#define St8xxMiscError          00004
#define St8xxBusy               00002
#define St8xxDataError          00001

/*
**  Detailed status.
*/

/*
**  Physical dimensions of 844 disks.
**  322 12-bit bytes per sector (64 cm wds + 2 bytes).  1st
**      byte is unused. 2nd byte contains byte count of data.                                              
**   24 sectors/track                                     
**   19 tracks/cylinder                                    
**  411 cylinders/unit on 844-2  and 844-21                          
**  823 cylinders/unit on 844-41 and 844-44                         
*/
#define MaxCylinders844_2       411
#define MaxCylinders844_4       823
#define MaxTracks844            19
#define MaxSectors844           24
#define SectorSize              322

/*
**  Address of 844 deadstart sector.
*/
#define DsCylinder844_2         410
#define DsCylinder844_4         822
#define DsTrack844              0
#define DsSector844             3

/*
**  Physical dimensions of 885 disk.
**  322 12-bit bytes per sector (64 cm wds + 2 bytes).  1st
**      byte is unused. 2nd byte contains byte count of data.                                              
**   32 sectors/track                                     
**   40 tracks/cylinder                                    
**  843 cylinders/unit on 885-11 and 885-12
*/
#define MaxCylinders885_1       843
#define MaxTracks885            40
#define MaxSectors885           32

/*
**  Address of 885 deadstart sector.
*/
#define DsCylinder885           841
#define DsTrack885              1
#define DsSector885             30

/*
**  Disk drive types.
*/
#define DiskType844             1
#define DiskType885             2

/*
**  Disk container types.
*/
#define CtUndefined             0
#define CtClassic               1
#define CtPacked                2

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
    PpWord      (*read)(struct diskParam *);
    void        (*write)(struct diskParam *, PpWord);
    DiskIO      ioDesc;
    i32         sector;
    i32         track;
    i32         cylinder;
    i32         sectorSize;
    DiskSize    size;
    PpWord      *bufPtr;
    void        *statusBuf;
    u16         detailedStatus[20];
    u8          diskNo;
    u8          unitNo;
    u8          diskType;
    i8          interlace;
    bool        readonly;
    PpWord      buffer[SectorSize];
    } DiskParam;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void dd8xxInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName, DiskSize *size, u8 diskType);
static FcStatus dd8xxFunc(PpWord funcCode);
static void dd8xxIo(void);
static void dd8xxActivate(void);
static void dd8xxDisconnect(void);
static void dd8xxSeek(DiskParam *dp);
static void dd8xxSeekNextSector(DiskParam *dp, bool gap);
static void dd8xxDump(PpWord data);
static void dd8xxFlush(void);
static PpWord dd8xxReadClassic(DiskParam *dp);
static PpWord dd8xxReadPacked(DiskParam *dp);
static void dd8xxWriteClassic(DiskParam *dp, PpWord data);
static void dd8xxWritePacked(DiskParam *dp, PpWord data);
static void dd8xxSectorWrite(DiskParam *dp, PpWord *sector);
static void dd844SetClearFlaw(DiskParam *dp, PpWord flawState);
static char *dd8xxFunc2String(PpWord funcCode);
static void dd8xxLoad(DevSlot *dp, int unitNo, char *fn);

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
static int diskCount = 0;
static PpWord mySector[SectorSize];

static DiskSize sizeDd844_2 = {MaxCylinders844_2, MaxTracks844, MaxSectors844};
static DiskSize sizeDd844_4 = {MaxCylinders844_4, MaxTracks844, MaxSectors844};
static DiskSize sizeDd885_1 = {MaxCylinders885_1, MaxTracks885, MaxSectors885};

#if DEBUG
static FILE *dd8xxLog = NULL;
static int waitcount = 0;
#endif

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
    dd8xxInit(eqNo, unitNo, channelNo, deviceName, &sizeDd844_2, DiskType844);
    }

void dd844Init_4(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    dd8xxInit(eqNo, unitNo, channelNo, deviceName, &sizeDd844_4, DiskType844);
    }

void dd885Init_1(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    dd8xxInit(eqNo, unitNo, channelNo, deviceName, &sizeDd885_1, DiskType885);
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
static void dd8xxInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName, DiskSize *size, u8 diskType)
    {
    DevSlot *ds;
    DiskParam *dp;
    DiskParam **dpvec;
    time_t mTime;
    struct tm *lTime;
    u8 yy, mm, dd;
    u8 containerType = CtPacked;
    char *opt = NULL;

    (void)eqNo;

#if DEBUG
    if (dd8xxLog == NULL)
        {
        dd8xxLog = fopen("dd8xxlog.txt", "wt");
        }
#endif

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

    dp->size = *size;
    dp->diskNo = diskCount++;
    dp->diskType = diskType;
    dp->unitNo = unitNo;
    dp->readonly = FALSE;
    
    /*
    **  Allocate the operator status buffer
    */
    dp->statusBuf = opInitStatus ((diskType == DiskType885) ? "DD885" : "DD844",
                                  channelNo, unitNo);

    /*
    **  Determine if any options have been specified.
    */
    if (deviceName != NULL)
        {
        opt = strchr (deviceName, ',');
        }

    if (opt != NULL)
        {
        /*
        **  Process options.
        */
        *opt++ = '\0';

        if (   strcmp (opt, "old")     == 0
            || strcmp (opt, "classic") == 0)
            {
            containerType = CtClassic;
            }
        else if (   strcmp (opt, "new")    == 0
                 || strcmp (opt, "packed") == 0)
            {
            containerType = CtPacked;
            }
        else
            {
            fprintf (stderr, "Unrecognized option name %s\n", opt);
            exit (1);
            }
        }
    else
        {
        /*
        **  No options specified - use default values.
        */
        switch (diskType)
            {
        case DiskType885:
            containerType = CtPacked;
            break;

        case DiskType844:
            containerType = CtClassic;
            break;
            }
        }
    opSetStatus (dp->statusBuf, deviceName);

    /*
    **  Setup environment for disk container type.
    */
    switch (containerType)
        {
    case CtClassic:
        dp->read = dd8xxReadClassic;
        dp->write = dd8xxWriteClassic;
        dp->sectorSize = SectorSize * 2;
        break;

    case CtPacked:
        dp->read = dd8xxReadPacked;
        dp->write = dd8xxWritePacked;
        dp->sectorSize = 512;
        break;
        }

    /*
    **  Initialize detailed status.
    */
    switch (diskType)
        {
    case DiskType885:
        dp->detailedStatus[ 0] =     0;             // strobe offset & address error status
        dp->detailedStatus[ 1] =  0340;             // checkword error status & sector count
        dp->detailedStatus[ 2] =     0;             // command code & error bits
        dp->detailedStatus[ 3] = 07440 + unitNo;    // dsu number
        dp->detailedStatus[ 4] =     0;             // address 1 of failing sector
        dp->detailedStatus[ 5] =     0;             // address 2 of failing sector
        dp->detailedStatus[ 6] =   010;             // non recoverable error status
        dp->detailedStatus[ 7] =   037;             // 11 bit correction factor
        dp->detailedStatus[ 8] = 01640;             // DSU status
        dp->detailedStatus[ 9] = 07201;             // DSU fault status
        dp->detailedStatus[10] =     0;             // DSU interlock status
        dp->detailedStatus[11] =     0;             // bit address of correctable read error
        dp->detailedStatus[12] = 02000;             // PP address of correctable read error
        dp->detailedStatus[13] =     0;             // first word of correction vector
        dp->detailedStatus[14] =     0;             // second word of correction vector
        dp->detailedStatus[15] =     0;             // DSC operating status word
        dp->detailedStatus[16] =     0;             // coupler buffer status
        dp->detailedStatus[17] =  0400;             // access A is connected & last command
        dp->detailedStatus[18] =     0;             // last command 2 and 3
        dp->detailedStatus[19] =     0;             // last command 4
        break;

    case DiskType844:
        dp->detailedStatus[ 0] =     0;             // strobe offset & address error status
        dp->detailedStatus[ 1] =     0;             // checkword error status & sector count
        dp->detailedStatus[ 2] =     0;             // command code & error bits
        dp->detailedStatus[ 3] = 04440 + unitNo;    // dsu number
        dp->detailedStatus[ 4] =     0;             // address 1 of failing sector
        dp->detailedStatus[ 5] =     0;             // address 2 of failing sector
        dp->detailedStatus[ 6] =   010;             // non recoverable error status
        dp->detailedStatus[ 7] =     0;             // 11 bit correction factor
        if (size == &sizeDd844_4)
            {
            dp->detailedStatus[ 8] = 00740;         // DSU status, double density
            }
        else
            {
            dp->detailedStatus[ 8] = 00700;         // DSU status, single density
            }
        dp->detailedStatus[ 9] = 04001;             // DSU fault status
        dp->detailedStatus[10] = 07520;             // DSU interlock status
        dp->detailedStatus[11] =     0;             // bit address of correctable read error
        dp->detailedStatus[12] =     0;             // PP address of correctable read error
        dp->detailedStatus[13] =     0;             // first word of correction vector
        dp->detailedStatus[14] =     0;             // second word of correction vector
        dp->detailedStatus[15] = 00020;             // DSC operating status word
        dp->detailedStatus[16] =     0;             // coupler buffer status
        dp->detailedStatus[17] =  0400;             // access A is connected & last command
        dp->detailedStatus[18] =     0;             // last command 2 and 3
        dp->detailedStatus[19] =     0;             // last command 4
        break;
        }

    /*
    **  Link device parameters.
    */
    dpvec = (DiskParam **) (ds->context[0]);
    if (dpvec == NULL)
        {
        dpvec = (DiskParam **) malloc (sizeof (DiskParam *) * MAXDDUNITS);
        if (dpvec == NULL)
            {
            fprintf (stderr, "Failed to allocate DiskParam vector");
            exit (1);
            }
        ds->context[0] = dpvec;
        }
    dpvec[unitNo] = dp;

    /*
    **  Open disk image, if a name is given.  Otherwise leave unit unloaded.
    */
    if (deviceName != NULL)
        {
        /*
        **  Try to open existing disk image.
        */
        if (!ddOpen (&dp->ioDesc, deviceName, TRUE))
            {
            /*
            **  Disk does not yet exist - manufacture one.
            */
            if (!ddCreate (&dp->ioDesc, deviceName))
                {
                fprintf(stderr, "Failed to open %s\n", deviceName);
                exit(1);
                }

            if (!ddLock (&dp->ioDesc))
                {
                fprintf (stderr, "Failed to lock %s\n", deviceName);
                exit (1);
                }

            /*
            **  Write last disk sector to reserve the space.
            */
            memset(mySector, 0, SectorSize * 2);
            dp->cylinder = size->maxCylinders - 1;
            dp->track = size->maxTracks - 1;
            dp->sector = size->maxSectors - 1;
            dd8xxSeek(dp);
            dd8xxSectorWrite(dp, mySector);

            /*
            **  Position to cylinder with the disk's factory and utility
            **  data areas.
            */
            switch (diskType)
                {
            case DiskType885:
                dp->cylinder = size->maxCylinders - 2;
                break;

            case DiskType844:
                dp->cylinder = size->maxCylinders - 1;
                break;
                }

            /*
            **  Zero entire cylinder containing factory and utility data areas.
            */
            memset(mySector, 0, SectorSize * 2);
            for (dp->track = 0; dp->track < size->maxTracks; dp->track++)
                {
                for (dp->sector = 0; dp->sector < size->maxSectors; dp->sector++)
                    {
                    dd8xxSeek(dp);
                    dd8xxSectorWrite(dp, mySector);
                    }
                }

            /*
            **  Write serial number and date of manufacture.
            */
            mySector[0]  = (channelNo & 070) << (8 - 3);
            mySector[0] |= (channelNo & 007) << (4 - 0);
            mySector[0] |= (unitNo    & 070) >> (3 - 0);
            mySector[1]  = (unitNo    & 007) << (8 - 0);
            mySector[1] |= (diskType  & 070) << (4 - 3);
            mySector[1] |= (diskType  & 007) << (0 - 0);

            time(&mTime);
            lTime = localtime(&mTime);
            yy = lTime->tm_year % 100;
            mm = lTime->tm_mon + 1;
            dd = lTime->tm_mday;

            mySector[2] = (dd / 10) << 8 | (dd % 10) << 4 | mm / 10;
            mySector[3] = (mm % 10) << 8 | (yy / 10) << 4 | yy % 10;

            dp->track = 0;
            dp->sector = 0;
            dd8xxSeek(dp);
            dd8xxSectorWrite(dp, mySector);
            }
        else
            {
            if (!ddLock (&dp->ioDesc))
                {
                fprintf (stderr, "Failed to lock %s\n", deviceName);
                exit (1);
                }
            }
        
        /*
        **  Reset disk seek position.
        */
        dp->cylinder = 0;
        dp->track = 0;
        dp->sector = 0;
        dp->interlace = 1;
        dd8xxSeek(dp);
        }

    /*
    **  Print a friendly message.
    */
    printf("Disk with %d cylinders initialised on channel %o unit %o\n",
        dp->size.maxCylinders, channelNo, unitNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 8xx disk drive.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus dd8xxFunc(PpWord funcCode)
    {
    i8 unitNo;
    DiskParam *dp;
    bool opened;
    
    unitNo = activeDevice->selectedUnit;
    if (unitNo != -1)
        {
        dp = ((DiskParam **) (activeDevice->context[0]))[unitNo];
        opened = ddOpened (&dp->ioDesc);
        }
    else
        {
        dp = NULL;
        opened = FALSE;
        }

    /*
    **  Decline functions for units which are not configured.
    */
    if (!opened)
        {
////        return(FcDeclined);   <<<< fix this properly as in mt679.c
        }

    /*
    **  Deal with deadstart function.
    */
    if ((funcCode & 0700) == Fc8xxDeadstart )
        {
        funcCode = Fc8xxDeadstart;
        activeDevice->selectedUnit = funcCode & 077;
        unitNo = activeDevice->selectedUnit;
        dp = ((DiskParam **) (activeDevice->context[0]))[unitNo];
        opened = ddOpened (&dp->ioDesc);
        }

#if DEBUG
    if (dp != NULL)
        {
        fprintf(dd8xxLog, "\n%06d PP:%02o CH:%02o DSK:%o f:%04o T:%-25s   c:%3d t:%2d s:%2d  >   ", 
            traceSequenceNo,
            activePpu->id,
            activeDevice->channel->id,
            dp->diskNo,
            funcCode,
            dd8xxFunc2String(funcCode),
            dp->cylinder,
            dp->track,
            dp->sector);
        }
    else
        {
        fprintf(dd8xxLog, "\n%06d PP:%02o CH:%02o DSK:? f:%04o T:%-25s  >   ", 
            traceSequenceNo,
            activePpu->id,
            activeDevice->channel->id,
            funcCode,
            dd8xxFunc2String(funcCode));
        }

    fflush(dd8xxLog);
#endif

    /*
    **  Process function request.
    */
    switch (funcCode)
        {
    default:
#if DEBUG
        fprintf(dd8xxLog, " !!!!!FUNC not implemented & declined!!!!!! ");
#endif
        return(FcDeclined);

    case Fc8xxConnect:
        /*
        **  Expect drive number.
        */
        activeDevice->recordLength = 1;     
        break;

    case Fc8xxSeekFull:
        if (dp == NULL)
            {
            return(FcDeclined);
            }
            
        /*
        **  Expect drive number, cylinder, track and sector.
        */
        dp->interlace = 1;
        activeDevice->recordLength = 4;
        break;

    case Fc8xxSeekHalf:
        if (dp == NULL)
            {
            return(FcDeclined);
            }
            
        /*
        **  Expect drive number, cylinder, track and sector.
        */
        dp->interlace = 2;
        activeDevice->recordLength = 4;
        break;

    case Fc8xxRead:
    case Fc8xxReadFlawedSector:
        activeDevice->recordLength = SectorSize;
        dp->bufPtr = dp->buffer;
        ddQueueRead (&dp->ioDesc, dp->buffer, dp->sectorSize);
        break;

    case Fc8xxWrite:
    case Fc8xxWriteFlawedSector:
    case Fc8xxWriteLastSector:
    case Fc8xxWriteVerify:
        activeDevice->recordLength = SectorSize;
        break;

    case Fc8xxReadCheckword:
        activeDevice->recordLength = 2;
        break;

    case Fc8xxOpComplete:
    case Fc8xxDropSeeks:
        return(FcProcessed);

    case Fc8xxGeneralStatus:
        activeDevice->recordLength = 1;
        break;
 
    case Fc8xxDetailedStatus:
    case Fc8xxDetailedStatus2:
        dp->detailedStatus[2] = (funcCode << 4) & 07760;

        switch (dp->diskType)
            {
        case DiskType885:
            dp->detailedStatus[4] = (dp->cylinder >> 4) & 077;
            dp->detailedStatus[5] = ((dp->cylinder << 8) | dp->track) & 07777;
            dp->detailedStatus[6] = (dp->sector << 4) | 010;
            if ((dp->track & 1) != 0)
                {
                dp->detailedStatus[9] |= 2;  /* odd track */
                }
            else
                {
                dp->detailedStatus[9] &= ~2;
                }

            break;

        case DiskType844:
            dp->detailedStatus[4] = ((dp->cylinder & 0777) << 3) | ((dp->track >> 2) & 07);
            dp->detailedStatus[5] = ((dp->track & 03) << 10) | ((dp->sector & 017) << 5) | ((dp->cylinder >> 9) & 01);
            dp->detailedStatus[6] = (dp->sector << 4) | 010;
            break;
            }

        if (funcCode == Fc8xxDetailedStatus)
            {
            activeDevice->recordLength = 12;
            }
        else
            {
            activeDevice->recordLength = 20;
            }
        break;
 
    case Fc8xxStartMemLoad:
        break;

    case Fc8xxReadUtilityMap:
    case Fc8xxReadFactoryData:
        activeDevice->recordLength = SectorSize;
        break;

    case Fc8xxDriveRelease:
        return(FcProcessed);

    case Fc8xxDeadstart:
        switch (dp->diskType)
            {
        case DiskType844:
            if (dp->size.maxCylinders == MaxCylinders844_2)
                {
                dp->cylinder = DsCylinder844_2;
                }
            else
                {
                dp->cylinder = DsCylinder844_4;
                }

            dp->track = DsTrack844;
            dp->sector = DsSector844;
            break;

        case DiskType885:
            dp->cylinder = DsCylinder885;
            dp->track = DsTrack885;
            dp->sector = DsSector885;
            break;
            }

        dd8xxSeek(dp);
        activeDevice->recordLength = SectorSize;
        break;

    case Fc8xxSetClearFlaw:
        if (dp->diskType != DiskType844)
            {
            return(FcDeclined);
            }

        activeDevice->recordLength = 1;
        break;

    case Fc8xxFormatPack:
        if (dp->size.maxTracks == MaxTracks844) // use diskType instead?
            {
            activeDevice->recordLength = 7;
            }
        else
            {
            activeDevice->recordLength = 18;
            }
        break;

    case Fc8xxManipulateProcessor:
        activeDevice->recordLength = 5;
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
#if DEBUG
        fprintf(dd8xxLog, " !!!!!FUNC not implemented but accepted!!!!!! ");
#endif
        logError(LogErrorLocation, "ch %o, function %04o not implemented\n", activeChannel->id, funcCode);
        break;
        }

    activeDevice->fcode = funcCode;

#if DEBUG
    fflush(dd8xxLog);
#endif
    return(FcAccepted);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 8xx disk drive.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dd8xxIo(void)
    {
    i8 unitNo;
    bool opened;
    DiskParam *dp;
    i32 pos;

    unitNo = activeDevice->selectedUnit;
    if (unitNo != -1)
        {
        dp = ((DiskParam **) (activeDevice->context[0]))[unitNo];
        opened = ddOpened (&dp->ioDesc);
        }
    else
        {
        dp = NULL;
        opened = FALSE;
        }

    switch (activeDevice->fcode)
        {
    case Fc8xxConnect:
        if (activeChannel->full)
            {
            unitNo = activeChannel->data & 077;
            if (unitNo != activeDevice->selectedUnit)
                {
                dp = ((DiskParam **) (activeDevice->context[0]))[unitNo];
                if (dp != NULL && ddOpened (&dp->ioDesc))
                    {
                    activeDevice->selectedUnit = unitNo;
                    activeDevice->status = 0;
                    dp->detailedStatus[12] &= ~01000;
                    }
                else
                    {
                    activeDevice->selectedUnit = -1;
                    activeDevice->status = 05020;
                    }
                }
            else
                {
                dp->detailedStatus[12] |= 01000;
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
                unitNo = activeChannel->data & 077;
                if (unitNo != activeDevice->selectedUnit)
                    {
                    dp = ((DiskParam **) (activeDevice->context[0]))[unitNo];
                    if (dp != NULL &&  ddOpened (&dp->ioDesc))
                        {
                        activeDevice->selectedUnit = unitNo;
                        dp->detailedStatus[12] &= ~01000;
                        }
                    else
                        {
                        activeDevice->selectedUnit = -1;
                        }
                    }
                else
                    {
                    dp->detailedStatus[12] |= 01000;
                    }

                break;

            case 3:
                if (dp != NULL)
                    {
                    dp->cylinder = activeChannel->data;
                    }
                break;

            case 2:
                if (dp != NULL)
                    {
                    dp->track = activeChannel->data;
                    }
                break;

            case 1:
                if (dp != NULL)
                    {
                    dp->sector = activeChannel->data;
                    dd8xxSeek(dp);
                    }
                else
                    {
                    activeDevice->status = 05020;
                    }
                break;

            default:
                activeDevice->recordLength = 0;
                break;
                }

#if DEBUG
            fprintf(dd8xxLog, " %04o[%d]", activeChannel->data, activeChannel->data);
#endif

            activeChannel->full = FALSE;
            }
        break;

    case Fc8xxDeadstart:
        if (ddIOPending (&dp->ioDesc))
            {
            /*
            **  If async I/O is still in progress, don't transfer
            **  any data yet.
            */
            break;
            }
        if (!activeChannel->full)
            {
            if (activeDevice->recordLength == SectorSize)
                {
                /*
                **  The first word in the sector contains the data length.
                */
                activeDevice->recordLength = dp->read(dp);
                if (activeDevice->recordLength > SectorSize)
                    {
                    activeDevice->recordLength = SectorSize;
                    }

                activeChannel->data = activeDevice->recordLength;
                }
            else
                {
                activeChannel->data = dp->read(dp);
                }

            activeChannel->full = TRUE;
            
            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                dd8xxSeekNextSector(dp, FALSE);
                }
            }
        break;

    case Fc8xxRead:
    case Fc8xxReadFlawedSector:
    case Fc8xxGapRead:
        if (ddIOPending (&dp->ioDesc))
            {
            /*
            **  If async I/O is still in progress, don't transfer
            **  any data yet.
            */
#if DEBUG
            waitcount++;
#endif
            break;
            }
#if DEBUG
        if (waitcount > 0)
            {
            fprintf(dd8xxLog, "Waited %d times for read done\n", waitcount);
            waitcount = 0;
            }
#endif
        if (!activeChannel->full)
            {
            activeChannel->data = dp->read(dp);
            activeChannel->full = TRUE;

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                dd8xxSeekNextSector(dp, activeDevice->fcode == Fc8xxGapRead);
                }
            }
        break;

    case Fc8xxWrite:
    case Fc8xxWriteFlawedSector:
    case Fc8xxWriteLastSector:
    case Fc8xxWriteVerify:
    case Fc8xxGapWrite:
    case Fc8xxGapWriteVerify:
        if (ddIOPending (&dp->ioDesc))
            {
            /*
            **  If async I/O is still in progress, don't transfer
            **  any data yet.
            */
#if DEBUG
            waitcount++;
#endif
            break;
            }
#if DEBUG
        if (waitcount > 0)
            {
            fprintf(dd8xxLog, "Waited %d times for write done\n", waitcount);
            waitcount = 0;
            }
#endif
        if (activeChannel->full)
            {
            if (dp->readonly)
                {
                activeDevice->status = 05020;
                }
            else
                {
                dp->write(dp, activeChannel->data);
                }
            activeChannel->full = FALSE;

            if (--activeDevice->recordLength == 0 && !dp->readonly)
                {
                    dd8xxSeekNextSector(dp, activeDevice->fcode == Fc8xxGapWrite ||
                                        activeDevice->fcode == Fc8xxGapWrite);
                }
            }
        break;

    case Fc8xxGeneralStatus:
        if (!activeChannel->full)
            {
            activeChannel->data = activeDevice->status;
            if (dp != NULL && ddIOPending (&dp->ioDesc))
                {
                /*
                **  Show busy if async I/O is still in progress.
                */
                activeChannel->data |= St8xxBusy;
                }
            activeChannel->full = TRUE;

#if DEBUG
            fprintf(dd8xxLog, " %04o[%d]", activeChannel->data, activeChannel->data);
#endif

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;

    case Fc8xxReadCheckword:
    case Fc8xxGapReadCheckword:
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
    case Fc8xxDetailedStatus:
        if (!activeChannel->full)
            {
            activeChannel->data = dp->detailedStatus[12 - activeDevice->recordLength];
            activeChannel->full = TRUE;

#if DEBUG
            fprintf(dd8xxLog, " %04o[%d]", activeChannel->data, activeChannel->data);
#endif

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;

    case Fc8xxDetailedStatus2:
        if (!activeChannel->full)
            {
            activeChannel->data = dp->detailedStatus[20 - activeDevice->recordLength];
            activeChannel->full = TRUE;
#if DEBUG
            fprintf(dd8xxLog, " %04o[%d]", activeChannel->data, activeChannel->data);
#endif

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;

    case Fc8xxReadFactoryData:
    case Fc8xxReadUtilityMap:
        if (ddIOPending (&dp->ioDesc))
            {
            /*
            **  If async I/O is still in progress, don't transfer
            **  any data yet.
            */
            break;
            }
        if (!activeChannel->full)
            {
            activeChannel->data = dp->read(dp);
            activeChannel->full = TRUE;

#if DEBUG
            fprintf(dd8xxLog, " %04o[%d]", activeChannel->data, activeChannel->data);
#endif

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;

    case Fc8xxSetClearFlaw:
        if (activeChannel->full)
            {
#if DEBUG
            fprintf(dd8xxLog, " %04o[%d]", activeChannel->data, activeChannel->data);
#endif
            dd844SetClearFlaw(dp, activeChannel->data);
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
    default:
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
**  Purpose:        Work out seek offset.
**
**  Parameters:     Name        Description.
**                  dp          Disk parameters (context).
**
**  Returns:        nothing
**
**  The calculated position is saved in the DiskParam struct.  If
**  async I/O is not being used, a seek is done to that offset.
**
**------------------------------------------------------------------------*/
static void dd8xxSeek(DiskParam *dp)
    {
    i32 result;

    dp->bufPtr = NULL;

    activeDevice->status = 0;

    if (dp->cylinder >= dp->size.maxCylinders)
        {
#if DEBUG
        fprintf(dd8xxLog, "ch %o, cylinder %d invalid\n", activeChannel->id, dp->cylinder); 
#endif
        logError(LogErrorLocation, "ch %o, cylinder %d invalid\n", activeChannel->id, dp->cylinder);
        activeDevice->status = 01000;
        return;
        }

    if (dp->track >= dp->size.maxTracks)
        {
#if DEBUG
        fprintf(dd8xxLog, "ch %o, track %d invalid\n", activeChannel->id, dp->track);
#endif
        logError(LogErrorLocation, "ch %o, track %d invalid\n", activeChannel->id, dp->track);
        activeDevice->status = 01000;
        return;
        }

    if (dp->sector >= dp->size.maxSectors)
        {
#if DEBUG
        fprintf(dd8xxLog, "ch %o, sector %d invalid\n", activeChannel->id, dp->sector);
#endif
        logError(LogErrorLocation, "ch %o, sector %d invalid\n", activeChannel->id, dp->sector);
        activeDevice->status = 01000;
        return;
        }

    result  = dp->cylinder * dp->size.maxTracks * dp->size.maxSectors;
    result += dp->track * dp->size.maxSectors;
    result += dp->sector;
    result *= dp->sectorSize;

    ddSeek (&dp->ioDesc, result);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Position to next sector taking into account interlace.
**
**  Parameters:     Name        Description.
**                  dp          Disk parameters (context).
**                  gap         TRUE if function was ReadGap or WriteGap
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void dd8xxSeekNextSector(DiskParam *dp, bool gap)
    {
    dp->sector += dp->interlace + (gap ? 2 : 0);

    if (dp->sector >= dp->size.maxSectors)
        {
        if (dp->interlace == 1)
            {
            dp->sector = 0;
            dp->track += 1;

            if (dp->track == dp->size.maxTracks)
                {
                /*
                **  Wrap to the start of the current cylinder.
                */
                dp->track = 0;
                dp->sector = 0;
                }
            }
        else
            {
            if (dp->sector == dp->size.maxSectors ||
                dp->sector == dp->size.maxSectors + 2)
                {
                dp->sector = 0;
                dp->track += 1;
                if (dp->track == dp->size.maxTracks)
                    {
                    /*
                    **  Now start all odd sectors.
                    */
                    dp->track = 0;
                    dp->sector = 1;
                    }
                }
            else
                {
                dp->sector = 1;
                dp->track += 1;

                if (dp->track == dp->size.maxTracks)
                    {
                    /*
                    **  Wrap to the start of the current cylinder.
                    */
                    dp->track = 0;
                    dp->sector = 0;
                    }
                }
            }
        }

    dd8xxSeek(dp);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform a 12 bit PP word read from a classic disk container.
**
**  Parameters:     Name        Description.
**                  dp          Disk parameters (context).
**
**  Returns:        PP word read.
**
**------------------------------------------------------------------------*/
static PpWord dd8xxReadClassic(DiskParam *dp)
    {
    int count;
    
    /*
    **  Read an entire sector if the current buffer is empty.
    **  Note that this doesn't apply to the regular 8xx read
    **  function, because that does a read ahead in the function
    **  processing routine.
    */
    if (dp->bufPtr == NULL)
        {
        dp->bufPtr = dp->buffer;
        ddQueueRead (&dp->ioDesc, dp->buffer, dp->sectorSize);
        ddWaitIO (&dp->ioDesc);
        count = dp->sectorSize;   // **** TEMP

        /*
        **  If we hit EOF, that means the container file is shorter
        **  than the disk size.  That can happen if it is created
        **  elsewhere and loaded as an existing container.  We
        **  treat it using regular sparse file rules -- missing
        **  bytes count as zeroes.
        */
        if (count == 0)
            {
            memset (dp->buffer, 0, dp->sectorSize);
            }
        }
    /*
    **  Fail gracefully if we read too much data.
    */
    if (dp->bufPtr >= dp->buffer + SectorSize)
        {
        return(0);
        }

    return(*dp->bufPtr++);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform a 12 bit PP word write to a classic disk container.
**
**  Parameters:     Name        Description.
**                  dp          Disk parameters (context).
**                  data        PP word to be written.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dd8xxWriteClassic(DiskParam *dp, PpWord data)
    {
    /*
    **  Fail gracefully if we write too much data.
    */
    if (dp->bufPtr >= dp->buffer + SectorSize)
        {
        return;
        }

    /*
    **  Reset pointer if the current buffer is empty.
    */
    if (dp->bufPtr == NULL)
        {
        dp->bufPtr = dp->buffer;
        if (ddIOPending (&dp->ioDesc))
            {
            /*
            **  If async I/O is still in progress, wait for it to finish.
            */
            ddWaitIO (&dp->ioDesc);
            }
        }

    *dp->bufPtr++ = data;

    /*
    **  Write the data if we got a full sector.
    */
    if (dp->bufPtr == dp->buffer + SectorSize)
        {
        ddQueueWrite (&dp->ioDesc, dp->buffer, dp->sectorSize);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform a 12 bit PP word read from a packed disk container.
**
**  Parameters:     Name        Description.
**                  dp          Disk parameters (context).
**
**  Returns:        PP word read.
**
**------------------------------------------------------------------------*/
static PpWord dd8xxReadPacked(DiskParam *dp)
    {
    u16 byteCount;
    static u8 sector[512];
    u8 *sp;
    PpWord *pp;
    int count;
    
    /*
    **  Read an entire sector if the current buffer is empty.
    **  Note that this doesn't apply to the regular 8xx read
    **  function, because that does a read ahead in the function
    **  processing routine.
    */
    if (dp->bufPtr == NULL)
        {
        dp->bufPtr = dp->buffer;
        ddQueueRead (&dp->ioDesc, dp->buffer, dp->sectorSize);
        }
    if (dp->bufPtr == dp->buffer)
    {
        /*
        **  We're at the start of the sector buffer.  Go unpack it.
        */
        memcpy (sector, dp->buffer, dp->sectorSize);
        ddWaitIO (&dp->ioDesc);
        count = dp->sectorSize;   // **** TEMP

        /*
        **  If we hit EOF, that means the container file is shorter
        **  than the disk size.  That can happen if it is created
        **  elsewhere and loaded as an existing container.  We
        **  treat it using regular sparse file rules -- missing
        **  bytes count as zeroes.
        */
        if (count == 0)
            {
            memset (sector, 0, dp->sectorSize);
            }

        /*
        **  Unpack the sector into the buffer.
        */
        sp = sector;
        pp = dp->buffer;
        for (byteCount = SectorSize; byteCount > 0; byteCount -= 2)
            {
            *pp++ = (sp[0] << 4) + (sp[1] >> 4);
            *pp++ = (sp[1] << 8) + (sp[2] >> 0);
            sp += 3;
            }
        }

    /*
    **  Fail gracefully if we read too much data.
    */
    if (dp->bufPtr >= dp->buffer + SectorSize)
        {
        return(0);
        }

    /*
    **  Return one data word.
    */
    return((*dp->bufPtr++) & Mask12);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform a 12 bit PP word write to a packed disk container.
**
**  Parameters:     Name        Description.
**                  dp          Disk parameters (context).
**                  data        PP word to be written.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dd8xxWritePacked(DiskParam *dp, PpWord data)
    {
    u16 byteCount;
    static u8 sector[512];
    u8 *sp;
    PpWord *pp;

    /*
    **  Fail gracefully if we write too much data.
    */
    if (dp->bufPtr >= dp->buffer + SectorSize)
        {
        return;
        }

    /*
    **  Reset pointer if the current buffer is empty.
    */
    if (dp->bufPtr == NULL)
        {
        dp->bufPtr = dp->buffer;
        if (ddIOPending (&dp->ioDesc))
            {
            /*
            **  If async I/O is still in progress, wait for it to finish.
            */
            ddWaitIO (&dp->ioDesc);
            }
        }

    *dp->bufPtr++ = data;

    /*
    **  Write the data if we got a full sector.
    */
    if (dp->bufPtr == dp->buffer + SectorSize)
        {
        /*
        **  Pack the buffer into a sector.
        */
        sp = sector;
        pp = dp->buffer;
        for (byteCount = SectorSize; byteCount > 0; byteCount -= 2)
            {
            *sp++  = (u8)(pp[0] >> 4);
            *sp    = (u8)(pp[0] << 4);
            *sp++ |= (u8)(pp[1] >> 8);
            *sp++  = (u8)(pp[1] >> 0);
            pp += 2;
            }

        /*
        **  Write the sector from the buffer in the DiskParam struct.
        **  We write it from there and not from the local variable
        **  because the write may be asynchronous, and the local
        **  variable is about to go away.
        */
        memcpy (dp->buffer, sector, dp->sectorSize);
        ddQueueWrite (&dp->ioDesc, dp->buffer, dp->sectorSize);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform a sector write to a disk container.
**
**  Parameters:     Name        Description.
**                  dp          Disk parameters (context).
**                  sector      Pointer to sector to write.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dd8xxSectorWrite(DiskParam *dp, PpWord *sector)
    {
    u16 byteCount;

    for (byteCount = SectorSize; byteCount > 0; byteCount--)
        {
        dp->write(dp, *sector++);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Manipulate 844 utility (flaw) map.
**
**  Parameters:     Name        Description.
**                  dp          Disk parameters (context).
**                  flawState   Flaw state from PP.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dd844SetClearFlaw(DiskParam *dp, PpWord flawState)
    {
    u8 unitNo;
    int index;
    PpWord flawWord0;
    PpWord flawWord1;
	PpWord sectorFlaw;
	PpWord trackFlaw;
    bool setFlaw;

    unitNo = activeDevice->selectedUnit;

    /*
    **  Assemble flaw words.
    */
	if ((flawState & 1) == 1)
        {
	    trackFlaw  = 1;
	    sectorFlaw = 0;
        }
    else
        {
	    trackFlaw  = 0;
	    sectorFlaw = 1;
        }

    setFlaw = (flawState & 2) != 0;

    flawWord0 = (PpWord)((sectorFlaw << 11) | (trackFlaw << 10) | (dp->cylinder & Mask10));
    flawWord1 = (PpWord)(((dp->track & Mask6) << 6) | (dp->sector & Mask6));

    /*
    **  Read the 844 utility map sector.
    */
    dp->cylinder = dp->size.maxCylinders - 1;
    dp->track = 0;
    dp->sector = 2;
    dd8xxSeek(dp);
    ddQueueRead (&dp->ioDesc, mySector, 2 * SectorSize);
    ddWaitIO (&dp->ioDesc);
    
    /*
    **  Process request.
    */
    if (setFlaw)
        {
        /*
        **  Find a free flaw entry.
        */
        index = 0;
        while (index < SectorSize) 
            {
            index += 2;
            if (mySector[index] == 0)
                {
                break;
                }
            }

        /*
        **  If a free flaw entry was found, set it.
        */
        if (index < SectorSize)
            {
            mySector[index + 0] = flawWord0;
            mySector[index + 1] = flawWord1;
            }
        }
    else
        {
        /*
        **  Find the matching entry.
        */
        index = 0;
        while (index < SectorSize) 
            {
            if (   mySector[index + 0] == flawWord0
                && mySector[index + 1] == flawWord1)
                {
                break; 
                }

            index += 2;
            }

        /*
        **  If a matching entry was found, clear it.
        */
        if (index < SectorSize)
            {
            mySector[index + 0] = 0;
            mySector[index + 1] = 0;
            }
        }

    /*
    **  Update the 844 utility map sector.
    */
    dd8xxSeek(dp);
    dd8xxSectorWrite(dp, mySector);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Convert function code to string.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        String equivalent of function code.
**
**------------------------------------------------------------------------*/
static char *dd8xxFunc2String(PpWord funcCode)
    {
#if DEBUG
    switch(funcCode)
        {
    case Fc8xxConnect                :  return "Connect";              
    case Fc8xxSeekFull               :  return "SeekFull";             
    case Fc8xxSeekHalf               :  return "SeekHalf";             
    case Fc8xxIoLength               :  return "IoLength";             
    case Fc8xxRead                   :  return "Read";                 
    case Fc8xxWrite                  :  return "Write";                
    case Fc8xxWriteVerify            :  return "WriteVerify";          
    case Fc8xxReadCheckword          :  return "ReadCheckword";        
    case Fc8xxOpComplete             :  return "OpComplete";           
    case Fc8xxDisableReserve         :  return "DisableReserve";       
    case Fc8xxGeneralStatus          :  return "GeneralStatus";        
    case Fc8xxDetailedStatus         :  return "DetailedStatus";       
    case Fc8xxContinue               :  return "Continue";             
    case Fc8xxDropSeeks              :  return "DropSeeks";            
    case Fc8xxFormatPack             :  return "FormatPack";           
    case Fc8xxOnSectorStatus         :  return "OnSectorStatus";       
    case Fc8xxDriveRelease           :  return "DriveRelease";         
    case Fc8xxReturnCylAddr          :  return "ReturnCylAddr";        
    case Fc8xxSetClearFlaw           :  return "SetClearFlaw";         
    case Fc8xxDetailedStatus2        :  return "DetailedStatus2";      
    case Fc8xxGapRead                :  return "GapRead";              
    case Fc8xxGapWrite               :  return "GapWrite";             
    case Fc8xxGapWriteVerify         :  return "GapWriteVerify";       
    case Fc8xxGapReadCheckword       :  return "GapReadCheckword";     
    case Fc8xxReadFactoryData        :  return "ReadFactoryData";      
    case Fc8xxReadUtilityMap         :  return "ReadUtilityMap";       
    case Fc8xxReadFlawedSector       :  return "ReadFlawedSector";     
    case Fc8xxWriteLastSector        :  return "WriteLastSector";      
    case Fc8xxWriteVerifyLastSector  :  return "WriteVerifyLastSector";
    case Fc8xxWriteFlawedSector      :  return "WriteFlawedSector";    
    case Fc8xxManipulateProcessor    :  return "ManipulateProcessor";  
    case Fc8xxDeadstart              :  return "Deadstart";            
    case Fc8xxStartMemLoad           :  return "StartMemLoad";         
        }
#endif
    return "UNKNOWN";
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform load/unload on 844/885 disk.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dd8xxLoad(DevSlot *ds, int unitNo, char *fn)
    {
    DiskParam *dp;
    u8 unitMode = 'r';
    char *p;
    static char msgBuf[80];
    long endPos = 0;
    
    if (unitNo < 0 || unitNo >= MAXDDUNITS)
        {
        opSetMsg ("$INVALID UNIT NO");
        return;
        }
    /*
    **  Check if the unit is even configured.
    */
    dp = ((DiskParam **) (activeDevice->context[0]))[unitNo];
    if (dp == NULL)
        {
        opSetMsg ("$UNIT NOT ALLOCATED");
        return;
        }

    if (fn == NULL)
        {
        /*
        **  Check if the unit is currently loaded.
        */
        if (!ddOpened (&dp->ioDesc))
            {
            opSetMsg ("$UNIT NOT LOADED");
            return;
            }
        ddClose (&dp->ioDesc);
        if (ds->selectedUnit == unitNo)
            {
            ds->selectedUnit = -1;
            }
        opSetStatus (dp->statusBuf, "");
        opSetMsg ("DD8xx unloaded");
        return;
        }
    
    /*
    **  Check if the unit has been unloaded.
    */
    if (ddOpened (&dp->ioDesc))
        {
        opSetMsg ("$UNIT NOT UNLOADED");
        return;
        }

    p = strchr (fn, ',');
    if (p != NULL)
        {
        *p = '\0';
        unitMode = 'w';
        }
    /*
    **  Open the file in the requested mode.
    */
    if (unitMode == 'w')
        {
        if (!ddOpen (&dp->ioDesc, fn, TRUE))
            {
            if (!ddCreate (&dp->ioDesc, fn))
                {
                opSetMsg ("Error creating disk container");
                return;
                }
            }
        }
    else
        {
        ddOpen (&dp->ioDesc, fn, FALSE);
        }
    if (ddOpened (&dp->ioDesc))
        {
        if (!ddLock (&dp->ioDesc))
            {
            opSetMsg ("Error locking disk container");
            ddClose (&dp->ioDesc);
            return;
            }
        }
    else
        {
        sprintf (msgBuf, "$Open error: %s", strerror (errno));
        opSetMsg(msgBuf);
        return;
        }

    /*
    **  Set format to packed.
    */
    dp->read = dd8xxReadPacked;
    dp->write = dd8xxWritePacked;
    dp->sectorSize = 512;
    dp->readonly = (unitMode != 'w');
    
    sprintf (msgBuf, "%s%s",
             fn, ((unitMode == 'w') ? "" : " (RO)"));
    opSetStatus (dp->statusBuf, msgBuf);
    sprintf (msgBuf, "DD8XX loaded with %s, %s",
             fn, ((unitMode == 'w') ? "write enabled" : "write locked"));
    opSetMsg (msgBuf);
    }


/*---------------------------  End Of File  ------------------------------*/
