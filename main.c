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
u32 cycles;

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
    int cpucycle;
    
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
        **  Count major cycles.
        */
        cycles++;

        /*
        ** Check if it's time for a screen update
        */
        if (--consoleCheckCycles < 0)
            {
            consoleCheckCycles = ConsoleCheckRate;
            consoleCheckOutput();
            }
        
        /*
        **  Execute PP, CPU and RTC.
        */
        ppStep();
        for (cpucycle = 0; cpucycle < cpuRatio; cpucycle++)
            {
#ifndef CPU_THREADS
            cpuStepAll();
#else
            cpuStep(cpu);
#endif
            }
        rtcTick();
        /*
        **  Count a major cycle
        */
        cycles++;
        }

    cpuExit();

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
