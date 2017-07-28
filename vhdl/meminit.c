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

void storebyte (int byte, membyte memp)
{
    int i;
    
    // Convert 8-bit integer to bit_vector
    for (i = 0; i < 8; i++)
    {
        if (byte & 0200)
        {
            memp[i] = one;
            //DDPRINTF ("1");
        }
        else 
        {
            memp[i] = zero;
            //DDPRINTF ("0");
        }
        byte <<= 1;
    }
    //DDPRINTF ("\n");
}

int getbyte (membyte memp)
{
    int i, ret = 0, bit = 0200;
    
    // Convert bit_vector to 8-bit integer
    for (i = 0; i < 8; i++)
    {
        if (memp[i] == one)
            ret |= bit;
        bit >>= 1;
    }

    return ret;
}


void meminit (int bnum, int offset, membyte memp)
{
    int i, pp, byte, bank;
    char fn[20];
    FILE *f;
    ppword *p;
    cpword *c;

    // bnum is (2 * pp number) + byte number for pp memory,
    // or 100 + (8 * bank) + byte number for cp memory.
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
            while (pplen[pp] < 4096 && !feof (f) && fscanf (f, "%ho", p++) != 0) 
            {
                pplen[pp]++;
            }
            fclose (f);
        }
        if (pplen[pp] >= 0)
        {
            p = ppdata[pp];
            if (offset < pplen[pp])
            {
                if (byte)
                {
                    storebyte (p[offset], memp);
                }
                else
                {
                    storebyte (p[offset] >> 8, memp);
                }
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
            cpdata = c = malloc (4096 * 32 * sizeof (cpword));
            if (c == NULL)
            {
                perror ("malloc");
                exit (1);
            }
            while (cplen < 4096 * 32 && !feof (f) && fscanf (f, "%llo", c++) != 0) 
            {
                cplen++;
            }
            fclose (f);
        }
        if (cplen >= 0)
        {
            offset = offset * 32 + bank;
            if (offset < cplen)
            {
                storebyte ((cpdata[offset] >> ((7 - byte) * 8)) & 0xff, memp);
            }
        }
    }
}

    
void memwrite (int bnum, int offset, membyte memp)
{
    int i, pp, byte, bank;
    char fn[20];
    FILE *f;
    static ppword p[10];
    static cpword c[32];
    cpword t;
    static int lastppoffset[10];
    static int lastcpoffset[32];

    // bnum is (2 * pp number) + byte number for pp memory,
    // or 100 + (8 * bank) + byte number for cp memory.
    // byte number is 0..n from high to low (0..1 for pp, 0..7 for cp)

    if (bnum < 100)
    {
        pp = bnum / 2;
        byte = bnum % 2;
        DDPRINTF ("memwrite pp %d byte %d\n", pp, byte);
        // PP memory is written bank 0 to 1
        if (byte == 0)
        {
            p[pp] = 0;
        }
        if (byte)
        {
            p[pp] |= getbyte (memp);
        }
        else
        {
            p[pp] |= getbyte (memp) << 8;
        }
        if (byte == 1)
        {
            DPRINTF ("pp%d write %04o: %04o\n", pp, offset, p[pp]);
        }        
    }
    else
    {
        bnum -= 100;
        bank = bnum / 8;
        byte = bnum % 8;
        // CP memory is written bank 7 to 0
        if (byte == 7)
        {
            c[bank] = 0;
        }
        t = ((cpword) (getbyte (memp))) << ((7 - byte) * 8);
        DDPRINTF ("memwrite cp bank %d byte %d is %020llo\n", bank, byte, t);
        c[bank] |= t;
        if (byte == 0)
        {
            DPRINTF ("cp write %06o: %020llo\n",
                     bank + (offset * 32), c[bank]);
        }        
    }
}

    
