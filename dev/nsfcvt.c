/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter, Gerard van der Grinten, Paul Koning
**  (see license.txt)
**
**  Name: nsfcvt.c
**
**  Description:
**      Format conversion for dd8xx image files
**      This variant converts NSF PLATO images (8 bytes per 60 bits)
**      to DtCyber packed form.
**
**--------------------------------------------------------------------------
*/

#include <stdio.h>

#define NEW_ORDER 1     // new ones have the 8 bytes reversed from the old order

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
**  Note that it does NOT advance the current sector.
**
**------------------------------------------------------------------------*/
static int dd8xxReadSector(FILE *fcb, void *buf)
{
    int i, j;
    u8 *bp;
    PpWord *pp;
    int sectorSize;
    CpWord t;
    
    sectorSize = SectorBytes;

    if (fread(sec, 1, sectorSize, fcb) != sectorSize)
        return 0;
    
    bp = sec;
    pp = (PpWord *) buf;
    *pp++ = 0;
    *pp++ = 0;
    
    for (i = 0; i < 64 * 8; i += 8)
    {
        t = 0;
#if NEW_ORDER
        bp += 8;
        for (j = 0; j < 8; j++)
            t = (t << 8) + *--bp;
        bp += 8;
#else
        for (j = 0; j < 8; j++)
            t = (t << 8) + *bp++;
#endif
        for (j = 0; j < 5; j++)
        {
            *pp++ = (t >> 48) & 07777;
            t <<= 12;
        }
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
**  Note that it does NOT advance the current sector.
**
**------------------------------------------------------------------------*/
static void dd8xxWriteSector(FILE *fcb, const void *buf)
{
    int i;
    u8 *bp;
    const PpWord *pp;
    int sectorSize;
    
    sectorSize = SectorBytes;
    
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
    while (dd8xxReadSector (inf, secbuf))
        dd8xxWriteSector (outf, secbuf);
    fclose (inf);
    fclose (outf);
    return 0;
}
