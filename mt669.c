/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: mt669.c
**
**  Description:
**      Perform simulation of CDC 6600 669 tape drives.
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
**  CDC 669 tape function and status codes.
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
**  0340   read backward with odd length parity
**  0041   reread forward
**  0141   reread backward
**  0341   reread backward with odd length par.
**  0042   repeat read
**  0050   write
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
**  7260 connects to EQ07, UN00 rewinds initiates a read forward
**  
*/
#define Fc669ClearUnit          00000
#define Fc669Release            00001
#define Fc669ClearReserve       00002
#define Fc669Rewind             00010
#define Fc669StopMotion         00011
#define Fc669GeneralStatus      00012
#define Fc669Forespace          00013
#define Fc669SearchTapeMarkF    00015
#define Fc669SearchTapeMarkB    00115
#define Fc669Reposition         00017
#define Fc669Connect            00020
#define Fc669FormatUnit         00030
#define Fc669ReadFwd            00040
#define Fc669ReadBkw            00140
#define Fc669Write              00050
#define Fc669WriteEOF           00051
#define Fc669Erase              00052
#define Fc669WriteOdd           00250
#define Fc669RewindUnload       00110
#define Fc669DetailedStatus     00112
#define Fc669Backspace          00113
#define Fc669ConnectRewindRead  00260
#define Fc669LoadConversion1    00131
#define Fc669LoadConversion2    00231
#define Fc669LoadConversion3    00331
#define Fc669SetReadClipNorm    00006
#define Fc669SetReadClipHigh    00106
#define Fc669SetReadClipLow     00206
#define Fc669MasterClear        00414
#define Fc6681Read2EOR          01400

/*
**  General status reply:
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
**  For detailed status, just be sure to return a block
**  of 8 words of zeroes.
**  
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
#define DEBUG                        0
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
static void mt669FuncRead(void);
static void mt669FuncForespace(void);
static void mt669FuncBackspace(void);
static void mt669FuncReadBkw(void);
static void mt669FuncReposition(void);

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

static char newTape[80];

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

    dp = channelAttach(channelNo, DtMt669); /* get a device entry on this channel */

    dp->activate = mt669Activate;
    dp->disconnect = mt669Disconnect;
    dp->func = mt669Func;
    dp->io = mt669Io;
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

    tp->detailedStatus[4] = 01200 + unitNo;
    tp->unitMode = 'r';
    tp->newTape = FALSE;

    /*
    **  Print a friendly message.
    */
    printf("MT669 initialised on channel %02o unit %o\n", channelNo, unitNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Load a new tape (operator interface).
**
**  Parameters:     Name        Description.
**                  params      parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void mt669LoadTape(char *params)
    {
    DevSlot *dp;
    int numParam;
    int channelNo;
    int unitNo;
    TapeBuf *tp;
    FILE *fcb;
    u8 unitMode;

    /*
    **  Operator inserted a new tape.
    */
    numParam = sscanf(params,"%o,%o,%c,%s",&channelNo, &unitNo, &unitMode, str);

    /*
    **  Check parameters.
    */
    if (numParam != 4)
        {
        printf("Not enough or invalid parameters\n");
        return;
        }

    if (channelNo < 0 || channelNo >= MaxChannels)
        {
        printf("Invalid channel no\n");
        return;
        }

    if (unitNo < 0 || unitNo >= MaxUnits)
        {
        printf("Invalid unit no\n");
        return;
        }

    if (unitMode != 'w' && unitMode != 'r')
        {
        printf("Invalid ring mode (r/w)\n");
        return;
        }

    if (str[0] == 0)
        {
        printf("Invalid file name\n");
        return;
        }

    /*
    **  Locate the device control block.
    */
    dp = channelFindDevice((u8)channelNo, DtMt669);
    if (dp == NULL)
        {
        printf("No tape on channel %o\n", channelNo);
        return;
        }

    /*
    **  Check if the unit is even configured.
    */
    if (dp->context[unitNo] == NULL)
        {
        printf("Unit %d not allocated\n", unitNo);
        return;
        }

    /*
    **  Check if the unit has been unloaded.
    */
    if (dp->fcb[unitNo] != NULL)
        {
        printf("Unit %d not unloaded\n", unitNo);
        return;
        }

    /*
    **  Open the file in the requested mode.
    */
    if (unitMode == 'w')
        {
        fcb = fopen(str, "r+b");
        if (fcb == NULL)
            {
            fcb = fopen(str, "w+b");
            }
        }
    else
        {
        fcb = fopen(str, "rb");
        }

    /*
    **  Check if the open succeeded.
    */
    if (fcb == NULL)
        {
        printf("Failed to open %s\n", str);
        return;
        }

    /*
    **  Setup status.
    */
    dp->fcb[unitNo] = fcb;
    tp = (TapeBuf *)dp->context[unitNo];
    tp->unitMode = unitMode;
    tp->newTape = TRUE;
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
 
    if(DEBUG)
        printf("MT669 function %03o unit %d ",funcCode, unitNo);

    switch (funcCode)
        {
    default:
        if(DEBUG)
            printf(" declined\n");
        return(FcDeclined);

    case Fc669Release:
        mt669FuncRelease();
        if(DEBUG)
            printf("\n");
        return(FcProcessed);

    case Fc669ClearUnit:
    case Fc669ClearReserve:
        activeDevice->fcode = 0;
        activeDevice->recordLength = 0;
        tp->recordLength = 0;
        tp->detailedStatus[0] = 0;
        if(DEBUG)
            printf("clear unit\n");
        return(FcProcessed);

    case Fc669StopMotion:
        activeDevice->fcode = 0;
        activeDevice->recordLength = 0;
        tp->recordLength = 0;
        if(DEBUG)
            printf(" stop motion\n");
        return(FcProcessed);

    case Fc669RewindUnload:
        tp->deviceStatus = tp->initialStatus = St669NineTrack;
        fclose(activeDevice->fcb[unitNo]);
        activeDevice->fcb[unitNo] = NULL;
        if(DEBUG)
            printf(" unit unloaded\n");
        return(FcProcessed);

    case Fc669SetReadClipNorm:
    case Fc669SetReadClipHigh:
    case Fc669SetReadClipLow:
        activeDevice->fcode = 0;
        if(DEBUG)
            printf(" set read clip\n");
        return(FcProcessed);

    case Fc669Rewind:
        activeDevice->fcode = funcCode;
        if (tp->initialStatus | St669Ready)
            {
            tp->deviceStatus = tp->initialStatus | St669LoadPoint;
            tp->blockNo = 0;
            fseek(activeDevice->fcb[unitNo], 0, SEEK_SET);
            }
        else
            {
            tp->deviceStatus = tp->initialStatus;
            }
        dc6681UnitStatus = 01121;
        if(DEBUG)
            printf(" rewound\n");
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
        recLen1 = 0;
        tp->bp = tp->ioBuffer;
        position = ftell(activeDevice->fcb[unitNo]);
        if (position > MAXTAPE)
            {
            tp->deviceStatus |= St669EOT;
            if(DEBUG)
                printf("---- EOT Reached ----\n");  
            }   
        tp->blockNo += 1;
        fwrite(&recLen1, sizeof(recLen1), 1, activeDevice->fcb[unitNo]);
        tp->deviceStatus = tp->initialStatus | St669TapeMark;
        if(DEBUG)
            printf(" wrote EOF\n");
        return(FcProcessed);

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
            if(DEBUG)
                printf("---- EOT Reached ----\n");  
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
        if(DEBUG)
            printf("\nLoad Conversion%d\n", funcCode);
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 0;
        break;

    case Fc669DetailedStatus:
        activeDevice->fcode = funcCode;
        activeDevice->recordLength = 8;
        tp->detailedStatus[6] = (PpWord)(tp->blockNo >> 12)  & 07777;
        tp->detailedStatus[7] = (PpWord)tp->blockNo  & 07777;
        if(DEBUG)
            printf(" detailed status ");
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
        if (activeDevice->context[unitNo] == NULL)
            {
            ppAbort((stderr, "channel %02o - invalid select: %04o",
               activeChannel->id, (u32)funcCode));
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
            ppAbort((stderr, "channel %02o - invalid select: %04o,",
                activeChannel->id, (u32)funcCode));
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
    if(DEBUG)
        printf("\n");
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
        ppAbort((stdout, "channel %02o - unsupported function code: %04o",
             activeChannel->id, activeDevice->fcode));
        break;

    case Fc669Rewind:
        dc6681UnitStatus = 01121;
    case Fc669GeneralStatus:
    case Fc669Erase:
        activeChannel->data = tp->deviceStatus;
        if(DEBUG)
            printf(" general status  = %04o\n",tp->deviceStatus);
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
            if(DEBUG)
                printf(" %d = %04o",indx,tp->detailedStatus[indx]);
            activeDevice->recordLength -= 1;
            if( activeDevice->recordLength == 0)
                {
                if(DEBUG)
                    printf("\n");
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
                        if(DEBUG)
                            printf(" No conversion\n");
                        break;
                    case 01:
                        convMode = 1;
                        if(DEBUG)
                            printf(" display <> ascii conv table\n");
                        break;
                    case 02:
                        convMode = 2;
                        if(DEBUG)
                            printf(" display <> ebcdic conv table\n");
                        break;
                    case 03:
                        convMode = 3;
                        if(DEBUG)
                        printf(" use BCD conv table\n");
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
            if(DEBUG)
                printf("table 1 in %04o = %04o %02x",
                activeDevice->recordLength,
                activeChannel->data,
                activeChannel->data
               );
            convTab1in[activeDevice->recordLength] = activeChannel->data & 077;
            if (activeChannel->data & 01000)
                {     
                convTab1out[activeChannel->data & 077] =
                     (u8)activeDevice->recordLength;
                if(DEBUG)
                    printf(" table 1 out %04o = %04o %02x",
                      activeChannel->data & 077,
                      activeDevice->recordLength,
                      activeDevice->recordLength
                    );
                 }  
                if(DEBUG)
                    printf("\n");
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
            if(DEBUG)
                printf(" %d bytes ",recLen0);
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

        fwrite(&recLen1, sizeof(recLen1), 1, fcb);
        fwrite(&rawBuffer, 1, recLen0, fcb);
        fwrite(&recLen1, sizeof(recLen1), 1, fcb);
        if(DEBUG)
            printf("                           ---- Wrote %d frames\n",recLen0);
        tp->deviceStatus &= (PpWord)(07777 - St669LoadPoint);
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
    u32 i;
    u16 c1, c2, c3;
    u16 *op;
    u8 *rp;
    u8 unitNo;
    TapeBuf *tp;
    i32 position;

    unitNo = activeDevice->selectedUnit;
    tp = (TapeBuf *)activeDevice->context[unitNo];
 
    activeDevice->fcode = Fc669ReadFwd;
    activeDevice->recordLength = tp->recordLength = 0;
    tp->detailedStatus[0] = 0;
    tp->deviceStatus = tp->initialStatus;

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
        if (tp->deviceStatus | St669LoadPoint)
            {
            tp->deviceStatus = tp->initialStatus; 
            tp->detailedStatus[0] = 010; /* say blank tape */
            tp->blockNo = 0;
            if(DEBUG)
                printf("---- Blank tape read ----\n");
            }
        else
            {
            tp->deviceStatus = tp->initialStatus | St669EOT;
            if(DEBUG)
                printf("---- Read reached EOT ----\n");
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

    tp->deviceStatus = tp->initialStatus;  /* reset status info */
    if (recLen1 == 0)
        {
        tp->deviceStatus |=  St669TapeMark;
        activeDevice->recordLength = 0;
        tp->recordLength = 0;
        tp->blockNo += 1;
        if(DEBUG)
            printf("---- EOF record ----\n");
        return;
        }

    /*
    **  Check if record length is reasonable.
    */
    if (recLen1 > MaxByteBuf)
        {
        ppAbort((stderr, "channel %02o - tape record too long: %d", activeChannel->id, recLen1));
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        activeDevice->recordLength = 0;
        tp->recordLength = 0;
        return;
        }

    /*
    **  Preset buffer.
    */
    memset(rawBuffer, 0, sizeof(rawBuffer));

    /*
    **  Read and verify the actual raw data.
    */
    len = fread(rawBuffer, 1, recLen1, activeDevice->fcb[unitNo]);

    if (recLen1 != (u32)len)
        {
        ppAbort((stderr, "channel %02o - short tape record read: %d", activeChannel->id, len));
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        activeDevice->recordLength = 0;
        tp->recordLength = 0;
        return;
        }

    /*
    **  Read and verify the TAP record length trailer.
    */
    len = fread(&recLen2, sizeof(recLen2), 1, activeDevice->fcb[unitNo]);

    if (len != 1 || recLen0 != recLen2)
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
            ppAbort((stderr, "channel %02o - invalid tape record trailer: %d", activeChannel->id, recLen2));
            tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
            activeDevice->recordLength = 0;
            tp->recordLength = 0;
            return;
            }
        }

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
        if ((recLen1 % 2 ) == 1)  /* fix odd frame record count */
            {
            recLen1++;
            op[recLen1] = 0;      /* fake data to satisfy 1MT */
            }

        for (i = 0; i < recLen1; i += 3)
            {
            c1 = *rp++;
            c2 = *rp++;
            c3 = *rp++;

            *op++ = ((c1 << 4) | (c2 >> 4)) & Mask12;
            *op++ = ((c2 << 8) | (c3 >> 0)) & Mask12;
            }

        activeDevice->recordLength = op - tp->ioBuffer;
        recLen0 = recLen1 / 3;
        recLen0 = recLen1 - recLen0 * 3;

        switch (recLen0)
           {
        case 0:     /* full 2 pp words */
            break;

        case 1:     /* 2 words + 8 bits */
            activeDevice->recordLength -= 1;
            break;

        case 2:     /* 2 words + 16 bits */
            /* signal lower 8 bits invallid */
            tp->deviceStatus |= St669OddCount;
            break;
           }
        break;

    case 1:
        /*
        **  Convert the Raw data from ascii to display
        */
        for (i = 0; i < recLen1; i += 2)
            {
            c1 = convTab1in[*rp++];
            c2 = (c1 << 6 )  & Mask12 ;
            c3 = convTab1in[*rp++];
            *op = (c2 | c3) & Mask12 ;
            op++;
            }

        activeDevice->recordLength = op - tp->ioBuffer;

        if (recLen1 % 2) 
            {   /* signal lower 6 bits invallid */
            tp->deviceStatus |= St669OddCount;
            }
        break;

    case 2:
        /*
        **  Convert the Raw data from ebcdic to display
        */
        for (i = 0; i < recLen1; i += 2)
            {
            c1 = convTab2in[(u8)*rp++];
            c2 = (c1 << 6 )  & Mask12 ; 
            c3 = convTab2in[(u8)*rp++];
            *op = (c2 | c3) & Mask12 ;  
            op++;
            }

        activeDevice->recordLength = op - tp->ioBuffer;

        if (recLen1 % 2) 
            {   /* signal lower 6 bits invallid */
            tp->deviceStatus |= St669OddCount;
            }
        break;

    case 3:
        /*  
        **  Convert the Raw data from BCD to display
        */
        for (i = 0; i < recLen1; i += 2)
            {
            c1 = convTab3in[(u8)*rp++];
            c2 = (c1 << 6 )  & Mask12 ;
            c3 = convTab3in[(u8)*rp++]; 
            *op = (c2 | c3) & Mask12 ;
            op++;
            }
                
        activeDevice->recordLength = op - tp->ioBuffer;
        if (recLen1 % 2)
            {   /* signal lower 6 bits invallid */
            tp->deviceStatus |= St669OddCount;
            }
        break;
    }

    tp->recordLength = activeDevice->recordLength; /* save */
    tp->blockNo += 1;

    if(DEBUG)
        printf("---- record with %d bytes  and %d PP words----\n", recLen1, activeDevice->recordLength);
#if CcDebug == 1
    sprintf(str, "---- record with %d bytes  and %d PP words----\n", recLen1, activeDevice->recordLength);
    tracePrint(str);
#endif
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
 
    activeDevice->recordLength = tp->recordLength = 0;
    tp->detailedStatus[0] = 0;
    tp->deviceStatus = tp->initialStatus;

    /*
    **  Determine if the tape is at the load point.
    */
    position = ftell(activeDevice->fcb[unitNo]);
    if (position == 0)
        {
        tp->deviceStatus |= St669LoadPoint;
        }

    if(DEBUG)
        printf(" back space from %ld",position);

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
        if (tp->deviceStatus | St669LoadPoint)
            {
            tp->deviceStatus = tp->initialStatus; 
            tp->detailedStatus[0] = 010; /* say blank tape */
            tp->blockNo = 0;
            if(DEBUG)
                printf("---- Blank tape forespace ----\n");
            }
        else
            {
            tp->deviceStatus = tp->initialStatus | St669EOT;
            if(DEBUG)
                printf("---- Forespace reached EOT ----\n");
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

    tp->deviceStatus = tp->initialStatus;  /* reset status info */
    if (recLen1 == 0)
        {
        tp->deviceStatus |=  St669TapeMark;
        tp->blockNo += 1;
        if(DEBUG)
            printf("---- EOF record ----\n");
        return;
        }

    /*
    **  Check if record length is reasonable.
    */
    if (recLen1 > MaxByteBuf)
        {
        ppAbort((stderr, "channel %02o - tape record too long: %d", activeChannel->id, recLen1));
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        return;
        }

    /*
    **  Skip the actual raw data.
    */
    if (fseek(activeDevice->fcb[unitNo], recLen1, SEEK_CUR) != 0)
        {
        ppAbort((stderr, "channel %02o - short tape record read: %d", activeChannel->id, len));
        tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
        return;
        }

    /*
    **  Read and verify the TAP record length trailer.
    */
    len = fread(&recLen2, sizeof(recLen2), 1, activeDevice->fcb[unitNo]);

    if (len != 1 || recLen0 != recLen2)
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
            ppAbort((stderr, "channel %02o - invalid tape record trailer: %d", activeChannel->id, recLen2));
            tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
            return;
            }
        }

    tp->blockNo += 1;

    position = ftell(activeDevice->fcb[unitNo]);
    if(DEBUG)
        printf(" to %ld \n",position);
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
 
    if (tp->initialStatus | St669Ready)
        {
        tp->deviceStatus = tp->initialStatus;
        position = ftell(activeDevice->fcb[unitNo]);
        if(DEBUG)
            printf(" back space from %ld",position);
        fseek(activeDevice->fcb[unitNo], -4, SEEK_CUR);
        len = fread(&recLen0, sizeof(recLen0), 1, activeDevice->fcb[unitNo]);
        fseek(activeDevice->fcb[unitNo], -4, SEEK_CUR);
        if (len != 1)
            {
            ppAbort((stderr, "channel %02o - invalid record length: %d",
                activeChannel->id, len));
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

        position = ftell(activeDevice->fcb[unitNo]);
        if (recLen1 != 0)
            {
            position -= 4 + recLen1;
            fseek(activeDevice->fcb[unitNo], position, SEEK_SET);
            len = fread(&recLen2, sizeof(recLen2), 1, activeDevice->fcb[unitNo]);

            if (len != 1 || recLen0 != recLen2)
                {
                /*
                **  This is more weird shit to deal with "padded" TAP records.
                */
                position -= 1;
                fseek(activeDevice->fcb[unitNo], position, SEEK_SET);
                len = fread(&recLen2, sizeof(recLen2), 1, activeDevice->fcb[unitNo]);

                if (len != 1 || recLen0 != recLen2)
                    {
                    ppAbort((stderr, "channel %02o - invalid record length2: %d %08X != %08X", activeChannel->id, len, recLen0, recLen2));
                    return;
                   }
                }

            fseek(activeDevice->fcb[unitNo], position, SEEK_SET);
            }
        else
            {
            tp->deviceStatus = tp->initialStatus | St669TapeMark;
            }

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

    position = ftell(activeDevice->fcb[unitNo]);
    if(DEBUG)
        printf(" to %ld \n",position);
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
    u16 c1, c2, c3;
    u16 *op;
    u8 *rp;
    u32 i;
    TapeBuf *tp;
    i32 position;

    unitNo = activeDevice->selectedUnit;
    tp = (TapeBuf *)activeDevice->context[unitNo];
 
    activeDevice->recordLength = tp->recordLength = 0;
    tp->deviceStatus = tp->initialStatus;
    position = ftell(activeDevice->fcb[unitNo]);
    if(DEBUG)
        printf(" read backwards from %ld",position);
    if (position == 0)
        {
        tp->deviceStatus = tp->initialStatus | St669LoadPoint;
        return;
        }

    fseek(activeDevice->fcb[unitNo], -4, SEEK_CUR);
    len = fread(&recLen0, sizeof(recLen0), 1, activeDevice->fcb[unitNo]);
    fseek(activeDevice->fcb[unitNo], -4, SEEK_CUR);
    if (len != 1)
        {
        ppAbort((stderr, "channel %02o - invalid record length: %d", activeChannel->id, len));
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

    if (recLen1 != 0) /* not an EOF */
        {
        position -= 8 + recLen1;
        fseek(activeDevice->fcb[unitNo], position, SEEK_SET);
        len = fread(&recLen2, sizeof(recLen2), 1, activeDevice->fcb[unitNo]);
        if (len != 1 || recLen0 != recLen2)
            {
            ppAbort((stderr, "channel %02o - invalid record length2: %d %08X != %08X", activeChannel->id, len, recLen0, recLen2));
            }
        if (recLen1 > MaxByteBuf)
            {
            ppAbort((stderr, "channel %02o - tape record too long: %d",
                activeChannel->id, recLen1));
            tp->deviceStatus = tp->initialStatus | St669Alert | St669Busy;
            activeDevice->recordLength = 0;
            tp->recordLength = 0;
            return;
            }

        /*
        **  Clean out the raw buffer.
        */
        memset(rawBuffer, 0, sizeof(rawBuffer));

        /*
        **  Convert the raw data into PP words suitable for a channel.
        */
        op = tp->ioBuffer;
        rp = rawBuffer;
   
        tp->deviceStatus = tp->initialStatus;
        switch (convMode)
           {
        case 0:
            /*
            **  Convert the raw data into PP Word data.
            */
            if ((recLen1 % 2 ) == 1)  /* fix odd frame record count */
                {
                recLen1++;
                op[recLen1] = 0;      /* fake data to satisfy 1MT */
                }
            for (i = 0; i < recLen1; i += 3)
                {
                c1 = *rp++;
                c2 = *rp++;
                c3 = *rp++;

                *op++ = ((c3 << 4) | (c2 >> 4)) & Mask12;
                *op++ = ((c2 << 8) | (c1 >> 0)) & Mask12;
                }
            activeDevice->recordLength = op - tp->ioBuffer;
            recLen0 = recLen1 / 3;
            recLen0 = recLen1 - recLen0 * 3;
            switch (recLen0)
               {
            case 0:     /* full 2 pp words */
                break;
            case 1:     /* 2 words + 8 bits */
                activeDevice->recordLength -= 1;
                break;
            case 2:     /* 2 words + 16 bits */
                /* signal lower 8 bits invallid */
                tp->deviceStatus |= St669OddCount;
                break;
               }
           break;

        case 1:
            /*
            **  Convert the Raw data from ascii to display
            */
            for (i = 0; i < recLen1; i += 2)
                {
                c1 = convTab1in[(*rp++ & 0x7f)];
                c2 = (c1 << 6 )  & Mask12 ;
                c3 = convTab1in[(*rp++ & 0x7f)];
                *op = (c3 | c2) & Mask12 ;
                op++;
                }
            activeDevice->recordLength = op - tp->ioBuffer;
            if (recLen1 % 2)
                {   /* signal lower 6 bits invallid */
                tp->deviceStatus |= St669OddCount;
                }
            break;
        case 2:
            /*
            **  Convert the Raw data from ebcdic to display
            */
            for (i = 0; i < recLen1; i += 2)
                {
                c1 = convTab2in[(u8)*rp++];
                c2 = (c1 << 6 )  & Mask12 ;
                c3 = convTab2in[(u8)*rp++];
                *op = (c3 | c2) & Mask12 ;
                op++;
                }
            activeDevice->recordLength = op - tp->ioBuffer;
            if (recLen1 % 2)
                {   /* signal lower 6 bits invallid */
                tp->deviceStatus |= St669OddCount;
                }
            break;
        case 3:
             /*
            **  Convert the Raw data from BCD to display
            */
            for (i = 0; i < recLen1; i += 2)
                {
                c1 = convTab3in[(u8)*rp++];
                c2 = (c1 << 6 )  & Mask12 ;
                c3 = convTab3in[(u8)*rp++];
                *op = (c3 | c2) & Mask12 ;
                op++;
                }
            activeDevice->recordLength = op - tp->ioBuffer;
            if (recLen1 % 2)
                {   /* signal lower 6 bits invallid */
                tp->deviceStatus |= St669OddCount;
                }
            break;
        }

        tp->recordLength = activeDevice->recordLength; /* save */
        tp->bp = op;

        if(DEBUG)
            printf("---- rev record with %d bytes  and %d PP words----\n",
              recLen1, activeDevice->recordLength);
        sprintf(str, "---- rev record with %d bytes  and %d PP words----\n",
                recLen1, activeDevice->recordLength);
        fseek(activeDevice->fcb[unitNo], position, SEEK_SET);
        }
    else
        {
        tp->deviceStatus |= St669TapeMark;
        if(DEBUG)
            printf("---- EOF record ----");
        }
    position = ftell(activeDevice->fcb[unitNo]);
    if(DEBUG)
        printf(" to %ld \n",position);
    if (position == 0)
        {
        tp->deviceStatus |= St669LoadPoint;
        tp->blockNo = 0;
        }
    else
        {
        tp->blockNo -= 1;
        }
   
    if(DEBUG)
        printf(" read backwards\n");
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
        if(DEBUG)
            printf(" write reposition from %ld",position);
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
         * read the tail tap header to determine lenght.
         */ 
        fseek(activeDevice->fcb[unitNo], -4, SEEK_CUR);
        len = fread(&recLen0, sizeof(recLen0), 1, activeDevice->fcb[unitNo]);   
        fseek(activeDevice->fcb[unitNo], -4, SEEK_CUR);
        position -= 4; 
        if (len != 1)
            {   
            ppAbort((stderr, "channel %02o - invalid record length: %04o",
                activeChannel->id, (u32)len));
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
    
        if (recLen1 != 0) /* not an EOF */
            {
            position -= 4 + recLen1;
            fseek(activeDevice->fcb[unitNo], position, SEEK_SET);
            }
        else
            {
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

