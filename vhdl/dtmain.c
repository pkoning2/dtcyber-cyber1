#include <stdio.h>
#include <stdlib.h>
#include "vhdl.h"
#include "const.h"
#include "types.h"
#include "proto.h"

bool debugDisplay = FALSE;
bool opDebugging = FALSE;
CpWord *cpMem;
CpuContext cpu[1];
u8 cpuCount;
volatile int monitorCpu;
u32 ecsFlagRegister;
u32 ecsMaxMemory;
CpWord *ecsMem;
PpSlot *ppu;
PpSlot *activePpu;
u8 ppuCount;
bool emulationActive = TRUE;
u32 cycles;
u32 rtcClock = 0;
u8 rtcIncrement;
char autoDateString[32];
char autoString[32];
long opPort;
long opConns;
long npuNetTelnetPort;
long npuNetTelnetConns;
long pniPort;
long pniConns;
u16 deadstartPanel[MaxDeadStart];
u8 deadstartCount;
long extSockets;
bool opPlatoAlert;

void opSetMsg (const char *p)
{
}

void opSetStatus (void *buf, const char *msg)
{
}

void * opInitStatus (const char *type, int ch, int un)
{
}

void opUpdateSysStatus (void)
{
}


bool cpuEcsFlagRegister(u32 ecsAddress)
{
}

bool cpuDdpTransfer(u32 ecsAddress, CpWord *data, bool writeToEcs)
{
}

void dumpCpu(void)
{
}

void dumpPpu(u8 pp)
{
}

void dumpCpuMem(FILE *f, u32 start, u32 end, u32 ra)
{
}

void dumpPpuMem(FILE *f, u8 pp, u32 start, u32 end)
{
}

void dumpCpuInfo (FILE *f)
{
}

void dumpPpuInfo (FILE *pf, u8 pp)
{
}

void cpuInit(char *model, u32 count, u32 memory, u32 ecsBanks,
             char *cmFile, char *ecsFile)
{
}

void ppInit(u8 count)
{
}

void rtcInit(char *model, u8 increment, long setMHz)
{
}

void npuInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
{
}

void initExt (void)
{
}

void initPni (void)
{
}

void dtmain (void)
{
    initStartup ("vhdl");
}
