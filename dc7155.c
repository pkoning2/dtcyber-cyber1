/*--------------------------------------------------------------------------
**
**  Copyright (c) 2002-2004, Gerard J van der Grinten (see license.txt)
**
**  Name: C7155.c
**
**  Description:
**      Perform simulation of CDC 7155 disk controller.
**
**      Supported devices:
**           844 disk - units 0 - 7   ( 8 drives)
**           buy OPTION_10398 for 844 units 10-17
**           885 disk - units 40 - 57 (16 drives)
**
**  Version gvdg - 15-04-2004
**
**--------------------------------------------------------------------------
*/

#ifndef DEBUG
#define DEBUG 01
#endif

#define OPTION_10398 1

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
 FCCN     EQU    0           CONNECT FUNCTION CODE
 FCS1     EQU    1           SEEK 1 TO 1 INTERLACE
 FCS2     EQU    2           SEEK 2 TO 1 INTERLACE
 FCRD     EQU    4           READ FUNCTION CODE 
 FCWR     EQU    5           WRITE FUNCTION CODE
 FCOC     EQU    10          OPERATION COMPLETE FUNCTION CODE 
 FCGR     EQU    11          DISABLE RESERVES - GRENADE 
 FCGS     EQU    12          GENERAL STATUS FUNCTION CODE 
 FCCO     EQU    14          CONTINUE FUNCTION CODE 
 FCDK     EQU    15          DROP SEEKS 
 FCDR     EQU    20          DRIVE RELEASE
 FCDS     EQU    23          DETAILED STATUS FUNCTION CODE
 FCRF     EQU    30          READ FACTORY DATA FUNCTION CODE
 FCRU     EQU    31          READ UTILITY SECTOR FUNCTION CODE
 FCRP     EQU    34          READ PROTECTED SECTOR
 FCWL     EQU    35          WRITE LAST SECTOR
 FCWP     EQU    37          WRITE PROTECTED SECTOR 
 FCRB     EQU    43          READ BACK COUPLER BUFFER 
 FCST     EQU    52          INPUT PROCESSOR STATUS 
 FCSU     EQU    55          SPIN UP AN ISD DRIVE 
 FCSD     EQU    56          SPIN DOWN AN ISD DRIVE 
 FCTD     EQU    56          INPUT TIMING DATA
 FCDP     EQU    61          AUTODUMP 
 FCMP     EQU    62          MANIPULATE PROCESSOR 
 FCID     EQU    63          INPUT DISPLAY DATA 
 FCFT     EQU    64          FUNCTION TIMING DIFFERENCE COUNTER 
 FCLC     EQU    71          AUTOLOAD CONTROL MODULE FROM PP
 FCAL     EQU    414         AUTOLOAD FUNCTION CODE 
          SPACE  4
**        ASSEMBLY CONSTANTS. 
  
  
 DSLN     EQU    24          DETAILED STATUS LENGTH 
 FRNW     EQU    3           FIRMWARE REVISION NUMBER WORD
 ARNW     EQU    20          FSC ADAPTOR REVISION NUMBER WORD 
 CMNW     EQU    21          CONTROL MODULE REVISION NUMBER WORD

**  CDC 7155 disk controller function and status codes.
**  
**  0000    Connect                                    
**  0001    Seek+fulltrack                       
**  0002    Seek+halftrack                       
**  0003    I/O length                              
**  0004    Read                                
**  0005    Write                                
**  0006    Write-verify                         
**  0007    Read checkword                   
**  0010    Operation complete (deselect)               
**  0011    Disable reserve             
**  0012    General status                       
**  0013    Detailed status                      
**  0014    Continue                             
**  0015    Drop seeks (deselect)        
**  0016    Format pack                          
**  0017    On-sector status
**  0020    Drive release
**  0021    Return cylinder address
**  0022    Set/clear flow
**  0024    Gap sector - read
**  0025    Gap sector - write
**  0026    Gap sector - write verify
**  0027    Gap sector - read checkword
**  0030    Read factory data                    
**  0031    Read utility map                     
**  0032    BLOCK TRANSFER BUF READ
**  0033    BLOCK TRANSFER BUF WRITE
**  0034    READ PROTECTED SECTOR
**  0035    WRITE LAST SECTOR
**  0036    WRITE VERIFY LAST SECTOR
**  0037    WRITE PROTECTED SECTOR
**  0040    READ SHORT
**  0041    SELECT STROBE AND OFFSET
**  0042    CLEAR CONNECTED ACCESS
**  0043    BUFFER READ 
**  0044    BUFFER WRITE 
**  0046    WRITE BUFFER TO DISK 
**  0047    SCAN CYLINDER ADDRESS
**  0050    OUTPUT PROC CHANNEL 
**  0051    EXEC CNTRL WORD SEQUENCE 
**  0052    INPUT PROC CHAN STAT
**  0053    ECHO OUTPUT CHANNELS 
**  0054    ISSUE PROC FLAG PLS 1 
**  0055    ENA INPUT CHAN TIMING 1
**  0056    INPUT TIMING DATA 
**  0057    ECHO ONE WORD 
**  0061    AUTODUMP 
**  0062    MANIPULATE 
**  0063    INPUT DISPLAY DATA 
**  0064    TIME DIFF COUNTER 
**  0066    FORCE ERROR 
**  0067    INTERLOCK AUTOLOAD 
**  01xx    Autoload from Disk
**  03xx    Disk Deadstart
**  0414    Start memory load from PP (hardware function)   
*/
#define Fc7155Connect               00000
#define Fc7155SeekFull              00001
#define Fc7155SeekHalf              00002
#define Fc7155IoLength              00003
#define Fc7155Read                  00004
#define Fc7155Write                 00005
#define Fc7155WriteVerify           00006
#define Fc7155ReadCheckword         00007
#define Fc7155OpComplete            00010
#define Fc7155DisableReserve        00011
#define Fc7155GeneralStatus         00012
#define Fc7155DetailedStatus        00013
#define Fc7155Continue              00014
#define Fc7155DropSeeks             00015
#define Fc7155FormatPack            00016
#define Fc7155OnSectorStatus        00017
#define Fc7155DriveRelease          00020
#define Fc7155ReturnCylAddr         00021
#define Fc7155SetClearFlaw          00022
#define Fc7155DetailedStatus2       00023
#define Fc7155GapRead               00024
#define Fc7155GapWrite              00025
#define Fc7155GapWriteVerify        00026
#define Fc7155GapReadCheckword      00027
#define Fc7155ReadFactoryData       00030
#define Fc7155ReadUtilityMap        00031
#define Fc7155BlockTransferRead     00032
#define Fc7155BlockTransferWrite    00033
#define Fc7155ReadProtectedSector   00034
#define Fc7155WriteLastSector       00035
#define Fc7155WriteVerifyLastSector 00036
#define Fc7155WriteProtectedSector  00037
#define Fc7155ReadShort             00040
#define Fc7155SelectStrobeOffset    00041
#define Fc7155ManipulateProcessor   00062
#define Fc7155AutoloadFromDisk      00100
#define Fc7155Deadstart             00300
#define Fc7155AutoloadFromPP        00414
#define Fc7155EchoOneWord           00720

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
#define St7155Abnormal           04000
#define St7155OppositeReserved   02000
#define St7155NonRecoverable     01000
#define St7155Recovering         00400
#define St7155CheckwordError     00200
#define St7155CorrectableAddress 00100
#define St7155CorrectableData    00040
#define St7155DSUmalfunction     00020
#define St7155DSUreserved        00010
#define St7155MiscError          00004
#define St7155Busy               00002
#define St7155DataError          00001

/*
**  Physical dimensions of 844 disks.
**  322 12-bit bytes per sector (64 cm wds + 2 bytes).  1st
**      byte is unused. 2nd byte contains byte count of data.

**   24 sectors/track
**   19 tracks/cylinder
**  823 cylinders/unit on 844-41 and 844-44
*/
#define MaxCylinder844         823
#define MaxTrack844            19
#define MaxSector844           24
#define SectorSize             322

/*
**  Address of 844 deadstart sector.
*/
#define DsCylinder844          822
#define DsTrack844             0
#define DsSector844            3

/*
**  Physical dimensions of 885 disks.
**  322 12-bit bytes per sector (64 cm wds + 2 bytes).  1st
**      byte is unused. 2nd byte contains byte count of data.                                              
**   32 sectors/track                                     
**   40 tracks/cylinder                                    
**  843 cylinders/unit on 885-2
*/
#define MaxCylinder885          843    /*  841 user cyl 842 = eng cyl */
#define MaxTrack885             40  /* spec says 20  1MV goes to 050 */
#define MaxSector885            32  /* 32 sectors */

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
#define CtClassic               1
#define CtPacked                2

#define  seekCylinderError      04  /* detailed status word 1 bit 2 */
#define  seekTrackError         02  /* detailed status word 1 bit 1 */
#define  seekSectorError        01  /* detailed status word 1 bit 0 */
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

struct diskParam;
struct unitParam;

typedef PpWord (*DiskRead)(struct unitParam *, FILE *fcb);
typedef void (*DiskWrite)(struct unitParam *, FILE *fcb, PpWord);
typedef struct diskSize
    {
    i32         maxCylinder;
    i32         maxTrack;
    i32         maxSector;
    } DiskSize;

typedef struct diskParam
    {
    PpWord      buffer[SectorSize];
    PpWord      *bufPtr;
    } DiskParam;

typedef struct unitParam
    {
    DiskRead    read;
    DiskWrite   write;
    i32         sectorSize;
    DiskSize    size;
    u32         sector;
    u32         track;
    u32         cylinder;
    u8          interlace;
    u16         maxCyl;
    u16         seekStatus;
    u16         nextSeekStatus;
    u8          diskNo;
    u8          unitNo;
    u8          diskType;
    DiskParam * controller;
    FILE      * fcb;
    u16         detailedStatus[20];
    } UnitParam;

typedef struct unitMux
   {
   UnitParam   *units[8];
   } UnitMux;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus dc7155Func(PpWord funcCode);
static void dc7155Io(void);
static void dc7155Activate(void);
static void dc7155Disconnect(void);
static i32 dc7155Seek(UnitParam *up);
static i32 dc7155SeekNextSector(UnitParam *up);
static void dc7155Dump(PpWord data);
static void dc7155Flush(void);

static PpWord dc7155ReadClassic(UnitParam *dp, FILE *fcb);
static PpWord dc7155ReadPacked(UnitParam *dp, FILE *fcb);
static void dc7155WriteClassic(UnitParam *dp, FILE *fcb, PpWord data);
static void dc7155WritePacked(UnitParam *dp, FILE *fcb, PpWord data);
static void dc7155SectorRead(UnitParam *dp, FILE *fcb, PpWord *sector);
static void dc7155SectorWrite(UnitParam *dp, FILE *fcb, PpWord *sector);

static void SetClearFlaw(UnitParam *up, u32 flaw, PpWord flawState);
void make_test_data(unsigned int loopcount);
static char* dc7155Func2String(PpWord funcCode);

unsigned short ranf();
void make_test_data(unsigned int loopcount);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
static FILE *DC7155;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static PpWord dmp[8];
static PpWord mySector[SectorSize];

static DiskSize sizeDd844 = {MaxCylinder844, MaxTrack844, MaxSector844};
static DiskSize sizeDd885 = {MaxCylinder885, MaxTrack885, MaxSector885};

static u16 dstatus844[20] =             
        {   0, /* 01 - strobe offset - seek status */
         0000, /* 02 - not applicable */
        00000, /* 03 - command code */
        07400, /* 04 - dsu number */
        00000, /* 05 - address 1 of failing sector */
        00000, /* 06 - address 2 of failing sector + flaw info */
        00000, /* 07 - not used */
        00000, /* 08 - not used */
        01740, /* 09 - DSU status */
        04000, /* 10 - DSU fault status */
        06560, /* 11 - pack on */
        00000, /* 12 - bit address of correctable read error */
        00000, /* 13 - pp address of correctable read error */
        00000, /* 14 - first word of correctable vector */
        00000, /* 15 - second word of correctable vector */
        00000, /* 16 - DSC operating status word */
        00000, /* 17 - coupler buffer status */
        00400, /* 18 - last command */
        00000, /* 19 - last command 2 and 3*/
        00000  /* 20 - last command 4 */
        };

static u16 dstatus885[20] =             
        {   0, /* 01 - strobe offset - seek status */
         0340, /* 02 - latch status */
            0, /* 03 - command code */
        07400, /* 04 - dsu number */
            0, /* 05 - address 1 of failing sector */
            0, /* 06 - address 2 of failing sector */
          010, /* 07 - non recoverable error status */
          037, /* 08 - 11 bit correction factor */
        01640, /* 09 - DSU status */
        07201, /* 10 - DSU fault status */
            0, /* 11 - DSU interlock status */
            0, /* 12 - bit address of correctable read error */
            0, /* 13 - pp address of correctable read error */
            0, /* 14 - first word of correctable vector */
            0, /* 15 - second word of correctable vector */
            0, /* 16 - DSC operating status word */
            0, /* 17 - coupler buffer status */
         0400, /* 18 - last command */
            0, /* 19 - last command 2 and 3*/
            0  /* 20 - last command 4 */
        };


static PpWord testdatainit[48]=
{
         0,      1,      2,      4,      8,     16,     32,     64,
       128,    256,    512,   1024,   2048,   4096, 0xffff,      0,
    0xffff, 0xfffe, 0xfffd, 0xfffb, 0xfff7, 0xffef, 0xffdf, 0xffbf,
    0xff7f, 0xfeff, 0xfdff, 0xfbff, 0xf7ff,      0, 0xffff,      0, 
     0xfc0,  0x03f,  0xfc0,   0x3f,  0xfc0,   0x3f,  0xfc0,   0x3f,
     0x5a5,  0xa5a,  0x5a5,  0xa5a,  0x5a5,  0xa5a,  0x5a5,  0xa5a
};
static PpWord testdata[SectorSize];                     /* for RWLOOP */
static unsigned short myRandom = 1;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

void dc7155Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*--------------------------------------------------------------------------
**  Purpose:        Initialise 7155 disk drive.
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
// void dc7155Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName, int DiskType)
void dc7155Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    DevSlot *ds;
    FILE *fcb;
    char fname[80];
    DiskParam *dp;
    UnitParam *up;
    UnitMux   *um;

    (void)eqNo;
    (void)unitNo;
    (void)deviceName;

    int DiskType;
    int muxIndex;
    int muxNo;
    u8 containerType = CtClassic;
    char *opt = NULL;

#if DEBUG
    if (DC7155 == NULL)
        {
        DC7155 = fopen("dc7155.trc","w+");
        fprintf(DC7155,"Log opened\n");
        fflush(DC7155);
        }
#endif

    ds = channelAttach(channelNo, eqNo, DtDd7155);
    ds->activate = dc7155Activate;
    ds->disconnect = dc7155Disconnect;
    ds->func = dc7155Func;
    ds->io = dc7155Io;
    ds->selectedUnit = unitNo;

    /*
     *  the context aray is used for:
     *  0 - mux for 844 units 0 - 7
     *  1,2,3 - not used
     *  4 and 5 - mux for 885 units 0 - 15 (i.e. units 40-57)
     *  6 - not used
     *  7 - Disk parameters (common for all drives)
     */
    if (ds->context[7] == NULL)
        {
        dp = (DiskParam *)calloc(1, sizeof(DiskParam));
        if (dp == NULL)
            {
            fprintf(stderr, "Failed to allocate dc7155 context block\n");
            exit(1);
            }
        ds->context[7] = dp;

        /* allocate the muxes */
        for (muxIndex = 0; muxIndex < 7 ; muxIndex++)
            {
            switch (muxIndex)
               {
            case 0:
#if OPTION_10398
            case 1:
#endif
            case 4:
            case 5:
                um = (UnitMux *)calloc(1, sizeof(UnitMux));
                if (um == NULL)
                    {
                    fprintf(stderr, "Failed to allocate dc7155 unit mux block %d\n",muxIndex);
                    exit(1);
                    }
                ds->context[muxIndex] = um;
                break;
            default:
                break;
                }
            }

        }
    else
        {
        dp = ds->context[7];
        }

    muxIndex = unitNo >> 3;
    muxNo = unitNo & 07;

    switch (muxIndex)
        {
        case 0:
#if OPTION_10398
            case 1:
#endif
            DiskType = DiskType844;
            break;
        case 4:
        case 5:
            DiskType = DiskType885;
            break;

        default:
            fprintf(stderr, "7155 unit %02o - index out of range (allowed 0-7 and 40-57)\n",unitNo);
            exit(1);
        }

    um = ds->context[muxIndex];  

    if (um->units[muxNo] == NULL)
        {
        up = (UnitParam *)calloc(1, sizeof(UnitParam));
        um->units[muxNo] = up;
        }
    else
        {
        fprintf(stderr, "DC7155 unit %2o already allocated\n",unitNo);
        fflush(DC7155);
        exit(1);
        }

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
        switch (DiskType)
            {
        case DiskType885:
            containerType = CtPacked;
            break;

        case DiskType844:
            containerType = CtClassic;
            break;
            }
        }

    /* 
    **  Setup environment for disk container type.
    */ 
    switch (containerType)
        {
    case CtClassic:
        up->read = dc7155ReadClassic;
        up->write = dc7155WriteClassic;
        up->sectorSize = SectorSize * 2; 
        break;

    case CtPacked:
        up->read = dc7155ReadPacked;
        up->write = dc7155WritePacked;
        up->sectorSize = 512;
        break;
        }

    if (DiskType == DiskType844)
        {
        up->size = sizeDd844;
        up->cylinder = sizeDd844.maxCylinder;
        memcpy(up->detailedStatus, dstatus844, sizeof(dstatus844));
        }
    else
        {
        up->size = sizeDd885;
        up->cylinder = sizeDd885.maxCylinder;
        memcpy(up->detailedStatus, dstatus885, sizeof(dstatus885));
        }
    up->diskType = DiskType;
    up->unitNo = unitNo;
    up->track = 0;
    up->sector = 0;
    up->seekStatus = 0;
    up->controller = dp; /* save backward link */

    if (deviceName == NULL)
        {
        if (DiskType == DiskType844)
            {
            sprintf(fname, "DD844_C%02o_u%2o", channelNo, unitNo);
            }
        else
            {
            sprintf(fname, "DD885_C%02o_u%2o", channelNo, unitNo);
            }
        }
    else
        {
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
        else
            {
      /* setup max disk size */
            if (DiskType == DiskType844)
                {
                up->cylinder = sizeDd844.maxCylinder - 1;
                up->track = sizeDd844.maxTrack - 1;
                up->sector = sizeDd844.maxSector - 1;
                }
            else
                {
                up->cylinder = sizeDd885.maxCylinder - 1;
                up->track = sizeDd885.maxTrack - 1;
                up->sector = sizeDd885.maxSector - 1;
                }
            fseek(fcb, dc7155Seek(up), SEEK_SET);
            memset(mySector, 0, SectorSize * 2);
            dc7155SectorWrite(up, fcb, mySector);

    /* initialize pack */

            if (DiskType == DiskType844)
                {
                up->cylinder = DsCylinder844;
                }
            else
                {
                up->cylinder = DsCylinder885;
                }
            /*
            **  Zero entire cylinder containing factory and utility data areas.
            */
            memset(mySector, 0, SectorSize * 2);
            for (up->track = 0; up->track < up->size.maxTrack; up->track++)
                {
                for (up->sector = 0; up->sector < up->size.maxSector; up->sector++)
                    {
                    fseek(fcb, dc7155Seek(up), SEEK_SET);
                    dc7155SectorWrite(up, fcb, mySector);
                    }
                }

            up->track = 0;
            up->sector = 0;

            /* initialise factory data */
            fseek(fcb, dc7155Seek(up), SEEK_SET);
            memset(mySector, 0, SectorSize * 2);
            mySector[0] = 8 << 8 | 8 << 4 | 5;  /* initialize serial */
            mySector[1] = 0 << 8 | (unitNo & 070) << 1 | unitNo & 07;
            mySector[2] = 4 << 8 | 8 << 4 | 0;  /* initialize date */
            mySector[3] = 1 << 8 | 1 << 4 | 6;
            dc7155SectorWrite(up, fcb, mySector);

            up->sector += 1;
            fseek(fcb, dc7155Seek(up), SEEK_SET);
            memset(mySector, 0, SectorSize * 2);
            dc7155SectorWrite(up, fcb, mySector);

            /* initialize utility map */

            up->sector += 1;
            fseek(fcb, dc7155Seek(up), SEEK_SET);
            dc7155SectorWrite(up, fcb, mySector);
            }
        }

    up->fcb = fcb;

    /*
    **  Print a friendly message.
    */
    printf("7155-%s unit initialised on channel %o, EQ %o unit %2o - %d cylinders, %s \n",
           (DiskType == DiskType844) ? "844" : "885",
           channelNo, eqNo, unitNo, up->size.maxCylinder - 1,
           (containerType == CtClassic) ? "classic" : "packed");
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 885 disk drive.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus dc7155Func(PpWord funcCode)
    {
    u8 unitNo;
    FILE *fcb;
    DiskParam *dp;
    UnitMux   *um;
    UnitParam *up;

    if ((funcCode & 0700) == Fc7155Deadstart )
        {
        activeDevice->selectedUnit = funcCode & 077;
        funcCode = Fc7155Deadstart;
        }
    if ((funcCode & 0700) == Fc7155AutoloadFromDisk )
        {
        activeDevice->selectedUnit = funcCode & 077;
        funcCode = Fc7155AutoloadFromDisk;
        }
    unitNo = activeDevice->selectedUnit;

    dp = (DiskParam *)activeDevice->context[7];
    um = (UnitMux *)activeDevice->context[unitNo >> 3];
    if (um != NULL)
        {
        up = (UnitParam *)um->units[unitNo & 07];
        }
    else
        {
        up = NULL;
        }
    if (up != NULL)
        {
        fcb = up->fcb;
        }
    else
        {
        fprintf(stderr, "Unit has no fcb - %02o\n",unitNo);
        return(FcDeclined);
        }

    switch (funcCode)
        {
    default:
        return(FcDeclined);

    case Fc7155Connect:
        /*
        **  Expect drive number.
        */
        activeDevice->recordLength = 1;     
        break;

    case Fc7155SeekFull:
        /*
        **  Expect drive number, cylinder, track and sector.
        */
        up->seekStatus = 0;
        up->interlace = 1;
        activeDevice->recordLength = 4;
        break;

    case Fc7155SeekHalf:
        /*
        **  Expect drive number, cylinder, track and sector.
        */
        up->seekStatus = 0;
        up->interlace = 2;
        activeDevice->recordLength = 4;
        break;

    case Fc7155Read:
        activeDevice->recordLength = SectorSize;
        break;

    case Fc7155ReadProtectedSector:
        if (up->diskType == DiskType844)
            {
            up->detailedStatus[5] |= 0006;
            }
         else
            {
            up->detailedStatus[4] |= 0206;
            }
        activeDevice->recordLength = SectorSize;
        break;

    case Fc7155WriteProtectedSector:
        if (up->diskType == DiskType844)
            {
            up->detailedStatus[5] |= 0006;
            }
         else
            {
            up->detailedStatus[4] |= 02000;
            }
        activeDevice->recordLength = SectorSize;
        break;

    case Fc7155Write:
    case Fc7155WriteLastSector:
    case Fc7155WriteVerify:
        activeDevice->recordLength = SectorSize;
        break;

    case Fc7155ReadCheckword:
        activeDevice->recordLength = 2;
        break;

    case Fc7155OpComplete:
    case Fc7155DropSeeks:
        up->seekStatus = 0;
        break;

    case Fc7155GeneralStatus:
        activeDevice->recordLength = 1;
        break;
 
    case Fc7155DetailedStatus:
    case Fc7155DetailedStatus2:
        up->detailedStatus[2] = (activeDevice->fcode << 4) & 07760;
        if (up->fcb == NULL)           /* not connected */
            {   
            up->detailedStatus[3] = 07476;
            up->detailedStatus[4] =     0;
            up->detailedStatus[5] =     0;
            up->detailedStatus[6] =   010;
            up->detailedStatus[7] =   040;
            up->detailedStatus[8] =     0; 
            }   
        else                                  /* connected */  
            {   
            up->detailedStatus[1] = (up->detailedStatus[1] & 07770) |
                                    up->seekStatus;
            up->detailedStatus[3] = (up->detailedStatus[3] & 07700) |
                                    (activeDevice->selectedUnit & 077);
            if (up->diskType == DiskType885)
                {
                up->detailedStatus[4] = (up->cylinder  >> 4) & 077;
                up->detailedStatus[5] = ((up->cylinder << 8) | up->track) & 07777;
                up->detailedStatus[6] = (up->sector    << 4) | 010;
                up->detailedStatus[7] = 037;
                up->detailedStatus[8] = 01640;
                }
            else
                { /* XXX - to be filled in */
                up->detailedStatus[4] = ((up->cylinder  << 4) & 07770)  | ((up->track >> 2) & 07);
                up->detailedStatus[5] = ((up->track     << 10) & 06000) | ((up->sector << 5) & 01740);
                up->detailedStatus[5] |= ((up->cylinder >> 8) & 01);
                up->detailedStatus[6] = 0;
                up->detailedStatus[7] = 0;
                up->detailedStatus[8] = 00740;
                }
            }   
        if (up->diskType == DiskType885)
            if (up->track & 1)
                {
                up->detailedStatus[9] |= 2;  /* odd track */
                }
            else
                {
                up->detailedStatus[9] &= 07775;
                }

        if (funcCode == Fc7155DetailedStatus)
            {
            activeDevice->recordLength = 12;
            }
        else
            {
            activeDevice->recordLength = 20;
            }
        break;
 
    case Fc7155AutoloadFromPP:
        activeDevice->recordLength = 16870;
        break;

    case Fc7155ReadUtilityMap:
        activeDevice->recordLength = SectorSize;
        break;

    case Fc7155ReadFactoryData:
        activeDevice->recordLength = SectorSize;
        break;

    case Fc7155DriveRelease:
        /*
        **  Silently ignore.
        */
        break;

    case Fc7155Deadstart:
        if (unitNo < 010)
            {
            up->cylinder = DsCylinder844;
            up->track = DsTrack844; 
            up->sector = DsSector844;
            }
        else
            {
            up->cylinder = DsCylinder885;
            up->track = DsTrack885; 
            up->sector = DsSector885;
            }
        fseek(fcb, dc7155Seek(up), SEEK_SET);
        activeDevice->recordLength = SectorSize;
        break;

    case Fc7155SetClearFlaw:
        activeDevice->recordLength = 1;
        break;

    case Fc7155FormatPack:
        activeDevice->recordLength = 18;
        break;

    case Fc7155ManipulateProcessor:
        activeDevice->recordLength = 5;
        break;

    case Fc7155ReturnCylAddr:
        activeDevice->recordLength = 1;
        break;

    case Fc7155IoLength:
    case Fc7155DisableReserve:
    case Fc7155Continue:
    case Fc7155OnSectorStatus:
    case Fc7155GapRead:
    case Fc7155GapWrite:
    case Fc7155GapWriteVerify:
    case Fc7155GapReadCheckword:
    case Fc7155AutoloadFromDisk:
        logError(LogErrorLocation, "ch %o, function %o not implemented\n",
                 activeChannel->id, funcCode);
        break;
        }

#if DEBUG
    fprintf(DC7155,"pp %o ch %o, dc7155 u%o - function %04o - %20s issued C%4d T%2d S%2d\n",
            activePpu->id, activeChannel->id, unitNo, funcCode, dc7155Func2String(funcCode),
            up->cylinder, up->track, up->sector);
            fflush(DC7155);
#endif

    activeDevice->fcode = funcCode;
    return(FcAccepted);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 885 disk drive.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dc7155Io(void)
    {
    u8 unitNo;
    FILE *fcb;
    DiskParam *dp;
    UnitParam *up;
    UnitMux   *um;
    u32 flaw;
    i32 pos;

    unitNo = activeDevice->selectedUnit;
    dp = (DiskParam *)activeDevice->context[7];
    um = (UnitMux *)activeDevice->context[unitNo >> 3];
    up = (um->units[unitNo & 07] != NULL) ? um->units[unitNo & 07] : NULL;
    if (up != NULL)
        {
        fcb = up->fcb;
        }
    else
        {
        fprintf(stderr, "7155 - I/O Unit has no fcb - %02o\n",unitNo);
        logError(LogErrorLocation, "7155 - I/O Unit has no fcb - %02o\n",unitNo);
        fcb = NULL;
        }

    switch (activeDevice->fcode)
        {
    default:
        logError(LogErrorLocation, "channel %02o - invalid function code: %4.4o\n",
                 activeChannel->id, (u32)activeDevice->fcode);
        break;

    case Fc7155Connect:
        if (activeChannel->full)
            {
            activeDevice->selectedUnit = activeChannel->data & 077;
            unitNo = activeDevice->selectedUnit;
            switch (unitNo >> 3)
                {
            case 0:
#if OPTION_10398
            case 1:
#endif
            case 4:
            case 5:
                um = (UnitMux *)activeDevice->context[unitNo >> 3];
                up = um->units[unitNo & 07];
                if (up != NULL)
                    {
                    if (up->fcb == NULL)
                        {
                        logError(LogErrorLocation, "channel %02o - invalid select: %4.4o\n",
                                 activeChannel->id, (u32)activeDevice->fcode);
                        }
                    }
                break;

            default:
                logError(LogErrorLocation, "channel %02o - invalid unit: %02o\n",
                     activeChannel->id, (activeChannel->data & 077));
                }

            activeChannel->full = FALSE;
            activeChannel->ioDevice = NULL;
            }
        break;

    case Fc7155SeekFull:
    case Fc7155SeekHalf:
        if (activeChannel->full)
            {
            switch (activeDevice->recordLength--)
                {
            case 4:
                activeDevice->selectedUnit = activeChannel->data & 077;
                unitNo = activeDevice->selectedUnit;
                dp = activeDevice->context[7];
                um = (UnitMux *)activeDevice->context[unitNo >> 3];
                up = um->units[unitNo & 07];
                if (up != NULL)
                    {
                    if (up->fcb == NULL)
                        {
                        logError(LogErrorLocation, "channel %02o - invalid select: %4.4o\n",
                             activeChannel->id, (u32)activeDevice->fcode);
                        }
                    else
                        {
                        fcb = up->fcb;
                        }
                    }
                else
                    {
                    fcb = NULL;
                    }
                break;

            case 3:
                if (fcb != NULL)
                    up->cylinder = activeChannel->data;
                break;

            case 2:
                if (fcb != NULL)
                    up->track = activeChannel->data;
                break;

            case 1:
                if (fcb != NULL)
                    up->sector = activeChannel->data;
                activeChannel->ioDevice = NULL;
                fseek(fcb, dc7155Seek(up), SEEK_SET);
                break;

            default:
                activeDevice->recordLength = 0;
                break;
                }

            activeChannel->full = FALSE;
            }
        break;

    case Fc7155Deadstart:
        if (!activeChannel->full)
            {
            if (activeDevice->recordLength == SectorSize)
                {
            /* the first word in the sector contains data length */
                activeDevice->recordLength = up->read(up, fcb);
#if DEBUG
                fprintf(DC7155,"deadstart sect size = %04o %03d\n",
                         activeDevice->recordLength,
                         activeDevice->recordLength);
                fflush(DC7155);
#endif
                activeChannel->data = activeDevice->recordLength;
                activeDevice->recordLength = SectorSize - 1;
                }
            else
                {
                activeChannel->data = up->read(up, fcb);
#if DEBUG
                fprintf(DC7155,"deadstart byte %04o = %04o\n",
                         321 - activeDevice->recordLength,
                         activeChannel->data);
                fflush(DC7155);
#endif
                }
            activeChannel->full = TRUE;
            
            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;

    case Fc7155Read:
    case Fc7155ReadProtectedSector:
        if (!activeChannel->full)
            {
            activeChannel->data = up->read(up, fcb);
            activeChannel->full = TRUE;

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                pos = dc7155SeekNextSector(up);
                if (pos >= 0)
                    {
                    fseek(fcb, pos, SEEK_SET);
                    }
                }
            }
        break;

    case Fc7155WriteProtectedSector:
        if (activeChannel->full)
            {
            up->write(up, fcb, activeChannel->data);
            activeChannel->full = FALSE;
#if DEBUG
            fprintf(DC7155,"protwrite byte %04o = %04o\n",
                    322 - activeDevice->recordLength,
                    activeChannel->data);
            fflush(DC7155);
#endif

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->active = FALSE;
                activeChannel->ioDevice = NULL;
                pos = dc7155SeekNextSector(up);
                if (pos >= 0)
                    {
                    fseek(fcb, pos, SEEK_SET);
                    }
                }
            }
        break;

    case Fc7155Write:
    case Fc7155WriteLastSector:
    case Fc7155WriteVerify:
        if (activeChannel->full)
            {
            up->write(up, fcb, activeChannel->data);
            activeChannel->full = FALSE;

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->ioDevice = NULL;
                pos = dc7155SeekNextSector(up);
                if (pos >= 0)
                    {
                    fseek(fcb, pos, SEEK_SET);
                    }
                }
            }
        break;

    case Fc7155GeneralStatus:
        if (!activeChannel->full)
            {
            activeChannel->data = activeDevice->status;
            activeChannel->full = TRUE;

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;

    case Fc7155ReadCheckword:
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
    case Fc7155DetailedStatus:
        if (!activeChannel->full)
            {
            activeChannel->data = up->detailedStatus[12 - activeDevice->recordLength];
            activeChannel->full = TRUE;
#if DEBUG
            fprintf(DC7155, "dc7155 detail status %2d = %04o\n",
               13 - activeDevice->recordLength,activeChannel->data);
            fflush(DC7155);
#endif

            if (--activeDevice->recordLength == 0)
                {
#if DEBUG
                fprintf(DC7155, "\n");
                fflush(DC7155);
#endif
                activeChannel->discAfterInput = TRUE;
                if (up->diskType == DiskType844)
                    {
                    up->detailedStatus[6] &= 07771;
                    }
                 else
                    {
                    up->detailedStatus[5] &= 05000;
                    }
                }
            }
        break;

    case Fc7155DetailedStatus2:
        if (!activeChannel->full)
            {
            activeChannel->data = up->detailedStatus[20 - activeDevice->recordLength];
            activeChannel->full = TRUE;
#if DEBUG
            fprintf(DC7155, "dc7155 detail status2 %2d = %04o\n",
               21 - activeDevice->recordLength,activeChannel->data);
            fflush(DC7155);
#endif

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                if (up->diskType == DiskType844)
                    {
                    up->detailedStatus[6] &= 07771;
                    }
                 else
                    {
                    up->detailedStatus[5] &= 05000;
                    }
#if DEBUG
                fprintf(DC7155, "\n");
                fflush(DC7155);
#endif
                }
            }
        break;

    case Fc7155ReadFactoryData:
    case Fc7155ReadUtilityMap:
        if (!activeChannel->full)
            {
            activeChannel->data = up->read(up, fcb);
            activeChannel->full = TRUE;

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;

    case Fc7155AutoloadFromPP:
        if (activeChannel->full)
            {
            activeChannel->full = FALSE;
            }
        break;

    case Fc7155SetClearFlaw:
        if (activeChannel->full)
            {
            flaw = (up->cylinder << 12) | (up->track <<6) | up->sector;
            SetClearFlaw(up, flaw, activeChannel->data);
            activeChannel->full = FALSE;
            if (--activeDevice->recordLength == 0)
                {
                }
            }
        break;

    case Fc7155FormatPack:
        if (activeChannel->full)
            {
            activeChannel->full = FALSE;
#if DEBUG
            fprintf(DC7155, "dc7155 format param %d = %04o\n",
               18 - activeDevice->recordLength,activeChannel->data);
            fflush(DC7155);
#endif
            if (--activeDevice->recordLength == 0)
                {
                }
            }
        break;

    case Fc7155ManipulateProcessor:
        if (activeChannel->full)
            {
            activeChannel->full = FALSE;
#if DEBUG
            fprintf(DC7155, "dc7155 Manipulate Processor %d = %04o\n",
               5 - activeDevice->recordLength, activeChannel->data);
            fflush(DC7155);
#endif
            if (--activeDevice->recordLength == 0)
                {
                }
            }
        break;

    case Fc7155ReturnCylAddr:
        if (!activeChannel->full)
            {
            activeChannel->full = TRUE;
            activeChannel->data = up->cylinder;
            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;

    case Fc7155IoLength:
    case Fc7155OpComplete:
    case Fc7155DisableReserve:
    case Fc7155Continue:
    case Fc7155DropSeeks:
    case Fc7155OnSectorStatus:
    case Fc7155DriveRelease:
    case Fc7155GapRead:
    case Fc7155GapWrite:
    case Fc7155GapWriteVerify:
    case Fc7155GapReadCheckword:
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
static void dc7155Activate(void)
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
static void dc7155Disconnect(void)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        Work out seek offset.
**  
**  Parameters:     Name        Description.
**                  up          Unit parameters (context).
**          
**  Returns:        Byte offset (not word!) or -1 when seek target
**                  is invalid.
**          
**------------------------------------------------------------------------*/
static i32 dc7155Seek(UnitParam *up)
    {           
    i32 result;     
    DiskParam *dp;
                    
    if (up == NULL)
        return(-1);
    dp = up->controller;
    dp->bufPtr = NULL;
                
    activeDevice->status = 0;
    
    if (up->cylinder >= up->size.maxCylinder)
        {
        up->seekStatus = seekCylinderError;
        logError(LogErrorLocation, "ch %o, cylinder %d invalid\n", activeChannel->id, up->cylinder);
        return(-1);
        }

    if (up->track >= up->size.maxTrack)
        {
        up->seekStatus = seekTrackError;
        logError(LogErrorLocation, "ch %o, track %d invalid\n", activeChannel->id , up->track);
        return(-1); 
        }   
            
    if (up->sector >= up->size.maxSector)
        {
        up->seekStatus = seekSectorError;
        logError(LogErrorLocation, "ch %o, sector %d invalid\n", activeChannel->id, up->sector);     
        return(-1); 
        }
                
    up->seekStatus = 0;
    result  = up->cylinder * up->size.maxTrack * up->size.maxSector;
    result += up->track * up->size.maxSector;
    result += up->sector;
    result *= up->sectorSize;
        
    return(result);
    }   

/*--------------------------------------------------------------------------
**  Purpose:        Position to next sector taking into account interlace.
**
**  Parameters:     Name        Description.
**                  up          Disk parameters (context).
**
**  Returns:        Byte offset (not word!) or -1 when seek target
**                  is invalid.
**
**------------------------------------------------------------------------*/
static i32 dc7155SeekNextSector(UnitParam *up)
    {
    i32 pos;

    up->sector += up->interlace;

    if (up->interlace == 1)
        {
        if (up->sector == up->size.maxSector)
            {
            up->sector = 0;
            up->track += 1;
            }
        }
    else
        {
        if (up->sector == up->size.maxSector)
            {
            up->sector = 0;     
            up->track += 1;     
            if (up->track == up->size.maxTrack)
                {   
                up->track = 0; 
                up->sector = 1;
                }
            }
        else if (up->sector == up->size.maxSector + 1)
            {
            up->sector = 1;
            up->track += 1;
            }
        }
            
    pos = dc7155Seek(up); 
    if (pos < 0)
        {
        up->nextSeekStatus = up->seekStatus;   /* keep it for later */
        up->seekStatus = 0;                    /* do not not know when yet */
        }
    return(pos); 
    }
/*--------------------------------------------------------------------------
**  Purpose:        Perform a 12 bit PP word read from a classic disk container.
**      
**  Parameters:     Name        Description.  
**                  up          Unit parameters (context).
**                  fcb         File control block.
**
**  Returns:        PP word read.
**              
**------------------------------------------------------------------------*/
static PpWord dc7155ReadClassic(UnitParam *up, FILE *fcb)
    {           
    DiskParam *dp = up->controller;
    
    /*      
    **  Read an entire sector if the current buffer is empty.
    */      
    if (dp->bufPtr == NULL)
        {   
        dp->bufPtr = dp->buffer;
        fread(dp->buffer, 1, up->sectorSize, fcb);
        }   
    
    /*    **  Fail gracefully if we read too much data.
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
**                  up          Unit parameters (context).
**                  fcb         File control block.
**                  data        PP word to be written.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dc7155WriteClassic(UnitParam *up, FILE *fcb, PpWord data)
    { 
    DiskParam *dp = up->controller;

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
        }

    *dp->bufPtr++ = data;

    /*
    **  Write the data if we got a full sector.
    */
    if (dp->bufPtr == dp->buffer + SectorSize)
        {
        fwrite(dp->buffer, 1, up->sectorSize, fcb);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform a 12 bit PP word read from a packed disk container.
**
**  Parameters:     Name        Description.
**                  up          Unit parameters (context).
**                  fcb         File control block.
**
**  Returns:        PP word read.
**
**------------------------------------------------------------------------*/
static PpWord dc7155ReadPacked(UnitParam *up, FILE *fcb)
    {
    DiskParam *dp = up->controller;
    u16 byteCount;
    static u8 sector[512];
    u8 *sp;
    PpWord *pp;
    
    /* 
    **  Read an entire sector if the current buffer is empty.
    */  
    if (dp->bufPtr == NULL)
        {
        dp->bufPtr = dp->buffer;
        fread(sector, 1, up->sectorSize, fcb);

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
**                  up          Unit parameters (context).
**                  fcb         File control block.
**                  data        PP word to be written.
**      
**  Returns:        Nothing.
**  
**------------------------------------------------------------------------*/
static void dc7155WritePacked(UnitParam *up, FILE *fcb, PpWord data)
    {
    DiskParam *dp = up->controller;
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
        **  Write the sector.
        */
        fwrite(sector, 1, up->sectorSize, fcb);
        }
    }
    
/*--------------------------------------------------------------------------
**  Purpose:        Perform a sector read from a disk container.
**      
**  Parameters:     Name        Description.
**                  up          Unit parameters (context).
**                  fcb         File control block.
**                  sector      Pointer to sector to read into.
**      
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dc7155SectorRead(UnitParam *up, FILE *fcb, PpWord *sector)
    {       
    u16 byteCount;  
            
    for (byteCount = SectorSize; byteCount > 0; byteCount--)
        { 
        *sector++ = up->read(up, fcb);
        } 
    }   
        
/*--------------------------------------------------------------------------
**  Purpose:        Perform a sector write to a disk container.
**
**  Parameters:     Name        Description.
**                  up          Unit parameters (context).
**                  fcb         File control block.
**                  sector      Pointer to sector to write.
**                  
**  Returns:        Nothing.    
**      
**------------------------------------------------------------------------*/
static void dc7155SectorWrite(UnitParam *up, FILE *fcb, PpWord *sector)
    {
    u16 byteCount;
    
    for (byteCount = SectorSize; byteCount > 0; byteCount--)
        {   
        up->write(up, fcb, *sector++);
        } 
    }   

/*--------------------------------------------------------------------------
**  Purpose:        Set or clear flaw entries in the device utility map
**
**  Parameters:     Name        Description.
**                  dp          Disk parameter block
**                  flaw        32 bit flaw entry
**                  flawState   set ot clear command
**
**  Returns:        none
**
**------------------------------------------------------------------------*/
static void SetClearFlaw(UnitParam *up, u32 flaw, PpWord flawState)
{
    u8 unitNo;
    FILE *fcb;
    int index;
    PpWord word0, word1;
    DiskParam *dp;

    unitNo = activeDevice->selectedUnit;
    fcb = up->fcb;
    dp = up->controller;

    up->cylinder = up->maxCyl;
    up->track = 0;
    up->sector = 0;

    index = 23 - (flawState & 01);
    word0 = (PpWord)(flaw >> 12) | (1 << 11) - (flawState & 01);
    word1 = (PpWord)flaw & 07777;
    fseek(fcb, dc7155Seek(up), SEEK_SET);
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
            fprintf(DC7155,"dc7155 set flaw , sector = %08o %04o %04o - %04o at %d\n",
                         flaw, word0, word1, flawState, index);
            fflush(DC7155);
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
            fprintf(DC7155,"dc7155 clear flaw , sector = %08o %04o %04o - %04o at %d\n",
                         flaw, word0, word1, flawState, index);
            fflush(DC7155);
#endif
            mySector[index] = 0;
            index += 1;
            mySector[index] = 0;
            }
        }
    fseek(fcb, dc7155Seek(up), SEEK_SET);
    dc7155SectorWrite(up, fcb, mySector);
}

/*--------------------------------------------------------------------------
**  Purpose:        fill a buffer with a test patern
**
**  Parameters:     Name        Description.
**                  loopcount   loop counter
**
**  Returns:        Byte offset (not word!)
**
**------------------------------------------------------------------------*/
void make_test_data(unsigned int loopcount)
{                    /* data selector, from zero for first round */
    register int i, j, k, d;
    
    if (loopcount < 2)  
        {
        for (i = j = 0; j < 12; j++)
            for (k = 0; k < 12; k++)
                {  
                d=0xfff; d ^= 1<<j; d ^= 1<<k;
                testdata[i++]=d;
                }
        for (;;)
            for (j = 0; j < 12; j++)
                {
                d=0xfff; d ^= 1 << j;
                testdata[i++]=d;
                if (i >= SectorSize)
                   goto done1;
                }
        done1:
        testdata[SectorSize-1] = 0;                 /* critical position */
        for (j = 0; j < SectorSize; j++)
            mySector[j] = testdata[j];
        }
    else if (loopcount < 4)
        {
        for (i = j =0; j < 12; j++)
            for (k = 0; k < 12; k++)
                {
                d = 0;
                d ^= 1 << j;
                d ^= 1 << k;
                testdata[i++] = d;
                }
        for (;;)
            for (j = 0; j < 12; j++)
                {
                d=0; d ^= 1 << j;
                testdata[i++] = d;
                if (i >= SectorSize)
                    goto done2;
                }
        done2:
        testdata[SectorSize-1] = 0;                 /* critical position */
        for (j=0; j<SectorSize; j++)
            mySector[j]=testdata[j];
        }
    else
        for (i = 0; i < SectorSize; i++)
            mySector[i] = testdata[i] = ranf();
}

unsigned short ranf()
{   
    myRandom *= 65539;
    return(myRandom >> 10);
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
static char *dc7155Func2String(PpWord funcCode) 
    {
#if DEBUG
    switch(funcCode)
        {
    case Fc7155Connect                :  return "Connect";
    case Fc7155SeekFull               :  return "SeekFull";
    case Fc7155SeekHalf               :  return "SeekHalf";
    case Fc7155IoLength               :  return "IoLength";
    case Fc7155Read                   :  return "Read";
    case Fc7155Write                  :  return "Write";
    case Fc7155WriteVerify            :  return "WriteVerify";
    case Fc7155ReadCheckword          :  return "ReadCheckword";
    case Fc7155OpComplete             :  return "OpComplete";
    case Fc7155DisableReserve         :  return "DisableReserve";
    case Fc7155GeneralStatus          :  return "GeneralStatus";
    case Fc7155DetailedStatus         :  return "DetailedStatus";
    case Fc7155Continue               :  return "Continue";
    case Fc7155DropSeeks              :  return "DropSeeks";
    case Fc7155FormatPack             :  return "FormatPack";
    case Fc7155OnSectorStatus         :  return "OnSectorStatus";
    case Fc7155DriveRelease           :  return "DriveRelease";
    case Fc7155ReturnCylAddr          :  return "ReturnCylAddr";
    case Fc7155SetClearFlaw           :  return "SetClearFlaw";
    case Fc7155DetailedStatus2        :  return "DetailedStatus2";
    case Fc7155GapRead                :  return "GapRead";
    case Fc7155GapWrite               :  return "GapWrite";
    case Fc7155GapWriteVerify         :  return "GapWriteVerify";
    case Fc7155GapReadCheckword       :  return "GapReadCheckword";
    case Fc7155ReadFactoryData        :  return "ReadFactoryData";
    case Fc7155ReadUtilityMap         :  return "ReadUtilityMap";
    case Fc7155ReadProtectedSector    :  return "ReadProtectedSector";
    case Fc7155WriteLastSector        :  return "WriteLastSector";
    case Fc7155WriteVerifyLastSector  :  return "WriteVerifyLastSector";
    case Fc7155WriteProtectedSector   :  return "WriteProtectedSector";
    case Fc7155ManipulateProcessor    :  return "ManipulateProcessor";
    case Fc7155AutoloadFromDisk       :  return "AutoloadFromDisk";
    case Fc7155Deadstart              :  return "Deadstart";
    case Fc7155AutoloadFromPP         :  return "AutoloadFromPP";
        }
#endif
    return "UNKNOWN";
    }

/*---------------------------  End Of File  ------------------------------*/
