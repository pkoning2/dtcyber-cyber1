/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Paul Koning, Tom Hunter (see license.txt)
**
**  Name: cr3447.c
**
**  Description:
**      Perform simulation of CDC 3447 card reader controller.
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
#include <errno.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/

#define DEBUG 0

/*
**  CDC 3447 card reader function and status codes.
**  
**      Function codes
**
**      ----------------------------------
**      |  Equip select  |   function    |
**      ----------------------------------
**      11              6 5             0
**
**      0700 = Deselect
**      0701 = Gate Card to Secondary bin
**      0702 = Read Non-stop
**      0704 = Status request
**
**      Note: To read one card, execute successive S702 and
**      S704 functions.
**      One column of card data per 12-bit data word.
*/
#define FcCr3447Deselect         00000
#define FcCr3447Binary           00001
#define FcCr3447BCD              00002
#define FcCr3447SetGateCard      00004
#define FcCr3447Clear            00005
#define FcCr3447IntReady         00020
#define FcCr3447NoIntReady       00021
#define FcCr3447IntEoi           00022
#define FcCr3447NoIntEoi         00023
#define FcCr3447IntError         00024
#define FcCr3447NoIntError       00025
#define Fc6681DevStatusReq       01300
#define Fc6681InputToEor         01400
#define Fc6681Input              01500

/*
**      Status reply flags
**
**      0001 = Ready
**      0002 = Busy
**      0004 = Binary card (7/9 punch)
**      0010 = File card (7/8 punch)
**      0020 = Jam
**      0040 = Input tray empty
**      0100 = End of file
**      0200 = Ready interrupt
**      0400 = EOI interrupt
**      1000 = Error interrupt
**      2000 = Read compare error
**      4000 = Reserved by other controller (3649 only)
**
*/
#define StCr3447Ready            00201  // includes ReadyInt
#define StCr3447Busy             00002
#define StCr3447Binary           00004
#define StCr3447File             00010
#define StCr3447Empty            00040
#define StCr3447Eof              01540  // includes Empty, EoiInt, ErrorInt
#define StCr3447ReadyInt         00200
#define StCr3447EoiInt           00400
#define StCr3447ErrorInt         01000
#define StCr3447CompareErr       02000
#define StCr3447NonIntStatus     02177

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
    bool    binary;
    bool    bincard;
    int     intmask;
    int	    status;
    int     col;
    char    card[82];
} CrContext;

    
/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus cr3447Func(PpWord funcCode);
static void cr3447Io(void);
static void cr3447Load(DevSlot *, int ,char *);
static void cr3447Activate(void);
static void cr3447Disconnect(void);
static void cr3447NextCard (DevSlot *dp, int unitNo, CrContext *cc);
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

static const unsigned short asc2pnch_table[256] =
{
 /*                                                                         */
 /* 000- */  0,      0,      0,      0,      0,      0,      0,      0,
 /*                                                                         */
 /* 010- */  0,      0,      0,      0,      0,      0,      0,      0,
 /*                                                                         */
 /* 020- */  0,      0,      0,      0,      0,      0,      0,      0,
 /*                                                                         */
 /* 030- */  0,      0,      0,      0,      0,      0,      0,      0,
 /*          space   !       "       #       $       %       &       '      */
 /* 040- */  0,      0,      0,      0,      02102,  0,      0,      0,
 /*          (       )       *       +       ,       -       .       /      */
 /* 050- */  01042,  04042,  02042,  04000,  01102,  02000,  04102,  01400,
 /*          0       1       2       3       4       5       6       7      */
 /* 060- */  01000,  00400,  00200,  00100,  00040,  00020,  00010,  00004,
 /*          8       9       :       ;       <       =       >       ?      */
 /* 070- */  00002,  00001,  0,      0,      0,      00102,  0,      0,
 /*          @       A       B       C       D       E       F       G      */
 /* 100- */  0,      04400,  04200,  04100,  04040,  04020,  04010,  04004,
 /*          H       I       J       K       L       M       N       O      */
 /* 110- */  04002,  04001,  02400,  02200,  02100,  02040,  02020,  02010,
 /*          P       Q       R       S       T       U       V       W      */
 /* 120- */  02004,  02002,  02001,  01200,  01100,  01040,  01020,  01010,
 /*          X       Y       Z       [       \       ]       ^       _      */
 /* 130- */  01004,  01002,  01001,  0,      0,      0,      0,      00042,
 /*          `       a       b       c       d       e       f       g      */
 /* 140- */  0,      04400,  04200,  04100,  04040,  04020,  04010,  04004,
 /*          h       i       j       k       l       m       n       o      */
 /* 150- */  04002,  04001,  02400,  02200,  02100,  02040,  02020,  02010,
 /*          p       q       r       s       t       u       v       w      */
 /* 160- */  02004,  02002,  02001,  01200,  01100,  01040,  01020,  01010,
 /*          x       y       z       {       |       }       ~              */
 /* 170- */  01004,  01002 , 01001,  0,      0,      00017,  00007,  0
};

/* 077 means invalid code */
static const unsigned char  asc2bcd_table[256] =
{
 /*                                                                         */
 /* 000- */  077,    077,    077,    077,    077,    077,    077,    077,
 /*                                                                         */
 /* 010- */  077,    077,    077,    077,    077,    077,    077,    077,
 /*                                                                         */
 /* 020- */  077,    077,    077,    077,    077,    077,    077,    077,
 /*                                                                         */
 /* 030- */  077,    077,    077,    077,    077,    077,    077,    077,
 /*          space   !       "       #       $       %       &       '      */
 /* 040- */  060,    077,    077,    077,    053,    077,    077,    077,
 /*          (       )       *       +       ,       -       .       /      */
 /* 050- */  074,    034,    054,    020,    073,    040,    033,    061,
 /*          0       1       2       3       4       5       6       7      */
 /* 060- */  000,    001,    002,    003,    004,    005,    006,    007,
 /*          8       9       :       ;       <       =       >       ?      */
 /* 070- */  010,    011,    077,    077,    077,    013,    077,    077,
 /*          @       A       B       C       D       E       F       G      */
 /* 100- */  077,    021,    022,    023,    024,    025,    026,    027,
 /*          H       I       J       K       L       M       N       O      */
 /* 110- */  030,    031,    041,    042,    043,    044,    045,    046,
 /*          P       Q       R       S       T       U       V       W      */
 /* 120- */  047,    050,    051,    062,    063,    064,    065,    066,
 /*          X       Y       Z       [       \       ]       ^       _      */
 /* 130- */  067,    070,    071,    077,    077,    077,    077,    014,
 /*          `       a       b       c       d       e       f       g      */
 /* 140- */  077,    021,    022,    023,    024,    025,    026,    027,
 /*          h       i       j       k       l       m       n       o      */
 /* 150- */  030,    031,    041,    042,    043,    044,    045,    046,
 /*          p       q       r       s       t       u       v       w      */
 /* 160- */  047,    050,    051,    062,    063,    064,    065,    066,
 /*          x       y       z       {       |       }       ~              */
 /* 170- */  067,    070,    071,    077,    077,    077,    077,    077
};


/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Initialise card reader.
**
**  Parameters:     Name        Description.
**                  eqNo        equipment number
**                  unitCount   number of units to initialise
**                  channelNo   channel number the device is attached to
**                  deviceName  optional device file name
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cr3447Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
{
    DevSlot *dp;
    CrContext *cc;
    FILE *fcb;
    
    (void)eqNo;

    dp = channelAttach(channelNo, DtCr3447);

    dp->activate = cr3447Activate;
    dp->disconnect = cr3447Disconnect;
    dp->func = cr3447Func;
    dp->io = cr3447Io;
    dp->load = cr3447Load;
    dp->selectedUnit = unitNo;
    cc = calloc (1, sizeof (CrContext));
    if (cc == NULL)
    {
        fprintf (stderr, "Failed to allocate CR3447 context block\n");
        exit (1);
    }
    dp->context[unitNo] = cc;

    if (deviceName != NULL)
    {
        fcb = fopen(deviceName, "r");
        if (fcb == NULL)
        {
            fprintf(stderr, "Failed to open %s\n", deviceName);
            exit(1);
        }

        dp->fcb[unitNo] = fcb;
        cc->status = StCr3447Ready;
        cr3447NextCard (dp, unitNo, cc);        // Read the first card
    }
    else
    {
        dp->fcb[unitNo] = NULL;
        cc->status = StCr3447Eof;
    }

    /*
    **  Initialise DCC6681 on this channel.
    */
    dcc6681Init(channelNo);

    /*
    **  Print a friendly message.
    */
    printf("CR3447 initialised on channel %o equipment %o\n", 
           channelNo, unitNo);
}

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 3447 card reader.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus cr3447Func(PpWord funcCode)
{
    CrContext *cc;
    u8 unitNo;
    FcStatus st;
    PpWord s;
    
    if (DEBUG)
        printf ("cr3447: function %04o\n", funcCode);
    
    /*
    ** Let the data channel converter have a pass over the function code first.
    */
    st = dcc6681Func(funcCode);
    if (st != FcDeclined)
    {
        return(st);
    }

    unitNo = activeDevice->selectedUnit;
    cc = (CrContext *)activeDevice->context[unitNo];

    if (DEBUG)
    {
        traceMask |= 1<<activePpu->id;
        traceClearMask |= 1<<activePpu->id;
    }
    
    switch (funcCode)
    {
    default:                    // all unrecognized codes are NOPs
        st = FcProcessed;
        break;

    case FcCr3447SetGateCard:
        st = FcProcessed;
        break;

    case Fc6681InputToEor:
    case Fc6681Input:
        cc->status = StCr3447Ready;
        if (DEBUG)
        {
            traceMask |= 1<<activePpu->id;
            traceClearMask |= 1<<activePpu->id;
        }
        // fall through
    case Fc6681DevStatusReq:
        st = FcAccepted;
        activeDevice->fcode = funcCode;
        break;

    case FcCr3447Binary:
        cc->binary = TRUE;
        st = FcProcessed;
        break;
        
    case FcCr3447Deselect:
    case FcCr3447Clear:
        cc->intmask = 0;
        // fall through
    case FcCr3447BCD:
        cc->binary = FALSE;
        st = FcProcessed;
        break;
        
    case FcCr3447IntReady:
        cc->intmask |= StCr3447ReadyInt;
        cc->status &= ~StCr3447ReadyInt;
        st = FcProcessed;
        break;
        
    case FcCr3447NoIntReady:
        cc->intmask &= ~StCr3447ReadyInt;
        cc->status &= ~StCr3447ReadyInt;
        st = FcProcessed;
        break;
        
    case FcCr3447IntEoi:
        cc->intmask |= StCr3447EoiInt;
        cc->status &= ~StCr3447EoiInt;
        st = FcProcessed;
        break;
        
    case FcCr3447NoIntEoi:
        cc->intmask &= ~StCr3447EoiInt;
        cc->status &= ~StCr3447EoiInt;
        st = FcProcessed;
        break;

    case FcCr3447IntError:
        cc->intmask |=StCr3447ErrorInt;
        cc->status &= ~StCr3447ErrorInt;
        st = FcProcessed;
        break;

    case FcCr3447NoIntError:
        cc->intmask &= ~StCr3447ErrorInt;
        cc->status &= ~StCr3447ErrorInt;
        st = FcProcessed;
        break;
    }
    if ((cc->status & cc->intmask) == 0)
        dcc6681ClearInt (unitNo);

    return(st);
}

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 3447 card reader.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void cr3447Io(void)
{
    CrContext *cc;
    u8 unitNo;
    char *cp;
    char c;
    PpWord p;
    
    unitNo = activeDevice->selectedUnit;
    cc = (CrContext *)activeDevice->context[unitNo];

    /*
    **  Process any data channel converter I/O.
    */
    if (dcc6681Io())
    {
        return;
    }
    
    if (DEBUG)
        printf ("cr3447: i/o %04o ", activeDevice->fcode);

    switch (activeDevice->fcode)
    {
    default:
        printf("unexpected IO for function %04o\n", activeDevice->fcode); 
        break;

    case 0:
        break;

    case Fc6681DevStatusReq:
        if (!activeChannel->full)
        {
            activeChannel->data = (cc->status & (cc->intmask | StCr3447NonIntStatus));
            activeChannel->full = TRUE;
        }
        break;
        
    case Fc6681InputToEor:
    case Fc6681Input:
        if (activeChannel->full)
        {
            break;
        }

        if (activeDevice->fcb[unitNo] == NULL)
        {
            cc->status = StCr3447Eof;
            break;
        }
        
        if (cc->col >= 80)
        {
            // Read the next card.
            // If the function is input to EOR, disconnect to indicate EOR
            cr3447NextCard (activeDevice, unitNo, cc);
            if (activeDevice->fcode == Fc6681InputToEor)
            {
                // End of card but we're still ready
                cc->status |= StCr3447EoiInt | StCr3447Ready;
                if (cc->status & StCr3447File)
                    cc->status |= StCr3447ErrorInt;
                activeChannel->discAfterInput = TRUE;
            }
        }
        else
        {
            c = cc->card[cc->col++];
            if (cc->binary || cc->bincard)
            {
                p = asc2pnch_table[c];
            }
            else
            {
                p = asc2bcd_table[c] << 6;
                c = cc->card[cc->col++];
                p += asc2bcd_table[c];
            }
            activeChannel->data = p;
            activeChannel->full = TRUE;
        }
        break;
    }
    if (DEBUG)
        printf ("data %04o, status now %04o\n", activeChannel->data, cc->status);
    if ((cc->status & cc->intmask) != 0)
        dcc6681SetInt (unitNo);
}

/*--------------------------------------------------------------------------
**  Purpose:        Perform load/unload on 3447 card reader.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void cr3447Load(DevSlot *dp, int unitNo, char *fn)
{
    CrContext *cc;
    FILE *fcb;
    static char msgBuf[80];
    
    if (unitNo < 0 || unitNo >= MaxUnits)
    {
        opSetMsg ("$INVALID UNIT NO");
        return;
    }
    cc = (CrContext *)dp->context[unitNo];

    if (cc == NULL)
    {
        opSetMsg ("$UNIT NOT ALLOCATED");
        return;
    }

    if (dp->fcb[unitNo] != NULL)
        fclose (dp->fcb[unitNo]);
    
    dp->fcb[unitNo] = NULL;
    cc->status = StCr3447Eof;

    if (fn != NULL)
    {
        fcb = fopen(fn, "r");
        if (fcb == NULL)
        {
            sprintf (msgBuf, "$Open error: %s", strerror (errno));
            opSetMsg(msgBuf);
            return;
        }

        dp->fcb[unitNo] = fcb;
        cc->status = StCr3447Ready;
        cr3447NextCard (dp, unitNo, cc);
    }
    if ((cc->status & cc->intmask) != 0)
        dcc6681SetInt (unitNo);
    if (fn == NULL)
        opSetMsg ("CR3447 unloaded");
    else
    {
        sprintf (msgBuf, "CR3447 loaded with %s", fn);
        opSetMsg (msgBuf);
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
static void cr3447Activate(void)
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
static void cr3447Disconnect(void)
{
    CrContext *cc;
    u8 unitNo;
    
    unitNo = activeDevice->selectedUnit;
    cc = (CrContext *)activeDevice->context[unitNo];
    if (cc != NULL)
    {
        cc->status |= StCr3447EoiInt;
        if ((cc->status & cc->intmask) != 0)
            dcc6681SetInt (unitNo);
        if (activeDevice->fcb[unitNo] != NULL && cc->col != 0)
            cr3447NextCard (activeDevice, unitNo, cc);
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Read next card, update card reader status.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void cr3447NextCard (DevSlot *dp, int unitNo, CrContext *cc)
{
    char *cp;
    char c;

    // Read the next card.
    cp = fgets (cc->card, sizeof (cc->card),
                dp->fcb[unitNo]);
    if (cp == NULL)
    {
        // If the last card wasn't a 6/7/8/9 card, fake one.
        if (cc->card[0] != '}')
        {
            strcpy (cc->card, "}\n");
        }
        else
        {
            fclose (dp->fcb[unitNo]);
            dp->fcb[unitNo] = NULL;
            cc->status |= StCr3447Eof;
            return;
        }
    }
    // Set "this card is binary" if rows 7 and 9 are punched in column 1.
    cc->bincard = ((asc2pnch_table[cc->card[0]] & 005) == 005);
    if (DEBUG)
        printf ("read card (binary: %d): %s", cc->bincard, cc->card);
    if ((cp = strchr (cc->card, '\n')) == NULL)
    {
        do 
        {
            c = fgetc (dp->fcb[unitNo]);
        } while (c != '\n');
        cp = &cc->card[80];
    }
    for ( ; cp <= &cc->card[80]; cp++)
        *cp = ' ';
    cc->col = 0;
    if (!cc->binary &&
        (asc2pnch_table[cc->card[0]] & 006) == 006)
    {
        cc->status |= StCr3447File;
    }
    if (cc->bincard)
        cc->status |= StCr3447Binary;
    if (DEBUG)
        printf ("status for card: %04o\n", cc->status);
}

/*---------------------------  End Of File  ------------------------------*/
