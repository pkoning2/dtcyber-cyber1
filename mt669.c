/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter and Gerard van der Grinten
**  (see license.txt)
**
**  Name: mt669.c
**
**  Description:
**      Perform simulation of CDC 6600 669 tape drives attached to a
**      7021-21 magnetic tape controller.
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
#ifndef WIN32
#include <unistd.h>
#endif
#include <errno.h>
#include <string.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define ScopeSupport 0

// #define REW_LDPT 0
/*
**  MTS tape function and status codes:
**  ==================================
**  
**  0001   release unit
**  0002   clear all reserves
**  0003   clear opposite reserve
**  0005   select opposite parity mode
**  0105   select opposite density
**  0006   select normal read clip
**  0106   select high read clip
**  0206   select low read clip
**  0306   select hyper read clip
**  0007   nominal read sprocket delay
**  0107   increase read sprocket delay
**  0207   decrease read sprocket delay
**  0010   rewind
**  0110   rewind/unload
**  0011   stop motion
**  0012   general status
**  0112   detailed status
**  0212   cumulative status
**  0312   units ready status
**  0013   forespace
**  0113   backspace
**  0213   long forespace
**  0313   long backspace
**  0014   controlled forespace
**  0114   controlled backspace
**  0015   search tapemark forward
**  0115   search tapemark backward
**  0016   erase reposition
**  0116   erase reposition to erase
**  0017   write reposition
**  0117   write reposition to erase
**  0020   connect unit (002x, x = unit no.)
**  0030   format unit
**  0131   code translation table 1 to memory
**  0231   code translation table 2 to memory
**  0331   code translation table 3 to memory
**  0132   load read ram
**  0232   load write ram
**  0332   load read/write ram
**  0133   copy read ram
**  0233   copy write ram
**  0034   format tcu status
**  0035   copy tcu status
**  0036   send tcu command
**  0040   read forward
**  0140   read backward
**  0150   write odd length (NOS 1.2)
**  0340   read backward with odd length parity
**  0041   reread forward
**  0141   reread backward
**  0341   reread backward with odd length par.
**  0042   repeat read
**  0050   write
**  0150   write odd length (for NOS 1.2)
**  0250   write odd length
**  0051   write tapemark
**  0052   erase
**  0152   erase to end of tape
**  0414   Master clear and restart (hardware function)
**  
**  e26u is a complex deadstart function.
**  
**  'e' is the target equipment. For MTS always 0
**  'u' is the target unit.
**  7260 connects to EQ07-UN00, rewinds and initiates a read forward.
**  
*/
#define Fc669ClearUnit          00000
#define Fc669Release            00001
#define Fc669ClearReserve       00002
#define Fc669SetReadClipNorm    00006
#define Fc669SetReadClipHigh    00106
#define Fc669SetReadClipLow     00206
#define Fc669Rewind             00010
#define Fc669RewindUnload       00110
#define Fc669StopMotion         00011
#define Fc669GeneralStatus      00012
#define Fc669DetailedStatus     00112
#define Fc669CumulativeStatus   00212
#define Fc669ReadyStatus        00312
#define Fc669Forespace          00013
#define Fc669Backspace          00113
#define Fc669SearchTapeMarkF    00015
#define Fc669SearchTapeMarkB    00115
#define Fc669Reposition         00017
#define Fc669Connect            00020
#define Fc669FormatUnit         00030
#define Fc669LoadConversion1    00131
#define Fc669LoadConversion2    00231
#define Fc669LoadConversion3    00331
#define Fc669ReadFwd            00040
#define Fc669ReadBkw            00140
#define Fc669Write              00050
#define Fc669WriteOdd12         00150
#define Fc669WriteOdd           00250
#define Fc669WriteEOF           00051
#define Fc669Erase              00052
#define Fc669EraseToEOT         00152
#define Fc669ConnectRewindRead  00260
#define Fc669MasterClear        00414
#define Fc6681Read2EOR          01400

/*
**  General status reply:
**  ====================
**  
**  bit    11         alert
**  bit    10         status from coupler (see bits 0 - 3)
**  bit    9          no unit is currently connected
**  bit    8          noise record read
**  bit    7          write ring in unit
**  bit    6          unit type (0 = 7 track, 1 = 9 track)
**  bit    5          odd count
**  bit    4          tapemark
**  
**  The following are for coupler status *gscs* = 0.
**  
**  bit    3          end of tape
**  bit    2          tape at loadpoint
**  bit    1          unit busy (tape in motion)
**  bit    0          unit loaded and ready
**  
**  The following are for coupler status *gscs* = 1.
**  
**  bit    3          controller memory parity error
**  bit    2          local autoload is in progress
**  bit    1          remote autoload is in progress
**  bit    0          deadman timeout occurred
**  
**  Detailed status reply:
**  =====================
**
**  8 words, most of which report errors if non-zeroes except:
**
**  Word 1 (offset 0)
**  
**  bit    5-0        error code (for details see below).
**  
**  Word 5 (offset 4)
**  
**  bit    11         forward tape motion if zero, backward if set
**  bit    10-8       tape speed: 1=100, 2=150, 4=200 ips
**  bit    7-6        tape density: 0=200 or 556, 1=800, 2=1600 cpi
**  bit    5          access error
**  bit    4          unit write and erase currents are on
**  bit    3-0        unit cable connector address in tape control unit
**
**  Word 6 (offset 5)
**
**  bit    3-0        number of blocks passed over during the last operation
**
**  Word 7 and 8 (offsets 6 and 7) are a 24 bit frame count. This is the
**  number of 8 bit characters for a 9-track tape and the number of 7 bit
**  characters for a 7-track tape.
**  
**  Only a few error code values (in Word 1) are of interest:
**  
**  0001   tape unit off-line or not powered
**  0004   tape unit not ready
**  0010   blank tape
**  0032   tape unit busy rewinding
**  0050   illegal function
**  0051   no tape unit connected
*/
#define St669Alert              04000
#define St669CouplerStatus      02000
#define St669NoUnit             01000
#define St669NoiseRecord        00400
#define St669WriteEnabled       00200
#define St669NineTrack          00100
#define St669OddCount           00040
#define St669TapeMark           00020
#define St669EOT                00010
#define St669LoadPoint          00004
#define St669Busy               00002
#define St669Ready              00001

/*
**  Misc constants.
*/
#define MaxPpBuf                010000
#define MaxByteBuf              014000

#define MAXTAPE              115000000

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
typedef struct tapeBuf
    {
    bool        newTape;
    u8          unitMode;
    u32         blockNo;
    PpWord      recordLength;
    PpWord      lastLength;
    PpWord      deviceStatus;
    PpWord      initialStatus;
    PpWord      detailedStatus[8];
    PpWord      ioBuffer[MaxPpBuf];
    PpWord      *bp;
    } TapeBuf;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus mt669Func(PpWord funcCode);
static void mt669Io(void);
static void mt669Activate(void);
static void mt669Disconnect(void);
static void mt669FuncRelease(void);
static void mt669Unpack(u32 recLen);
static void mt669FuncRead(void);
static void mt669FuncForespace(void);
static void mt669FuncBackspace(void);
static void mt669FuncReadBkw(void);
static void mt669FuncReposition(void);
static void mt669Load(DevSlot *dp, int unitNo, char *fn);

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
static char str[200];
static u8 rawBuffer[MaxByteBuf];

static int dc6681Status;
static int dc6681UnitStatus = 01121;

static u8 convTab1in[256];
static u8 convTab1out[64];
static u8 convTab2in[256];
static u8 convTab2out[64];
static u8 convTab3in[64];
static u8 convTab3out[64];
static int convMode = 0;

int anyWrite = 0;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Initialise 669 tape drives.
**
**  Parameters:     Name        Description.
**                  eqNo        equipment number
**                  unitNo      number of the unit to initialise
**                  channelNo   channel number the device is attached to
**                  deviceName  optional device file name
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void mt669Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    DevSlot *dp;
    FILE *fcb;
    TapeBuf *tp;

    (void)eqNo;

    dp = channelAttach(channelNo, eqNo, DtMt669); /* get a device entry on this channel */

    dp->activate = mt669Activate;
    dp->disconnect = mt669Disconnect;
    dp->func = mt669Func;
    dp->io = mt669Io;
    dp->load = mt669Load;
    dp->selectedUnit = unitNo;

    dp->context[unitNo] = calloc(1, sizeof(TapeBuf));
    if (dp->context[unitNo] == NULL)
        {
        fprintf(stderr, "Failed to allocate MT669 context block\n");
        exit(1);
        }

    tp = dp->context[unitNo];

    if (deviceName != NULL)
        {
        fcb = fopen(deviceName, "rb");
        if (fcb == NULL)
            {
            fprintf(stderr, "Failed to open %s\n", deviceName);
            exit(1);
            }

        dp->fcb[unitNo] = fcb;
        tp->deviceStatus = St669NineTrack | St669LoadPoint| St669Ready;
        tp->initialStatus =  St669NineTrack | St669Ready;
        }
    else
        {
        dp->fcb[unitNo] = NULL;
        tp->deviceStatus = St669NineTrack;
        tp->initialStatus = St669NineTrack;
        }

    /*
    **  Report: forward tape motion, speed=150 ips, density=1600 cpi
    **  and configured unit number.
    */
    tp->detailedStatus[4] = 01200 + unitNo;

    tp->unitMode = 'r';
    tp->newTape = FALSE;

    /*
    **  Print a friendly message.
    */
    printf("MT669 initialised on channel %02o unit %o\n", channelNo, unitNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform load/unload on 669 tape.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mt669Load(DevSlot *dp, int unitNo, char *fn)
{
    TapeBuf *tp;
    FILE *fcb;
    u8 unitMode = 'r';
    char *p;
    static char msgBuf[80];
    long endPos = 0;
    
    if (fn == NULL)
        {
        opSetMsg ("$UNLOAD NOT SUPPORTED ON MT669");
        return;
        }
    
    if (unitNo < 0 || unitNo >= MaxUnits)
        {
        opSetMsg ("$INVALID UNIT NO");
        return;
        }
    /*
    **  Check if the unit is even configured.
    */
    if (dp->context[unitNo] == NULL)
        {
        opSetMsg ("$UNIT NOT ALLOCATED");
        return;
        }

    /*
    **  Check if the unit has been unloaded.
    */
    if (dp->fcb[unitNo] != NULL)
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
        fcb = fopen(fn, "r+b");
        if (fcb == NULL)
            {
            fcb = fopen(fn, "w+b");
            }
        else
            {
            fseek (fcb, 0, SEEK_END);
            endPos = ftell (fcb);
            rewind (fcb);
            }
        }
    else
        {
        fcb = fopen(fn, "rb");
        }

    /*
    **  Check if the open succeeded.
    */
    if (fcb == NULL)
        {
        sprintf (msgBuf, "$Open error: %s", strerror (errno));
        opSetMsg(msgBuf);
        return;
        }

    /*
    **  Setup status.
    */
    dp->fcb[unitNo] = fcb;
    tp = (TapeBuf *)dp->context[unitNo];
    tp->unitMode = unitMode;
    tp->newTape = TRUE;
    if (endPos != 0)
        {
        sprintf (msgBuf, "$MT668 loaded, warning, file is not empty.");
        }
    else
        {
        sprintf (msgBuf, "MT669 loaded with %s, ring %s",
                 fn, ((unitMode == 'w') ? "in" : "out"));
        }
    opSetMsg (msgBuf);
}

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 669 tape drives.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus mt669Func(PpWord funcCode)
    {
    u32 recLen1;
    u8 unitNo;
    TapeBuf *tp;
    i32 position;

    unitNo = activeDevice->selectedUnit;
    tp = (TapeBuf *)activeDevice->context[unitNo];
 
    switch (funcCode)
        {
    default:
        return(FcDeclined);

    case Fc669Release:
        mt669FuncRelease();
        return(FcProcessed);

    case Fc669ClearUnit:
    case Fc669ClearReserve:
        activeDevice->fcode = 0;
        activeDevice->recordLength = 0;
        tp->recordLength = 0;
        tp->detailedStatus[0] = 0;
        return(FcProcessed);

    case Fc669StopMotion:
        activeDevice->fcode = 0;
        activeDevice->recordLength = 0;
        tp->recordLength = 0;
        return(FcProcessed);

    case Fc669RewindUnload:
        tp->initialStatus = St669NineTrack;
        tp->deviceStatus = tp->initialStatus;
        fclose(activeDevice->fcb[unitNo]);
        activeDevice->fcb[unitNo] = NULL;
        return(FcProcessed);

    case Fc669SetReadClipNorm:
    case Fc669SetReadClipHigh:
    case Fc669SetReadClipLow:
        activeDevice->fcode = 0;
        return(FcProcessed);

    case Fc669Rewind:
        activeDevice->fcode = funcCode;
        if (tp->initialStatus | St669Ready)
            {
#if ScopeSupport == 0
            tp->deviceStatus = tp->initialStatus | St669LoadPoint;
#else
            /*
            **  This is a horrible hack to get SCOPE/Hustler started.
            */
			if (unitNo == 3)
				tp->deviceStatus = tp->initialStatus;
			else
				tp->deviceStatus = tp->initialStatus | St669LoadPoint;
#endif
            tp->blockNo = 0;
            fseek(activeDevice->fcb[unitNo], 0, SEEK_SET);
            }
        else
            {
            tp->deviceStatus = tp->initialStatus;
            }

        dc6681UnitStatus = 01121;
        break;

    case Fc669ReadBkw:
        activeDevice->fcode = funcCode;
        mt669FuncReadBkw();
        break;

    case Fc669Reposition:
        activeDevice->fcode = funcCode;
        mt669FuncReposition();
        break;

    case Fc669Forespace:
        activeDevice->fcode = funcCode;
        mt669FuncForespace();
        break;

    case Fc669Backspace:
        activeDevice->fcode = funcCode;
        mt669FuncBackspace();
        break;

    case Fc669WriteEOF:
        activeDevice->fcode = funcCode;
        tp->bp = tp->ioBuffer;
        position = ftell(activeDevice->fcb[unitNo]);
        if (position > MAXTAPE)
            {
            tp->deviceStatus |= St669EOT;
            }
        tp->blockNo += 1;

        /*
        **  The following fseek makes fwrite behave as desired after an fread.
        */
        fseek(activeDevice->fcb[unitNo], 0, SEEK_CUR);

        /*
        **  Write a TAP tape mark.
        */
        recLen1 = 0;
        fwrite(&recLen1, sizeof(recLen1), 1, activeDevice->fcb[unitNo]);
        tp->deviceStatus = tp->initialStatus | St669TapeMark;

        /*
        **  The following fseek prepares for any subsequent fread.
        */
        fseek(activeDevice->fcb[unitNo], 0, SEEK_CUR);

        return(FcProcessed);

    case Fc669WriteOdd12:
        funcCode = Fc669WriteOdd;

    /*
    **  FALL THROUGH
    */

    case Fc669Write:
    case Fc669WriteOdd:
        activeDevice->fcode = funcCode;
        tp->bp = tp->ioBuffer;
        activeDevice->recordLength = 0;
        tp->deviceStatus = tp->initialStatus;
        anyWrite = funcCode;
        position = ftell(activeDevice->fcb[unitNo]);
        if (position > MAXTAPE)
            {
            tp->deviceStatus |= St669EOT;
            }
        tp->blockNo += 1;
        break;

    case 01300:
        activeChannel->status = 01001;
        activeDevice->fcode = funcCode;
        break;

    case 01200:
        activeChannel->status = 0;
        activeDevice->fcode = funcCode;
        break;

    case Fc669GeneralStatus:
        activeDevice->fcode = funcCode;
        break;

    case Fc669Erase:
        activeDevice->fcode = funcCode;
        tp->deviceStatus = tp->initialStatus;
        break;

    case Fc669MasterClear:
        activeDevice->fcode = funcCode;
        activeDevice->status = St669NineTrack | St669Ready;
        break;

    case Fc669LoadConversion1:
    case Fc669LoadConversion2:
    case Fc669LoadConversion3:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 0;
        break;

    case Fc669DetailedStatus:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 8;

#if ScopeSupport == 1
        tp->detailedStatus[6] = (PpWord)(tp->lastLength >> 12)  & 07777;
        tp->detailedStatus[7] = (PpWord)tp->lastLength  & 07777;
#else
        tp->detailedStatus[6] = (PpWord)(tp->blockNo >> 12)  & 07777;
        tp->detailedStatus[7] = (PpWord)tp->blockNo  & 07777;
#endif
        break;

    case Fc669Connect + 0:
    case Fc669Connect + 1:
    case Fc669Connect + 2:
    case Fc669Connect + 3:
    case Fc669Connect + 4:
    case Fc669Connect + 5:
    case Fc669Connect + 6:
    case Fc669Connect + 7:
//      activeDevice->fcode = 0;
        activeDevice->selectedUnit = funcCode & 07;
        unitNo = activeDevice->selectedUnit;
        tp = (TapeBuf *)activeDevice->context[unitNo];
        if (tp == NULL)
            {
            logError(LogErrorLocation, "channel %02o - invalid select: %04o", activeChannel->id, (u32)funcCode);
            return(FcDeclined);
            }
        if (activeDevice->fcb[unitNo] == NULL)
            {
            tp->deviceStatus = St669NineTrack;
            }
        else
            {
            activeDevice->status = tp->deviceStatus;
            }
        break;

    case Fc669ConnectRewindRead + 0:
    case Fc669ConnectRewindRead + 1:
    case Fc669ConnectRewindRead + 2:
    case Fc669ConnectRewindRead + 3:
    case Fc669ConnectRewindRead + 4:
    case Fc669ConnectRewindRead + 5:
    case Fc669ConnectRewindRead + 6:
    case Fc669ConnectRewindRead + 7:
        activeDevice->selectedUnit = funcCode & 07;
        unitNo = activeDevice->selectedUnit;
        tp = activeDevice->context[unitNo];
        if (activeDevice->context[unitNo] == NULL)
            {
            logError(LogErrorLocation, "channel %02o - invalid select: %04o,", activeChannel->id, (u32)funcCode);
            return(FcDeclined);
            }
        if (activeDevice->fcb[unitNo] == NULL)
            {
            tp->deviceStatus = St669NineTrack; /* unloaded */
            activeDevice->fcode = 0;
            break;
            }
        else
            {
            tp->deviceStatus = tp->initialStatus | St669LoadPoint;
            fseek(activeDevice->fcb[unitNo], 0, SEEK_SET);
            activeDevice->recordLength = 0;
            tp->blockNo = 0;
            tp->recordLength = 0;
            funcCode = Fc669ReadFwd;
            }

    /*
    **  FALL THROUGH
    */

    case Fc669ReadFwd:
    case Fc6681Read2EOR:
        activeDevice->fcode = funcCode;
        mt669FuncRead();
        break;

    case Fc669FormatUnit:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 2;  /* 3 for ATS units, 2 for MTS*/
        break;
        }

    return(FcAccepted);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on MT669.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mt669Io(void)
    {
    u8 unitNo;
    TapeBuf *tp;
    int indx;

    unitNo = activeDevice->selectedUnit;
    tp = (TapeBuf *)activeDevice->context[unitNo];

    switch (activeDevice->fcode)
        {
    default:
    case Fc669Connect + 0:
    case Fc669Connect + 1:
    case Fc669Connect + 2:
    case Fc669Connect + 3:
    case Fc669Connect + 4:
    case Fc669Connect + 5:
    case Fc669Connect + 6:
    case Fc669Connect + 7:
        logError(LogErrorLocation, "channel %02o - unsupported function code: %04o",
             activeChannel->id, activeDevice->fcode);
        break;

    case Fc669Rewind:
        dc6681UnitStatus = 01121;
    case Fc669GeneralStatus:
    case Fc669Erase:
        activeChannel->data = tp->deviceStatus;
        activeChannel->full = TRUE;
        activeChannel->discAfterInput = TRUE;
        break;

    case 01200:
        activeChannel->data = 0;
        activeChannel->full = TRUE;
        activeChannel->discAfterInput = FALSE;
        break;

    case 01300:
        activeChannel->data = dc6681UnitStatus;
        activeChannel->full = TRUE;
        activeChannel->discAfterInput = FALSE;
        break;

    case Fc669DetailedStatus:
        if (activeChannel->full)
            {
            break;
            }

        if (activeDevice->recordLength > 0)
            {
            indx = 8 - activeDevice->recordLength;
            activeChannel->data = tp->detailedStatus[indx];
            activeDevice->recordLength -= 1;
            if( activeDevice->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            activeChannel->full = TRUE;
            }
        break;

    case Fc669ReadBkw:
       dc6681UnitStatus = 01101;
        if (activeChannel->full)
            {          
            break;     
            }       
                       
        if (tp->recordLength == 0)
            {          
            activeChannel->active = FALSE;
            }          
              
        if (tp->recordLength > 0)
            {
            activeChannel->data = *tp->bp--;
            activeChannel->full = TRUE;
            tp->recordLength -= 1;
            if (tp->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;

    case Fc669ReadFwd:
    case Fc6681Read2EOR:
        dc6681UnitStatus = 01101;
        if (activeChannel->full)
            {
            break;
            }

        if (tp->recordLength == 0)
            {
            activeChannel->active = FALSE;
            }

        if (tp->recordLength > 0)
            {
            activeChannel->data = *tp->bp++;
            activeChannel->full = TRUE;
            tp->recordLength -= 1;
            if (tp->recordLength == 0)
                {
                activeChannel->discAfterInput = TRUE;
                }
            }
        break;
        
    case Fc669Write:
    case Fc669WriteOdd:
         if (activeChannel->full)
             {
             activeChannel->full = FALSE;
             activeDevice->recordLength += 1;
             *tp->bp++ = activeChannel->data;
             }
         break;

    case Fc669FormatUnit:
         if (activeDevice->recordLength > 0)
             {
             if (activeChannel->full)
                {
                indx = 2 - activeDevice->recordLength;
                if (indx == 0)
                    {
                    switch ((activeChannel->data & 03400) >> 8 )
                        {
                    case 0:
                        convMode = 0;
                        break;
                    case 01:
                        convMode = 1;
                        break;
                    case 02:
                        convMode = 2;
                        break;
                    case 03:
                        convMode = 3;
                        break;
                    case 04:
                    case 05:
                    case 06:
                    case 07:
                        break;
                        }
                    tp = activeDevice->context[activeChannel->data & 07];
                    }
                activeChannel->full = FALSE;
                activeDevice->recordLength -= 1;
                }
             }
         else
             {
//             activeChannel->active = FALSE;       // NOS 1.4 hates this
             }
         break;

    case Fc669LoadConversion1:
        if (activeChannel->full)
            {
            activeChannel->full = FALSE;
            convTab1in[activeDevice->recordLength] = activeChannel->data & 077;
            if (activeChannel->data & 01000)
                {     
                convTab1out[activeChannel->data & 077] =
                     (u8)activeDevice->recordLength;
                }  
             activeDevice->recordLength += 1;
             }      

    case Fc669LoadConversion2:
         if (activeChannel->full)
             {
             activeChannel->full = FALSE;
             convTab2in[activeDevice->recordLength] = activeChannel->data & 077;
             if (activeChannel->data & 01000)
                 {
                 convTab2out[activeChannel->data & 077] =
                     (u8)activeDevice->recordLength;    
                 }
             activeDevice->recordLength += 1;
             }

    case Fc669LoadConversion3:
         if (activeChannel->full)
             {
             activeChannel->full = FALSE;
             convTab3in[activeDevice->recordLength] = activeChannel->data & 077;
             convTab3out[activeChannel->data & 077] = (u8)activeDevice->recordLength;
             activeDevice->recordLength += 1;
             }

    case Fc669WriteEOF:
    case Fc669Forespace:
    case Fc669Backspace:
    case Fc669Reposition:
    case Fc669MasterClear:
        if (activeChannel->full)
            {
            activeChannel->full = FALSE;
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
static void mt669Activate(void)
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
static void mt669Disconnect(void)
    {
    FILE *fcb;
    TapeBuf *tp;
    u8 unitNo;
    u32 i;
    u32 recLen0;
    u32 recLen1;
    u32 recLen2;
    u16 *iop;
    u8 *rp;
    u8 c1, c2;

    if (anyWrite != 0)
        {
        unitNo = activeDevice->selectedUnit;
        fcb = activeDevice->fcb[unitNo];
        tp = (TapeBuf *)activeDevice->context[unitNo];
        tp->bp = tp->ioBuffer;

        recLen0 = 0;
        recLen2 = activeDevice->recordLength;
        iop = tp->ioBuffer;
        rp = rawBuffer;

        switch (convMode)
            {
        case 0:
            /*
            **  Convert the PP Word data into raw  data.
            */
            for (i = 0; i < recLen2; i += 2)
                {
                *rp++ = (*iop >> 4) & 0xff;
                c1 = (*iop << 4) & 0xf0;
                iop++;
                c2 = (*iop >> 8) & 0x0f;
                *rp++ = (c1 | c2) & 0xff;
                *rp++ = *iop & 0xff;
                iop++;
                }

            recLen0 = rp - rawBuffer;
            break;

        case 1:
            /*
            **  Convert the PP Word data to ASCII.
            */
            for (i = 0; i < recLen2; i += 1)
                 {
                 c1 = (*iop & 07700) >> 6;
                 c2 = (*iop & 00077);
                 *rp++ = convTab1out[c1] & 0x7f;
                 *rp++ = convTab1out[c2] & 0x7f;
                 iop++;
                 }

            recLen0 = rp - rawBuffer;
            if (anyWrite == Fc669WriteOdd)
                {
                recLen0 -= 1;  /* odd write - backup a byte */
                }
            break;

        case 2:
            /*
            **  Convert the PP Word data to ebcdic
            */
            for (i = 0; i < recLen2; i += 1)
                 {
                 c1 = (*iop & 07700) >> 6;
                 c2 = (*iop & 00077);
                 *rp++ = convTab2out[c1];
                 *rp++ = convTab2out[c2];
                 iop++;
                 }

            recLen0 = rp - rawBuffer;
            if (anyWrite == Fc669WriteOdd)
                {
                recLen0--;  /* odd write - backup a byte */
                }
            break;

        case 3:
           /*
            **  Convert the PP Word data to BCD
            */
            for (i = 0; i < recLen2; i += 1)
                 {
                 c1 = (*iop & 07700) >> 6;
                 c2 = (*iop & 00077);
                 *rp++ = convTab3out[c1];
                 *rp++ = convTab3out[c2]; 
                 iop++;
                 }
            recLen0 = rp - rawBuffer;
            if (anyWrite == Fc669WriteOdd) 
                {
                recLen0--;  /* odd write - backup a byte */
                } 
            break;
            }

        /*
        ** The TAP record length is little endian - convert if necessary.
        */
        if (bigEndian)
            {
            recLen1 = initConvertEndian(recLen0);
            }
        else
            {
            recLen1 = recLen0;
            }

        /*
        **  The following fseek makes fwrite behave as desired after an fread.
        */
        fseek(fcb, 0, SEEK_CUR);

        /*
        **  Write the TAP record.
        */
        fwrite(&recLen1, sizeof(recLen1), 1, fcb);
        fwrite(&rawBuffer, 1, recLen0, fcb);
        fwrite(&recLen1, sizeof(recLen1), 1, fcb);

        /*
        **  The following fseek prepares for any subsequent fread.
        */
        fseek(fcb, 0, SEEK_CUR);

        /*
        **  Clear load-point status.
        */
        tp->deviceStatus &= (PpWord)(~St669LoadPoint);
        anyWrite = 0;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process release function.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void mt669FuncRelease(void)
    {
    u8 unitNo = activeDevice->selectedUnit;
    TapeBuf *tp = activeDevice->context[unitNo];

    activeDevice->fcode = 0;
    activeDevice->status = St669NoUnit;

    if (tp->newTape)
        {
        /*
        **  Operator has inserted a new tape.
        */
        tp->newTape = FALSE;
        /*
        **  Setup status.
        */
        tp->deviceStatus = St669NineTrack | St669LoadPoint| St669Ready;
        tp->initialStatus =  St669NineTrack | St669Ready;
        if (tp->unitMode == 'w')
            {
            tp->deviceStatus  |= St669WriteEnabled;
            tp->initialStatus |= St669WriteEnabled;
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Unpack and convert read tape block.
**
**  Parameters:     Name        Description.
**                  recLen      read tape record length
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mt669Unpack(u32 recLen)
    {
    u32 i;
    u16 c1, c2, c3;
    u16 *op;
    u8 *rp;
    u8 unitNo;
    TapeBuf *tp;

    unitNo = activeDevice->selectedUnit;
    tp = (TapeBuf *)activeDevice->context[unitNo];

    /*
    **  Convert the raw data into PP words suitable for a channel.
    */
    op = tp->ioBuffer;
    rp = rawBuffer;

    switch (convMode)
       {
    case 0:
        /*
        **  Convert the raw data into PP Word data.
        */
#if ScopeSupport == 0
        if ((recLen % 2) == 1)
            {
            /*
            **  Handle odd frame record count.
            */
            recLen++;
            tp->deviceStatus |= St669OddCount;
            }
#endif

        for (i = 0; i < recLen; i += 3)
            {
            c1 = *rp++;
            c2 = *rp++;
            c3 = *rp++;

            *op++ = ((c1 << 4) | (c2 >> 4)) & Mask12;
            *op++ = ((c2 << 8) | (c3 >> 0)) & Mask12;
            }

        activeDevice->recordLength = op - tp->ioBuffer;

        switch (recLen % 3)
           {
        default:
            break;

        case 1:     /* 2 words + 8 bits */
            activeDevice->recordLength -= 1;
            break;

        case 2:
#if ScopeSupport == 1
            tp->deviceStatus |= St669OddCount;
#endif
            break;
           }
        break;

    case 1:
        /*
        **  Convert the Raw data from ascii to display
        */
        for (i = 0; i < recLen; i += 2)
            {
            c1 = convTab1in[*rp++];
            c2 = (c1 << 6 )  & Mask12 ;
            c3 = convTab1in[*rp++];
            *op = (c2 | c3) & Mask12 ;
            op++;
            }

        activeDevice->recordLength = op - tp->ioBuffer;

        if (recLen % 2) 
            {   /* signal lower 6 bits invallid */
            tp->deviceStatus |= St669OddCount;
            }
        break;

    case 2:
        /*
        **  Convert the Raw data from ebcdic to display
        */
        for (i = 0; i < recLen; i += 2)
            {
            c1 = convTab2in[(u8)*rp++];
            c2 = (c1 << 6 )  & Mask12 ; 
            c3 = convTab2in[(u8)*rp++];
            *op = (c2 | c3) & Mask12 ;  
            op++;
            }

        activeDevice->recordLength = op - tp->ioBuffer;

        if (recLen % 2) 
            {   /* signal lower 6 bits invalid */
            tp->deviceStatus |= St669OddCount;
            }
        break;

    case 3:
        /*  
        **  Convert the Raw data from BCD to display
        */
        for (i = 0; i < recLen; i += 2)
            {
            c1 = convTab3in[(u8)*rp++];
            c2 = (c1 << 6 )  & Mask12 ;
            c3 = convTab3in[(u8)*rp++]; 
            *op = (c2 | c3) & Mask12 ;
            op++;
            }
                
        activeDevice->recordLength = op - tp->ioBuffer;
        if (recLen % 2)
            {   /* signal lower 6 bits invalid */
            tp->deviceStatus |= St669OddCount;
            }
        break;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process read function.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mt669FuncRead(void)
    {
    u32 len;
    u32 recLen0;
    u32 recLen1;
    u32 recLen2;
    u8 unitNo;
    TapeBuf *tp;
    i32 position;

    unitNo = activeDevice->selectedUnit;
    tp = (TapeBuf *)activeDevice->context[unitNo];
 
    activeDevice->fcode = Fc669ReadFwd;
    activeDevice->recordLength = 0;
    tp->recordLength = 0;
    tp->detailedStatus[0] = 0;
    tp->deviceStatus = tp->initialStatus;

    if ((tp->initialStatus & St669Ready) == 0)
        {
        return;
        }

    /*
    **  Determine if the tape is at the load point.
    */
    position = ftell(activeDevice->fcb[unitNo]);
    if (position == 0)
        {
        tp->deviceStatus |= St669LoadPoint;
        }

    /*
    **  Read and verify TAP record length header.
    */
    len = fread(&recLen0, sizeof(recLen0), 1, activeDevice->fcb[unitNo]);

    if (len != 1)
        {
        if ((tp->deviceStatus & St669LoadPoint) != 0)
            {
            tp->deviceStatus = tp->initialStatus; 
            tp->detailedStatus[0] = 010; /* say blank tape */
            tp->blockNo = 0;
            }
        else
            {
            tp->deviceStatus = tp->initialStatus | St669EOT;
            }

        activeDevice->recordLength = 0;
        tp->recordLength = 0;
        return;
        }

    /*
    **  The TAP record length is little endian - convert if necessary.
    */
    if (bigEndian)
        {
        recLen1 = initConvertEndian(recLen0);
        }
    else
        {
        recLen1 = recLen0;
        }

    /*
    **  Check if record length is reasonable.
    */
    if (recLen1 > MaxByteBuf)
        {
        logError(LogErrorLocation, "channel %02o - tape record too long: %d", activeChannel->id, recLen1);
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        activeDevice->recordLength = 0;
        tp->recordLength = 0;
        return;
        }

    /*
    **  Reset status info
    */
    tp->deviceStatus = tp->initialStatus;

    if (recLen1 == 0)
        {
        /*
        **  Tape mark.
        */
        tp->deviceStatus |=  St669TapeMark;
        activeDevice->recordLength = 0;
        tp->recordLength = 0;
        tp->blockNo += 1;
        return;
        }

    /*
    **  Preset the raw buffer.
    */
    memset(rawBuffer, 0, sizeof(rawBuffer));

    /*
    **  Read and verify the actual raw data.
    */
    len = fread(rawBuffer, 1, recLen1, activeDevice->fcb[unitNo]);

    if (recLen1 != (u32)len)
        {
        logError(LogErrorLocation, "channel %02o - short tape record read: %d", activeChannel->id, len);
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        activeDevice->recordLength = 0;
        tp->recordLength = 0;
        return;
        }

    /*
    **  Read and verify the TAP record length trailer.
    */
    len = fread(&recLen2, sizeof(recLen2), 1, activeDevice->fcb[unitNo]);

    if (len != 1)
        {
        logError(LogErrorLocation, "channel %02o - missing tape record trailer", activeChannel->id);
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        return;
        }

    if (recLen0 != recLen2)
        {
        /*
        **  This is some weird shit to deal with "padded" TAP records. My brain refuses to understand
        **  why anyone would have the precise length of a record and then make the reader guess what
        **  the real length is.
        */
        if (bigEndian)
            {
            /*
            **  The TAP record length is little endian - convert if necessary.
            */
            recLen2 = initConvertEndian(recLen2);
            }

        if (recLen1 == ((recLen2 >> 8) & 0xFFFFFF))
            {
            fseek(activeDevice->fcb[unitNo], 1, SEEK_CUR);
            }
        else
            {
            logError(LogErrorLocation, "channel %02o - invalid tape record trailer: %d", activeChannel->id, recLen2);
            tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
            activeDevice->recordLength = 0;
            tp->recordLength = 0;
            return;
            }
        }

    /*
    **  Convert the raw data into PP words suitable for a channel.
    */
    mt669Unpack(recLen1);

    /*
    **  Setup length, buffer pointer and block number.
    */
    tp->recordLength = activeDevice->recordLength;
    tp->lastLength = (PpWord)recLen1;
    tp->bp = tp->ioBuffer;
    tp->blockNo += 1;

#if CcDebug == 1
    sprintf(str, "---- record with %d bytes  and %d PP words----\n", recLen1, activeDevice->recordLength);
    tracePrint(str);
#endif
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process read backward function.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mt669FuncReadBkw(void)
    {
    u32 len;
    u32 recLen0;
    u32 recLen1;
    u32 recLen2;
    u8 unitNo;
    TapeBuf *tp;
    i32 position;

    unitNo = activeDevice->selectedUnit;
    tp = (TapeBuf *)activeDevice->context[unitNo];
 
    activeDevice->recordLength = 0;
    tp->recordLength = 0;
    tp->detailedStatus[0] = 0;
    tp->deviceStatus = tp->initialStatus;

    if ((tp->initialStatus & St669Ready) == 0)
        {
        return;
        }

    /*
    **  Check if we are already at the beginning of the tape.
    */
    position = ftell(activeDevice->fcb[unitNo]);
    if (position == 0)
        {
        tp->deviceStatus = tp->initialStatus | St669LoadPoint;
        tp->blockNo = 0;
        return;
        }

    /*
    **  Position to the previous record's trailer and read the length
    **  of the record (leaving the file position ahead of the just read
    **  record trailer).
    */
    fseek(activeDevice->fcb[unitNo], -4, SEEK_CUR);
    len = fread(&recLen0, sizeof(recLen0), 1, activeDevice->fcb[unitNo]);
    fseek(activeDevice->fcb[unitNo], -4, SEEK_CUR);

    if (len != 1)
        {
        logError(LogErrorLocation, "channel %02o - missing tape record trailer", activeChannel->id);
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        return;
        }

    /*
    **  The TAP record length is little endian - convert if necessary.
    */
    if (bigEndian)
        {
        recLen1 = initConvertEndian(recLen0);
        }
   else
        {
        recLen1 = recLen0;
        }

    /*
    **  Check if record length is reasonable.
    */
    if (recLen1 > MaxByteBuf)
        {
        logError(LogErrorLocation, "channel %02o - tape record too long: %d", activeChannel->id, recLen1);
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        activeDevice->recordLength = 0;
        tp->recordLength = 0;
        return;
        }

    position -= 4;
    if (recLen1 != 0)
        {
        /*
        **  Skip backward over the TAP record body and header.
        */
        position -= 4 + recLen1;
        fseek(activeDevice->fcb[unitNo], position, SEEK_SET);

        /*
        **  Read and verify the TAP record header.
        */
        len = fread(&recLen2, sizeof(recLen2), 1, activeDevice->fcb[unitNo]);

        if (len != 1)
            {
            logError(LogErrorLocation, "channel %02o - missing TAP record header", activeChannel->id);
            tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
            return;
            }

        if (recLen0 != recLen2)
            {
            /*
            **  This is more weird shit to deal with "padded" TAP records.
            */
            position -= 1;
            fseek(activeDevice->fcb[unitNo], position, SEEK_SET);
            len = fread(&recLen2, sizeof(recLen2), 1, activeDevice->fcb[unitNo]);

            if (len != 1 || recLen0 != recLen2)
                {
                logError(LogErrorLocation, "channel %02o - invalid record length2: %d %08X != %08X", activeChannel->id, len, recLen0, recLen2);
                tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
                return;
               }
            }

        /*
        **  Preset the raw buffer.
        */
        memset(rawBuffer, 0, sizeof(rawBuffer));

        /*
        **  Read and verify the actual raw data.
        */
        len = fread(rawBuffer, 1, recLen1, activeDevice->fcb[unitNo]);

        if (recLen1 != (u32)len)
            {
            logError(LogErrorLocation, "channel %02o - short tape record read: %d", activeChannel->id, len);
            tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
            activeDevice->recordLength = 0;
            tp->recordLength = 0;
            return;
            }

        /*
        **  Position to the TAP record header.
        */
        fseek(activeDevice->fcb[unitNo], position, SEEK_SET);

        /*
        **  Convert the raw data into PP words suitable for a channel.
        */
        mt669Unpack(recLen1);

        /*
        **  Setup length and buffer pointer.
        */
        tp->recordLength = activeDevice->recordLength;
        tp->bp = tp->ioBuffer + activeDevice->recordLength;
        }
    else
        {
        /*
        **  A tape mark consists of only a single TAP record header of zero.
        */
        tp->deviceStatus |= St669TapeMark;
        }

    /*
    **  Set status and block number.
    */
    if (position == 0)
        {
        tp->deviceStatus |= St669LoadPoint;
        tp->blockNo = 0;
        }
    else
        {
        tp->blockNo -= 1;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process forespace function.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mt669FuncForespace(void)
    {
    u32 len;
    u32 recLen0;
    u32 recLen1;
    u32 recLen2;
    u8 unitNo;
    TapeBuf *tp;
    i32 position;

    unitNo = activeDevice->selectedUnit;
    tp = (TapeBuf *)activeDevice->context[unitNo];
 
    activeDevice->recordLength = 0;
    tp->recordLength = 0;
    tp->detailedStatus[0] = 0;
    tp->deviceStatus = tp->initialStatus;

    if ((tp->initialStatus & St669Ready) == 0)
        {
        return;
        }

    /*
    **  Determine if the tape is at the load point.
    */
    position = ftell(activeDevice->fcb[unitNo]);
    if (position == 0)
        {
        tp->deviceStatus |= St669LoadPoint;
        }

    /*
    **  Reset tape buffer pointer.
    */
    tp->bp = tp->ioBuffer;

    /*
    **  Read and verify TAP record length header.
    */
    len = fread(&recLen0, sizeof(recLen0), 1, activeDevice->fcb[unitNo]);

    if (len != 1)
        {
        if ((tp->deviceStatus & St669LoadPoint) != 0)
            {
            tp->deviceStatus = tp->initialStatus; 
            tp->detailedStatus[0] = 010; /* say blank tape */
            tp->blockNo = 0;
            }
        else
            {
            tp->deviceStatus = tp->initialStatus | St669EOT;
            }
        return;
        }

    /*
    **  The TAP record length is little endian - convert if necessary.
    */
    if (bigEndian)
        {
        recLen1 = initConvertEndian(recLen0);
        }
    else
        {
        recLen1 = recLen0;
        }

    /*
    **  Check if record length is reasonable.
    */
    if (recLen1 > MaxByteBuf)
        {
        logError(LogErrorLocation, "channel %02o - tape record too long: %d", activeChannel->id, recLen1);
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        return;
        }

    /*
    **  Reset status info
    */
    tp->deviceStatus = tp->initialStatus;

    if (recLen1 == 0)
        {
        /*
        **  Report a tape mark and return.
        */
        tp->deviceStatus |= St669TapeMark;
        tp->blockNo += 1;
        return;
        }

    /*
    **  Skip the actual raw data.
    */
    if (fseek(activeDevice->fcb[unitNo], recLen1, SEEK_CUR) != 0)
        {
        logError(LogErrorLocation, "channel %02o - short tape record read: %d", activeChannel->id, len);
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        return;
        }

    /*
    **  Read and verify the TAP record length trailer.
    */
    len = fread(&recLen2, sizeof(recLen2), 1, activeDevice->fcb[unitNo]);

    if (len != 1)
        {
        logError(LogErrorLocation, "channel %02o - missing tape record trailer", activeChannel->id);
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        return;
        }

    if (recLen0 != recLen2)
        {
        /*
        **  This is some weird shit to deal with "padded" TAP records. My brain refuses to understand
        **  why anyone would have the precise length of a record and then make the reader guess what
        **  the real length is.
        */
        if (bigEndian)
            {
            /*
            **  The TAP record length is little endian - convert if necessary.
            */
            recLen2 = initConvertEndian(recLen2);
            }

        if (recLen1 == ((recLen2 >> 8) & 0xFFFFFF))
            {
            fseek(activeDevice->fcb[unitNo], 1, SEEK_CUR);
            }
        else
            {
            logError(LogErrorLocation, "channel %02o - invalid tape record trailer: %d", activeChannel->id, recLen2);
            tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
            return;
            }
        }

    tp->blockNo += 1;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process backspace function.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mt669FuncBackspace(void)
    {
    u32 len;
    u32 recLen0;
    u32 recLen1;
    u32 recLen2;
    u8 unitNo;
    TapeBuf *tp;
    i32 position;

    unitNo = activeDevice->selectedUnit;
    tp = (TapeBuf *)activeDevice->context[unitNo];

    activeDevice->recordLength = 0;
    tp->recordLength = 0;
    tp->detailedStatus[0] = 0;
    tp->deviceStatus = tp->initialStatus;

    if ((tp->initialStatus & St669Ready) == 0)
        {
        return;
        }

    /*
    **  Check if we are already at the beginning of the tape.
    */
    position = ftell(activeDevice->fcb[unitNo]);
    if (position == 0)
        {
        tp->deviceStatus |= St669LoadPoint;
        tp->blockNo = 0;
        return;
        }

    /*
    **  Position to the previous record's trailer and read the length
    **  of the record (leaving the file position ahead of the just read
    **  record trailer).
    */
    fseek(activeDevice->fcb[unitNo], -4, SEEK_CUR);
    len = fread(&recLen0, sizeof(recLen0), 1, activeDevice->fcb[unitNo]);
    fseek(activeDevice->fcb[unitNo], -4, SEEK_CUR);

    if (len != 1)
        {
        logError(LogErrorLocation, "channel %02o - missing tape record trailer", activeChannel->id);
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        return;
        }

    /*
    **  The TAP record length is little endian - convert if necessary.
    */
    if (bigEndian)
        {
        recLen1 = initConvertEndian(recLen0);
        }
   else
        {
        recLen1 = recLen0;
        }

    /*
    **  Check if record length is reasonable.
    */
    if (recLen1 > MaxByteBuf)
        {
        logError(LogErrorLocation, "channel %02o - tape record too long: %d", activeChannel->id, recLen1);
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        return;
        }

    position -= 4;
    if (recLen1 != 0)
        {
        /*
        **  Skip backward over the TAP record body and header.
        */
        position -= 4 + recLen1;
        fseek(activeDevice->fcb[unitNo], position, SEEK_SET);

        /*
        **  Read and verify the TAP record header.
        */
        len = fread(&recLen2, sizeof(recLen2), 1, activeDevice->fcb[unitNo]);

        if (len != 1)
            {
            logError(LogErrorLocation, "channel %02o - missing TAP record header", activeChannel->id);
            tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
            return;
            }

        if (recLen0 != recLen2)
            {
            /*
            **  This is more weird shit to deal with "padded" TAP records.
            */
            position -= 1;
            fseek(activeDevice->fcb[unitNo], position, SEEK_SET);
            len = fread(&recLen2, sizeof(recLen2), 1, activeDevice->fcb[unitNo]);

            if (len != 1 || recLen0 != recLen2)
                {
                logError(LogErrorLocation, "channel %02o - invalid record length2: %d %08X != %08X", activeChannel->id, len, recLen0, recLen2);
                tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
                return;
               }
            }

        /*
        **  Position to the TAP record header.
        */
        fseek(activeDevice->fcb[unitNo], position, SEEK_SET);
        }
    else
        {
        /*
        **  A tape mark consists of only a single TAP record header of zero.
        */
        tp->deviceStatus = tp->initialStatus | St669TapeMark;
        }

    /*
    **  Set status and block number.
    */
    if (position == 0)
        {
        tp->deviceStatus |= St669LoadPoint;
        tp->blockNo = 0;
        }
    else
        {
        tp->blockNo -= 1;
        }
    }


/*--------------------------------------------------------------------------
**  Purpose:        Process reposition function.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mt669FuncReposition(void)
    {
    u32 len;
    u32 recLen0;
    u32 recLen1;
    u8 unitNo;
    TapeBuf *tp;
    i32 position;

    unitNo = activeDevice->selectedUnit;
    tp = (TapeBuf *)activeDevice->context[unitNo];
    if (tp->initialStatus | St669Ready)
        {
        tp->deviceStatus = tp->initialStatus;
        position = ftell(activeDevice->fcb[unitNo]);
        if (position == 0)
            {
            tp->deviceStatus |= St669LoadPoint;
            tp->blockNo = 0;
            return;
            }
        else
            {
            tp->blockNo -= 1;
            }
        /*
         * read the tail tap header to determine length.
         */ 
        fseek(activeDevice->fcb[unitNo], -4, SEEK_CUR);
        len = fread(&recLen0, sizeof(recLen0), 1, activeDevice->fcb[unitNo]);   
        fseek(activeDevice->fcb[unitNo], -4, SEEK_CUR);
        position -= 4; 
        if (len != 1)
            {   
            logError(LogErrorLocation, "channel %02o - invalid record length: %04o", activeChannel->id, (u32)len);
            }
        
        /*
        **  The TAP record length is little endian - convert if necessary.
        */      
        if (bigEndian)
            {
            recLen1 = initConvertEndian(recLen0);
            }
       else
            {
            recLen1 = recLen0;
            }
    
        if (recLen1 != 0)
            {
            /*
            **  Normal record.
            */
            position -= 4 + recLen1;
            fseek(activeDevice->fcb[unitNo], position, SEEK_SET);
            }
        else
            {
            /*
            **  Tape mark.
            */
            tp->deviceStatus |= St669TapeMark;
            }

        if (position == 0)
            {
            tp->deviceStatus |= St669LoadPoint;
            tp->blockNo = 0;
            }
        }
    }

/*---------------------------  End Of File  ------------------------------*/

