/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: dump.c
**
**  Description:
**      Perform dump of PP and CPU memory as well as post-mortem
**      disassembly of PP memory.
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
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

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

/*
**  ----------------
**  Public Variables
**  ----------------
*/

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static FILE *cpuDF;
static FILE **ppuDF;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialize dumping.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dumpInit(void)
    {
    u8 pp;
    char ppDumpName[20];

    ppuDF = calloc(ppuCount, sizeof(FILE *));
    if (ppuDF == NULL)
        {
        fprintf(stderr, "Failed to allocate ppu dump file handles\n");
        exit(1);
        }

    cpuDF = fopen("cpu.dmp", "wt");
    if (cpuDF == NULL)
        {
        ppAbort((stderr, "can't open cpu dump"));
        }

    for (pp = 0; pp < ppuCount; pp++)
        {
        sprintf(ppDumpName, "ppu%02o.dmp", pp);
        ppuDF[pp] = fopen(ppDumpName, "wt");
        if (ppuDF[pp] == NULL)
            {
            ppAbort((stderr, "can't open ppu[%02o] dump", pp));
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Dump all PPs and CPU.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dumpAll(void)
    {
    u8 pp;

    fprintf(stderr, "dumping core...");
    fflush(stderr);

    dumpCpu();
    for (pp = 0; pp < ppuCount; pp++)
        {
        dumpPpu(pp);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Dump CPU.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dumpCpu(void)
    {
    u8 i;
    CpWord data;

    fprintf(cpuDF, "P       %06o  ", cpu.regP);
    fprintf(cpuDF, "A%d %06o  ", 0, cpu.regA[0]);
    fprintf(cpuDF, "B%d %06o", 0, cpu.regB[0]);
    fprintf(cpuDF, "\n");
                           
    fprintf(cpuDF, "RAcm    %06o  ", cpu.regRaCm);
    fprintf(cpuDF, "A%d %06o  ", 1, cpu.regA[1]);
    fprintf(cpuDF, "B%d %06o", 1, cpu.regB[1]);
    fprintf(cpuDF, "\n");
                           
    fprintf(cpuDF, "FLcm    %06o  ", cpu.regFlCm);
    fprintf(cpuDF, "A%d %06o  ", 2, cpu.regA[2]);
    fprintf(cpuDF, "B%d %06o", 2, cpu.regB[2]);
    fprintf(cpuDF, "\n");
                           
    fprintf(cpuDF, "RAecs   %06o  ", cpu.regRaEcs);
    fprintf(cpuDF, "A%d %06o  ", 3, cpu.regA[3]);
    fprintf(cpuDF, "B%d %06o", 3, cpu.regB[3]);
    fprintf(cpuDF, "\n");
                           
    fprintf(cpuDF, "FLecs   %06o  ", cpu.regFlEcs);
    fprintf(cpuDF, "A%d %06o  ", 4, cpu.regA[4]);
    fprintf(cpuDF, "B%d %06o", 4, cpu.regB[4]);
    fprintf(cpuDF, "\n");
                           
    fprintf(cpuDF, "EM    %08o  ", cpu.exitMode);
    fprintf(cpuDF, "A%d %06o  ", 5, cpu.regA[5]);
    fprintf(cpuDF, "B%d %06o", 5, cpu.regB[5]);
    fprintf(cpuDF, "\n");
                           
    fprintf(cpuDF, "MA      %06o  ", cpu.regMa);
    fprintf(cpuDF, "A%d %06o  ", 6, cpu.regA[6]);
    fprintf(cpuDF, "B%d %06o", 6, cpu.regB[6]);
    fprintf(cpuDF, "\n");
                           
    fprintf(cpuDF, "ECOND       %02o  ", cpu.exitCondition);
    fprintf(cpuDF, "A%d %06o  ", 7, cpu.regA[7]);
    fprintf(cpuDF, "B%d %06o  ", 7, cpu.regB[7]);
    fprintf(cpuDF, "\n");
    fprintf(cpuDF, "STOP         %d  ", cpuStopped ? 1 : 0);
    fprintf(cpuDF, "\n");
    fprintf(cpuDF, "\n");

    for (i = 0; i < 8; i++)
        {
        fprintf(cpuDF, "X%d ", i);
        data = cpu.regX[i];
        fprintf(cpuDF, "%04o %04o %04o %04o %04o   ",
            (PpWord)((data >> 48) & Mask12),
            (PpWord)((data >> 36) & Mask12),
            (PpWord)((data >> 24) & Mask12),
            (PpWord)((data >> 12) & Mask12),
            (PpWord)((data      ) & Mask12));
        fprintf(cpuDF, "\n");
        }

    fprintf(cpuDF, "\n");
    dumpCpuMem (cpuDF, 0, cpuMaxMemory);
    }


/*--------------------------------------------------------------------------
**  Purpose:        Dump a range of CPU memory.
**
**  Parameters:     Name        Description.
**                  start       Start address.
**                  end         LWA + 1 of dump.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dumpCpuMem(FILE *f, u32 start, u32 end)
    {
    u32 addr;
    CpWord data, data2;
    CpWord lastData, lastData2;
    bool duplicateLine;
    u8 ch;
    u8 i;
    u8 shiftCount;

    if (f == NULL)
        {
        f = cpuDF;
        }
    lastData = ~cpMem[start];
    duplicateLine = FALSE;
    for (addr = start; addr < end; addr += 2)
        {
        data = cpMem[addr];
        if (addr + 1 == end)
            {
            // odd dump length
            fprintf(f, "%06o  ", addr & Mask18);
            fprintf(f, 
                    "%04o %04o %04o %04o %04o   "
                    "                          ",
                    (PpWord)((data >> 48) & Mask12),
                    (PpWord)((data >> 36) & Mask12),
                    (PpWord)((data >> 24) & Mask12),
                    (PpWord)((data >> 12) & Mask12),
                    (PpWord)((data      ) & Mask12));

            shiftCount = 60;
            for (i = 0; i < 10; i++)
                {
                shiftCount -= 6;
                ch = (u8)((data >> shiftCount) & Mask6);
                fprintf(f, "%c", cdcToAscii[ch]);
                }
            fprintf(f, "\n");
            break;
            }
        data2 = cpMem[addr + 1];        
        if (data == lastData && data2 == lastData2)
            {
            if (!duplicateLine)
                {
                fprintf(f, "     DUPLICATED LINES.\n");
                duplicateLine = TRUE;
                }
            }
        else
            {
            duplicateLine = FALSE;
            lastData = data;
            lastData2 = data2;
            fprintf(f, "%06o  ", addr & Mask18);
            fprintf(f,
                    "%04o %04o %04o %04o %04o  "
                    "%04o %04o %04o %04o %04o ",
                (PpWord)((data >> 48) & Mask12),
                (PpWord)((data >> 36) & Mask12),
                (PpWord)((data >> 24) & Mask12),
                (PpWord)((data >> 12) & Mask12),
                (PpWord)((data      ) & Mask12),
                (PpWord)((data2 >> 48) & Mask12),
                (PpWord)((data2 >> 36) & Mask12),
                (PpWord)((data2 >> 24) & Mask12),
                (PpWord)((data2 >> 12) & Mask12),
                (PpWord)((data2      ) & Mask12));

            shiftCount = 60;
            for (i = 0; i < 10; i++)
                {
                shiftCount -= 6;
                ch = (u8)((data >> shiftCount) & Mask6);
                fprintf(f, "%c", cdcToAscii[ch]);
                }
            shiftCount = 60;
            for (i = 0; i < 10; i++)
                {
                shiftCount -= 6;
                ch = (u8)((data2 >> shiftCount) & Mask6);
                fprintf(f, "%c", cdcToAscii[ch]);
                }
            }

        if (!duplicateLine) 
            {
            fprintf(f, "\n");
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Dump PPU.
**
**  Parameters:     Name        Description.
**                  pp          PPU number.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dumpPpu(u8 pp)
    {
    FILE *pf = ppuDF[pp];

    fprintf(pf, "P   %04o\n", ppu[pp].regP);
    fprintf(pf, "A %06o\n", ppu[pp].regA);
    fprintf(pf, "IO    %02o\n", ppu[pp].ioWaitType);
    fprintf(pf, "STOP  %02o\n", ppu[pp].stopped);
    fprintf(pf, "\n");
    dumpPpuMem (pf, pp, 0, PpMemSize);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Dump PPU memory.
**
**  Parameters:     Name        Description.
**                  pp          PPU number.
**                  start       Start address.
**                  end         LWA + 1 of dump.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dumpPpuMem(FILE *f, u8 pp, u32 start, u32 end)
    {
    u32 addr;
    PpWord *pm = ppu[pp].mem;
    u8 i;
    PpWord pw;
    bool duplicateLine = FALSE;
    bool dl;
    
    if (f == NULL)
        {
        f = ppuDF[pp];
        }
    for (addr = start; addr < end; addr += 8)
        {
        if (addr > start)
            {
            dl = TRUE;
            for (i = 0; i < 8; i++)
                {
                if ((pm[addr + i] & Mask12) != (pm[addr + i - 8] & Mask12))
                    {
                    dl = FALSE;
                    break;
                    }
                }
            if (dl)
                {
                if (!duplicateLine)
                    {
                    fprintf(f, "     DUPLICATED LINES.\n");
                    duplicateLine = TRUE;
                    }
                continue;
                }
            }
        duplicateLine = FALSE;
        fprintf(f, "%04o   ", addr & Mask12);
        for (i = 0; i < 8; i++)
            {
            if (addr + i < end)
                {
                if ((pm[addr + i] & Mask12) == 0)
                    {
                    fprintf (f, "---- ");
                    }
                else
                    {
                    fprintf (f, "%04o ", pm[addr + i] & Mask12);
                    }
                }
            else
                {
                fprintf (f, "     ");
                }
            fprintf (f, " ");
            }

        for (i = 0; i < 8; i++)
            {
            if (addr + i >= end)
                {
                break;
                }
            pw = pm[addr + i] & Mask12;
            fprintf(f, "%c%c", cdcToAscii[(pw >> 6) & Mask6], cdcToAscii[pw & Mask6]);
            }

        fprintf(f, "\n");
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Disassemble PPU memory.
**
**  Parameters:     Name        Description.
**                  pp          PPU number.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dumpDisassemblePpu(u8 pp)
    {
    u32 addr;
    PpWord *pm = ppu[pp].mem;
    char ppDisName[20];
    FILE *pf;
    char str[80];
    u8 cnt = 0;
    PpWord pw0, pw1;
    bool duplicateLine = FALSE;

    sprintf(ppDisName, "ppu%02o.dis", pp);
    pf = fopen(ppDisName, "wt");
    if (pf == NULL)
        {
        ppAbort((stderr, "can't open %s", ppDisName));
        return;
        }

    fprintf(pf, "P   %04o\n", ppu[pp].regP);
    fprintf(pf, "A %06o\n", ppu[pp].regA);
    fprintf(pf, "IO    %02o\n", ppu[pp].ioWaitType);
    fprintf(pf, "STOP  %02o\n", ppu[pp].stopped);
    fprintf(pf, "\n");

    for (addr = 0100; addr < PpMemSize; addr += cnt)
        {
        if ((cnt == 1 && pm[addr - 1] == pm[addr]) ||
            (cnt == 2 && pm[addr - 2] == pm[addr] &&
             pm[addr - 1] == pm[addr + 1]))
            {
            if (!duplicateLine)
                {
                fprintf(pf, "     DUPLICATED LINES.\n");
                duplicateLine = TRUE;
                }
            continue;
            }
        duplicateLine = FALSE;
        fprintf(pf, "%04o  ", addr & Mask12);

        cnt = traceDisassembleOpcode(str, pm + addr);
        fputs(str, pf);

        pw0 = pm[addr] & Mask12;
        pw1 = pm[addr + 1] & Mask12;
        fprintf(pf, "   %04o ", pw0);
        if (cnt == 2)
            {
            fprintf(pf, "%04o  ", pw1);
            }
        else
            {
            fprintf(pf, "      ");
            }
            
        fprintf(pf, "  %c%c", cdcToAscii[(pw0 >> 6) & Mask6], cdcToAscii[pw0 & Mask6]);

        if (cnt == 2)
            {
            fprintf(pf, "%c%c", cdcToAscii[(pw1 >> 6) & Mask6], cdcToAscii[pw1 & Mask6]);
            }

        fprintf(pf, "\n");
        }
    }

/*---------------------------  End Of File  ------------------------------*/
