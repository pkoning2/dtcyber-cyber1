/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
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
#define ConsoleCheckRate         5000

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
int ppKeyIn;
bool emulationActive = TRUE;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
int consoleCheckCycles;

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
    **  Setup error logging.
    */
    logInit();

    /*
    **  Allow optional command line parameter to specify section to run in "cyber.ini".
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
#if CcDebug == 1
    traceInit();
    dumpInit();
#endif

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
        ** Check if it's time for a screen update
        */
        if (--consoleCheckCycles < 0)
            {
            consoleCheckCycles = ConsoleCheckRate;
            consoleCheckOutput();
            /*
            **  Also check for any pending operator requests;
            **  those must be done in the main thread, not the 
            **  operator network thread.
            */
            operCheckRequests ();
            /*
            **  Also check for PNI action required.
            */
            pniCheck ();
            }
        
        /*
        **  Execute PP, CPU and RTC.
        */
        if (!usingThreads)
            {
            ppStepAll();
            }
        }

#if 0
    /*
    **  Example post-mortem dumps.
    */
#if 0
    dumpAll();
    dumpPpu(0);
    dumpDisassemblePpu(0);
    dumpCpu();
#else
    dumpAll();
#endif
#endif

    /*
    **  Shut down debug support.
    */
#if CcDebug == 1
    traceTerminate();
    dumpTerminate();
#endif

    /*
    **  Shut down emulation.
    */
    cpuTerminate();
    ppTerminate();
    channelTerminate();

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
