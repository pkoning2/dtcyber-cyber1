/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: NosICreate.c
**
**  Description:
**      Convert individual files into NOS I-format TAP image with one
**      logical record per file.
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

#define BlockSize               3840

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
static char inFileName[80];
static u8 buf[BlockSize + 10];
static u32 blockNumber = 0;
static FILE *ifd;
static FILE *ofd;
static u16 levelNumber = 0;
static int byteCount;
static u16 ppWordCount;
static u64 blockTerm;
static u32 recLen;

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
	fprintf(stderr,"usage: NosICreate <new-tap-image> [optional file count]\n");
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
**  Purpose:        Convert file to logical record in NOS I-format and write
**                  to TAP image.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void writeNosItoTap(void)
    {
    bool last;

    do
        {
        byteCount = fread(buf, 1, BlockSize, ifd);
        if (byteCount < 0)
            {
		    perror(inFileName);
		    exit(EXIT_FAILURE);
            }

        last = byteCount < BlockSize;

        if (byteCount % 15 == 0)
            {
            ppWordCount = (byteCount / 15) * 10;
            ppWordCount += 4;

            blockTerm =
                ((u64)ppWordCount << 36) |
                ((u64)blockNumber << 12) |
                ((u64)levelNumber <<  0);

            buf[byteCount + 0]  = (u8)(blockTerm >> 40);
            buf[byteCount + 1]  = (u8)(blockTerm >> 32);
            buf[byteCount + 2]  = (u8)(blockTerm >> 24);
            buf[byteCount + 3]  = (u8)(blockTerm >> 16);
            buf[byteCount + 4]  = (u8)(blockTerm >>  8);
            buf[byteCount + 5]  = (u8)(blockTerm >>  0);
            byteCount += 6;
            }
        else if (byteCount % 15 == 9)
            {
            ppWordCount = ((byteCount - 9) / 15) * 10;
//            ppWordCount += (6 + 4);
            ppWordCount += (5 + 4);

            blockTerm =
                ((u64)ppWordCount << 36) |
                ((u64)blockNumber << 12) |
                ((u64)levelNumber <<  0);

            buf[byteCount - 2] &= 0xF0;
            buf[byteCount - 2] |= (u8)(blockTerm >> 44) & 0x0F;
            buf[byteCount - 1]  = (u8)(blockTerm >> 36);
            buf[byteCount + 0]  = (u8)(blockTerm >> 28);
            buf[byteCount + 1]  = (u8)(blockTerm >> 20);
            buf[byteCount + 2]  = (u8)(blockTerm >> 12);
            buf[byteCount + 3]  = (u8)(blockTerm >>  4);
            buf[byteCount + 4]  = (u8)(blockTerm <<  4);
            buf[byteCount + 5]  = 0;
            byteCount += 6;
            }
        else
            {
            fprintf(stderr, "unexpected length %d in file %s\n", byteCount, inFileName); 
            getchar();
  		    exit(EXIT_FAILURE);
            }

        recLen = byteCount;

        blockNumber += 1;

        /*
        **  The TAP record length is little endian - convert if necessary.
        */
        if (bigEndian)
            {
            recLen = convertEndian(recLen);
            }

        if (fwrite(&recLen, sizeof(recLen), 1, ofd) != 1)
            {
		    perror("TAP image");
    		exit(EXIT_FAILURE);
            }

        if (fwrite(buf, 1, byteCount, ofd) != (size_t)byteCount)
            {
		    perror("TAP image");
    		exit(EXIT_FAILURE);
            }

        if (fwrite(&recLen, sizeof(recLen), 1, ofd) != 1)
            {
		    perror("TAP image");
    		exit(EXIT_FAILURE);
            }
        } while (!last);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Create zero length logical record in NOS I-format and
**                  write to TAP image.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void writeZeroNosItoTap(u8 levelNumber)
    {
    ppWordCount = 4;

    blockTerm =
        ((u64)ppWordCount << 36) |
        ((u64)blockNumber << 12) |
        ((u64)levelNumber <<  0);

    buf[0] = (u8)(blockTerm >> 40);
    buf[1] = (u8)(blockTerm >> 32);
    buf[2] = (u8)(blockTerm >> 24);
    buf[3] = (u8)(blockTerm >> 16);
    buf[4] = (u8)(blockTerm >>  8);
    buf[5] = (u8)(blockTerm >>  0);

    byteCount = 6;

    recLen = byteCount;

    blockNumber += 1;

    /*
    **  The TAP record length is little endian - convert if necessary.
    */
    if (bigEndian)
        {
        recLen = convertEndian(recLen);
        }

    if (fwrite(&recLen, sizeof(recLen), 1, ofd) != 1)
        {
		perror("TAP image");
    	exit(EXIT_FAILURE);
        }

    if (fwrite(buf, 1, byteCount, ofd) != (size_t)byteCount)
        {
		perror("TAP image");
    	exit(EXIT_FAILURE);
        }

    if (fwrite(&recLen, sizeof(recLen), 1, ofd) != 1)
        {
		perror("TAP image");
    	exit(EXIT_FAILURE);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Convert individual files into NOS I-format TAP image
**                  with one logical record per file.
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
    u16 fileNumber = 0;
    int fileCount = 1;

	if (argc < 2 || argc > 3)
        {
	    usage();
	    exit(EXIT_FAILURE);
	    }

    /*
    **  Convert optional file count.
    */
    if (argc == 3 && sscanf(argv[2],"%d", &fileCount) != 1)
        {
	    usage();
	    exit(EXIT_FAILURE);
        }

    /*
    **  Open TAP file.
    */
	if ((ofd = fopen(argv[1], "wb")) == NULL)
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

    for (fileNumber = 1; fileNumber <= fileCount; fileNumber++)
        {
        /*
        **  Process all files in the format ir%04d.
        */
        sprintf(inFileName, "ir%04d", fileNumber);

        /*
        **  Open a file and write zero length NOS I-format record if open fails.
        */
	    ifd = fopen(inFileName, "rb");
        if (ifd == NULL)
            {
            /*
            **  Create zero length NOS I-format record.
            */
            writeZeroNosItoTap(0);
            continue;
            }

        printf("processing %d\n", fileNumber);

        writeNosItoTap();

        fclose(ifd);
        }

    writeZeroNosItoTap(017);
	fclose (ofd);
    }

/*---------------------------  End Of File  ------------------------------*/

