/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: disk.c
**
**  Description:
**      Perform simulation of CDC 844 disk drives.
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

**  CDC 844 disk drive function and status codes.
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
**  0414    Start memory load (hardware function)   
*/
#define Fc844Connect            00000
#define Fc844SeekFull           00001
#define Fc844SeekHalf           00002
#define Fc844IoLength           00003
#define Fc844Read               00004
#define Fc844Write              00005
#define Fc844WriteVerify        00006
#define Fc844ReadCheckword      00007
#define Fc844OpComplete         00010
#define Fc844DisableReserve     00011
#define Fc844GeneralStatus      00012
#define Fc844DetailedStatus     00013
#define Fc844Continue           00014
#define Fc844DropSeeks          00015
#define Fc844FormatPack         00016
#define Fc844OnSectorStatus     00017
#define Fc844DriveRelease       00020
#define Fc844ReturnCylAddr      00021
#define Fc844SetClearFlaw       00022
#define Fc844DetailedStatus2    00023
#define Fc844GapRead            00024
#define Fc844GapWrite           00025
#define Fc844GapWriteVerify     00026
#define Fc844GapReadCheckword   00027
#define Fc844ReadFactoryData    00030
#define Fc844ReadUtilityMap     00031
#define Fc844ReadFlawedSector   00034
#define Fc844WriteLastSector    00035
#define Fc844WriteFlawedSector  00037
#define Fc844Deadstart          00300
#define Fc844StartMemLoad       00414

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
#define St844Busy               00002

/*
**  Physical dimensions of disk.
**  322 12-bit bytes per sector (64 cm wds + 2 bytes).  1st
**      byte is unused. 2nd byte contains byte count of data.                                              
**   24 sectors/track                                     
**   19 tracks/cylinder                                    
**  411 cylinders/unit on 844-21
**  823 cylinders/unit on 844-41
**  Note that the last cylinder is reserved for "factory data".
*/
//#define MaxCylinders            823
#define MaxCylinders            1682
#define MaxTracks               19
#define MaxSectors              24
#define SectorSize              322
#define SectorBytes             512

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
    u8          *bp;
    u32         sector;
    u32         track;
    u32         cylinder;
    u8          interlace;
    u8          sec[SectorBytes];
    bool        seekNeeded;
    bool        even;
    } DiskParam;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus dd844Func(PpWord funcCode);
static void dd844Io(void);
static void dd844Activate(void);
static void dd844Disconnect(void);
static void dd844Seek(FILE *fcb, DiskParam *dp);
static void dd844SeekNextSector(DiskParam *dp);
static void dd844Dump(PpWord data);
static void dd844Flush(void);

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
static PpWord dmp[8];

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialise 844 disk drive.
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
void dd844Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    DevSlot *dp;
    FILE *fcb;
    char fname[80];
    DiskParam *dskp;

    (void)eqNo;
    (void)unitNo;
    (void)deviceName;

    dp = channelAttach(channelNo, DtDd844);
    dp->activate = dd844Activate;
    dp->disconnect = dd844Disconnect;
    dp->func = dd844Func;
    dp->io = dd844Io;
    dp->selectedUnit = unitNo;

    dskp = (DiskParam *) calloc(1, sizeof(DiskParam));
    dp->context[unitNo] = dskp;
    if (dp->context[unitNo] == NULL)
        {
        fprintf(stderr, "failed to allocate dd844 context block\n");
        exit(1);
        }
    dskp->seekNeeded = TRUE;
    if (deviceName == NULL)
        {
        sprintf(fname, "DD844_C%02ou%1o", channelNo, unitNo);
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
        }

    dp->fcb[unitNo] = fcb;

    /*
    **  Print a friendly message.
    */
    printf("DD844 initialised on channel %o unit %o\n", channelNo, unitNo);
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
static FcStatus dd844Func(PpWord funcCode)
    {
    u8 unitNo;
    FILE *fcb;
    DiskParam *dp;
    PpWord dsSecSize;

    if ((funcCode & 0700) == Fc844Deadstart )
        {
        activeDevice->selectedUnit = funcCode & 077;
        funcCode = Fc844Deadstart;
        }

    unitNo = activeDevice->selectedUnit;
    fcb = activeDevice->fcb[unitNo];
    dp = (DiskParam *)activeDevice->context[unitNo];

    switch (funcCode)
        {
    default:
        return(FcDeclined);

    case Fc844Connect:
        /*
        **  Expect drive number.
        */
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 1;     
        break;

    case Fc844SeekFull:
        /*
        **  Expect drive number, cylinder, track and sector.
        */
        dp->interlace = 1;
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 4;
        break;

    case Fc844SeekHalf:
        /*
        **  Expect drive number, cylinder, track and sector.
        */
        dp->interlace = 2;
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 4;
        break;

    case Fc844Read:
    case Fc844ReadFlawedSector:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = SectorSize;
//        printf (" reading u%d c%d t%d s%d\n", unitNo, dp->cylinder, dp->track, dp->sector);
        dd844Seek(fcb, dp);
        fread(dp->sec, 1, SectorBytes, fcb);
        dp->bp = dp->sec;
        dp->even = TRUE;
        break;

    case Fc844Write:
    case Fc844WriteFlawedSector:
    case Fc844WriteLastSector:
    case Fc844WriteVerify:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = SectorSize;
//        printf (" writing u%d c%d t%d s%d\n", unitNo, dp->cylinder, dp->track, dp->sector);
        dd844Seek(fcb, dp);
        dp->bp = dp->sec;
        dp->even = TRUE;
        break;

    case Fc844ReadCheckword:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 2;
        break;

    case Fc844OpComplete:
    case Fc844DropSeeks:
        break;

    case Fc844GeneralStatus:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 1;
        break;
 
    case Fc844DetailedStatus:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 12;
        break;
 
    case Fc844DetailedStatus2:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 20;
        break;
 
    case Fc844StartMemLoad:
        activeDevice->fcode = funcCode;
        break;

    case Fc844ReadUtilityMap:
    case Fc844ReadFactoryData:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = SectorSize;
        break;

    case Fc844DriveRelease:
        /*
        **  Silently ignore.
        */
        break;

    case Fc844Deadstart:
        #if 1
            dp->cylinder = 0632; /* 844 */
            dp->track = 0;
            dp->sector = 3;
        #else
            dp->cylinder = 01057;
            dp->track = 0;
            dp->sector = 2;
        #endif
        dp->seekNeeded = TRUE;
        dd844Seek(fcb, dp);
        fread(dp->sec, 1, SectorBytes, fcb);
        dp->bp = dp->sec;
        dp->even = TRUE;
        activeDevice->fcode = funcCode;
        /* the first word in the sector contains data length */
        dsSecSize = (dp->sec[0] << 4) + (dp->sec[1] >> 4);
        fprintf(devF,"deadstart sect size = %04o\n", dsSecSize);
        activeDevice->recordLength = dsSecSize;
        break;

    case Fc844SetClearFlaw:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 1;
        break;

    case Fc844FormatPack:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 7;
        break;

    case Fc844IoLength:
    case Fc844DisableReserve:
    case Fc844Continue:
    case Fc844OnSectorStatus:
    case Fc844ReturnCylAddr:
    case Fc844GapRead:
    case Fc844GapWrite:
    case Fc844GapWriteVerify:
    case Fc844GapReadCheckword:
        ppAbort((stderr, "ch %o, function %o not implemented\n", activeChannel->id, funcCode));
        break;
        }

//    fprintf(devF, "pp %o ch %o, function %04o issued\n", activePpu->id, activeChannel->id, funcCode);
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
static void dd844Io(void)
    {
    u8 unitNo;
    FILE *fcb;
    DiskParam *dp;

    unitNo = activeDevice->selectedUnit;
    fcb = activeDevice->fcb[unitNo];
    dp = (DiskParam *)activeDevice->context[unitNo];

    switch (activeDevice->fcode)
        {
    default:
        ppAbort((stderr, "channel %02o - invalid function code: %4.4o\n", activeChannel->id, (u32)activeDevice->fcode));
        break;

    case Fc844Connect:
        if (activeChannel->full)
            {
            activeDevice->selectedUnit = activeChannel->data & 07;
            if (activeDevice->fcb[activeDevice->selectedUnit] == NULL)
                {
                ppAbort((stderr, "channel %02o - invalid select: %4.4o", activeChannel->id, (u32)activeDevice->fcode));
                }

            activeChannel->full = FALSE;
            activeChannel->ioDevice = NULL;
            }
        break;

    case Fc844SeekFull:
    case Fc844SeekHalf:
        if (activeChannel->full)
            {
            switch (activeDevice->recordLength--)
                {
            case 4:
                activeDevice->selectedUnit = activeChannel->data & 07;
                unitNo = activeDevice->selectedUnit;
                if (activeDevice->fcb[activeDevice->selectedUnit] == NULL)
                    {
                    ppAbort((stderr, "channel %02o - invalid select: %4.4o", activeChannel->id, (u32)activeDevice->fcode));
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
                activeChannel->ioDevice = NULL;
                dd844Seek(fcb, dp);
                break;

            default:
                activeDevice->recordLength = 0;
                break;
                }

            activeChannel->full = FALSE;
            }
        break;

    case Fc844Deadstart:
    case Fc844Read:
    case Fc844ReadFlawedSector:
        if (!activeChannel->full)
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
            activeChannel->full = TRUE;

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                dd844SeekNextSector(dp);
                }
            }
        break;

    case Fc844Write:
    case Fc844WriteFlawedSector:
    case Fc844WriteLastSector:
    case Fc844WriteVerify:
        if (activeChannel->full)
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
            activeChannel->full = FALSE;

            if (--activeDevice->recordLength == 0)
                {
                activeChannel->ioDevice = NULL;
                fwrite(dp->sec, 1, SectorBytes, fcb);
                dd844SeekNextSector(dp);
                }
            }
        break;

    case Fc844ReadCheckword:
    case Fc844GeneralStatus:
    case Fc844DetailedStatus:
    case Fc844DetailedStatus2:
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

    case Fc844ReadFactoryData:
    case Fc844ReadUtilityMap:
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

    case Fc844StartMemLoad:
    case Fc844SetClearFlaw:
    case Fc844FormatPack:
    case Fc844IoLength:
    case Fc844OpComplete:
    case Fc844DisableReserve:
    case Fc844Continue:
    case Fc844DropSeeks:
    case Fc844OnSectorStatus:
    case Fc844DriveRelease:
    case Fc844ReturnCylAddr:
    case Fc844GapRead:
    case Fc844GapWrite:
    case Fc844GapWriteVerify:
    case Fc844GapReadCheckword:
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
static void dd844Activate(void)
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
static void dd844Disconnect(void)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        Seek to the currently set cyl/track/sector
**
**  Parameters:     Name        Description.
**                  fcb         pointer to FILE struct.
**                  dp          pointer to DiskParam struct.
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
static void dd844Seek(FILE *fcb, DiskParam *dp)
    {
    u32 result;

    if (dp->cylinder >= MaxCylinders)
        {
        ppAbort((stderr, "ch %o, cylinder %o invalid\n", activeChannel->id, dp->cylinder));
        }

    if (dp->track >= MaxTracks)
        {
        ppAbort((stderr, "ch %o, track %o invalid\n", activeChannel->id, dp->track));
        }

    if (dp->sector >= MaxSectors)
        {
        ppAbort((stderr, "ch %o, sector %o invalid\n", activeChannel->id, dp->sector));
        }

    if (dp->seekNeeded)
        {
        result  = dp->cylinder * MaxTracks * MaxSectors;
        result += dp->track * MaxSectors;
        result += dp->sector;
        result *= SectorBytes;
        fseek(fcb, result, SEEK_SET);
        dp->seekNeeded = FALSE;
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
static void dd844SeekNextSector(DiskParam *dp)
    {
    dp->sector += dp->interlace;

    if (dp->interlace == 1)
        {
        if (dp->sector == MaxSectors)
            {
            dp->sector = 0;
            dp->track += 1;
            }
        }
    else
        {
        dp->seekNeeded = TRUE;
        if (dp->sector == MaxSectors)
            {
            dp->sector = 0;
            dp->track += 1;
            if (dp->track == MaxTracks)
                {
                dp->track = 0;
                dp->sector = 1;
                }
            }
        else if (dp->sector == MaxSectors + 1)
            {
            dp->sector = 1;
            dp->track += 1;
            }
        }
    }

/*---------------------------  End Of File  ------------------------------*/
