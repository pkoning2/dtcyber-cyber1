#ifndef PROTO_H
#define PROTO_H
/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: func.h
**
**  Description:
**      This file defines external function prototypes.
**
**--------------------------------------------------------------------------
*/

#include "dtnetsubs.h"

/*
**  --------------------
**  Function Prototypes.
**  --------------------
*/

/*
**  init.c
*/
void initStartup(const char *);
u32 initConvertEndian(u32 value);
bool initOpenSection(const char *name);
char *initGetNextLine(void);
long initSavePosition (void);
void initRestorePosition (long pos);

/*
**  deadstart.c
*/
void deadStart(void);

/*
**  rtc.c
*/
void rtcInit(char *model, u8 increment, long setMHz);
void rtcTick(void);

/*
**  channel.c
*/
void channelInit(u8 count);
void channelTerminate(void);
DevSlot *channelFindDevice(u8 channelNo, u8 devType);
DevSlot *channelAttach(u8 channelNo, u8 eqNo, u8 devType);
void channelFunction(PpWord funcCode);
void channelActivate(void);
void channelDisconnect(void);
void channelProbe(void);
void channelIo(void);
void channelStep(void);

/*
**  pp.c
*/
void ppInit(u8 count);
void ppTerminate(void);
void ppStep(void);

/*
**  cpu.c
*/
void cpuInit(char *model, u32 cpus, u32 memory, u32 ecsBanks, char *cmFile, char *ecsFile);
void cpuTerminate(void);
u32 cpuGetP(u8 cpnum);
void cpuStepAll(void);
bool cpuEcsFlagRegister(u32 ecsAddress);
bool cpuDdpTransfer(u32 ecsAddress, CpWord *data, bool writeToEcs);
bool cpuPpReadMem(u32 address, CpWord *data);
void cpuPpWriteMem(u32 address, CpWord data);
bool cpuEcsAccess(u32 address, CpWord *data, bool writeToEcs);
int cpuIssueExchange(u8 cpnum, u32 addr, int monitor);
CpWord * cpuAccessMem(CPUVARGS2 (CpWord, int length));

/*
**  dcc6681.c
*/
DevSlot *dcc6681Attach(u8 channelNo, u8 eqNo, u8 unitNo, u8 devType);
DevSlot *dcc6681FindDevice(u8 channelNo, u8 equipmentNo, u8 devType);
void dcc6681Interrupt(bool status);
void dcc6681InterruptDev(DevSlot *dp, bool status);

/*
**  mt607.c
*/
void mt607Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  mt669.c
*/
void mt669Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  cr405.c
*/
void cr405Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  cp3446.c
*/
void cp3446Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  cr3447.c
*/
void cr3447Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  lp1612.c
*/
void lp1612Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  lp3000.c
*/
void lp501Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void lp512Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  console.c
*/
void consoleInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void consoleCheckOutput(void);
void consoleSetKeyboardTrue(bool flag);

/*
**  dd6603.c
*/
void dd6603Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  dd8xx.c
*/
void dd844Init_2(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void dd844Init_4(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void dd885Init_1(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  dc7155.c
*/
void dc7155Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void dc7155Init_2(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void dc7155Init_3(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void dc7155Init_4(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
/*
**  dcc6681.c
*/
void dcc6681Terminate(DevSlot *dp);
void dcc6681Interrupt(bool status);

/*
**  ddp.c
*/
void ddpInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  mux6676.c
*/
void mux6676Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  niu.c
*/
void niuInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
CpWord niuConn (u32 portNo);
void niuDoAlert (int code);

/*
**  npu.c
*/
void npuInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  tpmux.c
*/
void tpMuxInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  trace.c
*/
void traceInit(void);
void traceStop(void);
void traceTerminate(void);
void traceSequence(void);
void traceRegisters(void);
void traceOpcode(void);
void tracePM(void);
u8 traceDisassembleOpcode(char *str, PpWord *pm);
void traceChannelFunction(PpWord funcCode);
void tracePrint(char *str);
void traceCpuPrint(CpuContext *activeCpu, char *str);
void traceChannel(u8 ch);
void traceEnd(void);
void traceReset(void);
void traceCpu(CpuContext *activeCpu, 
              u32 p, u8 opFm, u8 opI, u8 opJ, u8 opK, u32 opAddress);
void traceExchange(CpuContext *cc, u32 addr, char *title);
void traceCM(u32 start, u32 end);

/*
**  ext.c
*/
CpWord extOp (CPUVARGS1 (CpWord req));
void initExt (void);

/*
**  pni.c
*/
CpWord pniOp (CPUVARGS1 (CpWord req));
void initPni (void);
void pniCheck (void);
CpWord pniConn (u32 portNo);

/*
**  dump.c
*/
void dumpInit(void);
void dumpTerminate(void);
void dumpAll(void);
void dumpCpu(void);
void dumpCpuInfo(FILE *f);
void dumpCpuMem(FILE *f, u32 start, u32 end, u32 ra);
void dumpEcs(FILE *f, u32 start, u32 end);
void dumpPpu(u8 pp);
void dumpPpuInfo(FILE *f, u8 ppu);
void dumpPpuMem(FILE *f, u8 ppu, u32 start, u32 end);
void dumpDisassemblePpu(u8 pp);

/*
**  float.c
*/
CpWord floatAdd(CpWord v1, CpWord v2, bool doRound, bool doDouble);
CpWord floatMultiply(CpWord v1, CpWord v2, bool doRound, bool doDouble);
CpWord floatDivide(CpWord v1, CpWord v2, bool doRound);

/*
**  shift.c
*/
CpWord shiftLeftCircular(CpWord data, u32 count);
CpWord shiftRightArithmetic(CpWord data, u32 count);
CpWord shiftPack(CpWord coeff, u32 expo);
CpWord shiftUnpack(CpWord number, u32 *expo);
CpWord shiftNormalize(CpWord number, u32 *shift, bool round);
CpWord shiftMask(u8 count);

/*
**  operator.c
*/
void opInit(void);
void opSetMsg (const char *p);
void opSetStatus (void *buf, const char *msg);
void * opInitStatus (const char *type, int ch, int un);
void operCheckRequests (void);
void opUpdateSysStatus (void);

/*
**  log.c
*/
void logInit(void);
void logError(char *file, int line, char *fmt, ...);

/*
**  dtdisksubs.c
*/
bool ddOpen (DiskIO *io, const char *name, bool writable);
bool ddCreate (DiskIO *io, const char *name);
void ddClose (DiskIO *io);
bool ddIOPending (DiskIO *io);
bool ddLock (DiskIO *io);
void ddQueueRead (DiskIO *io, void *buf, int bytes);
void ddQueueWrite (DiskIO *io, const void *buf, int bytes);
void ddSeek (DiskIO *io, off_t pos);
void ddWaitIO (DiskIO *io);

/* For efficiency this is a define */
#define ddOpened(io) ((io)->fd > 0)

/*
**  -----------------
**  Global variables.
**  -----------------
*/

extern bool emulationActive;
extern PpSlot *ppu;
extern ChSlot *channel;
extern u8 ppuCount;
extern u8 channelCount;
extern PpSlot *activePpu;
extern ChSlot *activeChannel;
extern DevSlot *activeDevice;
extern DevSlot *active3000Device;
extern CpuContext cpu[];
extern u8 cpuCount;
extern volatile int monitorCpu;
extern volatile int exchangeCpu;
extern CpWord *cpMem;
extern CpWord *ecsMem;
extern u32 cpuMaxMemory;
extern u32 cpuMemMask;
extern u32 ecsMaxMemory;
extern int ppKeyIn;
extern const u8 asciiToCdc[256];
extern const char cdcToAscii[64];
extern const u8 asciiToConsole[256];
extern const char consoleToAscii[64];
extern const u16 asciiTo026[256];
extern const u16 asciiTo029[256];
extern const u8  asciiToBcd[256];
extern const char bcdToAscii[64];
extern const char extBcdToAscii[64];
extern const int asciiToPlatoString[256];
extern const unsigned char platoStringToAscii[4][65];
extern u32 traceMask;
extern u32 traceClearMask;
extern u64 chTraceMask;
extern u8 traceCp;
extern u32 traceCycles;
extern u32 traceSequenceNo;
extern DevDesc deviceDesc[];
extern u8 deviceCount;
extern bool bigEndian;
extern bool opActive;
extern long telnetPort;
extern long telnetConns;
extern long npuNetTelnetConns;
extern long npuNetTelnetPort;
extern int niuStations;
extern int niuStationEnd;
extern long opPort;
extern long opConns;
extern long dd60Port;
extern long dd60Conns;
extern long tpmuxPort;
extern long tpmuxConns;
extern FILE **ppuTF;
extern u32 cycles;
extern long cpuRatio;
extern bool debugDisplay;
extern bool opDebugging;
extern bool opPlatoAlert;
extern bool keyboardTrue;
extern u32 rtcClock;
extern char autoDateString[];
extern char autoString[];
extern char platoSection[];
extern u32 channelDelayMask;
extern long cmWaitRatio;
extern NetPortSet *connlist;
extern void (*updateConnections) (void);
extern long extSockets;
extern int dtErrno;

/*---------------------------  End Of File  ------------------------------*/
#endif /* PROTO_H */


