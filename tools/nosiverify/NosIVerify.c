/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: NosIVerify.c
**
**  Description:
**      Verify NOS I-format TAP image.
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

#define MaxByteBuf              32768

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef signed char  i8;
typedef signed short i16;
typedef signed int   i32;
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
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


static u8 rawBuffer[MaxByteBuf];

static union
    {
    u32 number;
    u8 bytes[4];
    } endianCheck;

static bool bigEndian;

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
	fprintf(stderr,"usage: NosIVerify <tap-image>\n");
    }

/*--------------------------------------------------------------------------
**  Purpose:        Convert endian-ness.
**
**  Parameters:     Name        Description.
**                  value       value to be converted.
**
**  Returns:        Conversion result.
**
**------------------------------------------------------------------------*/
u32 convertEndian(u32 value)
    {
    u32 result;

    result  = (value & 0xff000000) >> 24;
    result |= (value & 0x00ff0000) >>  8;
    result |= (value & 0x0000ff00) <<  8;
    result |= (value & 0x000000ff) << 24;

    return(result);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Verify NOS I-format TAP image.
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
	FILE *ifd;
    u32 len;
    u32 recLen0;
    u32 recLen1;
    u32 recLen2;
    u64 blockTerm;
    u16 byteCount;
    u32 blockNumber;
    u32 expectedBlockNumber = 0;
    u16 levelNumber;
    int begin = 1;
    
	if (argc != 2)
        {
	    usage();
	    exit(EXIT_FAILURE);
	    }

	if ((ifd = fopen (argv[1], "rb")) <= 0)
        {
		perror(argv[1]);
		exit(EXIT_FAILURE);
	    }

    /*
    **  Determine endianness of the host.
    */
    endianCheck.bytes[0] = 0;
    endianCheck.bytes[1] = 0;
    endianCheck.bytes[2] = 0;
    endianCheck.bytes[3] = 1;
    bigEndian = endianCheck.number == 1;

    while (1)
        {
        /*
        **  Read and verify TAP record length header.
        */
        len = fread(&recLen0, sizeof(recLen0), 1, ifd);

        if (len != 1)
            {
                if (feof (ifd))
                {
                    break;
                }
    		perror("reading TAP header");
            break;
            }

        /*
        **  The TAP record length is little endian - convert if necessary.
        */
        if (bigEndian)
            {
            recLen1 = convertEndian(recLen0);
            }
        else
            {
            recLen1 = recLen0;
            }

        if (recLen1 == 0)
            {
            printf ("tape mark\n");
            begin = 1;
            continue;
            }
        
        if (recLen1 != 0)
            {
            if (recLen1 > MaxByteBuf)
                {
                fprintf(stderr, "tape record too long: %d", recLen1);
                break;
                }

            /*
            **  Read and verify the actual raw data.
            */
            memset(rawBuffer, 0, sizeof(rawBuffer));
            len = fread(rawBuffer, 1, recLen1, ifd);

            if (recLen1 != (u32)len)
                {
                fprintf(stderr, "short tape record read: %d", len);
                break;
                }

            /*
            **  Read and verify the TAP record length trailer.
            */
            len = fread(&recLen2, sizeof(recLen2), 1, ifd);

            if (len != 1 || recLen0 != recLen2)
            {
                /*
                **  This is some weird shit to deal with "padded" TAP records. My brain refuses to understand
                **  why anyone would have the precise length of a record and then make the reader guess what
                **  the real length is.
                */
                if (bigEndian)
                {
                    /*
                    **  The TAP record length is little endian - convert if necessary.
                    */
                    recLen2 = convertEndian(recLen2);
                }

                if (recLen1 == ((recLen2 >> 8) & 0xFFFFFF))
                {
                    fseek(ifd, 1, SEEK_CUR);
                }
                else
                {
                    fprintf(stderr, "invalid tape record trailer: %d", recLen2);
                    break;
                }
            }

            if (begin)
                {
                if (recLen1 == 80)
                    {
                    printf ("ANSI label: %80s\n", rawBuffer);
                    continue;
                    }
                begin = 0;
                }
            
            recLen1 -= 6;

            if (recLen1 % 15 == 0)
                {
                blockTerm =
                    ((u64)rawBuffer[recLen1 + 0] << 40) |
                    ((u64)rawBuffer[recLen1 + 1] << 32) |
                    ((u64)rawBuffer[recLen1 + 2] << 24) |
                    ((u64)rawBuffer[recLen1 + 3] << 16) |
                    ((u64)rawBuffer[recLen1 + 4] <<  8) |
                    ((u64)rawBuffer[recLen1 + 5] <<  0);

                blockTerm &= Mask48;
                }
            else if (recLen1 % 15 == 9)
                {
                recLen1 -= 2;
                blockTerm =
                    ((u64)rawBuffer[recLen1 + 0] << 44) |
                    ((u64)rawBuffer[recLen1 + 1] << 36) |
                    ((u64)rawBuffer[recLen1 + 2] << 28) |
                    ((u64)rawBuffer[recLen1 + 3] << 20) |
                    ((u64)rawBuffer[recLen1 + 4] << 12) |
                    ((u64)rawBuffer[recLen1 + 5] <<  4) |
                    ((u64)rawBuffer[recLen1 + 6] >>  4);

                recLen1 += 2;

                blockTerm &= Mask48;
                }
            else
                {
                blockTerm =
                    ((u64)rawBuffer[recLen1 - 2] << 52) |
                    ((u64)rawBuffer[recLen1 - 1] << 48) |
                    ((u64)rawBuffer[recLen1 + 0] << 40) |
                    ((u64)rawBuffer[recLen1 + 1] << 32) |
                    ((u64)rawBuffer[recLen1 + 2] << 24) |
                    ((u64)rawBuffer[recLen1 + 3] << 16) |
                    ((u64)rawBuffer[recLen1 + 4] <<  8) |
                    ((u64)rawBuffer[recLen1 + 5] <<  0);

                blockTerm &= Mask60;
                fprintf(stdout, "record length %5d unexpected %020llo\n",
                    recLen1 + 6,
                    blockTerm);
                continue;
                }

            byteCount = (u16)((blockTerm >> 36) & Mask12);
            blockNumber = (u32)((blockTerm >> 12) & Mask24);
            levelNumber = (u16)(blockTerm & Mask12);

            if (blockNumber != expectedBlockNumber)
                {
                fprintf(stdout, "Bad block terminator - expected %05o - got %05o\n", expectedBlockNumber, blockNumber);
                }

            expectedBlockNumber += 1;

            fprintf(stdout, "record length %5d, seq %5d, term %016llo   ",
                    recLen1 + 6, blockNumber, blockTerm);

                {
                int j;
                u8 *ip;
                u16 *dp;
                u16 c1, c2, c3;
                u16 dmp[10];

                /*
                **  Convert the raw data into PP words.
                */
                memset(dmp, 0, sizeof(dmp));
                dp = dmp;
                ip = rawBuffer;

                for (j = 0; j < 15; j += 3)
                    {
                    c1 = *ip++;
                    c2 = *ip++;
                    c3 = *ip++;

                    *dp++ = ((c1 << 4) | (c2 >> 4)) & Mask12;
                    *dp++ = ((c2 << 8) | (c3 >> 0)) & Mask12;
                    }

		        for (j = 0; j < 10; j++)
                    {
		            fprintf (stdout, " %04o", dmp[j]);
                    }

		        fprintf (stdout, "  ");

		        for (j = 0; j < 10; j++)
                    {
                    fprintf(stdout, "%c", cdcToAscii[(dmp[j] >> 6) & Mask6]);
                    fprintf(stdout, "%c", cdcToAscii[(dmp[j] >> 0) & Mask6]);
    		        }

		        fprintf(stdout, "\n");
                }

            }
        }

	fclose (ifd);
    }

/*---------------------------  End Of File  ------------------------------*/

