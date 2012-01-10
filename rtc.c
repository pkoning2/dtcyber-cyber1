/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: rtc.c
**
**  Description:
**      Perform simulation of CDC 6600 real-time clock.
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
#if defined(_WIN32)
#include <winsock.h>
#elif defined(__APPLE__)
#include <DriverServices.h>
#elif defined(__GNUC__) && defined(__x86_64)
#include <sys/time.h>
#else
#include <unistd.h>
#endif

#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#if defined(__GNUC__) && defined(__i386)
#elif defined(__GNUC__) && defined(__x86_64)
#elif defined(_WIN32)
#elif defined(__GNUC__) && defined(__APPLE__)
#else
#error "Don't know how to do real-time clock emulation"
#endif
/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#if defined(__GNUC__) && defined(__APPLE__)
#define rdtscll(val) \
    val = UnsignedWideToUInt64 (AbsoluteToNanoseconds (UpTime ()))
#elif defined(__GNUC__) && defined(__i386)
#define rdtscll(val) \
    __asm__ __volatile__("rdtsc" : "=A" (val))
#elif defined(__GNUC__) && defined(__x86_64)
#define rdtscll(val) \
    do  {            \
        struct timeval tv;                          \
        gettimeofday (&tv, NULL);                   \
        val = tv.tv_sec * 1000000 + tv.tv_usec;     \
    } while (0)
#elif defined(_WIN32)
#define rdtscll(val) \
	do { \
		LARGE_INTEGER foo; \
		QueryPerformanceCounter (&foo); \
		val = foo.QuadPart; \
	} while (0)
#endif

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
static FcStatus rtcFunc(ChSlot *activeChannel, DevSlot *activeDevice,
                        PpWord funcCode);
static void rtcIo(ChSlot *activeChannel, DevSlot *activeDevice);
static void rtcActivate(ChSlot *activeChannel, DevSlot *activeDevice);
static void rtcDisconnect(ChSlot *activeChannel, DevSlot *activeDevice);
static void rtcInit2 (long MHz);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
u32 rtcClock = 0;
u8 rtcIncrement;
bool mtr = FALSE;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static bool rtcFull;
static u64 Hz;
static u32 maxDelta;
static u64 rtcPrev;
static bool caughtUp = FALSE;
/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialise RTC.
**
**  Parameters:     Name        Description.
**                  model       Cyber model number
**                  increment   clock increment per iteration.
**                  setMHz      cycle counter frequency in MHz.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void rtcInit(char *model, u8 increment, long setMHz)
    {
    DevSlot *dp;

    dp = channelAttach(ChClock, 0, DtRtc);

    dp->activate = rtcActivate;
    dp->disconnect = rtcDisconnect;
    dp->func = rtcFunc;
    dp->io = rtcIo;
    dp->selectedUnit = 0;

    if (increment == 0)
        {
        rtcInit2 (setMHz);
        }
    rtcIncrement = increment;
    channel[ChClock].ioDevice = dp;

    if (strcmp(model, "6600") == 0)
        {
        channel[ChClock].active = TRUE;
        channel[ChClock].full = TRUE;
        rtcFull = TRUE;
        }
    else
        {
        channel[ChClock].active = FALSE;
        channel[ChClock].full = FALSE;
        rtcFull = FALSE;
        }
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on RTC pseudo device.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static FcStatus rtcFunc(ChSlot *activeChannel, DevSlot *activeDevice,
                        PpWord funcCode)
    {
    (void)funcCode;

    return(FcDeclined);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void rtcIo(ChSlot *activeChannel, DevSlot *activeDevice)
    {
    u32 clock = 0;
    int pp;
    u64 rtcCycles = 0, now;
    u32 us = 0;
    
    pp = activeChannel->ppu;
    if (pp != 0 || !mtr || rtcIncrement != 0)
        {
        if (pp == 0)
            {
            mtr = TRUE;
            }
        clock = ppu[pp].cycles;
        }
    else
        {
        if (rtcPrev == 0)
            {
            rdtscll (rtcPrev);
            }
        rdtscll (now);
        if (now < rtcPrev)
            {
            printf ("ERROR: clock went backwards: now = %lld, prev = %lld, cycles = %lld, us = %d\n",
                    now, rtcPrev, rtcCycles, us);
            now = rtcPrev;
            }
        
        rtcCycles = now - rtcPrev;
        if (rtcCycles > maxDelta)
            {
#ifdef DEBUG
            /* print a message if more than 1 second behind */
            if (caughtUp && rtcCycles > Hz)
                {
                printf ("Clock in catch-up mode, %lld cycles behind (%lld microseconds)\n",
                        rtcCycles, (rtcCycles * ULL (1000000)) / Hz);
                caughtUp = FALSE;
                }
#endif  /* DEBUG */
            rtcCycles = maxDelta;
            }
        else
            {
            caughtUp = TRUE;
            }
        us = (rtcCycles * ULL (1000000)) / Hz;
        clock = rtcClock + us;
        
        rtcPrev += (us * Hz) / ULL (1000000);
        rtcClock = clock;
        if (us < 800)
            {
            ppu[0].state = 'T';
            }
        }
    activeChannel->full = rtcFull;
    activeChannel->data = clock & Mask12;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle channel activation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void rtcActivate(ChSlot *activeChannel, DevSlot *activeDevice)
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
static void rtcDisconnect(ChSlot *activeChannel, DevSlot *activeDevice)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        RTC initialization when using the Pentium cycle counter
**
**  Parameters:     Name        Description.
**                  setMHz      cycle counter frequency in MHz.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void rtcInit2(long setMHz)
    {
#if defined(__x86_64) && !defined(__APPLE__)
    Hz = ULL (1000000);
#else
    u64 hz = 0;
#if defined(_WIN32)
	LARGE_INTEGER lhz;
#elif !defined(__APPLE__)
    FILE *procf;
    char procbuf[512];
    u64 now, prev;
    char *p;
    double procMHz;
#endif

    if (setMHz == 0)
        {
#if defined(_WIN32)
		if (!QueryPerformanceFrequency (&lhz))
			{
			fprintf (stderr, "High resolution timer not available\n");
			exit (1);
			}
		hz = lhz.QuadPart;
#elif defined(__APPLE__)
        hz = 1000000000ULL;     /* 10^9, because timer is in ns */
#else
        procf = fopen ("/proc/cpuinfo", "r");
        if (procf != NULL)
            {
            fread (procbuf, sizeof (procbuf), 1, procf);
            p = strstr (procbuf, "cpu MHz");
            if (p != NULL)
                {
                p = strchr (p, ':') + 1;
                }
            if (p != NULL)
                {
                sscanf (p, " %lf", &procMHz);
                hz = procMHz * 1000000.0;
                }
            fclose (procf);
            }
        if (hz == 0)
            {
            sleep (1);
            rdtscll (prev);
            sleep (1);
            rdtscll (now);
            hz = now - prev;
            }
#endif  /* !__APPLE__ */
        Hz = hz;
        }
    else
        {
        Hz = setMHz * ULL(1000000);
        }
#endif  /* !__x86_64 */
    maxDelta = Hz / ULL (1250);   /* 800 microseconds, to keep mtr happy */
    printf ("using high resolution hardware clock at %lld Hz\n", Hz);
    }

/*---------------------------  End Of File  ------------------------------*/
