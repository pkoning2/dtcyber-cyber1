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

/*
**  --------------------
**  Function Prototypes.
**  --------------------
*/

/*
**  init.c
*/
void initStartup(char *);
u32 initConvertEndian(u32 value);

/*
**  deadstart.c
*/
void deadStart(void);

/*
**  rtc.c
*/
void rtcInit(char *model, u8 increment, long setMHz);
void rtcTick(void);
u64 rtcMicrosec(void);

/*
**  channel.c
*/
void channelInit(u8 count);
DevSlot *channelFindDevice(u8 channelNo, u8 devType);
DevSlot *channelAttach(u8 channelNo, u8 eqNo, u8 devType);
void channelFunction(PpWord funcCode);
void channelActivate(void);
void channelDisconnect(void);
void channelIo(void);

/*
**  pp.c
*/
void ppInit(u8 count);
void ppStep(void);

/*
**  cpu.c
*/
void cpuInit(char *model, u32 cpus, u32 memory, u32 ecsBanks, char *cmFile, char *ecsFile);
void cpuExit(void);
u32 cpuGetP(u8 cpnum);
void cpuStepAll(void);
void cpuStep(CpuContext *activeCpu);
bool cpuPpReadMem(u32 address, CpWord *data);
void cpuPpWriteMem(u32 address, CpWord data);
bool cpuEcsAccess(u32 address, CpWord *data, bool writeToEcs);
int cpuIssueExchange(u8 cpnum, u32 addr, int monitor);

/*
**  dcc6681.c
*/
DevSlot *dcc6681Attach(u8 channelNo, u8 eqNo, u8 unitNo, u8 devType);
DevSlot *dcc6681FindDevice(u8 channelNo, u8 equipmentNo, u8 devType);
void dcc6681Interrupt(bool status);

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
void lp1612RemovePaper(char *params);

/*
**  lp3000.c
*/
void lp501Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void lp512Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void lp3000RemovePaper(char *params);

/*
**  console.c
*/
void consoleInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  dd6603.c
*/
void dd6603Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  dd8xx.c
*/
void dd844Init_2(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void dd844Init_4(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void dd885Init_2(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void dd885Init_4(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  dcc6681.c
*/
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
bool niuPresent(void);
void niuLocalChar(u8 ch, int stat);
void niuLocalKey(u16 key, int stat);
typedef void niuProcessOutput (int, u32);
void niuSetOutputHandler (niuProcessOutput *h, int stat);

/*
**  tpmux.c
*/
void tpMuxInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  trace.c
*/
void traceInit(void);
void traceFinish(void);
void traceStop(void);
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
**  dump.c
*/
void dumpInit(void);
void dumpAll(void);
void dumpCpu(void);
void dumpCpuMem(FILE *f, u32 start, u32 end);
void dumpEcs(FILE *f, u32 start, u32 end);
void dumpPpu(u8 pp);
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
**  window_{win32,x11}.c
*/
void windowInit(void);
void windowSetFont(u8 font);
void windowSetX(u16 x);
void windowSetY(u16 y);
void windowQueue(char ch);
void windowGetChar(void);
void windowClose(void);
void windowCheckOutput(void);
void windowOperEnd(void);
int windowGetOperFontWidth(int font);
void windowSetKeyboardTrue(bool flag);

/*
**  ptermcom.c
*/
void ptermComInit(void);
void ptermComClose(void);

/*
**  pterm_{win32,x11}.c
*/
void ptermInit(const char *windowName, bool closeOk);
void ptermClose(void);
void ptermSetWeMode(u8 wemode);
void ptermDrawChar(int x, int y, int snum, int cnum);
void ptermLoadChar(int snum, int cnum, const u16 *data);
void ptermDrawPoint(int x, int y);
void ptermDrawLine(int x1, int y1, int x2, int y2);
void ptermFullErase(void);
void ptermSetWeMode(u8 wemode);
void ptermTouchPanel(bool enable);

/*
**  operator.c
*/
void opInit(void);
void opRequest(void);
void opSetMsg (char *p);
void opWaitOperMode(void);
void opWait(void);

/*
**  log.c
*/
void logInit(void);
void logError(char *file, int line, char *fmt, ...);

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
extern CpuContext *cpu;
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
extern const i8 asciiToPlato[128];
extern const i8 altKeyToPlato[128];
extern u32 traceMask;
extern u32 traceClearMask;
extern u64 chTraceMask;
extern u8 traceCp;
extern u32 traceCycles;
extern DevDesc deviceDesc[];
extern u8 deviceCount;
extern bool bigEndian;
extern bool opActive;
extern u16 telnetPort;
extern u16 telnetConns;
extern u16 platoPort;
extern u16 platoConns;
extern FILE **ppuTF;
extern u32 cycles;
extern int cpuRatio;
extern bool debugDisplay;
extern bool keyboardTrue;

/*---------------------------  End Of File  ------------------------------*/
#endif /* PROTO_H */


