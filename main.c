/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: main.c
**
**  Description:
**      Perform simulation of CDC 6600 mainframe system.
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
char ppKeyIn;
bool emulationActive = TRUE;

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
**  Purpose:        System initialisation and main program loop.
**
**  Parameters:     Name        Description.
**                  argc        Argument count.
**                  argv        Array of argument strings.
**
**  Returns:        Zero.
**
**------------------------------------------------------------------------*/
int main(int argc, char **argv)
    {
    (void)argc;
    (void)argv;

    /*
    **  Allow optional command line parameter to specifiy section to run in "cyber.ini".
    */
    if (argc == 2)
        {
        initStartup(argv[1]);
        }
    else
        {
        initStartup("cyber");
        }

    /*
    **  Setup debug support.
    */
    traceInit();
    dumpInit();

    /*
    **  Setup operator interface.
    */
    opInit();

    /*
    **  Initiate deadstart sequence.
    */
    deadStart();

    /*
    **  Emulation loop.
    */
    while (emulationActive)
        {
        /*
        **  Deal with operator interface requests.
        */
        if (opActive)
            {
            opRequest();
            }

        /*
        **  Execute PP, CPU and RTC.
        */
        ppStep();
        cpuStep();
        rtcTick();
        }

    windowClose();

#if 0
    /*
    **  Example post-mortem dumps.
    */
    dumpAll();
    dumpPpu(0);
    dumpDisassemblePpu(0);
    dumpCpu();
#endif

    return(0);
    }


/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*---------------------------  End Of File  ------------------------------*/
