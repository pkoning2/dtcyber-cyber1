/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: atod.c
**
**  Description:
**      ASCII to CDC Display Code conversion.
**
**--------------------------------------------------------------------------
*/

/*
**  -------------
**  Include Files
**  -------------
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
**  -----------------
**  Private Constants
**  -----------------
*/

/*
**  Bit masks.
*/
#define Mask3                   07
#define Mask6                   077
#define Mask9                   0777
#define Mask10                  01777
#define Mask11                  03777
#define Mask12                  07777
#define Mask15                  077777
#define Mask18                  0777777
#define Mask24                  077777777
#define Mask30                  07777777777
#define Mask48                  000007777777777777777
#define Mask50                  000037777777777777777
#define Mask60                  077777777777777777777

#define BlockSize               15

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef signed char  i8;
typedef signed short i16;
typedef signed long  i32;
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;
#ifdef __GNUC__
typedef long long i64;
typedef unsigned long long u64;
#else
typedef __int64 i64;
typedef unsigned __int64 u64;
#endif
#ifndef __cplusplus
typedef int bool;
#endif

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static char asciiToCdc[256] =
    {
    /* 000- */  0,      0,      0,      0,      0,      0,      0,      0,
    /* 010- */  0,      0,      0,      0,      0,      0,      0,      0,
    /* 020- */  0,      0,      0,      0,      0,      0,      0,      0,
    /* 030- */  0,      0,      0,      0,      0,      0,      0,      0,
    /* 040- */  055,    066,    064,    060,    053,    063,    067,    070,
    /* 050- */  051,    052,    047,    045,    056,    046,    057,    050,
    /* 060- */  033,    034,    035,    036,    037,    040,    041,    042,
    /* 070- */  043,    044,    0,      077,    072,    054,    073,    071,
    /* 100- */  074,    001,    002,    003,    004,    005,    006,    007,
    /* 110- */  010,    011,    012,    013,    014,    015,    016,    017,
    /* 120- */  020,    021,    022,    023,    024,    025,    026,    027,
    /* 130- */  030,    031,    032,    061,    075,    062,    076,    065,
    /* 140- */  0,      001,    002,    003,    004,    005,    006,    007,
    /* 150- */  010,    011,    012,    013,    014,    015,    016,    017,
    /* 160- */  020,    021,    022,    023,    024,    025,    026,    027,
    /* 170- */  030,    031,    032,    061,    0,      0,      0,      0
    };

static u8 inBuf[BlockSize];
static u16 dmp[10];

static FILE *ifd;
static FILE *ofd;
static bool odd = 1;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Print usage note.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void usage(void)
    {
    fprintf(stderr,"usage: atod <ASCII file> <NOS file>\n");
    }

/*--------------------------------------------------------------------------
**  Purpose:        Flush 60 bit word to file.
**
**  Parameters:     Name        Description.
**                  w           60 bit word to flush.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void flushWord(u64 w)
    {
    static u64 out = 0;
    int i;

    if (odd)
        {
        out = w & Mask60;
        i = 60 - 8;
        }
    else
        {
        out <<= 60;
        out |= w & Mask60;
        i = 64 - 8;
        }

    odd = !odd;

    for (; i >= 0; i -= 8)
        {
        fputc((int)((out >> i) & 0xFF), ofd);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        CDC Display Code to ASCII conversion.
**
**  Parameters:     Name        Description.
**                  argc        argument count
**                  argv        array of argument values.
**
**  Returns:        exit code.
**
**------------------------------------------------------------------------*/
int main (int argc, char *argv[])
    {
    u8 disp;
    u8 bitpos;
    u64 w;
    int ch;

    if (argc != 3)
        {
        usage();
        exit(EXIT_FAILURE);
        }

    if ((ifd = fopen(argv[1], "rt")) == NULL)
        {
        perror(argv[1]);
        exit(EXIT_FAILURE);
        }
    
    if ((ofd = fopen(argv[2], "wb")) == NULL)
        {
        perror(argv[1]);
        exit(EXIT_FAILURE);
        }
    
    bitpos = 60;
    w = 0;
    while ((ch = fgetc(ifd)) != EOF)
        {
        if (ch == '\n')
            {
            /*
            ** Write rest of line + NOS EOL.
            */
            flushWord(w);

            if (bitpos < 12)
                {
                flushWord(0);
                }

            bitpos = 60;
            w = 0;
            continue;
            }

        bitpos -= 6;
        disp = asciiToCdc[ch & 0x7F];
        w |= (u64)disp << bitpos;
        if (bitpos == 0)
            {
            flushWord(w);
            bitpos = 60;
            w = 0;
            }
        }

    if (!odd)
        {
        flushWord(0);
        }

    fclose (ifd);
    fclose (ofd);
    }

/*---------------------------  End Of File  ------------------------------*/
