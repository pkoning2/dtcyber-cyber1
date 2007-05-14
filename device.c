/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: device.c
**
**  Description:
**      Device support for CDC 6600.
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
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/

/*
**  ----------------
**  Public Variables
**  ----------------
*/
DevDesc deviceDesc[] =
    {
    { "MT607",    mt607Init },
    { "MT669",    mt669Init },
    { "DD6603",   dd6603Init },
    { "DD844-2",  dd844Init_2 },
    { "DD844-4",  dd844Init_4 },
    { "DD844",    dd844Init_4 },
    { "DD885-1",  dd885Init_1 },
    { "DD885",    dd885Init_1 },
    { "CR405",    cr405Init },
    { "LP1612",   lp1612Init },
    { "LP501",    lp501Init },
    { "LP512",    lp512Init },
    { "CO6612",   consoleInit },
    { "MUX6676",  mux6676Init },
    { "CP3446",   cp3446Init },
    { "CR3447",   cr3447Init },
    { "TPM",      tpMuxInit },
    { "DDP",      ddpInit },
    { "NIU",      niuInit },
#ifdef NPU_SUPPORT
    { "NPU",      npuInit, },
#endif
    };

u8 deviceCount = sizeof(deviceDesc) / sizeof(deviceDesc[0]);

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


/*---------------------------  End Of File  ------------------------------*/


