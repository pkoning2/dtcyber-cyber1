/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: atod.c
**
**  Description:
**      CDC Display Code to ASCII conversion.
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
static char cdcToAscii[64] =
    {
    /* 00-07 */ ':',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
    /* 10-17 */ 'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
    /* 20-27 */ 'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
    /* 30-37 */ 'X',    'Y',    'Z',    '0',    '1',    '2',    '3',    '4',
    /* 40-47 */ '5',    '6',    '7',    '8',    '9',    '+',    '-',    '*',
    /* 50-57 */ '/',    '(',    ')',    '$',    '=',    ' ',    ',',    '.',
    /* 60-67 */ '#',    '[',    ']',    '%',    '"',	'_',	'!',	'&',
    /* 70-77 */ '\'',   '?',    '<',    '>',    '@',    '\\',   '^',    ';'
    };


static u8 inBuf[BlockSize];
static u16 dmp[10];

static FILE *ifd;

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
    fprintf(stderr,"usage: dtoa <NOS file>\n");
    }

/*--------------------------------------------------------------------------
**  Purpose:        ASCII to CDC Display Code conversion.
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
    int i;
    int j;
    int k;
    int byteCount;
    u8 *ip;
    u16 *dp;
    u8 *bp;
    char outBuf[20];
    u16 c1, c2, c3;

    if (argc != 2)
        {
        usage();
        exit(EXIT_FAILURE);
        }

    if ((ifd = fopen(argv[1], "rb")) == NULL)
        {
        perror(argv[1]);
        exit(EXIT_FAILURE);
        }
    
    for (i = 0; ; i += 10)
        {
        byteCount = fread(inBuf, 1, BlockSize, ifd);
        if (byteCount <= 0)
            {
            break;
            }

        /*
        **  Convert the raw data into PP words.
        */
        memset(dmp, 0, sizeof(dmp));
        dp = dmp;
        ip = inBuf;

        for (j = 0; j < byteCount; j += 3)
            {
            c1 = *ip++;
            c2 = *ip++;
            c3 = *ip++;

            *dp++ = ((c1 << 4) | (c2 >> 4)) & Mask12;
            *dp++ = ((c2 << 8) | (c3 >> 0)) & Mask12;
            }

        bp = outBuf;

        for (j = 0; j < 10; j++)
            {
            *bp++ = cdcToAscii[(dmp[j] >> 6) & Mask6];
            *bp++ = cdcToAscii[(dmp[j] >> 0) & Mask6];
            }

        /*
        **  for each word determine if it is an EOL.
        */
        j = 9;
        if (outBuf[8] == ':' && outBuf[9] == ':')
            {
            /*
            **  Find last real char.
            */
            for (j = 7; j >= 0; j--)
                {
                if (outBuf[j] != ':')
                    break;
                }
            }

        for (k = 0; k <= j; k++)
            {
            putchar(outBuf[k]);
            }

        if (j < 9)
            {
            putchar('\n');
            }

        /*
        **  for each word determine if it is an EOL.
        */
        j = 19;
        if (outBuf[18] == ':' && outBuf[19] == ':')
            {
            /*
            **  Find last real char.
            */
            for (j = 17; j >= 10; j--)
                {
                if (outBuf[j] != ':')
                    break;
                }
            }

        for (k = 10; k <= j; k++)
            {
            putchar(outBuf[k]);
            }

        if (j < 19)
            {
            putchar('\n');
            }
        }

    fclose (ifd);

    printf("finished - hit ENTER\n");
    getchar();
    }

/*---------------------------  End Of File  ------------------------------*/
