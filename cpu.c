/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: cpu.c
**
**  Description:
**      Perform simulation of CDC 6600 CPU.
**
**--------------------------------------------------------------------------
*/

/*
**  -------------
**  Include Files
**  -------------
*/
#ifndef DUAL_CPU
#undef CPU_THREADS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "const.h"
#include "types.h"
#include "proto.h"
#ifdef CPU_THREADS
#include <pthread.h>
#endif

/*
**  -----------------
**  Private Constants
**  -----------------
*/

/*
**  CPU exit codes.
*/
#define EmNone                  000000
#define EmAddressOutOfRange     010000
#define EmOperandOutOfRange     020000
#define EmIndefiniteOperand     040000

/*
**  CPU exit conditions.
*/
#define EcNone                  00
#define EcAddressOutOfRange     01
#define EcOperandOutOfRange     02
#define EcIndefiniteOperand     04

/*
**  ECS bank size taking into account the 5k reserve.
*/
#define EcsBankSize             (131072 - 5120)

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

/* 
**  Macros for enabling dual CPU emulation.  These are constructed
**  so the case when dual CPU emulation is turned off do NOT get the
**  added (even if small) overhead of passing a cpu context pointer
**  to all the emulation primitives.
*/
#ifdef DUAL_CPU
#define CPUVARG         CpuContext *activeCpu
#define CPUVARGS2(x, y) CpuContext *activeCpu, x, y
#define CPUVARGS3(x, y, z) CpuContext *activeCpu, x, y, z
#define CPUARG          activeCpu
#define CPUARGS2(x, y)  activeCpu, x, y
#define CPUARGS3(x, y, z) activeCpu, x, y, z
#define MAXCPUS         2
#else
#define CPUVARG         void
#define CPUVARGS2(x, y) x, y
#define CPUVARGS3(x, y, z) x, y, z
#define CPUARG
#define CPUARGS2(x, y)  x, y
#define CPUARGS3(x, y, z) x, y, z
#define MAXCPUS         1
#define cpuStep         cpuStepAll
#endif

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef struct opDispatch
    {
    void (*execute)(CPUVARG);
    u32   length;
    } OpDispatch;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
#define cpuFetchOpWord cpuReadMem
static INLINE bool cpuReadMem(CPUVARGS2 (u32 address, CpWord *data));
static INLINE bool cpuWriteMem(CPUVARGS2 (u32 address, CpWord *data));
static INLINE void cpuRegASemantics(CPUVARG);
static INLINE u32 cpuAdd18(u32 op1, u32 op2);
static INLINE u32 cpuSubtract18(u32 op1, u32 op2);
static void cpuEcsTransfer(CPUVARGS2 (bool writeToEcs, bool toReg));
static bool cpuCmuGetByte(CPUVARGS3 (u32 address, u32 pos, u8 *byte));
static bool cpuCmuPutByte(CPUVARGS3 (u32 address, u32 pos, u8 byte));
static void cpuCmuMoveIndirect(CPUVARG);
static void cpuCmuMoveDirect(CPUVARG);
static void cpuCmuCompareCollated(CPUVARG);
static void cpuCmuCompareUncollated(CPUVARG);
#if DebugOps == 1
static void cpuTraceCtl(CPUVARG);
#endif
static void cpuExchangeJump(CPUVARG);
#ifdef CPU_THREADS
static void cpuCreateThread(int cpuNum);
static ThreadFunRet cpuThread(void *param);
#elif defined (DUAL_CPU)
static void cpuStep(CPUVARG);
#endif /* CPU_THREADS */

static void cpOp00(CPUVARG);
static void cpOp01(CPUVARG);
static void cpOp02(CPUVARG);
static void cpOp03(CPUVARG);
static void cpOp04(CPUVARG);
static void cpOp05(CPUVARG);
static void cpOp06(CPUVARG);
static void cpOp07(CPUVARG);
static void cpOp10(CPUVARG);
static void cpOp11(CPUVARG);
static void cpOp12(CPUVARG);
static void cpOp13(CPUVARG);
static void cpOp14(CPUVARG);
static void cpOp15(CPUVARG);
static void cpOp16(CPUVARG);
static void cpOp17(CPUVARG);
static void cpOp20(CPUVARG);
static void cpOp21(CPUVARG);
static void cpOp22(CPUVARG);
static void cpOp23(CPUVARG);
static void cpOp24(CPUVARG);
static void cpOp25(CPUVARG);
static void cpOp26(CPUVARG);
static void cpOp27(CPUVARG);
static void cpOp30(CPUVARG);
static void cpOp31(CPUVARG);
static void cpOp32(CPUVARG);
static void cpOp33(CPUVARG);
static void cpOp34(CPUVARG);
static void cpOp35(CPUVARG);
static void cpOp36(CPUVARG);
static void cpOp37(CPUVARG);
static void cpOp40(CPUVARG);
static void cpOp41(CPUVARG);
static void cpOp42(CPUVARG);
static void cpOp43(CPUVARG);
static void cpOp44(CPUVARG);
static void cpOp45(CPUVARG);
static void cpOp46(CPUVARG);
static void cpOp47(CPUVARG);
static void cpOp50(CPUVARG);
static void cpOp51(CPUVARG);
static void cpOp52(CPUVARG);
static void cpOp53(CPUVARG);
static void cpOp54(CPUVARG);
static void cpOp55(CPUVARG);
static void cpOp56(CPUVARG);
static void cpOp57(CPUVARG);
static void cpOp60(CPUVARG);
static void cpOp61(CPUVARG);
static void cpOp62(CPUVARG);
static void cpOp63(CPUVARG);
static void cpOp64(CPUVARG);
static void cpOp65(CPUVARG);
static void cpOp66(CPUVARG);
static void cpOp67(CPUVARG);
static void cpOp70(CPUVARG);
static void cpOp71(CPUVARG);
static void cpOp72(CPUVARG);
static void cpOp73(CPUVARG);
static void cpOp74(CPUVARG);
static void cpOp75(CPUVARG);
static void cpOp76(CPUVARG);
static void cpOp77(CPUVARG);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
CpWord *cpMem;
CpWord *ecsMem;
u32 ecsFlagRegister;
u8 cpuCount;
CpuContext cpu[MAXCPUS];
volatile int monitorCpu;
volatile int exchangeCpu = -1;
u32 cpuMaxMemory;
u32 cpuMemMask;
u32 ecsMaxMemory;

/*
**  -----------------
**  Private Variables
**  -----------------
*/

#ifndef DUAL_CPU
#define activeCpu (&cpu[0])
#endif

static volatile u32 exchangeTo;
#ifdef CPU_THREADS
#if !defined(_WIN32)
static pthread_t cpu_thread;
//pthread_cond_t exchange_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t exchange_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t flagreg_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif

static FILE *cmHandle;
static FILE *ecsHandle;

static const u32 cpOp01Length[8] = { 30, 30, 30, 30, 15, 15, 15, 15 };

/*
**  Opcode decode and dispatch table.
*/
static OpDispatch decodeCpuOpcode[] =
    {
    { cpOp00, 15 },
    { cpOp01, 0 },
    { cpOp02, 30 },
    { cpOp03, 30 },
    { cpOp04, 30 },
    { cpOp05, 30 },
    { cpOp06, 30 },
    { cpOp07, 30 },
    { cpOp10, 15 },
    { cpOp11, 15 },
    { cpOp12, 15 },
    { cpOp13, 15 },
    { cpOp14, 15 },
    { cpOp15, 15 },
    { cpOp16, 15 },
    { cpOp17, 15 },
    { cpOp20, 15 },
    { cpOp21, 15 },
    { cpOp22, 15 },
    { cpOp23, 15 },
    { cpOp24, 15 },
    { cpOp25, 15 },
    { cpOp26, 15 },
    { cpOp27, 15 },
    { cpOp30, 15 },
    { cpOp31, 15 },
    { cpOp32, 15 },
    { cpOp33, 15 },
    { cpOp34, 15 },
    { cpOp35, 15 },
    { cpOp36, 15 },
    { cpOp37, 15 },
    { cpOp40, 15 },
    { cpOp41, 15 },
    { cpOp42, 15 },
    { cpOp43, 15 },
    { cpOp44, 15 },
    { cpOp45, 15 },
    { cpOp46, 15 },
    { cpOp47, 15 },
    { cpOp50, 30 },
    { cpOp51, 30 },
    { cpOp52, 30 },
    { cpOp53, 15 },
    { cpOp54, 15 },
    { cpOp55, 15 },
    { cpOp56, 15 },
    { cpOp57, 15 },
    { cpOp60, 30 },
    { cpOp61, 30 },
    { cpOp62, 30 },
    { cpOp63, 15 },
    { cpOp64, 15 },
    { cpOp65, 15 },
    { cpOp66, 15 },
    { cpOp67, 15 },
    { cpOp70, 30 },
    { cpOp71, 30 },
    { cpOp72, 30 },
    { cpOp73, 15 },
    { cpOp74, 15 },
    { cpOp75, 15 },
    { cpOp76, 15 },
    { cpOp77, 15 },
    };


/*
**--------------------------------------------------------------------------
**
**  Inline Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Read CPU memory and verify that address is within limits.
**
**  Parameters:     Name        Description.
**                  address     RA relative address to read.
**                  data        Pointer to 60 bit word which gets the data.
**
**  Returns:        TRUE if access failed, FALSE otherwise;
**
**------------------------------------------------------------------------*/
static INLINE bool cpuReadMem(CPUVARGS2 (u32 address, CpWord *data))
    {
    u32 absAddr = cpuAdd18 (address, activeCpu->regRaCm) & cpuMemMask;
    
    if (address >= activeCpu->regFlCm)
        {
        activeCpu->exitCondition |= EcAddressOutOfRange;
        if ((activeCpu->exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (activeCpu->regRaCm < cpuMaxMemory)
                {
                cpMem[activeCpu->regRaCm] = ((CpWord)activeCpu->exitCondition << 48) | ((CpWord)(activeCpu->regP + 1) << 30);
                }

            activeCpu->regP = 0;
            *data = cpMem[0] & Mask60;
            // ????????????? jump to monitor address ??????????????
            return(TRUE);
            }
        else
            {
            /* out of range address and exit not selected reads location 0 */
            absAddr = 0;
            }
        }

    *data = cpMem[absAddr] & Mask60;

    return(FALSE);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Write CPU memory and verify that address is within limits.
**
**  Parameters:     Name        Description.
**                  address     RA relative address to write.
**                  data        Pointer to 60 bit word which holds the data.
**
**  Returns:        TRUE if access failed, FALSE otherwise;
**
**------------------------------------------------------------------------*/
static INLINE bool cpuWriteMem(CPUVARGS2 (u32 address, CpWord *data))
    {
    u32 absAddr = cpuAdd18 (address, activeCpu->regRaCm) & cpuMemMask;
    
    if (address >= activeCpu->regFlCm)
        {
        activeCpu->exitCondition |= EcAddressOutOfRange;
        if ((activeCpu->exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (activeCpu->regRaCm < cpuMaxMemory)
                {
                cpMem[activeCpu->regRaCm] = ((CpWord)activeCpu->exitCondition << 48) | ((CpWord)(activeCpu->regP + 1) << 30);
                }

            activeCpu->regP = 0;
            // ????????????? jump to monitor address ??????????????
            return(TRUE);
            }

        return(FALSE);
        }

    cpMem[absAddr] = *data & Mask60;

    return(FALSE);
    }


/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/


/*--------------------------------------------------------------------------
**  Purpose:        Initialise CPU.
**
**  Parameters:     Name        Description.
**                  model       CPU model string
**                  count       Number of CPUs
**                  memory      configured central memory
**                  ecsBanks    configured number of ECS banks
**                  cmFile      name of CM backing file or NULL
**                  ecsFile     name of ECS backing file or NULL
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cpuInit(char *model, u32 count, u32 memory, u32 ecsBanks,
             char *cmFile, char *ecsFile)
    {
    int cpuNum;
    
    /*
    **  Initialize cpu count.
    */
    if (count > MAXCPUS)
        {
        fprintf (stderr, "CPU count %d too large, max is %d\n",
                 count, MAXCPUS);
        exit (1);
        }
    cpuCount = count;

    /*
    **  Allocate configured central memory.
    */
    cpMem = calloc(memory, sizeof(CpWord));
    if (cpMem == NULL)
        {
        fprintf(stderr, "Failed to allocate CPU memory\n");
        exit(1);
        }

    cpuMaxMemory = memory;
    cpuMemMask = memory - 1;

    /*
    **  Allocate configured ECS memory.
    */
    ecsMem = calloc(ecsBanks * EcsBankSize, sizeof(CpWord));
    if (ecsMem == NULL)
        {
        fprintf(stderr, "Failed to allocate ECS memory\n");
        exit(1);
        }

    ecsMaxMemory = ecsBanks * EcsBankSize;

    /*
    **  Initialize the monitor mode state.  -1 means no CPU in monitor mode.
    */
    monitorCpu = -1;

    /*
    **  Initialise all cpus.
    */
    for (cpuNum = 0; cpuNum < cpuCount; cpuNum++)
        {
        cpu[cpuNum].id = cpuNum;
        cpu[cpuNum].cpuStopped = TRUE;
#ifdef CPU_THREADS
        if (cpuNum > 0)
            {
            cpuCreateThread(cpuNum);
            }
#endif
        }

    /*
    **  Optionally read in persistent CM contents.
    */
    if (*cmFile != '\0')
        {
        /*
        **  Try to open existing file.
        */
        cmHandle = fopen(cmFile, "r+b");
        if (cmHandle != NULL)
            {
            /*
            **  Read CM contents.
            */
            if (fread(cpMem, sizeof(CpWord), cpuMaxMemory, cmHandle) != cpuMaxMemory)
                {
                printf("Unexpected length of CM backing file\n");
                }
            }
        else
            {
            /*
            **  Create a new file.
            */
            cmHandle = fopen(cmFile, "w+b");
            if (cmHandle == NULL)
                {
                fprintf(stderr, "Failed to allocate CM memory\n");
                exit(1);
                }
            }
        }

    /*
    **  Optionally read in persistent ECS contents.
    */
    if (*ecsFile != '\0')
        {
        /*
        **  Try to open existing file.
        */
        ecsHandle = fopen(ecsFile, "r+b");
        if (ecsHandle != NULL)
            {
            /*
            **  Read ECS contents.
            */
            if (fread(ecsMem, sizeof(CpWord), ecsMaxMemory, ecsHandle) != ecsMaxMemory)
                {
                printf("Unexpected length of ECS backing file\n");
                }
            }
        else
            {
            /*
            **  Create a new file.
            */
            ecsHandle = fopen(ecsFile, "w+b");
            if (ecsHandle == NULL)
                {
                fprintf(stderr, "Failed to allocate ECS memory\n");
                exit(1);
                }
            }
        }

    /*
    **  Print a friendly message.
    */
    printf("CPU model %s initialised (CM: %o, ECS: %o)\n", model, cpuMaxMemory, ecsMaxMemory);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Terminate CPU and optionally persist CM.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cpuTerminate(void)
    {
    /*
    **  Optionally save CM.
    */
    if (cmHandle != NULL)
        {
        fseek(cmHandle, 0, SEEK_SET);
        if (fwrite(cpMem, sizeof(CpWord), cpuMaxMemory, cmHandle) != cpuMaxMemory)
            {
            printf("Error writing CM backing file\n");
            }
        }

    /*
    **  Optionally save ECS.
    */
    if (ecsHandle != NULL)
        {
        fseek(ecsHandle, 0, SEEK_SET);
        if (fwrite(ecsMem, sizeof(CpWord), ecsMaxMemory, ecsHandle) != ecsMaxMemory)
            {
            printf("Error writing ECS backing file\n");
            }
        }

    /*
    **  Free allocated memory.
    */
    free(cpMem);
    free(ecsMem);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Return CPU P register.
**
**  Parameters:     Name        Description.
**                  cp          CPU number
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
u32 cpuGetP(u8 cp)
    {
    if (cp >= cpuCount)
        {
        cp = 0;
        }
    return((cpu[cp].regP) & Mask18);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Tell a CPU to exchange
**
**  Parameters:     Name        Description.
**                  cp          CPU number
**                  addr        Exchange package address
**                  monitor     0 to leave monitor mode
**                              <0 to leave it alone,
**                              >0 to enter monitor mode.
**                              monitor == 2 means exchange to MA.
**
**  Returns:
**      0 if the CPU is ready to exchange, non-0 to tell caller to
**      retry. 
**      The non-0 value is a reason code:
**      1. Another exchange is pending.
**      2. monitor is 1 (enter monitor mode) but another CPU is currently
**         in monitor mode.  CPU XJ will stall for that case, PP MXN/MAN
**         turns into a pass.
**
**------------------------------------------------------------------------*/
FILE **cpuTF;
int foo;
int cpuIssueExchange(u8 cp, u32 addr, int monitor)
    {
    if (cp >= cpuCount)
        {
        cp = 0;
        }
    if (monitor > 0 && cp > 0)
        {
        foo++;
        }
#ifdef CPU_THREADS
    pthread_mutex_lock (&exchange_mutex);
#endif
    if (monitor > 0 && monitorCpu != -1)
        {
#ifdef CPU_THREADS
        pthread_mutex_unlock (&exchange_mutex);
#endif
        return 2;
        }
    if (exchangeCpu != -1)
        {
#ifdef CPU_THREADS
        pthread_mutex_unlock (&exchange_mutex);
#endif
        return 1;
        }
    if (monitor == 2)
        {
        exchangeTo = cpu[cp].regMa;
        }
    else
        {
        exchangeTo = addr;
        }
    if (monitor > 0)
        {
        monitorCpu = cp;
        }
    else if (monitor == 0)
        {
        monitorCpu = -1;
        }
    exchangeCpu = cp;
#ifdef CPU_THREADS
    pthread_mutex_unlock (&exchange_mutex);
#endif
    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Read CPU memory from PP and verify that address is
**                  within limits.
**
**  Parameters:     Name        Description.
**                  address     Absolute CM address to read.
**                  data        Pointer to 60 bit word which gets the data.
**
**  Returns:        TRUE if access failed, FALSE otherwise;
**
**------------------------------------------------------------------------*/
bool cpuPpReadMem(u32 address, CpWord *data)
    {
    address %= cpuMaxMemory;
    *data = cpMem[address & cpuMemMask] & Mask60;
    return(TRUE);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Write CPU memory from PP and verify that address is
**                  within limits.
**
**  Parameters:     Name        Description.
**                  address     Absolute CM address
**                  data        60 bit word which holds the data to be written.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
void cpuPpWriteMem(u32 address, CpWord data)
    {
    address %= cpuMaxMemory;
    cpMem[address & cpuMemMask] = data & Mask60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process CM/ECS pointer and verify address is within limits.
**
**  Parameters:     Name        Description.
**                  address     RA relative address to accesss.
**                  length      Number of words to be accessed.
**
**  Returns:        Pointer to first word, or NULL if out of range
**
**------------------------------------------------------------------------*/
CpWord * cpuAccessMem(CpWord address, int length)
    {
	u32 absAddr;

	if (address & (1ULL << 59))
	    {
		/* ECS pointer */
		address &= Mask24;
		
		if (address + length > activeCpu->regFlEcs)
		    {
			return NULL;
		    }
		else
		    {
			return ecsMem + address + activeCpu->regRaEcs;
		    }
	    }
	else
	    {
		/* CM pointer */
		address &= Mask18;
		
		if (address + length > activeCpu->regFlCm)
		    {
			return NULL;
		    }
		else
		    {
			return cpMem + address + activeCpu->regRaCm;
		    }
	    }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute next instruction in the CPUs.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
#if !defined (CPU_THREADS) && defined (DUAL_CPU)
void cpuStepAll (void)
    {
    int cpuNum;
    
    for (cpuNum = 0; cpuNum < cpuCount; cpuNum++)
        {
        cpuStep(cpu + cpuNum);
        }
    }
#endif

/*--------------------------------------------------------------------------
**  Purpose:        Transfer one word to/from ECS.
**
**  Parameters:     Name        Description.
**                  address     ECS word address
**                  data        Pointer to CM word to access
**                  writeToEcs  TRUE if this is a write to ECS, FALSE if
**                              this is a read.
**
**  Returns:        TRUE if access failed, FALSE otherwise.
**
**------------------------------------------------------------------------*/
bool cpuEcsAccess(u32 address, CpWord *data, bool writeToEcs)
    {
    /*
    **  Check if this is a flag register access.
    */
    if ((address & ((u32)1 << 23)) != 0)
        {
        u32 flagFunction = (address >> 21) & Mask3;
        u32 flagWord = address & Mask18;
        bool result = FALSE;
        
#ifdef CPU_THREADS
        /* No need to lock the mutex for a flag read */
        if (flagFunction != 6)
            {
            pthread_mutex_lock (&flagreg_mutex);
            }
#endif

        switch (flagFunction)
            {
        case 4:
            /*
            **  Ready/Select.
            */
            if ((ecsFlagRegister & flagWord ) != 0)
                {
                /*
                **  Error exit.
                */
                result = TRUE;
                break;
                }

            ecsFlagRegister |= flagWord;
            break;

        case 5:
            /*
            **  Selective set.
            */
            ecsFlagRegister |= flagWord;
            break;

        case 6:
            /*
            **  Status.
            */
            if ((ecsFlagRegister & flagWord ) != 0)
                {
                /*
                **  Error exit.
                */
                result = TRUE;
                break;
                }

            break;

        case 7:
            /*
            **  Selective clear,
            */
            ecsFlagRegister = (ecsFlagRegister & ~flagWord) & Mask18;
            break;
            }

#ifdef CPU_THREADS
        if (flagFunction != 6)
            {
            pthread_mutex_unlock (&flagreg_mutex);
            }
#endif

        return result;
        }
    /*
    **  Perform the transfer.
    */
    if (writeToEcs)
        {
        if (address >= ecsMaxMemory)
            {
            /*
            **  Error exit.
            */
            return TRUE;
            }
        ecsMem[address] = *data;
        }
    else
        {
        if (address >= ecsMaxMemory)
            {
            *data = 0;
            return TRUE;
            }
        else
            {
            *data = ecsMem[address];
            }
        }
    return FALSE;
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

#ifdef CPU_THREADS
/*--------------------------------------------------------------------------
**  Purpose:        Create a thread for a CPU
**
**  Parameters:     Name        Description.
**                  cpuNum      Cpu number to set up
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cpuCreateThread(int cpuNum)
    {
#if defined(_WIN32)
#error "Windows multithread CPU support TBS"
#else
    int rc;

    /*
    **  Create POSIX thread with default attributes.
    */
    rc = pthread_create(&cpu_thread, NULL, cpuThread, cpu + cpuNum);
    if (rc < 0)
        {
        fprintf(stderr, "Failed to create cpu %d thread\n", cpuNum);
        exit(1);
        }
#endif
    }

/*--------------------------------------------------------------------------
**  Purpose:        Thread execution function for a CPU
**
**  Parameters:     Name        Description.
**                  param       pointer to CPU context
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static ThreadFunRet cpuThread(void *param)
    {
    CpuContext *activeCpu = (CpuContext *) param;
    
    for (;;)
        {
        cpuStep (activeCpu);
        }
    }
#endif /* CPU_THREADS */

/*--------------------------------------------------------------------------
**  Purpose:        Execute next instruction in some CPU.
**
**  Parameters:     Name        Description.
**                  cpuNum      Cpu number to step
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cpuStep(CPUVARG)
    {
    u32 oldRegP;
    u32 length;
    int cpucycle;
    
    /*
    **  If this CPU needs to be exchanged, do that first.
    **  Note that this check must come BEFORE the "stopped" check.
    */
    if (activeCpu->id == exchangeCpu)
        {
        cpuExchangeJump (CPUARG);
        }
        
    if (activeCpu->cpuStopped)
        {
        return;
        }

    for (cpucycle = 0; cpucycle < cpuRatio; cpucycle++)
        {
        /*
        **  Execute one CM word atomically.
        */
        do
            {
            /*
            **  Decode based on type.
            */
            activeCpu->opFm = (u8)((activeCpu->opWord >> (activeCpu->opOffset - 6)) & Mask6);
            activeCpu->opI       = (u8)((activeCpu->opWord >> (activeCpu->opOffset -  9)) & Mask3);
            activeCpu->opJ       = (u8)((activeCpu->opWord >> (activeCpu->opOffset - 12)) & Mask3);
            length = decodeCpuOpcode[activeCpu->opFm].length;
            if (length == 0)
                {
                length = cpOp01Length[activeCpu->opI];
                }
        
            if (length == 15)
                {
                activeCpu->opK       = (u8)((activeCpu->opWord >> (activeCpu->opOffset - 15)) & Mask3);
                activeCpu->opAddress = 0;

                activeCpu->opOffset -= 15;
                }
            else
                {
                if (activeCpu->opOffset == 15)
                    {
                    /*
                    **  Stop when packing is invalid - this is the
                    **  behaviour of the 6400 and 6500. 
                    */
#if CcDebug == 1
                    traceCpuPrint(activeCpu, "Invalid packing\n");
#endif
                    activeCpu->cpuStopped = TRUE;
                    return;
                    }

                activeCpu->opK       = 0;
                activeCpu->opAddress = (u32)((activeCpu->opWord >> (activeCpu->opOffset - 30)) & Mask18);

                activeCpu->opOffset -= 30;
                }

            oldRegP = activeCpu->regP;

            /*
            **  Force B0 to 0.
            */
            activeCpu->regB[0] = 0;

            /*
            **  Execute instruction.
            */
            decodeCpuOpcode[activeCpu->opFm].execute (CPUARG);

            /*
            **  Force B0 to 0.
            */
            activeCpu->regB[0] = 0;

#if CcDebug == 1
            /*
            **  Don't trace COS's idle loop.
            */
            //    if ((activeCpu->regRaCm + activeCpu->regP) > 02062)

            /*
            **  To make things faster, do a quick check first to see if any
            **  tracing at all has been requested (i.e., trace mask != 0).
            **  That makes a surprisingly large difference.
            **
            **  Don't trace NOS's idle loop.
            **  If control point tracing is set, act accordingly.
            **  The control point check relies on the property that the
            **  monitor address in the CPU points to the exchange package
            **  area at the start of the CP area for the CP when that CP
            **  has the CPU.  This is more portable than looking for
            **  the active CP value in low memory.
            */
            if (traceMask != 0 &&
                /*activeCpu->regRaCm != 0 && */
                activeCpu->regP > 0100)
                {
                if (traceCp == 0 || 
                    (activeCpu->id != monitorCpu &&
                     activeCpu->regMa == (traceCp << 7)))
                    {
                    traceCpu(activeCpu, oldRegP, activeCpu->opFm,
                             activeCpu->opI, activeCpu->opJ,
                             activeCpu->opK, activeCpu->opAddress);
                    }
#if DebugOps == 1
                if (activeCpu->opFm == 061 && activeCpu->opI == 0 && activeCpu->opJ != 0)
                    {
                    cpuTraceCtl (CPUARG);
                    }
#endif
                }
#endif

            if (activeCpu->cpuStopped)
                {
                if (activeCpu->opOffset == 0)
                    {
                    activeCpu->regP = (activeCpu->regP + 1) & Mask18;
                    }
#if CcDebug == 1
                traceCpuPrint(activeCpu, (char*)"Stopped\n");
#endif
                return;
                }

            /*
            **  Fetch next instruction word if necessary.
            */
            if (activeCpu->opOffset == 0)
                {
                activeCpu->regP = (activeCpu->regP + 1) & Mask18;
                activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
                activeCpu->opOffset = 60;
                }
            } while (activeCpu->opOffset != 60);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform ECS flag register operation.
**
**  Parameters:     Name        Description.
**                  ecsAddress  ECS address (flag register function and data)
**
**  Returns:        TRUE if accepted, FALSE otherwise.
**
**------------------------------------------------------------------------*/
bool cpuEcsFlagRegister(u32 ecsAddress)
    {
    u32 flagFunction = (ecsAddress >> 21) & Mask3;
    u32 flagWord = ecsAddress & Mask18;
    bool result = TRUE;     /* Assume ok */
        
#ifdef CPU_THREADS
    /* No need to lock the mutex for a flag read */
    if (flagFunction != 6)
        {
        pthread_mutex_lock (&flagreg_mutex);
        }
#endif


    switch (flagFunction)
        {
    case 4:
        /*
        **  Ready/Select.
        */
        if ((ecsFlagRegister & flagWord ) != 0)
            {
            /*
            **  Error exit.
            */
            result = FALSE;
            break;
            }

        ecsFlagRegister |= flagWord;
        break;

    case 5:
        /*
        **  Selective set.
        */
        ecsFlagRegister |= flagWord;
        break;

    case 6:
        /*
        **  Status.
        */
        if ((ecsFlagRegister & flagWord ) != 0)
            {
            /*
            **  Error exit.
            */
            result = FALSE;
            break;
            }

        break;

    case 7:
        /*
        **  Selective clear,
        */
        ecsFlagRegister = (ecsFlagRegister & ~flagWord) & Mask18;
        break;
        }

#ifdef CPU_THREADS
    if (flagFunction != 6)
        {
        pthread_mutex_unlock (&flagreg_mutex);
        }
#endif
    /*
    **  Exit with status
    */
    return(result);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Transfer on 60 bit word to/from DDP/ECS.
**
**  Parameters:     Name        Description.
**                  ecsAddress  ECS word address
**                  data        Pointer to 60 bit word
**                  writeToEcs  TRUE if this is a write to ECS, FALSE if
**                              this is a read.
**
**  Returns:        TRUE if accepted, FALSE otherwise.
**
**------------------------------------------------------------------------*/
bool cpuDdpTransfer(u32 ecsAddress, CpWord *data, bool writeToEcs)
    {
    /*
    **  Normal (non flag-register) access must be within ECS boundaries.
    */
    if (ecsAddress >= ecsMaxMemory)
        {
        /*
        **  Abort.
        */
        return(FALSE);
        }

    /*
    **  Perform the transfer.
    */
    if (writeToEcs)
        {
        ecsMem[ecsAddress] = *data & Mask60;
        }
    else
        {
        *data = ecsMem[ecsAddress] & Mask60;
        }

    /*
    **  Normal accept.
    */
    return(TRUE);
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Perform exchange jump.
**
**  Parameters:     Name        Description.
**                  activeCpu   CPU context pointer
**
**  Returns:        Nothing
**
**  We come here when the CPU step loop notices that an exchange
**  has been requested for this CPU.  The exchange address is given
**  by variable exchangeTo.
**  When we're done, other threads waiting for the exchange to be
**  completed are signaled to wake up.
**
**------------------------------------------------------------------------*/
void cpuExchangeJump(CPUVARG)
    {
    CpuContext tmp;
    CpWord *mem;
    u32 addr;
    CpWord t;
    
    /*
    **  Only perform exchange jump on instruction boundary or when stopped.
    */
    if (activeCpu->opOffset != 60 && !activeCpu->cpuStopped)
        {
        return;
        }

#if CcDebug == 1
    traceExchange(activeCpu, exchangeTo, "Old");
#endif

    /*
    **  Clear any spurious address bits.  Note that the exchange package
    **  pointer is handled modulo the memory size, so it wraps at the
    **  top of memory.  The books aren't exactly clear but that's what
    **  the implication is.  (Specifically, there's no sign anywhere of
    **  a range check, so wrapping is what you'd expect the memory logic
    **  to produce as a result.)
    */
    exchangeTo &= Mask18;
    addr = exchangeTo;

    /*
    **  Save current context.
    */
    tmp = *activeCpu;

    /*
    **  Setup new context.
    */
    mem = cpMem + (addr & cpuMemMask);

    activeCpu->regP     = (u32)((*mem >> 36) & Mask18);
    activeCpu->regA[0]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[0]  = 0;

    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regRaCm  = (u32)((*mem >> 36) & Mask18);
    activeCpu->regA[1]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[1]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regFlCm  = (u32)((*mem >> 36) & Mask18);
    activeCpu->regA[2]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[2]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->exitMode = (u32)((*mem >> 36) & Mask24);
    activeCpu->regA[3]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[3]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regRaEcs = (u32)((*mem >> 36) & Mask24Ecs);
    activeCpu->regA[4]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[4]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regFlEcs = (u32)((*mem >> 36) & Mask24Ecs);
    activeCpu->regA[5]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[5]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regMa    = (u32)((*mem >> 36) & Mask18);
    activeCpu->regA[6]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[6]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regA[7]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[7]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regX[0]  = *mem & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regX[1]  = *mem & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regX[2]  = *mem & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regX[3]  = *mem & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regX[4]  = *mem & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regX[5]  = *mem & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regX[6]  = *mem & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    activeCpu->regX[7]  = *mem & Mask60;

    activeCpu->exitCondition = EcNone;

#if CcDebug == 1
    traceExchange(activeCpu, exchangeTo, "New");
#endif

    /*
    **  Save old context.
    */
    addr = exchangeTo;
    mem = cpMem + (addr & cpuMemMask);
    *mem = ((CpWord)(tmp.regP     & Mask18) << 36) |
           ((CpWord)(tmp.regA[0] & Mask18) << 18);
    mem = cpMem + (++addr & cpuMemMask);
    *mem = ((CpWord)(tmp.regRaCm  & Mask24) << 36) |
           ((CpWord)(tmp.regA[1] & Mask18) << 18) |
           ((CpWord)(tmp.regB[1] & Mask18));
    mem = cpMem + (++addr & cpuMemMask);
    *mem = ((CpWord)(tmp.regFlCm  & Mask24) << 36) |
           ((CpWord)(tmp.regA[2] & Mask18) << 18) |
           ((CpWord)(tmp.regB[2] & Mask18));
    mem = cpMem + (++addr & cpuMemMask);
    *mem = ((CpWord)(tmp.exitMode & Mask24) << 36) |
           ((CpWord)(tmp.regA[3] & Mask18) << 18) |
           ((CpWord)(tmp.regB[3] & Mask18));
    mem = cpMem + (++addr & cpuMemMask);
    *mem = ((CpWord)(tmp.regRaEcs & Mask24) << 36) |
           ((CpWord)(tmp.regA[4] & Mask18) << 18) |
           ((CpWord)(tmp.regB[4] & Mask18));
    mem = cpMem + (++addr & cpuMemMask);
    *mem = ((CpWord)(tmp.regFlEcs & Mask24) << 36) |
           ((CpWord)(tmp.regA[5] & Mask18) << 18) |
           ((CpWord)(tmp.regB[5] & Mask18));
    mem = cpMem + (++addr & cpuMemMask);
    *mem = ((CpWord)(tmp.regMa    & Mask24) << 36) |
           ((CpWord)(tmp.regA[6] & Mask18) << 18) |
           ((CpWord)(tmp.regB[6] & Mask18));
    mem = cpMem + (++addr & cpuMemMask);
    *mem = ((CpWord)(tmp.regA[7] & Mask18) << 18) |
           ((CpWord)(tmp.regB[7] & Mask18));
    mem = cpMem + (++addr & cpuMemMask);
    *mem = tmp.regX[0] & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    *mem = tmp.regX[1] & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    *mem = tmp.regX[2] & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    *mem = tmp.regX[3] & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    *mem = tmp.regX[4] & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    *mem = tmp.regX[5] & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    *mem = tmp.regX[6] & Mask60;
    mem = cpMem + (++addr & cpuMemMask);
    *mem = tmp.regX[7] & Mask60;

    /*
    **  Exchange is done, wake up whoever is waiting for that.
    */
#ifdef CPU_THREADS
    pthread_mutex_lock (&exchange_mutex);
    exchangeCpu = -1;
//    pthread_cond_broadcast (&exchange_cond);
    pthread_mutex_unlock (&exchange_mutex);
#else
    exchangeCpu = -1;
#endif

    /*
    **  Activate CPU.
    */
    activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
    if (activeCpu->cpuStopped)
        {
        activeCpu->opWord = 0;
        }
    
    t = activeCpu->opWord;
    /*
    **  Check for the idle loop.  Usually that's just an "eq *" but in recent
    **  flavors of NOS it's a few Cxi instructions then "eq *".  If we see
    **  the idle loop, pretend the CPU is stopped.  That way we don't spend
    **  time emulating the idle loop instructions, which will speed up other
    **  stuff (such as the PPUs and their I/O) if the CPU is idle.
    */
    while ((t >> 54) == 047)
        {
        t = (t << 15) & Mask60;
        }
    if ((t >> 30) == (00400000000 | activeCpu->regP))
        {
        activeCpu->cpuStopped = TRUE;
        }
    activeCpu->opOffset = 60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Turn on/off tracing under program control.
**
**  Parameters:     name        Description.
**                  activeCpu   CPU context pointer
**
**  Returns:        none.
**
**  Trace control ops are SB0 Bj+K, for various non-zero j and some K.
**  These are all NOPs, but are not normally used in software so they
**  can safely be used here for DtCyber specific magic.
**
**  Currently defined:
**      sb0 b1+42   turn on tracing for this CP until canceled.
**      sb0 b1+76   turn off CPU tracing.
**      sb0 b2+K    turn on CPU tracing (any CP) for K cycles.
**
**  These things are controlled by a separate conditional compile, not
**  the usual CcDebug one, because it's not fully debugged and also
**  because it may (?) be falsely triggered by some 180 series opcodes.
**
**------------------------------------------------------------------------
*/
#if DebugOps == 1
static void cpuTraceCtl (CPUVARG)
    {
    if (activeCpu->opJ == 1 && activeCpu->opAddress == 000042)
        {
        /* sb0 b1+42 is the magic word to turn on tracing */
        traceCp = (cpMem[060] >> 31) & 037;
        traceMask |= TraceCpu (activeCpu->id);
        }
    else if (activeCpu->opJ == 1 && activeCpu->opAddress == 000076)
        {
        traceMask &= ~TraceCpu (activeCpu->id);
        }
    else if (activeCpu->opJ == 2)
        {
        traceMask |= TraceCpu (activeCpu->id);
        traceCycles = activeCpu->opAddress;
        }
    }
#endif


/*--------------------------------------------------------------------------
**  Purpose:        Implement A register sematics.
**
**  Parameters:     Name        Description.
**                  activeCpu   CPU context pointer
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static INLINE void cpuRegASemantics(CPUVARG)
    {
    if (activeCpu->opI == 0)
        {
        return;
        }

    if (activeCpu->opI <= 5)
        {
        /*
        **  Read semantics.
        */
        activeCpu->cpuStopped = cpuReadMem(CPUARGS2 (activeCpu->regA[activeCpu->opI], activeCpu->regX + activeCpu->opI));
        }
    else
        {
        /*
        **  Write semantics.
        */
        activeCpu->cpuStopped = cpuWriteMem(CPUARGS2 (activeCpu->regA[activeCpu->opI], activeCpu->regX + activeCpu->opI));
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        18 bit ones-complement addition with subtractive adder
**
**  Parameters:     Name        Description.
**                  op1         18 bit operand1
**                  op2         18 bit operand2
**
**  Returns:        18 bit result.
**
**------------------------------------------------------------------------*/
static INLINE u32 cpuAdd18(u32 op1, u32 op2)
    {
    u32 acc18;

    acc18 = (op1 & Mask18) - (~op2 & Mask18);
    if ((acc18 & Overflow18) != 0)
        {
        acc18 -= 1;
        acc18 &= Mask18;
        }

    return(acc18);
    }

/*--------------------------------------------------------------------------
**  Purpose:        18 bit ones-complement subtraction
**
**  Parameters:     Name        Description.
**                  op1         18 bit operand1
**                  op2         18 bit operand2
**
**  Returns:        18 bit result.
**
**------------------------------------------------------------------------*/
static INLINE u32 cpuSubtract18(u32 op1, u32 op2)
    {
    u32 acc18;

    acc18 = (op1 & Mask18) - (op2 & Mask18);
    if ((acc18 & Overflow18) != 0)
        {
        acc18 -= 1;
        acc18 &= Mask18;
        }

    return(acc18);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Transfer block to/from ECS initiated by a CPU instruction.
**
**  Parameters:     Name        Description.
**                  activeCpu   CPU context pointer
**                  writeToEcs  TRUE if this is a write to ECS, FALSE if
**                              this is a read.
**                  toReg       TRUE if this is a register transfer (RX/WX),
**                              FALSE if it is a CM transfer (RE/WE).
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void cpuEcsTransfer(CPUVARGS2 (bool writeToEcs, bool toReg))
    {
    u32 wordCount;
    u32 ecsAddress;
    u32 cmAddress;
    bool takeErrorExit = FALSE;
    CpWord *cmPtr = 0;
    
    /*
    **  ECS must exist and instruction (if RE/WE) must be located in the upper 30 bits.
    */
    if (ecsMaxMemory == 0 || (!toReg && activeCpu->opOffset != 30))
        {
        activeCpu->exitCondition |= EcAddressOutOfRange;       // <<<<<<<<<<<<< this may be wrong - manual does not specify
        activeCpu->regP = 0;
        activeCpu->cpuStopped = TRUE;
        return;
        }

    /*
    **  Calculate word count, source and destination addresses.
    */
    if (toReg)
        {
        wordCount = 1;
        ecsAddress = (u32)(activeCpu->regX[activeCpu->opK] & Mask24);
        cmAddress = activeCpu->opJ;
        cmPtr = activeCpu->regX + cmAddress;
        }
    else
        {
        wordCount = cpuAdd18(activeCpu->regB[activeCpu->opJ], activeCpu->opAddress);
        ecsAddress = (u32)(activeCpu->regX[0] & Mask24);
        cmAddress = activeCpu->regA[0] & Mask18;
        }

    /*
    **  Check if this is a flag register access.
    **
    **  The ECS book (60225100) says that a flag register reference occurs
    **  when bit 23 is set in the relative address AND in the ECS FL.
    **
    **  Note that the ECS RA is NOT added to the relative address.
    */
    if (   (ecsAddress          & ((u32)1 << 23)) != 0
        && (activeCpu->regFlEcs & ((u32)1 << 23)) != 0)
        {
        if (!cpuEcsFlagRegister(ecsAddress))
            {
            /*
            **  Error exit.
            */
            return;
            }

        /*
        **  Normal exit.
        */
        activeCpu->regP = (activeCpu->regP + 1) & Mask18;
        activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
        activeCpu->opOffset = 60;
        return;
        }

    /*
    **  Deal with possible negative zero word count.
    */
    if (wordCount == Mask18)
        {
        wordCount = 0;
        }

    /*
    **  Check for positive word count, CM and ECS range.
    */
    if (   (wordCount & Sign18) != 0
           || (!toReg && activeCpu->regFlCm  < cmAddress + wordCount)
           || activeCpu->regFlEcs < ecsAddress + wordCount)
        {
        activeCpu->exitCondition |= EcAddressOutOfRange;
        if ((activeCpu->exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            activeCpu->regP = 0;
            activeCpu->cpuStopped = TRUE;
            }
        else
            {
            activeCpu->regP = (activeCpu->regP + 1) & Mask18;
            activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
            activeCpu->opOffset = 60;
            }

        return;
        }

    /*
    **  Add base addresses.
    */
    cmAddress = cpuAdd18 (activeCpu->regRaCm, cmAddress);
    ecsAddress += activeCpu->regRaEcs;

    /*
    **  Perform the transfer.
    */

    while (wordCount--)
        {
        cmAddress  &= cpuMemMask;
        if (!toReg)
            {
            cmPtr = cpMem + cmAddress;
            }
        
        if (ecsAddress >= ecsMaxMemory && !writeToEcs)
            {
            /*
            **  Zero CM, but take error exit once zeroing is finished.
            */
            *cmPtr = 0;
            takeErrorExit = TRUE;
            }
        else
            {
            takeErrorExit = cpuEcsAccess (ecsAddress, cmPtr, writeToEcs);
            }

        if (takeErrorExit && writeToEcs)
            {
            break;
            }
        /*
        **  Increment CM address.
        */
        cmAddress = cpuAdd18(cmAddress, 1);
        ecsAddress++;
        }
    
    if (takeErrorExit || toReg)
        {
        /*
        **  Error exit to lower 30 bits of instruction word.
        **  RX/WX also exit to next instruction parcel.
        */
        return;
        }

    /*
    **  Normal exit to next instruction word.
    */
    activeCpu->regP = (activeCpu->regP + 1) & Mask18;
    activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
    activeCpu->opOffset = 60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        CMU get a byte.
**
**  Parameters:     Name        Description.
**                  activeCpu   CPU context pointer
**                  address     CM word address
**                  pos         character position
**                  byte        pointer to byte
**
**  Returns:        TRUE if access failed, FALSE otherwise.
**
**------------------------------------------------------------------------*/
static bool cpuCmuGetByte(CPUVARGS3 (u32 address, u32 pos, u8 *byte))
    {
    u32 location;
    CpWord data;

    /*
    **  Validate access.
    */
    if (address >= activeCpu->regFlCm)
        {
        activeCpu->exitCondition |= EcAddressOutOfRange;
        if ((activeCpu->exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (activeCpu->regRaCm < cpuMaxMemory)
                {
                cpMem[activeCpu->regRaCm] = ((CpWord)activeCpu->exitCondition << 48) | ((CpWord)(activeCpu->regP + 1) << 30);
                }

            activeCpu->regP = 0;
            activeCpu->cpuStopped = TRUE;
            // ????????????? jump to monitor address ??????????????
            }

        return(TRUE);
        }

    /*
    **  Calculate absolute address with wraparound.
    */
    location = cpuAdd18(activeCpu->regRaCm, address);
    location &= cpuMemMask;

    /*
    **  Fetch the word.
    */
    data = cpMem[location] & Mask60;

    /*
    **  Extract and return the byte.
    */
    *byte = (u8)((data >> ((9 - pos) * 6)) & Mask6);

    return(FALSE);
    }

/*--------------------------------------------------------------------------
**  Purpose:        CMU put a byte.
**
**  Parameters:     Name        Description.
**                  activeCpu   CPU context pointer
**                  address     CM word address
**                  pos         character position
**                  byte        data byte to put
**
**  Returns:        TRUE if access failed, FALSE otherwise.
**
**------------------------------------------------------------------------*/
static bool cpuCmuPutByte(CPUVARGS3 (u32 address, u32 pos, u8 byte))
    {
    u32 location;
    CpWord data;

    /*
    **  Validate access.
    */
    if (address >= activeCpu->regFlCm)
        {
        activeCpu->exitCondition |= EcAddressOutOfRange;
        if ((activeCpu->exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (activeCpu->regRaCm < cpuMaxMemory)
                {
                cpMem[activeCpu->regRaCm] = ((CpWord)activeCpu->exitCondition << 48) | ((CpWord)(activeCpu->regP + 1) << 30);
                }

            activeCpu->regP = 0;
            activeCpu->cpuStopped = TRUE;
            // ????????????? jump to monitor address ??????????????
            }

        return(TRUE);
        }

    /*
    **  Calculate absolute address with wraparound.
    */
    location = cpuAdd18(activeCpu->regRaCm, address);
    location &= cpuMemMask;

    /*
    **  Fetch the word.
    */
    data = cpMem[location] & Mask60;

    /*
    **  Mask the destination position.
    */
    data &= ~(((CpWord)Mask6) << ((9 - pos) * 6));

    /*
    **  Store byte into position
    */
    data |= (((CpWord)byte) << ((9 - pos) * 6));

    /*
    **  Store the word.
    */
    cpMem[location] = data & Mask60;

    return(FALSE);
    }

/*--------------------------------------------------------------------------
**  Purpose:        CMU move indirect.
**
**  Parameters:     Name        Description.
**                  activeCpu   CPU context pointer
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void cpuCmuMoveIndirect(CPUVARG)
    {
    CpWord descWord = 0;
    u32 k1, k2;
    u32 c1, c2;
    u32 ll;
    u8 byte;

    //<<<<<<<<<<<<<<<<<<<<<<<< don't forget to optimise c1 == c2 cases.

    /*
    **  Fetch the descriptor word.
    */
    activeCpu->opAddress = (u32)((activeCpu->opWord >> 30) & Mask18);
    activeCpu->opAddress = cpuAdd18(activeCpu->regB[activeCpu->opJ], activeCpu->opAddress);
    activeCpu->cpuStopped = cpuReadMem(CPUARGS2 (activeCpu->opAddress, &descWord));
    if (activeCpu->cpuStopped)
        {
        return;
        }

    /*
    **  Decode descriptor word.
    */
    k1 = (u32)(descWord >> 30) & Mask18;
    k2 = (u32)(descWord >>  0) & Mask18;
    c1 = (u32)(descWord >> 22) & Mask4;
    c2 = (u32)(descWord >> 18) & Mask4;
    ll = (u32)((descWord >> 26) & Mask4) | (u32)((descWord >> (48 - 4)) & (Mask9 << 4));

    /*
    **  Check for address out of range.
    */
    if (c1 > 9 || c2 > 9)
        {
        activeCpu->exitCondition |= EcAddressOutOfRange;
        if ((activeCpu->exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (activeCpu->regRaCm < cpuMaxMemory)
                {
                cpMem[activeCpu->regRaCm] = ((CpWord)activeCpu->exitCondition << 48) | ((CpWord)(activeCpu->regP + 1) << 30);
                }

            activeCpu->regP = 0;
            // ????????????? jump to monitor address ??????????????
            activeCpu->cpuStopped = TRUE;
            return;
            }

        /*
        **  No transfer.
        */
        ll = 0;
        }

    /*
    **  Perform the actual move.
    */
    while (ll--)
        {
        /*
        **  Transfer one byte, but abort if access fails.
        */
        if (   cpuCmuGetByte(CPUARGS3 (k1, c1, &byte))
            || cpuCmuPutByte(CPUARGS3 (k2, c2, byte)))
            {
            if (activeCpu->cpuStopped)
                {
                return;
                }

            /*
            **  Exit to next instruction.
            */
            break;
            }

        /*
        **  Advance addresses.
        */
        if (++c1 > 9)
            {
            c1  = 0;
            k1 += 1;
            }

        if (++c2 > 9)
            {
            c2  = 0;
            k2 += 1;
            }
        }

    /*
    **  Clear register X0 after the move.
    */
    activeCpu->regX[0] = 0;

    /*
    **  Normal exit to next instruction word.
    */
    activeCpu->regP = (activeCpu->regP + 1) & Mask18;
    activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
    activeCpu->opOffset = 60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        CMU move direct.
**
**  Parameters:     Name        Description.
**                  activeCpu   CPU context pointer
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void cpuCmuMoveDirect(CPUVARG)
    {
    u32 k1, k2;
    u32 c1, c2;
    u32 ll;
    u8 byte;

    //<<<<<<<<<<<<<<<<<<<<<<<< don't forget to optimise c1 == c2 cases.

    /*
    **  Decode opcode word.
    */
    k1 = (u32)(activeCpu->opWord >> 30) & Mask18;
    k2 = (u32)(activeCpu->opWord >>  0) & Mask18;
    c1 = (u32)(activeCpu->opWord >> 22) & Mask4;
    c2 = (u32)(activeCpu->opWord >> 18) & Mask4;
    ll = (u32)((activeCpu->opWord >> 26) & Mask4) | (u32)((activeCpu->opWord >> (48 - 4)) & (Mask3 << 4));

    /*
    **  Check for address out of range.
    */
    if (c1 > 9 || c2 > 9)
        {
        activeCpu->exitCondition |= EcAddressOutOfRange;
        if ((activeCpu->exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (activeCpu->regRaCm < cpuMaxMemory)
                {
                cpMem[activeCpu->regRaCm] = ((CpWord)activeCpu->exitCondition << 48) | ((CpWord)(activeCpu->regP + 1) << 30);
                }

            activeCpu->regP = 0;
            // ????????????? jump to monitor address ??????????????
            activeCpu->cpuStopped = TRUE;
            return;
            }

        /*
        **  No transfer.
        */
        ll = 0;
        }

    /*
    **  Perform the actual move.
    */
    while (ll--)
        {
        /*
        **  Transfer one byte, but abort if access fails.
        */
        if (   cpuCmuGetByte(CPUARGS3 (k1, c1, &byte))
            || cpuCmuPutByte(CPUARGS3 (k2, c2, byte)))
            {
            if (activeCpu->cpuStopped)
                {
                return;
                }

            /*
            **  Exit to next instruction.
            */
            break;
            }

        /*
        **  Advance addresses.
        */
        if (++c1 > 9)
            {
            c1  = 0;
            k1 += 1;
            }

        if (++c2 > 9)
            {
            c2  = 0;
            k2 += 1;
            }
        }

    /*
    **  Clear register X0 after the move.
    */
    activeCpu->regX[0] = 0;

    /*
    **  Normal exit to next instruction word.
    */
    activeCpu->regP = (activeCpu->regP + 1) & Mask18;
    activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
    activeCpu->opOffset = 60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        CMU compare collated.
**
**  Parameters:     Name        Description.
**                  activeCpu   CPU context pointer
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void cpuCmuCompareCollated(CPUVARG)
    {
    CpWord result = 0;
    u32 k1, k2;
    u32 c1, c2;
    u32 ll;
    u32 collTable;
    u8 byte1, byte2;

    /*
    **  Decode opcode word.
    */
    k1 = (u32)(activeCpu->opWord >> 30) & Mask18;
    k2 = (u32)(activeCpu->opWord >>  0) & Mask18;
    c1 = (u32)(activeCpu->opWord >> 22) & Mask4;
    c2 = (u32)(activeCpu->opWord >> 18) & Mask4;
    ll = (u32)((activeCpu->opWord >> 26) & Mask4) | (u32)((activeCpu->opWord >> (48 - 4)) & (Mask3 << 4));

    /*
    **  Setup collating table.
    */
    collTable = activeCpu->regA[0];

    /*
    **  Check for addresses and collTable out of range.
    */
    if (c1 > 9 || c2 > 9 || collTable >= activeCpu->regFlCm || activeCpu->regRaCm + collTable >= cpuMaxMemory)
        {
        activeCpu->exitCondition |= EcAddressOutOfRange;
        if ((activeCpu->exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (activeCpu->regRaCm < cpuMaxMemory)
                {
                cpMem[activeCpu->regRaCm] = ((CpWord)activeCpu->exitCondition << 48) | ((CpWord)(activeCpu->regP + 1) << 30);
                }

            activeCpu->regP = 0;
            // ????????????? jump to monitor address ??????????????
            activeCpu->cpuStopped = TRUE;
            return;
            }

        /*
        **  No transfer.
        */
        ll = 0;
        }

    /*
    **  Perform the actual compare.
    */
    while (ll--)
        {
        /*
        **  Check the two bytes raw.
        */
        if (   cpuCmuGetByte(CPUARGS3 (k1, c1, &byte1))
            || cpuCmuGetByte(CPUARGS3 (k2, c2, &byte2)))
            {
            if (activeCpu->cpuStopped)
                {
                return;
                }

            /*
            **  Exit to next instruction.
            */
            break;
            }

        if (byte1 != byte2)
            {
            /*
            **  Bytes differ - check using collating table.
            */
            if (   cpuCmuGetByte(CPUARGS3((collTable + ((byte1 >> 3 & Mask3))),
                                            byte1 & Mask3, &byte1))
                || cpuCmuGetByte(CPUARGS3((collTable + ((byte2 >> 3 & Mask3))),
                                            byte2 & Mask3, &byte2)))
                {
                if (activeCpu->cpuStopped)
                    {
                    return;
                    }

                /*
                **  Exit to next instruction.
                */
                break;
                }

            if (byte1 != byte2)
                {
                /*
                **  Bytes differ in their collating sequence as well - terminate comparision
                **  and calculate result.
                */
                result = ll + 1;
                if (byte1 < byte2)
                    {
                    result = ~result & Mask60;
                    }

                break;
                }
            }

        /*
        **  Advance addresses.
        */
        if (++c1 > 9)
            {
            c1  = 0;
            k1 += 1;
            }

        if (++c2 > 9)
            {
            c2  = 0;
            k2 += 1;
            }
        }

    /*
    **  Store result in X0.
    */
    activeCpu->regX[0] = result;

    /*
    **  Normal exit to next instruction word.
    */
    activeCpu->regP = (activeCpu->regP + 1) & Mask18;
    activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
    activeCpu->opOffset = 60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        CMU compare uncollated.
**
**  Parameters:     Name        Description.
**                  activeCpu   CPU context pointer
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void cpuCmuCompareUncollated(CPUVARG)
    {
    CpWord result = 0;
    u32 k1, k2;
    u32 c1, c2;
    u32 ll;
    u8 byte1, byte2;

    /*
    **  Decode opcode word.
    */
    k1 = (u32)(activeCpu->opWord >> 30) & Mask18;
    k2 = (u32)(activeCpu->opWord >>  0) & Mask18;
    c1 = (u32)(activeCpu->opWord >> 22) & Mask4;
    c2 = (u32)(activeCpu->opWord >> 18) & Mask4;
    ll = (u32)((activeCpu->opWord >> 26) & Mask4) | (u32)((activeCpu->opWord >> (48 - 4)) & (Mask3 << 4));

    /*
    **  Check for address out of range.
    */
    if (c1 > 9 || c2 > 9)
        {
        activeCpu->exitCondition |= EcAddressOutOfRange;
        if ((activeCpu->exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (activeCpu->regRaCm < cpuMaxMemory)
                {
                cpMem[activeCpu->regRaCm] = ((CpWord)activeCpu->exitCondition << 48) | ((CpWord)(activeCpu->regP + 1) << 30);
                }

            activeCpu->regP = 0;
            // ????????????? jump to monitor address ??????????????
            activeCpu->cpuStopped = TRUE;
            return;
            }

        /*
        **  No transfer.
        */
        ll = 0;
        }

    /*
    **  Perform the actual compare.
    */
    while (ll--)
        {
        /*
        **  Check the two bytes raw.
        */
        if (   cpuCmuGetByte(CPUARGS3 (k1, c1, &byte1))
            || cpuCmuGetByte(CPUARGS3 (k2, c2, &byte2)))
            {
            if (activeCpu->cpuStopped)
                {
                return;
                }

            /*
            **  Exit to next instruction.
            */
            break;
            }

        if (byte1 != byte2)
            {
            /*
            **  Bytes differ - terminate comparision
            **  and calculate result.
            */
            result = ll + 1;
            if (byte1 < byte2)
                {
                result = ~result & Mask60;
                }

            break;
            }

        /*
        **  Advance addresses.
        */
        if (++c1 > 9)
            {
            c1  = 0;
            k1 += 1;
            }

        if (++c2 > 9)
            {
            c2  = 0;
            k2 += 1;
            }
        }

    /*
    **  Store result in X0.
    */
    activeCpu->regX[0] = result;

    /*
    **  Normal exit to next instruction word.
    */
    activeCpu->regP = (activeCpu->regP + 1) & Mask18;
    activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
    activeCpu->opOffset = 60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Functions to implement all opcodes
**
**  Parameters:     Name        Description.
**                  activeCpu   CPU context pointer
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/

static void cpOp00(CPUVARG)
    {
    /*
    **  PS.
    */
    activeCpu->cpuStopped = TRUE;

#if NeedToUnderstandPSBetter

    ??? how should this behave if CEJ/MEJ is enabled ???

            activeCpu->regP = (activeCpu->regP + 1) & Mask18;
    activeCpu->monitorMode = TRUE;
    cpuExchangeJump(activeCpu->regMa);

#endif
    }

static void cpOp01(CPUVARG)
    {
    CpWord acc60;
    int monitor;
    u32 addr, oldP;
    u8 oldOffset;
    int ret;

    switch (activeCpu->opI)
        {
    case 0:
        /*
        **  RJ  K
        */
        acc60 = ((CpWord)0400 << 48) | ((CpWord)((activeCpu->regP + 1) & Mask18) << 30);
        activeCpu->cpuStopped = cpuWriteMem(CPUARGS2 (activeCpu->opAddress, &acc60));
        activeCpu->regP = activeCpu->opAddress;
        activeCpu->opOffset = 0;
        break;
        
    case 1:
        /*
        **  RE  Bj+K
        */
        cpuEcsTransfer(CPUARGS2 (FALSE, FALSE));
        break;
        
    case 2:
        /*
        **  WE  Bj+K
        */
        cpuEcsTransfer(CPUARGS2 (TRUE, FALSE));
        break;
        
    case 3:
        /*
        **  XJ  K
        */
        if (activeCpu->id == monitorCpu)
            {
            monitor = 0;    /* leave monitor mode */
            addr = activeCpu->opAddress + activeCpu->regB[activeCpu->opJ];
            }
        else
            {
            monitor = 2;    /* enter monitor mode at MA */
            addr = 0;
            }
        oldP = activeCpu->regP;
        oldOffset = activeCpu->opOffset;

        /* Set up CPU state for the continuation point */
        activeCpu->regP = (activeCpu->regP + 1) & Mask18;
        activeCpu->opOffset = 60;
        /* 
        **  Don't need to fetch the next instruction word; that will
        **  be done when we exchange back to this point.
        */

        if ((ret = cpuIssueExchange (activeCpu->id, addr, monitor)) != 0)
            {
            /*
            **  Need to retry until ready to exchange.
            **  In the multi-threaded version,
            **  wait on the exchange condition variable.
            **  That variable is signaled at each exchange, so
            **  it's a good time to drive the retry.
            */
#ifdef CPU_THREADS
            pthread_mutex_lock (&exchange_mutex);
            if (monitorCpu != -1)
                {
//                pthread_cond_wait (&exchange_cond, &exchange_mutex);
                }
            pthread_mutex_unlock (&exchange_mutex);
#endif
            /*
            **  Point back to the XJ instruction
            */
            activeCpu->regP = oldP;
            activeCpu->opOffset = oldOffset + 30;
            return;
            }
        break;
        
    case 4:
        /*
        **  RXj Xj
        */
        cpuEcsTransfer(CPUARGS2 (FALSE, TRUE));
        break;
        
    case 5:
        /*
        **  WXj Xk
        */
        cpuEcsTransfer(CPUARGS2 (TRUE, TRUE));
        break;
        
    case 6:
        /*
        **  RI or IBj -- which is 7600 only.  So we steal it for 
        **  dealing with the world outside the emulator.
        */
        activeCpu->regX[activeCpu->opJ] = extOp (activeCpu->regX[activeCpu->opK]);
        break;
        }
    }

static void cpOp02(CPUVARG)
    {
    /*
    **  JP  Bi+K
    */
    activeCpu->regP = cpuAdd18(activeCpu->regB[activeCpu->opI], activeCpu->opAddress);
    activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
    activeCpu->opOffset = 60;
    }

static void cpOp03(CPUVARG)
    {
    bool jump = FALSE;
    CpWord acc60;

    switch (activeCpu->opI)
        {
    case 0:
        /*
        **  ZR  Xj K
        */
        jump = activeCpu->regX[activeCpu->opJ] == 0 || activeCpu->regX[activeCpu->opJ] == NegativeZero;
        break;

    case 1:
        /*
        **  NZ  Xj K
        */
        jump = activeCpu->regX[activeCpu->opJ] != 0 && activeCpu->regX[activeCpu->opJ] != NegativeZero;
        break;

    case 2:
        /*
        **  PL  Xj K
        */
        jump = (activeCpu->regX[activeCpu->opJ] & Sign60) == 0;
        break;

    case 3:
        /*
        **  NG  Xj K
        */
        jump = (activeCpu->regX[activeCpu->opJ] & Sign60) != 0;
        break;

    case 4:
        /*
        **  IR  Xj K
        */
        acc60 = activeCpu->regX[activeCpu->opJ] >> 48;
        jump = acc60 != 03777 && acc60 != 04000;
        break;

    case 5:
        /*
        **  OR  Xj K
        */
        acc60 = activeCpu->regX[activeCpu->opJ] >> 48;
        jump = acc60 == 03777 || acc60 == 04000;
        break;

    case 6:
        /*
        **  DF  Xj K
        */
        acc60 = activeCpu->regX[activeCpu->opJ] >> 48;
        jump = acc60 != 01777 && acc60 != 06000;
        break;

    case 7:
        /*
        **  ID  Xj K
        */
        acc60 = activeCpu->regX[activeCpu->opJ] >> 48;
        jump = acc60 == 01777 || acc60 == 06000;
        break;
        }

    if (jump)
        {
        activeCpu->regP = activeCpu->opAddress;
        activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
        activeCpu->opOffset = 60;
        }
    }

static void cpOp04(CPUVARG)
    {
    /*
    **  EQ  Bi Bj K
    */
    if (activeCpu->regB[activeCpu->opI] == activeCpu->regB[activeCpu->opJ])
        {
        activeCpu->regP = activeCpu->opAddress;
        activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
        activeCpu->opOffset = 60;
        }
    }

static void cpOp05(CPUVARG)
    {
    /*
    **  NE  Bi Bj K
    */
    if (activeCpu->regB[activeCpu->opI] != activeCpu->regB[activeCpu->opJ])
        {
        activeCpu->regP = activeCpu->opAddress;
        activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
        activeCpu->opOffset = 60;
        }
    }

static void cpOp06(CPUVARG)
    {
    /*
    **  GE  Bi Bj K
    */
    i32 signDiff = (activeCpu->regB[activeCpu->opI] & Sign18) - (activeCpu->regB[activeCpu->opJ] & Sign18);
    u32 acc18;

    if (signDiff > 0)
        {
        return;
        }
    if (signDiff == 0)
        {
        acc18 = (activeCpu->regB[activeCpu->opI] & Mask18) - (activeCpu->regB[activeCpu->opJ] & Mask18);
        if ((acc18 & Overflow18) != 0 && (acc18 & Mask18) != 0)
            {
            acc18 -= 1;
            }

        if ((acc18 & Sign18) != 0)
            {
            return;
            }
        }

    activeCpu->regP = activeCpu->opAddress;
    activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
    activeCpu->opOffset = 60;
    }

static void cpOp07(CPUVARG)
    {
    /*
    **  LT  Bi Bj K
    */
    i32 signDiff = (activeCpu->regB[activeCpu->opI] & Sign18) - (activeCpu->regB[activeCpu->opJ] & Sign18);
    u32 acc18;

    if (signDiff < 0)
        {
        return;
        }

    if (signDiff == 0)
        {
        acc18 = (activeCpu->regB[activeCpu->opI] & Mask18) - (activeCpu->regB[activeCpu->opJ] & Mask18);
        if ((acc18 & Overflow18) != 0 && (acc18 & Mask18) != 0)
            {
            acc18 -= 1;
            }

        if ((acc18 & Sign18) == 0 || acc18 == 0)
            {
            return;
            }
        }

    activeCpu->regP = activeCpu->opAddress;
    activeCpu->cpuStopped = cpuFetchOpWord(CPUARGS2 (activeCpu->regP, &activeCpu->opWord));
    activeCpu->opOffset = 60;
    }

static void cpOp10(CPUVARG)
    {
    /*
    **  BXi Xj
    */
    activeCpu->regX[activeCpu->opI] = activeCpu->regX[activeCpu->opJ] & Mask60;
    }

static void cpOp11(CPUVARG)
    {
    /*
    **  BXi Xj*Xk
    */
    activeCpu->regX[activeCpu->opI] = (activeCpu->regX[activeCpu->opJ] & activeCpu->regX[activeCpu->opK]) & Mask60;
    }

static void cpOp12(CPUVARG)
    {
    /*
    **  BXi Xj+Xk
    */
    activeCpu->regX[activeCpu->opI] = (activeCpu->regX[activeCpu->opJ] | activeCpu->regX[activeCpu->opK]) & Mask60;
    }

static void cpOp13(CPUVARG)
    {
    /*
    **  BXi Xj-Xk
    */
    activeCpu->regX[activeCpu->opI] = (activeCpu->regX[activeCpu->opJ] ^ activeCpu->regX[activeCpu->opK]) & Mask60;
    }

static void cpOp14(CPUVARG)
    {
    /*
    **  BXi -Xj
    */
    activeCpu->regX[activeCpu->opI] = ~activeCpu->regX[activeCpu->opK] & Mask60;
    }

static void cpOp15(CPUVARG)
    {
    /*
    **  BXi -Xk*Xj
    */
    activeCpu->regX[activeCpu->opI] = (activeCpu->regX[activeCpu->opJ] & ~activeCpu->regX[activeCpu->opK]) & Mask60;
    }

static void cpOp16(CPUVARG)
    {
    /*
    **  BXi -Xk+Xj
    */
    activeCpu->regX[activeCpu->opI] = (activeCpu->regX[activeCpu->opJ] | ~activeCpu->regX[activeCpu->opK]) & Mask60;
    }

static void cpOp17(CPUVARG)
    {
    /*
    **  BXi -Xk-Xj
    */
    activeCpu->regX[activeCpu->opI] = (activeCpu->regX[activeCpu->opJ] ^ ~activeCpu->regX[activeCpu->opK]) & Mask60;
    }

static void cpOp20(CPUVARG)
    {
    /*
    **  LXi jk
    */
    u8 jk;

    jk = (u8)((activeCpu->opJ << 3) | activeCpu->opK);
    activeCpu->regX[activeCpu->opI] = shiftLeftCircular(activeCpu->regX[activeCpu->opI] & Mask60, jk);
    }

static void cpOp21(CPUVARG)
    {
    /*
    **  AXi jk
    */
    u8 jk;

    jk = (u8)((activeCpu->opJ << 3) | activeCpu->opK);
    activeCpu->regX[activeCpu->opI] = shiftRightArithmetic(activeCpu->regX[activeCpu->opI] & Mask60, jk);
    }

static void cpOp22(CPUVARG)
    {
    /*
    **  LXi Bj Xk
    */
    u32 count;
    CpWord acc60;

    count = activeCpu->regB[activeCpu->opJ] & Mask18;
    acc60 = activeCpu->regX[activeCpu->opK] & Mask60;

    if ((count & Sign18) == 0)
        {
        count &= Mask6;
        activeCpu->regX[activeCpu->opI] = shiftLeftCircular(acc60, count);
        }
    else
        {
        count = ~count;
        count &= Mask11;
        if ((count & ~Mask6) != 0)
            {
            activeCpu->regX[activeCpu->opI] = 0;
            }
        else
            {
            activeCpu->regX[activeCpu->opI] = shiftRightArithmetic(acc60, count);
            }
        }
    }

static void cpOp23(CPUVARG)
    {
    /*
    **  AXi Bj Xk
    */
    u32 count;
    CpWord acc60;

    count = activeCpu->regB[activeCpu->opJ] & Mask18;
    acc60 = activeCpu->regX[activeCpu->opK] & Mask60;

    if ((count & Sign18) == 0)
        {
        count &= Mask11;
        if ((count & ~Mask6) != 0)
            {
            activeCpu->regX[activeCpu->opI] = 0;
            }
        else
            {
            activeCpu->regX[activeCpu->opI] = shiftRightArithmetic(acc60, count);
            }
        }
    else
        {
        count = ~count;
        count &= Mask6;
        activeCpu->regX[activeCpu->opI] = shiftLeftCircular(acc60, count);
        }
    }

static void cpOp24(CPUVARG)
    {
    /*
    **  NXi Bj Xk
    */
    activeCpu->regX[activeCpu->opI] = shiftNormalize(activeCpu->regX[activeCpu->opK], &activeCpu->regB[activeCpu->opJ], FALSE);
    }

static void cpOp25(CPUVARG)
    {
    /*
    **  ZXi Bj Xk
    */
    activeCpu->regX[activeCpu->opI] = shiftNormalize(activeCpu->regX[activeCpu->opK], &activeCpu->regB[activeCpu->opJ], TRUE);
    }

static void cpOp26(CPUVARG)
    {
    /*
    **  UXi Bj Xk
    */
    if (activeCpu->opJ == 0)
        {
        activeCpu->regX[activeCpu->opI] = shiftUnpack(activeCpu->regX[activeCpu->opK], NULL);
        }
    else
        {
        activeCpu->regX[activeCpu->opI] = shiftUnpack(activeCpu->regX[activeCpu->opK], &activeCpu->regB[activeCpu->opJ]);
        }
    }

static void cpOp27(CPUVARG)
    {
    /*
    **  PXi Bj Xk
    */
    if (activeCpu->opJ == 0)
        {
        activeCpu->regX[activeCpu->opI] = shiftPack(activeCpu->regX[activeCpu->opK], 0);
        }
    else
        {
        activeCpu->regX[activeCpu->opI] = shiftPack(activeCpu->regX[activeCpu->opK], activeCpu->regB[activeCpu->opJ]);
        }
    }

static void cpOp30(CPUVARG)
    {
    /*
    **  FXi Xj+Xk
    */
    activeCpu->regX[activeCpu->opI] = floatAdd(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], FALSE, FALSE);
    }

static void cpOp31(CPUVARG)
    {
    /*
    **  FXi Xj-Xk
    */
    activeCpu->regX[activeCpu->opI] = floatAdd(activeCpu->regX[activeCpu->opJ], (~activeCpu->regX[activeCpu->opK] & Mask60), FALSE, FALSE);
    }

static void cpOp32(CPUVARG)
    {
    /*
    **  DXi Xj+Xk
    */
    activeCpu->regX[activeCpu->opI] = floatAdd(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], FALSE, TRUE);
    }

static void cpOp33(CPUVARG)
    {
    /*
    **  DXi Xj-Xk
    */
    activeCpu->regX[activeCpu->opI] = floatAdd(activeCpu->regX[activeCpu->opJ], (~activeCpu->regX[activeCpu->opK] & Mask60), FALSE, TRUE);
    }

static void cpOp34(CPUVARG)
    {
    /*
    **  RXi Xj+Xk
    */
    activeCpu->regX[activeCpu->opI] = floatAdd(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], TRUE, FALSE);
    }

static void cpOp35(CPUVARG)
    {
    /*
    **  RXi Xj-Xk
    */
    activeCpu->regX[activeCpu->opI] = floatAdd(activeCpu->regX[activeCpu->opJ], (~activeCpu->regX[activeCpu->opK] & Mask60), TRUE, FALSE);
    }

static void cpOp36(CPUVARG)
    {
    /*
    **  IXi Xj+Xk
    */
    CpWord acc60;

    acc60 = (activeCpu->regX[activeCpu->opJ] & Mask60) - (~activeCpu->regX[activeCpu->opK] & Mask60);
    if ((acc60 & Overflow60) != 0)
        {
        acc60 -= 1;
        }

    activeCpu->regX[activeCpu->opI] = acc60 & Mask60;
    }

static void cpOp37(CPUVARG)
    {
    /*
    **  IXi Xj-Xk
    */
    CpWord acc60;

    acc60 = (activeCpu->regX[activeCpu->opJ] & Mask60) - (activeCpu->regX[activeCpu->opK] & Mask60);
    if ((acc60 & Overflow60) != 0)
        {
        acc60 -= 1;
        }

    activeCpu->regX[activeCpu->opI] = acc60 & Mask60;
    }

static void cpOp40(CPUVARG)
    {
    /*
    **  FXi Xj*Xk
    */
    activeCpu->regX[activeCpu->opI] = floatMultiply(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], FALSE, FALSE);
    }

static void cpOp41(CPUVARG)
    {
    /*
    **  RXi Xj*Xk
    */
    activeCpu->regX[activeCpu->opI] = floatMultiply(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], TRUE, FALSE);
    }

static void cpOp42(CPUVARG)
    {
    /*
    **  DXi Xj*Xk
    */
    activeCpu->regX[activeCpu->opI] = floatMultiply(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], FALSE, TRUE);
    }

static void cpOp43(CPUVARG)
    {
    /*
    **  MXi jk
    */
    u8 jk;

    jk = (u8)((activeCpu->opJ << 3) | activeCpu->opK);
    activeCpu->regX[activeCpu->opI] = shiftMask(jk);
    }

static void cpOp44(CPUVARG)
    {
    /*
    **  FXi Xj/Xk
    */
    activeCpu->regX[activeCpu->opI] = floatDivide(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], FALSE);
    }

static void cpOp45(CPUVARG)
    {
    /*
    **  RXi Xj/Xk
    */
    activeCpu->regX[activeCpu->opI] = floatDivide(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], TRUE);
    }

static void cpOp46(CPUVARG)
    {
    switch (activeCpu->opI)
        {
    default:
        /*
        **  NO (pass).
        */
        return;

    case 4:
    case 5:
    case 6:
    case 7:
        if (activeCpu->opOffset != 45)
            {
            /*
            **  Cyber 70 Model 72,73,74 Instruction Description Ref Man says that 
            **  instruction must be in parcel 0, if not, it is interpreted as a
            **  pass instruction (NO).
            */
            return;
            }
        break;
        }

#if CcCMU
    switch (activeCpu->opI)
        {
    case 4:
        /*
        **  Move indirect.
        */
        cpuCmuMoveIndirect(CPUARG);
        break;

    case 5:
        /*
        **  Move direct.
        */
        cpuCmuMoveDirect(CPUARG);
        break;

    case 6:
        /*
        **  Compare collated.
        */
        cpuCmuCompareCollated(CPUARG);
        break;

    case 7:
        /*
        **  Compare uncollated.
        */
        cpuCmuCompareUncollated(CPUARG);
        break;
        }
#endif
    }

static void cpOp47(CPUVARG)
    {
    /*
    **  CXi Xk
    */
    CpWord acc60;

    acc60 = activeCpu->regX[activeCpu->opK] & Mask60;
    acc60 = ((acc60 & ULL(0xAAAAAAAAAAAAAAAA)) >>  1) +
            (acc60 & ULL(0x5555555555555555));
    acc60 = ((acc60 & ULL(0xCCCCCCCCCCCCCCCC)) >>  2) +
            (acc60 & ULL(0x3333333333333333));
    acc60 = ((acc60 & ULL(0xF0F0F0F0F0F0F0F0)) >>  4) + 
            (acc60 & ULL(0x0F0F0F0F0F0F0F0F));
    acc60 = ((acc60 & ULL(0xFF00FF00FF00FF00)) >>  8) +
            (acc60 & ULL(0x00FF00FF00FF00FF));
    acc60 = ((acc60 & ULL(0xFFFF0000FFFF0000)) >> 16) +
            (acc60 & ULL(0x0000FFFF0000FFFF));
    acc60 = ((acc60 & ULL(0xFFFFFFFF00000000)) >> 32) +
            (acc60 & ULL(0x00000000FFFFFFFF));
    activeCpu->regX[activeCpu->opI] = acc60 & Mask60;
    }

static void cpOp50(CPUVARG)
    {
    /*
    **  SAi Aj+K
    */
    activeCpu->regA[activeCpu->opI] = cpuAdd18(activeCpu->regA[activeCpu->opJ], activeCpu->opAddress);

    cpuRegASemantics (CPUARG);
    }

static void cpOp51(CPUVARG)
    {
    /*
    **  SAi Bj+K
    */
    activeCpu->regA[activeCpu->opI] = cpuAdd18(activeCpu->regB[activeCpu->opJ],  activeCpu->opAddress);

    cpuRegASemantics (CPUARG);
    }

static void cpOp52(CPUVARG)
    {
    /*
    **  SAi Xj+K
    */
    activeCpu->regA[activeCpu->opI] = cpuAdd18((u32)activeCpu->regX[activeCpu->opJ], activeCpu->opAddress);

    cpuRegASemantics (CPUARG);
    }

static void cpOp53(CPUVARG)
    {
    /*
    **  SAi Xj+Bk
    */
    activeCpu->regA[activeCpu->opI] = cpuAdd18((u32)activeCpu->regX[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    cpuRegASemantics (CPUARG);
    }

static void cpOp54(CPUVARG)
    {
    /*
    **  SAi Aj+Bk
    */
    activeCpu->regA[activeCpu->opI] = cpuAdd18(activeCpu->regA[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    cpuRegASemantics (CPUARG);
    }

static void cpOp55(CPUVARG)
    {
    /*
    **  SAi Aj-Bk
    */
    activeCpu->regA[activeCpu->opI] = cpuSubtract18(activeCpu->regA[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    cpuRegASemantics (CPUARG);
    }

static void cpOp56(CPUVARG)
    {
    /*
    **  SAi Bj+Bk
    */
    activeCpu->regA[activeCpu->opI] = cpuAdd18(activeCpu->regB[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    cpuRegASemantics (CPUARG);
    }

static void cpOp57(CPUVARG)
    {
    /*
    **  SAi Bj-Bk
    */
    activeCpu->regA[activeCpu->opI] = cpuSubtract18(activeCpu->regB[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    cpuRegASemantics (CPUARG);
    }

static void cpOp60(CPUVARG)
    {
    /*
    **  SBi Aj+K
    */
    activeCpu->regB[activeCpu->opI] = cpuAdd18(activeCpu->regA[activeCpu->opJ], activeCpu->opAddress);
    }

static void cpOp61(CPUVARG)
    {
    /*
    **  SBi Bj+K
    */
    activeCpu->regB[activeCpu->opI] = cpuAdd18(activeCpu->regB[activeCpu->opJ], activeCpu->opAddress);
    }

static void cpOp62(CPUVARG)
    {
    /*
    **  SBi Xj+K
    */
    activeCpu->regB[activeCpu->opI] = cpuAdd18((u32)activeCpu->regX[activeCpu->opJ], activeCpu->opAddress);
    }

static void cpOp63(CPUVARG)
    {
    /*
    **  SBi Xj+Bk
    */
    activeCpu->regB[activeCpu->opI] = cpuAdd18((u32)activeCpu->regX[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);
    }

static void cpOp64(CPUVARG)
    {
    /*
    **  SBi Aj+Bk
    */
    activeCpu->regB[activeCpu->opI] = cpuAdd18(activeCpu->regA[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);
    }

static void cpOp65(CPUVARG)
    {
    /*
    **  SBi Aj-Bk
    */
    activeCpu->regB[activeCpu->opI] = cpuSubtract18(activeCpu->regA[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);
    }

static void cpOp66(CPUVARG)
    {
    /*
    **  SBi Bj+Bk
    */
    activeCpu->regB[activeCpu->opI] = cpuAdd18(activeCpu->regB[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);
    }

static void cpOp67(CPUVARG)
    {
    /*
    **  SBi Bj-Bk
    */
    activeCpu->regB[activeCpu->opI] = cpuSubtract18(activeCpu->regB[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);
    }

static void cpOp70(CPUVARG)
    {
    /*
    **  SXi Aj+K
    */
    CpWord acc60;

    acc60 = (CpWord)cpuAdd18(activeCpu->regA[activeCpu->opJ], activeCpu->opAddress);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    activeCpu->regX[activeCpu->opI] = acc60 & Mask60;
    }

static void cpOp71(CPUVARG)
    {
    /*
    **  SXi Bj+K
    */
    CpWord acc60;

    acc60 = (CpWord)cpuAdd18(activeCpu->regB[activeCpu->opJ], activeCpu->opAddress);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    activeCpu->regX[activeCpu->opI] = acc60 & Mask60;
    }

static void cpOp72(CPUVARG)
    {
    /*
    **  SXi Xj+K
    */
    CpWord acc60;

    acc60 = (CpWord)cpuAdd18((u32)activeCpu->regX[activeCpu->opJ], activeCpu->opAddress);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    activeCpu->regX[activeCpu->opI] = acc60 & Mask60;
    }

static void cpOp73(CPUVARG)
    {
    /*
    **  SXi Xj+Bk
    */
    CpWord acc60;

    acc60 = (CpWord)cpuAdd18((u32)activeCpu->regX[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    activeCpu->regX[activeCpu->opI] = acc60 & Mask60;
    }

static void cpOp74(CPUVARG)
    {
    /*
    **  SXi Aj+Bk
    */
    CpWord acc60;

    acc60 = (CpWord)cpuAdd18(activeCpu->regA[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    activeCpu->regX[activeCpu->opI] = acc60 & Mask60;
    }

static void cpOp75(CPUVARG)
    {
    /*
    **  SXi Aj-Bk
    */
    CpWord acc60;

    acc60 = (CpWord)cpuSubtract18(activeCpu->regA[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);


    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    activeCpu->regX[activeCpu->opI] = acc60 & Mask60;
    }

static void cpOp76(CPUVARG)
    {
    /*
    **  SXi Bj+Bk
    */
    CpWord acc60;

    acc60 = (CpWord)cpuAdd18(activeCpu->regB[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    activeCpu->regX[activeCpu->opI] = acc60 & Mask60;
    }

static void cpOp77(CPUVARG)
    {
    /*
    **  SXi Bj-Bk
    */
    CpWord acc60;

    acc60 = (CpWord)cpuSubtract18(activeCpu->regB[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    activeCpu->regX[activeCpu->opI] = acc60 & Mask60;
    }

/*---------------------------  End Of File  ------------------------------*/
