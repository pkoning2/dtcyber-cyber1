/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Paul Koning, Tom Hunter (see license.txt)
**
**  Name: cp3446.c
**
**  Description:
**      Perform simulation of CDC 3446 card punch controller.
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
#include <time.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/

#define DEBUG 0
#define CP_LC 0

/*
**  CDC 3446 card punch function and status codes.
*/
#define FcCp3446Deselect         00000
#define FcCp3446Binary           00001
#define FcCp3446BCD              00002
#define FcCp3446SelectOffset     00003
#define FcCp3446CheckLastCard    00004
#define FcCp3446Clear            00005
#define FcCp3446IntReady         00020
#define FcCp3446NoIntReady       00021
#define FcCp3446IntEoi           00022
#define FcCp3446NoIntEoi         00023
#define FcCp3446IntError         00024
#define FcCp3446NoIntError       00025
#define Fc6681DevStatusReq       01300
#define Fc6681Output             01600
#define Fc6681MasterClear        01700

/*
**      Status reply flags
**
**      0001 = Ready
**      0002 = Busy
**      0100 = Failed to feed
**      0200 = Ready interrupt
**      0400 = EOI interrupt
**      1000 = Error interrupt
**      2000 = Compare error
**      4000 = Reserved by other controller (3644 only)
**
*/
#define StCp3446Ready            00201  // includes ReadyInt
#define StCp3446Busy             00002
#define StCp3446ReadyInt         00200
#define StCp3446EoiInt           00400
#define StCp3446ErrorInt         01000
#define StCp3446CompareErr       02000
#define StCp3446NonIntStatus     02177

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
    int     intmask;
    int	    status;
    int     col;
    int     lastnbcol;
    const u16 *table;
    u32     getcardcycle;
    char    card[82];
} CrContext;

    
/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus cp3446Func(PpWord funcCode);
static void cp3446Io(void);
static void cp3446Activate(void);
static void cp3446Disconnect(void);
static void cp3446FlushCard (DevSlot *up, CrContext *cc);
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
**  Purpose:        Initialise card punch.
**
**  Parameters:     Name        Description.
**                  eqNo        equipment number
**                  unitCount   number of units to initialise
**                  channelNo   channel number the device is attached to
**                  deviceName  optional card output file name, 
**                              may be followed by comma and"026" (default)
**                              or "029" to select translation mode
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cp3446Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    DevSlot *up;
    char fname[80];
    CrContext *cc;
    
    up = dcc6681Attach(channelNo, eqNo, 0, DtCp3446);

    up->activate = cp3446Activate;
    up->disconnect = cp3446Disconnect;
    up->func = cp3446Func;
    up->io = cp3446Io;

    /*
    **  Only one card punch unit is possible per equipment.
    */
    if (up->context[0] != NULL)
        {
        fprintf (stderr, "Only one CP3446 unit is possible per equipment\n");
        exit (1);
        }

    cc = calloc (1, sizeof (CrContext));
    if (cc == NULL)
        {
        fprintf (stderr, "Failed to allocate CP3446 context block\n");
        exit (1);
        }

    up->context[0] = (void *)cc;
    cc->lastnbcol = -1;
    cc->col = 0;
    cc->status = StCp3446Ready;
    
    /*
    **  Open the device file.
    */
    sprintf(fname, "CP3446_C%02o_E%o", channelNo, eqNo);
    up->fcb[0] = fopen(fname, "w");
    if (up->fcb[0] == NULL)
        {
        fprintf(stderr, "Failed to open %s\n", fname);
        exit(1);
        }

    /*
    **  Setup character set translation table.
    */
    cc->table = asciiTo026;     // default translation table
    if (deviceName != NULL)
        {
        if (strcmp (deviceName, "029") == 0)
            {
            cc->table = asciiTo029;
            }
        else if (strcmp (deviceName, "026") != 0)
            {
            fprintf (stderr, "Unrecognized card code name %s\n", deviceName);
            exit (1);
            }
        }
    
    /*
    **  Print a friendly message.
    */
    printf("CP3446 initialised on channel %o equipment %o\n", channelNo, eqNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Remove cards from 3446 card punch.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cp3446RemoveCards(char *params)
    {
    CrContext *cc;
    DevSlot *dp;
    int numParam;
    int channelNo;
    int equipmentNo;
    FILE *fcb;
    time_t currentTime;
    struct tm t;
    char fname[80];
    char fnameNew[80];
    static char msgBuf[80];

    /*
    **  Operator wants to remove cards.
    */
    numParam = sscanf(params,"%o,%o",&channelNo, &equipmentNo);

    /*
    **  Check parameters.
    */
    if (numParam != 2)
        {
        printf("Not enough or invalid parameters\n");
        return;
        }

    if (channelNo < 0 || channelNo >= MaxChannels)
        {
        printf("Invalid channel no\n");
        return;
        }

    if (equipmentNo < 0 || equipmentNo >= MaxEquipment)
        {
        printf("Invalid equipment no\n");
        return;
        }

    /*
    **  Locate the device control block.
    */
    dp = dcc6681FindDevice((u8)channelNo, (u8)equipmentNo, DtCp3446);
    if (dp == NULL)
        {
        printf("No card punch on channel %o and equipment %o\n", channelNo, equipmentNo);
        return;
        }

    /*
    **  Close the old device file.
    */
    cc = (CrContext *) (dp->context[0]);
    cp3446FlushCard (dp, cc);
    fflush(dp->fcb[0]);
    fclose(dp->fcb[0]);
    dp->fcb[0] = NULL;

    /*
    **  Rename the device file to the format "CP3446_yyyymmdd_hhmmss".
    */
    sprintf(fname, "CP3446_C%02o_E%o", channelNo, equipmentNo);

    time(&currentTime);
    t = *localtime(&currentTime);
    sprintf(fnameNew, "CP3446_%04d%02d%02d_%02d%02d%02d",
            t.tm_year + 1900,
            t.tm_mon + 1,
            t.tm_mday,
            t.tm_hour,
            t.tm_min,
            t.tm_sec);

    if (rename(fname, fnameNew) != 0)
        {
        printf("Could not rename %s to %s - %s\n", fname, fnameNew, strerror(errno));
        return;
        }

    /*
    **  Open the device file.
    */
    fcb = fopen(fname, "w");

    /*
    **  Check if the open succeeded.
    */
    if (fcb == NULL)
        {
        printf("Failed to open %s\n", fname);
        return;
        }

    printf("Punch cards removed from 3446 card puncher\n");

    /*
    **  Setup status.
    */
    dp->fcb[0] = fcb;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 3446 card punch.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus cp3446Func(PpWord funcCode)
    {
    CrContext *cc;
    FcStatus st;
    
    if (DEBUG)
        printf ("cp3446: function %04o\n", funcCode);
    
    cc = (CrContext *)active3000Device->context[0];

    switch (funcCode)
        {
    default:                    // all unrecognized codes are NOPs
        st = FcProcessed;
        break;

    case FcCp3446CheckLastCard: // this is sent at end of deck, so flush
        fflush(active3000Device->fcb[0]);
        // fall through
    case FcCp3446SelectOffset:
    case Fc6681MasterClear:
        st = FcProcessed;
        break;

    case Fc6681Output:
        cc->status = StCp3446Ready;
        // fall through
    case Fc6681DevStatusReq:
        st = FcAccepted;
        active3000Device->fcode = funcCode;
        break;

    case FcCp3446Binary:
        cc->binary = TRUE;
        st = FcProcessed;
        break;
        
    case FcCp3446Deselect:
    case FcCp3446Clear:
        cc->intmask = 0;
        // fall through
    case FcCp3446BCD:
        cc->binary = FALSE;
        st = FcProcessed;
        break;
        
    case FcCp3446IntReady:
        cc->intmask |= StCp3446ReadyInt;
        cc->status &= ~StCp3446ReadyInt;
        st = FcProcessed;
        break;
        
    case FcCp3446NoIntReady:
        cc->intmask &= ~StCp3446ReadyInt;
        cc->status &= ~StCp3446ReadyInt;
        st = FcProcessed;
        break;
        
    case FcCp3446IntEoi:
        cc->intmask |= StCp3446EoiInt;
        cc->status &= ~StCp3446EoiInt;
        st = FcProcessed;
        break;
        
    case FcCp3446NoIntEoi:
        cc->intmask &= ~StCp3446EoiInt;
        cc->status &= ~StCp3446EoiInt;
        st = FcProcessed;
        break;

    case FcCp3446IntError:
        cc->intmask |=StCp3446ErrorInt;
        cc->status &= ~StCp3446ErrorInt;
        st = FcProcessed;
        break;

    case FcCp3446NoIntError:
        cc->intmask &= ~StCp3446ErrorInt;
        cc->status &= ~StCp3446ErrorInt;
        st = FcProcessed;
        break;
        }

    dcc6681Interrupt((cc->status & cc->intmask) != 0);
    return(st);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 3446 card punch.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void cp3446Io(void)
    {
    CrContext *cc;
    char c;
    PpWord p;
    int i;
    
    cc = (CrContext *)active3000Device->context[0];

    if (DEBUG)
        printf ("cp3446: i/o %04o ", active3000Device->fcode);

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
            activeChannel->data = (cc->status & (cc->intmask | StCp3446NonIntStatus));
            activeChannel->full = TRUE;
            }
        break;
        
    case Fc6681Output:
        // Don't admit to having new data immediately after completing
        // a card, otherwise 1CD may get stuck occasionally.
        // So we simulate card in motion for 20 major cycles.
        if (!activeChannel->full ||
            cycles - cc->getcardcycle < 20)
            {
            break;
            }

        if (cc->col >= 80)
            {
            // Write the card we just finished.
            cp3446FlushCard (active3000Device, cc);
            }
        else
            {
            p = activeChannel->data;
            activeChannel->full = FALSE;
            
            if (cc->binary)
                {
                c = ' ';
                // Sorry about the linear search; it's either that
                // or a substantially hairier translation.
                for (i = 040; i < 0177; i++)
                    {
                    if (cc->table[i] == p)
                        {
                        c = i;
                        break;
                        }
                    }    
                }
            else
                {
                c = bcdToAscii[p >> 6];
#if (CP_LC == 1)
                c = tolower (c);
#endif
                if ((cc->card[cc->col] = c) != ' ')
                    {
                    cc->lastnbcol = cc->col;
                    }

                cc->col++;
                c = bcdToAscii[p & 077];
                }
#if (CP_LC == 1)
            c = tolower (c);
#endif
            if ((cc->card[cc->col] = c) != ' ')
                {
                cc->lastnbcol = cc->col;
                }

            cc->col++;
            }
        break;
        }
    
    if (DEBUG)
        {
        printf("data %04o, status now %04o\n", activeChannel->data, cc->status);
        }

    dcc6681Interrupt((cc->status & cc->intmask) != 0);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle channel activation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void cp3446Activate(void)
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
static void cp3446Disconnect(void)
    {
    CrContext *cc;
    
    cc = (CrContext *)active3000Device->context[0];
    if (cc != NULL)
        {
        cc->status |= StCp3446EoiInt;
        dcc6681Interrupt((cc->status & cc->intmask) != 0);
        if (active3000Device->fcb[0] != NULL && cc->col != 0)
            {
            cp3446FlushCard(active3000Device, cc);
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Punch current card, update card punch status.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void cp3446FlushCard(DevSlot *up, CrContext *cc)
    {
    int lc;
    
    if (cc->col == 0)
        {
        return;
        }
    
    // Remember the cycle counter when the card punch started
    cc->getcardcycle = cycles;
    
    // Put in the line terminator, omitting trailing blanks
    lc = cc->lastnbcol + 1;
    cc->card[lc++] = '\n';

    // Write the line and reset things for next card
    fwrite(cc->card, 1, lc, up->fcb[0]);
    cc->col = 0;
    cc->lastnbcol = -1;
    }

/*---------------------------  End Of File  ------------------------------*/

