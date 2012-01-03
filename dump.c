/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
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
static void dumpMem(FILE *f, u32 start, u32 end, u32 ra, CpWord *mem);

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

    ppuDF = calloc(ppuCount, sizeof(FILE *));
    if (ppuDF == NULL)
        {
        fprintf(stderr, "Failed to allocate ppu dump file handles\n");
        exit(1);
        }

    }

/*--------------------------------------------------------------------------
**  Purpose:        Terminate dumping.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dumpTerminate(void)
    {
    u8 pp;

    if (cpuDF != NULL)
        {
        fclose(cpuDF);
        }

    for (pp = 0; pp < ppuCount; pp++)
        {
        if (ppuDF[pp] != NULL)
            {
            fclose(ppuDF[pp]);
            }
        }

    free(ppuDF);
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
**  Purpose:        Dump CPU info (exchange packages, basically)
**
**  Parameters:     Name        Description.
**                  f           File descriptor to write to.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dumpCpuInfo (FILE *f)
    {
    u8 i, j;
    CpWord data;

    for (i = 0; i < cpuCount; i++)
        {
        fprintf (f, "Cpu %d%s:\n", i,
                 (monitorCpu == i) ? " (in monitor mode)" : "");
        
        fprintf(f, "P       %06o  ", cpu[i].regP);
        fprintf(f, "A%d %06o  ", 0, cpu[i].regA[0]);
        fprintf(f, "B%d %06o", 0, cpu[i].regB[0]);
        fprintf(f, "\n");
                           
        fprintf(f, "RAcm    %06o  ", cpu[i].regRaCm);
        fprintf(f, "A%d %06o  ", 1, cpu[i].regA[1]);
        fprintf(f, "B%d %06o", 1, cpu[i].regB[1]);
        fprintf(f, "\n");
                           
        fprintf(f, "FLcm    %06o  ", cpu[i].regFlCm);
        fprintf(f, "A%d %06o  ", 2, cpu[i].regA[2]);
        fprintf(f, "B%d %06o", 2, cpu[i].regB[2]);
        fprintf(f, "\n");
                           
        fprintf(f, "RAecs %08o  ", cpu[i].regRaEcs);
        fprintf(f, "A%d %06o  ", 3, cpu[i].regA[3]);
        fprintf(f, "B%d %06o", 3, cpu[i].regB[3]);
        fprintf(f, "\n");
                           
        fprintf(f, "FLecs %08o  ", cpu[i].regFlEcs);
        fprintf(f, "A%d %06o  ", 4, cpu[i].regA[4]);
        fprintf(f, "B%d %06o", 4, cpu[i].regB[4]);
        fprintf(f, "\n");
                           
        fprintf(f, "EM    %08o  ", cpu[i].exitMode);
        fprintf(f, "A%d %06o  ", 5, cpu[i].regA[5]);
        fprintf(f, "B%d %06o", 5, cpu[i].regB[5]);
        fprintf(f, "\n");
                           
        fprintf(f, "MA      %06o  ", cpu[i].regMa);
        fprintf(f, "A%d %06o  ", 6, cpu[i].regA[6]);
        fprintf(f, "B%d %06o", 6, cpu[i].regB[6]);
        fprintf(f, "\n");
                           
        fprintf(f, "ECOND       %02o  ", cpu[i].exitCondition);
        fprintf(f, "A%d %06o  ", 7, cpu[i].regA[7]);
        fprintf(f, "B%d %06o  ", 7, cpu[i].regB[7]);
        fprintf(f, "\n");
        fprintf(f, "STOP         %d  ", cpu[i].cpuStopped ? 1 : 0);
        fprintf(f, "\n");
        fprintf(f, "\n");
    
        for (j = 0; j < 8; j++)
            {
            fprintf(f, "X%d ", j);
            data = cpu[i].regX[j];
            fprintf(f, "%04o %04o %04o %04o %04o   ",
                    (PpWord)((data >> 48) & Mask12),
                    (PpWord)((data >> 36) & Mask12),
                    (PpWord)((data >> 24) & Mask12),
                    (PpWord)((data >> 12) & Mask12),
                    (PpWord)((data      ) & Mask12));
            fprintf(f, "\n");
            }

        fprintf(f, "\n");
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

    if (cpuDF == NULL)
        {
        cpuDF = fopen("cpu.dmp", "wt");
        if (cpuDF == NULL)
            {
            logError(LogErrorLocation, "can't open cpu dump");
            }
        }

    dumpCpuInfo (cpuDF);
    dumpCpuMem (cpuDF, 0, cpuMaxMemory, 0);
    }


/*--------------------------------------------------------------------------
**  Purpose:        Dump a range of CPU memory.
**
**  Parameters:     Name        Description.
**                  f           File descriptor to write to.
**                  start       Start address.
**                  end         LWA + 1 of dump.
**                  ra          RA to apply to start and end addresses
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dumpCpuMem(FILE *f, u32 start, u32 end, u32 ra)
    {
    dumpMem(f, start, end, ra, cpMem);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Dump a range of ECS memory.
**
**  Parameters:     Name        Description.
**                  f           File descriptor to write to.
**                  start       Start address.
**                  end         LWA + 1 of dump.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dumpEcs(FILE *f, u32 start, u32 end)
    {
    dumpMem(f, start, end, 0, ecsMem);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Dump PPU info (processor state).
**
**  Parameters:     Name        Description.
**                  f           File descriptor to write to.
**                  pp          PPU number.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dumpPpuInfo (FILE *pf, u8 pp)
    {
    fprintf(pf, "P   %04o\n", ppu[pp].regP);
    fprintf(pf, "A %06o\n", ppu[pp].regA);
    fprintf(pf, "IO    %02o\n", ppu[pp].ioWaitType);
    fprintf(pf, "STOP  %02o\n", ppu[pp].stopped);
    fprintf(pf, "\n");
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
    char ppDumpName[20];
    FILE *pf;

    if (ppuDF[pp] == NULL)
        {
        sprintf(ppDumpName, "ppu%02o.dmp", pp);
        ppuDF[pp] = fopen(ppDumpName, "wt");
        if (ppuDF[pp] == NULL)
            {
            logError(LogErrorLocation, "can't open ppu[%02o] dump", pp);
            return;
            }
        }
    pf = ppuDF[pp];

    dumpPpuInfo (pf, pp);
    dumpPpuMem (pf, pp, 0, PpMemSize);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Dump PPU memory.
**
**  Parameters:     Name        Description.
**                  f           File descriptor to write to.
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
    char ppDumpName[20];
    
    if (f == NULL)
        {
        if (ppuDF[pp] == NULL)
            {
            sprintf(ppDumpName, "ppu%02o.dmp", pp);
            ppuDF[pp] = fopen(ppDumpName, "wt");
            if (ppuDF[pp] == NULL)
                {
                logError(LogErrorLocation, "can't open ppu[%02o] dump", pp);
                return;
                }
            }
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
    fflush (f);
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
        logError(LogErrorLocation, "can't open %s", ppDisName);
        return;
        }

    fprintf(pf, "P   %04o\n", ppu[pp].regP);
    fprintf(pf, "A %06o\n", ppu[pp].regA);
    fprintf(pf, "IO    %02o\n", ppu[pp].ioWaitType);
    fprintf(pf, "STOP  %02o\n", ppu[pp].stopped);
    fprintf(pf, "\n");

    /* Dump the direct cells as data (not disassembly) */
    dumpPpuMem(pf, pp, 0, 077);
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
    fflush (pf);
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Dump a range of 60-bit memory.
**
**  Parameters:     Name        Description.
**                  file        File descriptor to write to
**                  start       Start address.
**                  end         LWA + 1 of dump.
**                  ra          RA to apply to start and end addresses
**                  mem         Start of memory array
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dumpMem(FILE *f, u32 start, u32 end, u32 ra, CpWord *mem)
    {
    u32 addr;
    CpWord data, data2;
    CpWord lastData, lastData2 = 0;
    bool duplicateLine;
    u8 ch;
    u8 i;
    u8 shiftCount;

    if (f == NULL)
        {
        if (cpuDF == NULL)
            {
            cpuDF = fopen("cpu.dmp", "wt");
            if (cpuDF == NULL)
                {
                logError(LogErrorLocation, "can't open cpu dump");
                }
            }
        f = cpuDF;
        }
    lastData = ~mem[start + ra];
    if (start + 1 + ra < cpuMaxMemory)
        {
        lastData2 = ~mem[start + 1 + ra];
        }
    duplicateLine = FALSE;
    for (addr = start; addr < end; addr += 2)
        {
        data = mem[addr + ra];
        if (addr + 1 == end)
            {
            /* odd dump length */
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
        data2 = mem[addr + 1 + ra];        
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
    fflush (f);
    }


/*---------------------------  End Of File  ------------------------------*/
