/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: lp3000.c
**
**  Description:
**      Perform simulation of CDC 3000 series printers/controllers.
**
**      This combines the 501 and 512 printers, and the 3152 and 3555
**      controllers, because they all look pretty similar.
**
**      501 vs. 512 is selected by which Init function is called from 
**      init.c via the device table; 3555 is the default but 3152/3256/3659
**      emulation can be selected by supplying a name string of "3152".
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
#include <time.h>
#include <errno.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/

/* Flags stored in the context field: */
#define Lp3000Type501           00001
#define Lp3000Type512           00002
#define Lp3000Type3152          00010
#define Lp3000Type3555          00020
#define Lp3555FillImageMem      00100
#define Lp3000IntReady          00200   /* Same code as int status bit */
#define Lp3000IntEnd            00400   /* ditto */
#define Lp3000IntReadyEna       02000
#define Lp3000IntEndEna         04000

/*
**      Function codes
**
*/

/* Codes common to 3152/3256/3659 and 3555 */
#define FcPrintRelease          00000
#define FcPrintSingle           00001
#define FcPrintDouble           00002
#define FcPrintLastLine         00003
#define FcPrintEject            00004
#define FcPrintAutoEject        00005
#define FcPrintNoSpace          00006

#define Fc6681DevStatusReq      01300
#define FcControllerOutputEna   01600
#define Fc6681MasterClear       01700

/* Codes for 3152/3256/3659 */
#define Fc3152ClearFormat       00010
#define Fc3152PostVFU1          00011
#define Fc3152PostVFU2          00012
#define Fc3152PostVFU3          00013
#define Fc3152PostVFU4          00014
#define Fc3152PostVFU5          00015
#define Fc3152PostVFU6          00016
#define Fc3152SelectPreprint    00020
#define Fc3152PreVFU1           00021
#define Fc3152PreVFU2           00022
#define Fc3152PreVFU3           00023
#define Fc3152PreVFU4           00024
#define Fc3152PreVFU5           00025
#define Fc3152PreVFU6           00026
#define Fc3152SelIntReady       00030
#define Fc3152RelIntReady       00031
#define Fc3152SelIntEnd         00032
#define Fc3152RelIntEnd         00033
#define Fc3152SelIntError       00034
#define Fc3152RelIntError       00035
#define Fc3152Release2          00040

/* Codes for 3555 */
#define Fc3555CondClearFormat   00007
#define Fc3555Sel8Lpi           00010
#define Fc3555Sel6Lpi           00011
#define Fc3555FillMemory        00012
#define Fc3555SelExtArray       00013
#define Fc3555ClearExtArray     00014
#define Fc3555SelIntReady       00020
#define Fc3555RelIntReady       00021
#define Fc3555SelIntEnd         00022
#define Fc3555RelIntEnd         00023
#define Fc3555SelIntError       00024
#define Fc3555RelIntError       00025
#define Fc3555ReloadMemEnable   00026
#define Fc3555ClearFormat       00030
#define Fc3555PostVFU1          00031
#define Fc3555PostVFU2          00032
#define Fc3555PostVFU3          00033
#define Fc3555PostVFU4          00034
#define Fc3555PostVFU5          00035
#define Fc3555PostVFU6          00036
#define Fc3555PostVFU7          00037
#define Fc3555PostVFU8          00040
#define Fc3555PostVFU9          00041
#define Fc3555PostVFU10         00042
#define Fc3555PostVFU11         00043
#define Fc3555PostVFU12         00044
#define Fc3555SelectPreprint    00050
#define Fc3555PreVFU1           00051
#define Fc3555PreVFU2           00052
#define Fc3555PreVFU3           00053
#define Fc3555PreVFU4           00054
#define Fc3555PreVFU5           00055
#define Fc3555PreVFU6           00056
#define Fc3555PreVFU7           00057
#define Fc3555PreVFU8           00060
#define Fc3555PreVFU9           00061
#define Fc3555PreVFU10          00062
#define Fc3555PreVFU11          00063
#define Fc3555PreVFU12          00064
#define Fc3555MaintStatus       00065
#define Fc3555ClearMaint        00066

/*
**      Status reply
**
**  3152/3256/3659 vs. 3555 have different status codes for the
**  most part, but the few we care about are common:
**
*/
#define StPrintReady            00001
#define StPrintIntReady         00200
#define StPrintIntEnd           00400

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

typedef struct lpContext 
    {
    int flags;
    bool printed;
    bool keepInt;
    } LpContext;


/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void lp3000Init(u8 unitNo, u8 eqNo, u8 channelNo, int flags);
static FcStatus lp3000Func(PpWord funcCode);
static void lp3000Io(void);
static void lp3000Activate(void);
static void lp3000Disconnect(void);
static void lp3000Load(DevSlot *up, int, char *fn);


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
**  Purpose:        Initialise 501 line printer.
**
**  Parameters:     Name        Description.
**                  eqNo        equipment number
**                  unitNo      unit number
**                  channelNo   channel number the device is attached to
**                  deviceName  "3152" to get 3152 controller, null or "3555" for 3555
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void lp501Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    int flags = Lp3000Type501;
    
    (void)eqNo;
    (void)deviceName;

    if (deviceName == NULL ||
        strcmp (deviceName, "3555") == 0)
        {
        flags |= Lp3000Type3555;
        }
    else if (strcmp (deviceName, "3152") == 0)
        {
        flags |= Lp3000Type3152;
        }
    else
        {
        fprintf (stderr, "Unrecognized LP501 controller type %s\n",
                 deviceName);
        exit (1);
        }

    lp3000Init (unitNo, eqNo, channelNo, flags);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Initialise 512 line printer.
**
**  Parameters:     Name        Description.
**                  eqNo        equipment number
**                  unitNo      unit number
**                  channelNo   channel number the device is attached to
**                  deviceName  "3152" to get 3152 controller, null or "3555" for 3555
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void lp512Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    int flags = Lp3000Type512;
    
    (void)eqNo;
    (void)deviceName;

    if (deviceName == NULL ||
        strcmp (deviceName, "3555") == 0)
        {
        flags |= Lp3000Type3555;
        }
    else if (strcmp (deviceName, "3152") == 0)
        {
        flags |= Lp3000Type3152;
        }
    else
        {
        fprintf (stderr, "Unrecognized LP512 controller type %s\n",
                 deviceName);
        exit (1);
        }

    lp3000Init (unitNo, eqNo, channelNo, flags);
    }
 
/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Initialise 3000 class line printer.
**
**  Parameters:     Name        Description.
**                  unitNo      unit number
**                  eqNo        equipment number
**                  channelNo   channel number the device is attached to
**                  flags       Printer type flags
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void lp3000Init(u8 unitNo, u8 eqNo, u8 channelNo, int flags)
    {
    DevSlot *up;
    char fname[80];
    LpContext *lc;
    
    up = dcc6681Attach(channelNo, eqNo, unitNo, DtLp5xx);

    up->activate = lp3000Activate;
    up->disconnect = lp3000Disconnect;
    up->func = lp3000Func;
    up->io = lp3000Io;
    up->load = lp3000Load;

    /*
    **  Only one printer unit is possible per equipment.
    */
    if (up->context[0] != NULL)
        {
        fprintf (stderr, "Only one LP5xx unit is possible per equipment\n");
        exit (1);
        }

    lc = (LpContext *) calloc(1, sizeof(LpContext));
    if (lc == NULL)
        {
        fprintf (stderr, "Failed to allocate LP5xx context block\n");
        exit (1);
        }

    up->context[0] = lc;
    lc->flags = flags;
    
    /*
    **  Open the device file.
    */
    sprintf(fname, "LP5xx_C%02o_E%o", channelNo, eqNo);
    up->fcb[0] = fopen(fname, "w");
    if (up->fcb[0] == NULL)
        {
        fprintf(stderr, "Failed to open %s\n", fname);
        exit(1);
        }

    /*
    **  Print a friendly message.
    */
    printf("LP%d/%d initialised on channel %o equipment %o\n",
           (flags & Lp3000Type3555) ? 3555 : 3152,
           (flags & Lp3000Type501) ? 501 : 512,
           channelNo, eqNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform load/unload on printer.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void lp3000Load(DevSlot *up, int eqNo, char *fn)
    {
    FILE *fcb;
    time_t currentTime;
    struct tm t;
    char fname[80];
    char fnameNew[80];
    static char msgBuf[80];

    if (fn != NULL)
        {
        opSetMsg ("$LOAD NOT SUPPORTED ON LP5xx");
        return;
        }
    
    /*
    **  Close the old device file.
    */
    fflush(up->fcb[0]);
    fclose(up->fcb[0]);
    up->fcb[0] = NULL;

    /*
    **  Rename the device file to the format "LP5xx_yyyymmdd_hhmmss".
    */
    sprintf(fname, "LP5xx_C%02o_E%o", up->channel->id, up->eqNo);

    time(&currentTime);
    t = *localtime(&currentTime);
    sprintf(fnameNew, "LP5xx_%04d%02d%02d_%02d%02d%02d",
        t.tm_year + 1900,
        t.tm_mon + 1,
        t.tm_mday,
        t.tm_hour,
        t.tm_min,
        t.tm_sec);

    if (rename(fname, fnameNew) != 0)
        {
        sprintf (msgBuf, "$Rename error (%s to %s): %s",
                 fname, fnameNew, strerror(errno));
        opSetMsg(msgBuf);
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
        sprintf (msgBuf, "$Open error (%s): %s",
                 fname, strerror (errno));
        opSetMsg(msgBuf);
        return;
        }

    /*
    **  Setup status.
    */
    up->fcb[0] = fcb;
    sprintf (msgBuf, "LP5xx unloaded to %s", fnameNew);
    opSetMsg (msgBuf);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 3000 line printer.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus lp3000Func(PpWord funcCode)
    {
    FILE *fcb;
    LpContext *lc;

    fcb = active3000Device->fcb[0];
    lc = (LpContext *)active3000Device->context[0];
    
    /*
    **  Note that we don't emulate the VFU, so all VFU control codes
    **  are implemented as NOPs.
    */

    /* Start with the common codes */
    switch (funcCode)
        {
    case FcPrintAutoEject:
    case FcPrintNoSpace:
    case Fc6681MasterClear:
        /* Treat these as NOPs */
        return(FcProcessed);

    case FcPrintRelease:
        /* clear all interrupt conditions */
        lc->flags &= ~(StPrintIntReady | StPrintIntEnd);

        /* Release is sent at end of job, so flush the print file */
        if (lc->printed)
            {
            fflush (fcb);
            lc->printed = FALSE;
            }
        return(FcProcessed);
        
    case FcPrintSingle:
    case FcPrintLastLine:
        /* Treat last-line codes as a single blank line */
        fputc('\n', fcb);
        return(FcProcessed);

    case FcPrintEject:
        /* Turn eject into a formfeed character */
        fputc('\f', fcb);
        return(FcProcessed);

    case FcPrintDouble:
        fprintf(fcb, "\n\n");
        return(FcProcessed);

    case FcControllerOutputEna:
        if (lc->flags & Lp3555FillImageMem)
            {
            /* Tweak the function code to tell I/O handler to toss this data */
            funcCode++;
            /* Now clear the flag */
            lc->flags &= ~Lp3555FillImageMem;
            }
        /* Initially clear interrupt status flags */
        lc->flags &= ~(StPrintIntReady | StPrintIntEnd);

        /*
        **  Update interrupt status to reflect what status
        **  will be when this transfer is finished.
        **  Ok, so that's cheating a bit...
        */
        if (lc->flags & Lp3000IntReadyEna)
            {
            lc->flags |= Lp3000IntReady;
            }
        if (lc->flags & Lp3000IntEndEna)
            {
            lc->flags |= Lp3000IntEnd;
            }
    
        /* Update interrupt summary flag in unit block */
        dcc6681Interrupt((lc->flags & (Lp3000IntReady | Lp3000IntEnd)) != 0);

        /* fall through */
    case Fc6681DevStatusReq:
        active3000Device->fcode = funcCode;
        return(FcAccepted);
        }
    
    if (lc->flags & Lp3000Type3555)
        {
        switch (funcCode)
            {
        default:
            printf("Unknown LP3555 function %04o\n", funcCode);
            return(FcProcessed);

        case Fc3555CondClearFormat:
        case Fc3555Sel8Lpi:
        case Fc3555Sel6Lpi:
        case Fc3555SelExtArray:
        case Fc3555ClearExtArray:
        case Fc3555SelIntError:
        case Fc3555RelIntError:
        case Fc3555ReloadMemEnable:
        case Fc3555ClearFormat:
        case Fc3555PostVFU1:
        case Fc3555PostVFU2:
        case Fc3555PostVFU3:
        case Fc3555PostVFU4:
        case Fc3555PostVFU5:
        case Fc3555PostVFU6:
        case Fc3555PostVFU7:
        case Fc3555PostVFU8:
        case Fc3555PostVFU9:
        case Fc3555PostVFU10:
        case Fc3555PostVFU11:
        case Fc3555PostVFU12:
        case Fc3555SelectPreprint:
        case Fc3555PreVFU1:
        case Fc3555PreVFU2:
        case Fc3555PreVFU3:
        case Fc3555PreVFU4:
        case Fc3555PreVFU5:
        case Fc3555PreVFU6:
        case Fc3555PreVFU7:
        case Fc3555PreVFU8:
        case Fc3555PreVFU9:
        case Fc3555PreVFU10:
        case Fc3555PreVFU11:
        case Fc3555PreVFU12:
        case Fc3555MaintStatus:
        case Fc3555ClearMaint:
            /* All of the above are NOPs */
            return(FcProcessed);

        case Fc3555FillMemory:
            /*
            **  Remember that we saw this function, but this doesn't actually
            **  start any I/O yet.
            */
            lc->flags |= Lp3555FillImageMem;
            return(FcProcessed);

        case Fc3555SelIntReady:
            /*
            **  Enable next int.  If an I/O was done since the last
            **  time an int enable was issued, don't clear the
            **  current int.  That's because things go very slowly
            **  otherwise; printer drivers typically issue the write,
            **  then enable the int shortly after.  We've already set
            **  "ready" by then, unlike physical printers.
            */
            lc->flags |= Lp3000IntReady | Lp3000IntReadyEna;
            if (lc->keepInt)
                {
                lc->keepInt = FALSE;
                }
            else
                {
                lc->flags &= ~Lp3000IntReady;
                }
            /* Update interrupt summary flag in unit block */
            dcc6681Interrupt((lc->flags & (Lp3000IntReady | Lp3000IntEnd)) != 0);
            return(FcProcessed);

        case Fc3555RelIntReady:
            lc->flags &= ~(Lp3000IntReadyEna | Lp3000IntReady);
            /* Update interrupt summary flag in unit block */
            dcc6681Interrupt((lc->flags & (Lp3000IntReady | Lp3000IntEnd)) != 0);
            return(FcProcessed);

        case Fc3555SelIntEnd:
            lc->flags |= Lp3000IntEnd | Lp3000IntEndEna;
            if (lc->keepInt)
                {
                lc->keepInt = FALSE;
                }
            else
                {
                lc->flags &= ~Lp3000IntEnd;
                }
            /* Update interrupt summary flag in unit block */
            dcc6681Interrupt((lc->flags & (Lp3000IntReady | Lp3000IntEnd)) != 0);
            return(FcProcessed);

        case Fc3555RelIntEnd:
            lc->flags &= ~(Lp3000IntEndEna | Lp3000IntEnd);
            /* Update interrupt summary flag in unit block */
            dcc6681Interrupt((lc->flags & (Lp3000IntReady | Lp3000IntEnd)) != 0);
            return(FcProcessed);
            }
        }
    else
        {
        switch (funcCode)
            {
        default:
            printf("Unknown LP3152 function %04o\n", funcCode);
            return(FcProcessed);

        case Fc3152ClearFormat:
        case Fc3152PostVFU1:
        case Fc3152PostVFU2:
        case Fc3152PostVFU3:
        case Fc3152PostVFU4:
        case Fc3152PostVFU5:
        case Fc3152PostVFU6:
        case Fc3152SelectPreprint:
        case Fc3152PreVFU1:
        case Fc3152PreVFU2:
        case Fc3152PreVFU3:
        case Fc3152PreVFU4:
        case Fc3152PreVFU5:
        case Fc3152PreVFU6:
        case Fc3152SelIntError:
        case Fc3152RelIntError:
        case Fc3152Release2:
            /* All of the above are NOPs */
            return(FcProcessed);

        case Fc3152SelIntReady:
            /*
            **  Enable next int.  If an I/O was done since the last
            **  time an int enable was issued, don't clear the
            **  current int.  That's because things go very slowly
            **  otherwise; printer drivers typically issue the write,
            **  then enable the int shortly after.  We've already set
            **  "ready" by then, unlike physical printers.
            */
            lc->flags |= Lp3000IntReady | Lp3000IntReadyEna;
            if (lc->keepInt)
                {
                lc->keepInt = FALSE;
                }
            else
                {
                lc->flags &= ~Lp3000IntReady;
                }
            /* Update interrupt summary flag in unit block */
            dcc6681Interrupt((lc->flags & (Lp3000IntReady | Lp3000IntEnd)) != 0);
            return(FcProcessed);

        case Fc3152RelIntReady:
            lc->flags &= ~(Lp3000IntReadyEna | Lp3000IntReady);
            /* Update interrupt summary flag in unit block */
            dcc6681Interrupt((lc->flags & (Lp3000IntReady | Lp3000IntEnd)) != 0);
            return(FcProcessed);

        case Fc3152SelIntEnd:
            lc->flags |= Lp3000IntEnd | Lp3000IntEndEna;
            if (lc->keepInt)
                {
                lc->keepInt = FALSE;
                }
            else
                {
                lc->flags &= ~Lp3000IntEnd;
                }
            /* Update interrupt summary flag in unit block */
            dcc6681Interrupt((lc->flags & (Lp3000IntReady | Lp3000IntEnd)) != 0);
            return(FcProcessed);

        case Fc3152RelIntEnd:
            lc->flags &= ~(Lp3000IntEndEna | Lp3000IntEnd);
            /* Update interrupt summary flag in unit block */
            dcc6681Interrupt((lc->flags & (Lp3000IntReady | Lp3000IntEnd)) != 0);
            return(FcProcessed);
            }
        }
    active3000Device->fcode = funcCode;
    return(FcAccepted);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 3000 line printer.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void lp3000Io(void)
    {
    FILE *fcb;
    LpContext *lc;

    fcb = active3000Device->fcb[0];
    lc = (LpContext *)active3000Device->context[0];
    
    /*
    **  Process printer I/O.
    */
    switch (active3000Device->fcode)
        {
    default:
        activeChannel->full = FALSE;
        break;

    case FcControllerOutputEna:
        if (activeChannel->full)
            {
            if (lc->flags & Lp3000Type501)
                {
                /* 501 printer, output display code */
                fputc(bcdToAscii[(activeChannel->data >> 6) & 077], fcb);
                fputc(bcdToAscii[activeChannel->data & 077], fcb);
                }
            else
                {
                /* 512 printer, output ASCII */
                fputc(activeChannel->data & 0377, fcb);
                }
            activeChannel->full = FALSE;
            lc->printed = TRUE;
            lc->keepInt = TRUE;
            }
        break;

    case FcControllerOutputEna + 1:
        /* Fill image memory, just ignore that data */
        activeChannel->full = FALSE;
        break;

    case Fc6681DevStatusReq:
        /* Indicate ready plus whatever interrupts are enabled */
        activeChannel->data = StPrintReady | 
                              (lc->flags &
                               (StPrintIntReady | StPrintIntEnd));
        activeChannel->full = TRUE;
        active3000Device->fcode = 0;
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
static void lp3000Activate(void)
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
static void lp3000Disconnect(void)
    {
    FILE *fcb = active3000Device->fcb[0];

    if (active3000Device->fcode == FcControllerOutputEna)
        {
        /* Rule is "space after the line is printed" so do that here */
        fputc('\n', fcb);
        active3000Device->fcode = 0;
        }
    }

/*---------------------------  End Of File  ------------------------------*/
