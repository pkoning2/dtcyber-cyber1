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
#include <Carbon.h>
#elif defined(__GNUC__) && defined(__x86_64)
#include <sys/time.h>
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
#if defined(__GNUC__) && defined(__i386)
#define rdtscll(val) \
    __asm__ __volatile__("rdtsc" : "=A" (val))
#elif defined(__GNUC__) && defined(__x86_64)
#define rdtscll(val) \
    do  {            \
        struct timeval tv;                          \
        gettimeofday (&tv, NULL);                   \
        val = tv.tv_sec * 1000000 + tv.tv_usec;     \
    } while (0)
#elif defined(__GNUC__) && defined(__APPLE__)
#define rdtscll(val) \
    val = UnsignedWideToUInt64 (AbsoluteToNanoseconds (UpTime ()))
#elif defined(_WIN32)
#define rdtscll(val) \
	do { \
		LARGE_INTEGER foo; \
		QueryPerformanceCounter (&foo); \
		val = foo.QuadPart; \
	} while (0)
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

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static u8 rtcIncrement;
static bool rtcFull;
#if RDTSC
static u64 Hz;
static u32 MHz;
static u32 maxDelta;
static u64 rtcPrev;
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
#if RDTSC
    u32 us;
    u64 cycles, now;
    
    if (rtcIncrement == 0)
        {
        if (rtcPrev == 0)
            {
            rdtscll (rtcPrev);
            }
        rdtscll (now);
        cycles = now - rtcPrev;
        if (cycles > maxDelta)
            {
            cycles = maxDelta;
            }
        us = cycles / MHz;
        rtcPrev += us * MHz;
        rtcClock += us;
        }
#endif    
    activeChannel->full = rtcFull;
    activeChannel->data = rtcClock & Mask12;
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
#if defined(__x86_64)
    Hz = 1000000ULL;
    MHz = 1;
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
        hz = 1000000000ULL;     // 10^9, because timer is in ns
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
#endif  // !__APPLE__
        Hz = hz;
        MHz = hz / ULL(1000000);
        }
    else
        {
        MHz = setMHz;
        Hz = MHz * ULL(1000000);
        }
#endif  // !__x86_64
    maxDelta = 900 * MHz;   // less than 1 ms to keep mtr happy
    printf ("using high resolution hardware clock at %d MHz\n", MHz);
    }
#endif

/*---------------------------  End Of File  ------------------------------*/
