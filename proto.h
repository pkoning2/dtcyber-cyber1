#ifndef PROTO_H
#define PROTO_H
/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
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
void rtcInit(char *model, u8 increment);
void rtcTick(void);

/*
**  channel.c
*/
void channelInit(u8 count);
DevSlot *channelFindDevice(u8 channelNo, u8 devType);
DevSlot *channelAttach(u8 channelNo, u8 devType);
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
void cpuInit(char *model, u32 memory, u32 ecsBanks);
u32 cpuGetP(void);
bool cpuExchangeJump(u32 addr);
void cpuStep(void);
bool cpuPpReadMem(u32 address, CpWord *data);
void cpuPpWriteMem(u32 address, CpWord data);
bool cpuEcsAccess(u32 address, CpWord *data, bool writeToEcs);

/*
**  dcc6681.c
*/
Dev3kSlot *dcc6681Attach(u8 channelNo, u8 unitNo, u8 devType);

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
**  cr3447.c
*/
void cr3447Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  lp1612.c
*/
void lp1612Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void lp1612RemovePaper(char *params);

/*
**  lp501.c
*/
void lp501Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void lp501RemovePaper(char *params);

/*
**  lp512.c
*/
void lp512Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
void lp512RemovePaper(char *params);

/*
**  console.c
*/
void consoleInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  dd6603.c
*/
void dd6603Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

/*
**  dd844.c
*/
void dd844Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);

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

/*
**  trace.c
*/
void traceInit(void);
void traceFinish(void);
void traceSequence(void);
void traceRegisters(void);
void traceOpcode(void);
void tracePM(void);
u8 traceDisassembleOpcode(char *str, PpWord *pm);
void traceChannelFunction(PpWord funcCode);
void tracePrint(char *str);
void traceCpuPrint(char *str);
void traceChannel(u8 ch);
void traceEnd(void);
void traceCpu(u32 p, u8 opFm, u8 opI, u8 opJ, u8 opK, u32 opAddress);
void traceExchange(CpuContext *cc, u32 addr, char *title);
void traceCM(u32 start, u32 end);
/*
**  dump.c
*/
void dumpInit(void);
void dumpAll(void);
void dumpCpu(void);
void dumpCpuMem(FILE *f, u32 start, u32 end);
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
void windowUpdate(void);
void windowGetChar(void);
void windowClose(void);
void windowCheckOutput(void);

/*
**  operator.c
*/
void opInit(void);
void opRequest(void);
void opSetMsg (char *p);

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
extern CpuContext cpu;
extern bool cpuStopped;
extern CpWord *cpMem;
extern u32 cpuMaxMemory;
extern u32 ecsMaxMemory;
extern char ppKeyIn;
extern const char asciiToCdc[];
extern const char cdcToAscii[];
extern const unsigned short asciiToPunch[];
extern const unsigned char  asciiToBcd[];
extern const unsigned char bcdToAscii[];
extern const unsigned char extBcdToAscii[];
extern u16 traceMask;
extern u16 traceClearMask;
extern u16 chTraceMask;
extern DevDesc deviceDesc[];
extern u8 deviceCount;
extern bool bigEndian;
extern bool opActive;
extern u16 telnetPort;
extern u16 telnetConns;
extern u16 platoPort;
extern u16 platoConns;
extern FILE **ppuTF;
extern Dev3kSlot *activeUnit;

/*---------------------------  End Of File  ------------------------------*/
#endif /* PROTO_H */


