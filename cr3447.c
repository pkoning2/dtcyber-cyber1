/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Paul Koning, Tom Hunter (see license.txt)
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
**  CDC 3447 card reader function and status codes.
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
    const u16 *table;
    u32     getcardcycle;
    char    card[82];
    } CrContext;

    
/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus cr3447Func(PpWord funcCode);
static void cr3447Io(void);
static void cr3447Load(DevSlot *, int, char *);
static void cr3447Activate(void);
static void cr3447Disconnect(void);
static void cr3447NextCard (DevSlot *up, CrContext *cc);
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
**                  deviceName  optional card source file name, 
**                              may be followed by comma and"026" (default)
**                              or "029" to select translation mode
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cr3447Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    DevSlot *up;
    CrContext *cc;
    FILE *fcb;
    char *opt;
    
    up = dcc6681Attach(channelNo, eqNo, 0, DtCr3447);

    up->activate = cr3447Activate;
    up->disconnect = cr3447Disconnect;
    up->func = cr3447Func;
    up->io = cr3447Io;
    up->load = cr3447Load;

    /*
    **  Only one card reader unit is possible per equipment.
    */
    if (up->context[0] != NULL)
        {
        fprintf (stderr, "Only one CR3447 unit is possible per equipment\n");
        exit (1);
        }

    cc = calloc (1, sizeof (CrContext));
    if (cc == NULL)
        {
        fprintf (stderr, "Failed to allocate CR3447 context block\n");
        exit (1);
        }
    up->context[0] = cc;

    if (deviceName != NULL)
        {
        opt = strchr (deviceName, ',');
        if (opt != NULL)
            {
            *opt++ = '\0';
            }
        fcb = fopen(deviceName, "r");
        if (fcb == NULL)
            {
            fprintf(stderr, "Failed to open %s\n", deviceName);
            exit(1);
            }

        up->fcb[0] = fcb;
        cc->status = StCr3447Ready;
        cr3447NextCard (up, cc);        // Read the first card
        }
    else
        {
        opt = NULL;
        up->fcb[0] = NULL;
        cc->status = StCr3447Eof;
        }

    cc->table = asciiTo026;     // default translation table
    if (opt != NULL)
        {
        if (strcmp (opt, "029") == 0)
            {
            cc->table = asciiTo029;
            }
        else if (strcmp (opt, "026") != 0)
            {
            fprintf (stderr, "Unrecognized card code name %s\n", opt);
            exit (1);
            }
        }
    else
        {
        opt = "026";
        }
    
    /*
    **  Print a friendly message.
    */
    printf("CR3447 initialised on channel %o equipment %o, default code %s\n", 
           channelNo, eqNo, opt);
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
    FcStatus st;
    
    cc = (CrContext *)active3000Device->context[0];

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
        // fall through
    case Fc6681DevStatusReq:
        st = FcAccepted;
        active3000Device->fcode = funcCode;
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

    dcc6681Interrupt((cc->status & cc->intmask) != 0);
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
    char c;
    PpWord p;
    
    cc = (CrContext *)active3000Device->context[0];

    switch (active3000Device->fcode)
        {
    default:
        printf("unexpected IO for function %04o\n", active3000Device->fcode); 
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
        // Don't admit to having new data immediately after completing
        // a card, otherwise 1CD may get stuck occasionally.
        // So we simulate card in motion for 20 major cycles.
        if (   activeChannel->full
            || cycles - cc->getcardcycle < 20)
            {
            break;
            }

        if (active3000Device->fcb[0] == NULL)
            {
            cc->status = StCr3447Eof;
            break;
            }
        
        if (cc->col >= 80)
            {
            // Read the next card.
            // If the function is input to EOR, disconnect to indicate EOR
            cr3447NextCard (active3000Device, cc);
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
                p = cc->table[c];
                }
            else
                {
                p = asciiToBcd[c] << 6;
                c = cc->card[cc->col++];
                p += asciiToBcd[c];
                }
            activeChannel->data = p;
            activeChannel->full = TRUE;
            }
        break;
        }
    
    dcc6681Interrupt((cc->status & cc->intmask) != 0);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform load/unload on 3447 card reader.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void cr3447Load(DevSlot *up, int eqNo, char *fn)
    {
    CrContext *cc;
    FILE *fcb;
    static char msgBuf[80];
    
    cc = (CrContext *) (up->context[0]);

    if (up->fcb[0] != NULL)
        fclose (up->fcb[0]);
    
    up->fcb[0] = NULL;
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

        up->fcb[0] = fcb;
        cc->status = StCr3447Ready;
        cr3447NextCard (up, cc);
        }
    
    dcc6681Interrupt((cc->status & cc->intmask) != 0);
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
    
    cc = (CrContext *)active3000Device->context[0];
    if (cc != NULL)
        {
        cc->status |= StCr3447EoiInt;
        dcc6681Interrupt((cc->status & cc->intmask) != 0);
        if (active3000Device->fcb[0] != NULL && cc->col != 0)
            {
            cr3447NextCard(active3000Device, cc);
            }
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
static void cr3447NextCard (DevSlot *up, CrContext *cc)
    {
    char *cp;
    char c;

    /* 
    **  Remember the cycle counter when the card was called for.
    */
    cc->getcardcycle = cycles;
    
    /*
    **  Read the next card.
    */
    cp = fgets (cc->card, sizeof (cc->card), up->fcb[0]);
    if (cp == NULL)
        {
        /*
        **  If the last card wasn't a 6/7/8/9 card, fake one.
        */
        if (cc->card[0] != '}')
            {
            strcpy (cc->card, "}\n");
            }
        else
            {
            fclose (up->fcb[0]);
            up->fcb[0] = NULL;
            cc->status = StCr3447Eof;
            return;
            }
        }

    /* 
    **  Set "this card is binary" if rows 7 and 9 are punched in column 1.
    */
    cc->bincard = (cc->table[cc->card[0]] & 005) == 005;

    /*
    **  Skip over any characters past column 80 (if line is longer).
    */
    if ((cp = strchr (cc->card, '\n')) == NULL)
        {
        do 
            {
            c = fgetc(up->fcb[0]);
            } while (c != '\n' && c != EOF);
        cp = &cc->card[80];
        }

    /*
    **  Blank fill line shorter then 80 characters.
    */
    for ( ; cp <= &cc->card[80]; cp++)
        {
        *cp = ' ';
        }

    cc->col = 0;
    if (   !cc->binary
        && (cc->table[cc->card[0]] & 006) == 006)
        {
        cc->status |= StCr3447File;
        }

    if (cc->bincard)
        {
        cc->status |= StCr3447Binary;
        }
    }

/*---------------------------  End Of File  ------------------------------*/
