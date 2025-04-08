/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter, Gerard van der Grinten, Paul Koning
**  (see license.txt)
**
**  Name: cerl2mf.c
**
**  Description:
**      Format conversion for dd8xx image files
**      This variant converts CERL PLATO images (with 12 block of stuff,
**      such as the flaw map, preceding the pack label) to a Masterfile.
**      Both input and output are DtCyber packed form; the input is
**      a disk image (one Cyber sector per 512 host bytes); the output
**      is packed data (sectors adjacent to each other, no padding).
**
**--------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SectorSize              322
#define SectorBytes             512
#define Mask12  07777

typedef unsigned char u8;
typedef unsigned short PpWord;
typedef unsigned long long CpWord;

PpWord secbuf[SectorSize];
u8 sec[SectorSize * 2];
u8 wsec[SectorSize * 2];


/*--------------------------------------------------------------------------
**  Purpose:        Read sector to a local buffer
**
**  Parameters:     Name        Description.
**                  fcb         FILE struct for input disk image file
**                  buf         buffer pointer (PpWord[322])
**
**  Returns:        Nothing.
**
**  This routine reads the sector set by the most recent seek.
**
**------------------------------------------------------------------------*/
static int dd8xxReadSector(FILE *fcb, void *buf)
{
    int i;
    u8 *bp;
    PpWord *pp;
    PpWord p1, p2;
    int sectorSize;
    
    sectorSize = 512;

    if (fread(sec, 1, sectorSize, fcb) != sectorSize)
        return 0;
    
    bp = sec;
    bp += 3;	    // skip the first 2 PP words (3 bytes)
    pp = (PpWord *) buf;
    for (i = 2; i < 0502; i += 2)
    {
        p1 = *bp++ << 4;
        p1 += *bp >> 4;
        p2 = (*bp++ & 0x0f) << 8;
        p2 += *bp++;
        *pp++ = p1;
        *pp++ = p2;
    }
    return 1;
}


/*--------------------------------------------------------------------------
**  Purpose:        Write sector from a local buffer
**
**  Parameters:     Name        Description.
**                  fcb         FILE struct for input disk image file
**                  buf         buffer pointer (PpWord[322])
**
**  Returns:        Nothing.
**
**  This routine writes the sector set by the most recent seek.
**
**------------------------------------------------------------------------*/
static void dd8xxWriteSector(FILE *fcb, const void *buf)
{
    int i;
    u8 *bp;
    const PpWord *pp;
    int sectorSize;
    
    sectorSize = 480;
    
    bp = wsec;
    pp = (const PpWord *) buf;
    for (i = 0; i < SectorSize; i += 2)
    {
        *bp++ = *pp >> 4;
        *bp = *pp++ << 4;
        *bp++ |= *pp >> 8;
        *bp++ = *pp++;
    }
    if (fwrite(wsec, 1, sectorSize, fcb) != sectorSize)
    {
        perror ("dd8xxWriteSector");
        exit (1);
    }
}

int main (int argc, char **argv)
{
    int i;
    
    FILE *inf, *outf;
    inf = fopen (argv[1], "rb");
    if (inf == 0)
    {
        perror ("input file open");
        exit (1);
    }
    outf = fopen (argv[2], "wb");
    if (outf == 0)
    {
        perror ("output file open");
        exit (1);
    }
    memset (secbuf, 0, sizeof (secbuf));

    // Write the initial 0 block
    for (i = 0; i < 5; i++)
        dd8xxWriteSector (outf, secbuf);

    // Skip to the label block
    for (i = 0; i < 60; i++)
        dd8xxReadSector (inf, secbuf);

    // Copy the pack directory
    for (i = 0; i < 80; i++)
    {
        dd8xxReadSector (inf, secbuf);
        if (i == 0)
        {
            // Tweak the pack label
            secbuf[10] = 0303;
            secbuf[11] = 0;
        }
        
        dd8xxWriteSector (outf, secbuf);
    }

    memset (secbuf, 0, sizeof (secbuf));

    // Write 11 blocks of filler
    for (i = 0; i < 55; i++)
        dd8xxWriteSector (outf, secbuf);

    // Copy the data
    i = 0;
    while (dd8xxReadSector (inf, secbuf))
    {
        i++;
        dd8xxWriteSector (outf, secbuf);
    }
    printf ("%d block copied\n", i / 5);
    
    fclose (inf);
    fclose (outf);
    return 0;
}
