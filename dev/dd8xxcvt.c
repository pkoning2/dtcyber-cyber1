/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter, Gerard van der Grinten, Paul Koning
**  (see license.txt)
**
**  Name: dd8xxcvt.c
**
**  Description:
**      Format conversion for dd8xx image files
**
**--------------------------------------------------------------------------
*/

#include <stdio.h>

#define SectorSize              322
#define SectorBytes             512
#define Mask12  07777

typedef unsigned char u8;
typedef unsigned short PpWord;

PpWord secbuf[SectorSize];


/*--------------------------------------------------------------------------
**  Purpose:        Read sector to a local buffer
**
**  Parameters:     Name        Description.
**                  fcb         FILE struct for input disk image file
**                  oldFormat   1 if old format image, 0 if new format
**                  buf         buffer pointer (PpWord[322])
**
**  Returns:        Nothing.
**
**  This routine reads the sector set by the most recent seek.
**  Note that it does NOT advance the current sector.
**
**------------------------------------------------------------------------*/
static int dd8xxReadSector(FILE *fcb, int oldFormat, void *buf)
{
    int i;
    u8 *bp;
    PpWord *pp;
    int sectorSize;
    u8 sec[SectorSize * 2];

    sectorSize = (oldFormat) ? SectorSize * 2 : SectorBytes;

    if (fread(sec, 1, sectorSize, fcb) != sectorSize)
        return 0;
    
    bp = sec;
    pp = (PpWord *) buf;
    if (oldFormat)
    {
        for (i = 0; i < SectorSize; i++)
        {
            *pp++ = *(PpWord *) bp;
            bp += sizeof (PpWord);
        }
    }
    else
    {
        for (i = 0; i < SectorSize; i += 2)
        {
            *pp++ = (*bp << 4) + (*(bp + 1) >> 4);
            *pp++ = (*(bp + 1) << 8) + *(bp + 2);
            bp += 3;
        }
    }
    return 1;
}


/*--------------------------------------------------------------------------
**  Purpose:        Write sector from a local buffer
**
**  Parameters:     Name        Description.
**                  fcb         FILE struct for input disk image file
**                  oldFormat   1 if old format image, 0 if new format
**                  buf         buffer pointer (PpWord[322])
**
**  Returns:        Nothing.
**
**  This routine writes the sector set by the most recent seek.
**  Note that it does NOT advance the current sector.
**
**------------------------------------------------------------------------*/
static void dd8xxWriteSector(FILE *fcb, int oldFormat, const void *buf)
{
    int i;
    u8 *bp;
    const PpWord *pp;
    int sectorSize;
    u8 sec[SectorSize * 2];
    
    sectorSize = (oldFormat) ? SectorSize * 2 : SectorBytes;

    bp = sec;
    pp = (const PpWord *) buf;
    if (oldFormat)
    {
        for (i = 0; i < SectorSize; i++)
        {
            *(PpWord *) bp = *pp++ & Mask12;
            bp += sizeof (PpWord);
        }
    }
    else
    {
        for (i = 0; i < SectorSize; i += 2)
        {
            *bp++ = *pp >> 4;
            *bp = *pp++ << 4;
            *bp++ |= *pp >> 8;
            *bp++ = *pp++;
            }
        }
    if (fwrite(sec, 1, sectorSize, fcb) != sectorSize)
    {
        perror ("dd8xxWriteSector");
        exit (1);
    }
}

int main (int argc, char **argv)
{
    FILE *inf, *outf;
    int readOld, writeOld;
    
    if (strcmp (argv[1], "-o") == 0)
    {
        readOld = 0;
        writeOld = 1;
    }
    else if (strcmp (argv[1], "-n") == 0)
    {
        readOld = 1;
        writeOld = 0;
    }
    else
    {
        fprintf (stderr, "need -n (convert to new) or -o (convert to old)\n");
        exit (1);
    }
    inf = fopen (argv[2], "rb");
    if (inf == 0)
    {
        perror ("input file open");
        exit (1);
    }
    outf = fopen (argv[3], "wb");
    if (outf == 0)
    {
        perror ("output file open");
        exit (1);
    }
    while (dd8xxReadSector (inf, readOld, secbuf))
        dd8xxWriteSector (outf, writeOld, secbuf);
    fclose (inf);
    fclose (outf);
    return 0;
}
