#define DEBUG 1
//-----------------------------------------------------------------------------
//
// CDC 6600 model
//
// Authors: Paul Koning, Dave Redell
//
// Based on the original design by Seymour Cray and his team
//
//-----------------------------------------------------------------------------
//
// This function is invoked from VHDL code for a memory bank when
// reset is asserted.  It is used to supply initial data.

#include <stdio.h>
#include <stdlib.h>
#include "vhdl.h"

#ifdef DEBUG
#define DPRINTF printf
#if DEBUG > 1
#define DDPRINTF printf
#else
#define DDPRINTF(f...)
#endif
#else
#define DPRINTF(f...)
#endif

typedef unsigned short ppword;
typedef unsigned long long cpword;

ppword *ppdata[10];
cpword *cpdata;
int pplen[10];
int cplen;

void storebyte (int byte, membyte *memp)
{
    int i;
    enum bit *bp = &memp[0][0];
    
    // Convert 8-bit integer to bit_vector
    for (i = 0; i < 8; i++)
    {
        if (byte & 0200)
        {
            *bp++ = one;
            //DDPRINTF ("1");
        }
        else 
        {
            *bp++ = zero;
            //DDPRINTF ("0");
        }
        byte <<= 1;
    }
    //DDPRINTF ("\n");
}

void meminit (int bnum, membyte *memp)
{
    int i, pp, byte, bank;
    char fn[20];
    FILE *f;
    ppword *p;
    cpword *c;

    // bnum is (2 * pp number) + byte number for pp memory,
    // or 100 * (8 * bank) + byte number for cp memory.
    // byte number is 0..n from high to low (0..1 for pp, 0..7 for cp)

    if (bnum < 100)
    {
        pp = bnum / 2;
        byte = bnum % 2;
        DDPRINTF ("meminit pp %d byte %d\n", pp, byte);
        if (pplen[pp] == 0)
        {
            sprintf (fn, "pp%d.dat", pp);
            f = fopen (fn, "r");
            if (f == NULL)
            {
                DPRINTF ("no init data %s\n", fn);
                pplen[pp] = -1;
                return;
            }
            ppdata[pp] = p = malloc (4096 * sizeof (ppword));
            if (p == NULL)
            {
                perror ("malloc");
                exit (1);
            }
            while (fscanf (f, "%ho", p++) != 0) 
            {
                pplen[pp]++;
            }
            fclose (f);
        }
        if (pplen[pp] >= 0)
        {
            p = ppdata[pp];
            i = pplen[pp];
            while (--i >= 0)
            {
                if (byte)
                {
                    storebyte (*p++, memp);
                }
                else
                {
                    storebyte ((*p++) >> 8, memp);
                }
                memp += sizeof (membyte);
            }
        }
    }
    else
    {
        bnum -= 100;
        bank = bnum / 8;
        byte = bnum % 8;
        DDPRINTF ("meminit cp bank %d byte %d\n", bank, byte);
        if (cplen == 0)
        {
            f = fopen ("cp.dat", "r");
            if (f == NULL)
            {
                DPRINTF ("no init data cp.dat\n");
                cplen = -1;
                return;
            }
            cpdata = c = malloc (4096 * sizeof (cpword));
            if (c == NULL)
            {
                perror ("malloc");
                exit (1);
            }
            while (fscanf (f, "%llo", c++) != 0) 
            {
                cplen++;
            }
            fclose (f);
        }
        if (cplen >= 0)
        {
            c = cpdata;
            i = cplen;
            while (--i >= 0)
            {
                storebyte (((*p++) >> ((7 - byte) * 8)) & 0xff, memp);
                memp += sizeof (membyte);
            }
        }
    }
}

    
