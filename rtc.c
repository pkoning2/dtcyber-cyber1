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
#include <mach/mach.h>
#include <mach/mach_time.h>
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
#define RDTSC 1
#elif defined(__GNUC__) && defined(__x86_64)
#define RDTSC 1
#elif defined(_WIN32)
#define RDTSC 1
#elif defined(__GNUC__) && defined(__APPLE__)
#define RDTSC 1
#else
#define RDTSC 0
#endif
/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#if RDTSC
#if defined(__GNUC__) && defined(__APPLE__)
static inline u64 rdtscll(void)
    {
    static mach_timebase_info_data_t sTimebaseInfo;
    u64 val, q, r;
    
    val = mach_absolute_time ();
    if ( sTimebaseInfo.denom == 0 )
        {
        (void) mach_timebase_info(&sTimebaseInfo);
        }
    // We want to find val * sTimebaseInfo.numer / sTimebaseInfo.denom
    // but on PowerPC (though not Intel) those are large numbers and
    // 64 bit overflow may result.  So do it the hard way.
    q = val / sTimebaseInfo.denom;
    r = val % sTimebaseInfo.denom;
    return (q * sTimebaseInfo.numer) +
           (r * sTimebaseInfo.numer / sTimebaseInfo.denom);
    }
#elif defined(__GNUC__) && defined(__i386)
static inline u64 rdtscll(void)
    {
    u64 val;
    __asm__ __volatile__("rdtsc" : "=A" (val));
    return val;
    }
#elif defined(__GNUC__) && defined(__x86_64)
static inline u64 rdtscll(void)
    {
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (u64) tv.tv_sec * 1000000ULL + (u64) tv.tv_usec;
    }
#elif defined(_WIN32)
static inline u64 rdtscll(void)
    {
    LARGE_INTEGER foo;
    QueryPerformanceCounter (&foo);
    return foo.QuadPart;
	}
#endif
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
static FcStatus rtcFunc(PpWord funcCode);
static void rtcIo(void);
static void rtcActivate(void);
static void rtcDisconnect(void);
#if RDTSC
static void rtcInit2 (long MHz);
#endif

/*
**  ----------------
**  Public Variables
**  ----------------
*/
u32 rtcClock = 0;
u8 rtcIncrement;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static bool rtcFull;
#if RDTSC
static u64 Hz;
static u32 maxDelta;
static u64 rtcPrev;
static bool caughtUp = FALSE;
static bool mtr = FALSE;
#endif
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

    dp = channelAttach(014, 0, DtRtc);

    dp->activate = rtcActivate;
    dp->disconnect = rtcDisconnect;
    dp->func = rtcFunc;
    dp->io = rtcIo;
    dp->selectedUnit = 0;

    if (increment == 0)
        {
#if RDTSC
        rtcInit2 (setMHz);
#else
        fprintf (stderr, "Invalid clock increment 0, defaulting to 1\n");
        increment = 1;
#endif
        }
    rtcIncrement = increment;
    activeChannel->ioDevice = dp;

    if (strcmp(model, "6600") == 0)
        {
        activeChannel->active = TRUE;
        activeChannel->full = TRUE;
        rtcFull = TRUE;
        }
    else
        {
        activeChannel->active = FALSE;
        activeChannel->full = FALSE;
        rtcFull = FALSE;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Do a clock tick
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**  If *increment* is specified as non-zero, this process counts emulator
**  cycles as an approximation of real time.  If *increment* is zero, then
**  the channel read handler keeps track of real time and adjusts the
**  RTC reading to match.  The rtcClock variable is 32 bits, so other parts
**  of the emulator can use it to keep track of elapsed time efficiently;
**  it is masked to 12 bits in the channel read handler.
**
**------------------------------------------------------------------------*/
void rtcTick(void)
    {
    rtcClock = rtcClock + rtcIncrement;
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
static FcStatus rtcFunc(PpWord funcCode)
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
static void rtcIo(void)
    {
    u32 clock = 0;
    
#if RDTSC
    u64 rtcCycles = 0, now;
    u32 us = 0;
    
    if (rtcIncrement == 0)
        {
        if (rtcPrev == 0)
            {
            rtcPrev = rdtscll ();
            }
        now = rdtscll ();
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
        
        if (activePpu->id == 0)
            {
            mtr = TRUE;
            rtcPrev += (us * Hz) / ULL (1000000);
            rtcClock = clock;
            }
        else if (!mtr)
            {
            rtcPrev += (us * Hz) / ULL (1000000);
            rtcClock = clock;
            }
        }
#else
    clock = rtcClock;
#endif    
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
static void rtcActivate(void)
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
static void rtcDisconnect(void)
    {
    }

#if RDTSC
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
            prev = rdtscll ();
            sleep (1);
            now = rdtscll ();
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
#endif

/*---------------------------  End Of File  ------------------------------*/
