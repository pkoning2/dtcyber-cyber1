/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: cpu.c
**
**  Description:
**      Perform CDC 6600 simulation of CPU.
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
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef struct opDispatch
    {
    void (*execute)(CpuContext *activeCpu);
    u8   length;
    } OpDispatch;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
//static bool cpuFetchOpWord(u32 address, CpWord *data);
#define cpuFetchOpWord cpuReadMem
static INLINE bool cpuReadMem(CpuContext *activeCpu, 
                              u32 address, CpWord *data);
static INLINE bool cpuWriteMem(CpuContext *activeCpu,
                               u32 address, CpWord *data);
static INLINE void cpuRegASemantics(CpuContext *activeCpu);
static INLINE u32 cpuAdd18(u32 op1, u32 op2);
static INLINE u32 cpuSubtract18(u32 op1, u32 op2);
static void cpuEcsTransfer(CpuContext *activeCpu, bool writeToEcs);
static bool cpuCmuGetByte(CpuContext *activeCpu, 
                          u32 address, u32 pos, u8 *byte);
static bool cpuCmuPutByte(CpuContext *activeCpu, 
                          u32 address, u32 pos, u8 byte);
static void cpuCmuMoveIndirect(CpuContext *activeCpu);
static void cpuCmuMoveDirect(CpuContext *activeCpu);
static void cpuCmuCompareCollated(CpuContext *activeCpu);
static void cpuCmuCompareUncollated(CpuContext *activeCpu);
#if CcDebug == 1
static void cpuTraceCtl(CpuContext *activeCpu);
#endif
static void cpuExchangeJump(CpuContext *activeCpu);
#ifdef CPU_THREADS
static void cpuCreateThread(int cpuNum);
#if defined(_WIN32)
static void cpuThread(void *param);
#define RETURN return
#else
static void *cpuThread(void *param);
#define RETURN return 0
#endif
#endif // CPU_THREADS

static void cpOp00(CpuContext *activeCpu);
static void cpOp01(CpuContext *activeCpu);
static void cpOp02(CpuContext *activeCpu);
static void cpOp03(CpuContext *activeCpu);
static void cpOp04(CpuContext *activeCpu);
static void cpOp05(CpuContext *activeCpu);
static void cpOp06(CpuContext *activeCpu);
static void cpOp07(CpuContext *activeCpu);
static void cpOp10(CpuContext *activeCpu);
static void cpOp11(CpuContext *activeCpu);
static void cpOp12(CpuContext *activeCpu);
static void cpOp13(CpuContext *activeCpu);
static void cpOp14(CpuContext *activeCpu);
static void cpOp15(CpuContext *activeCpu);
static void cpOp16(CpuContext *activeCpu);
static void cpOp17(CpuContext *activeCpu);
static void cpOp20(CpuContext *activeCpu);
static void cpOp21(CpuContext *activeCpu);
static void cpOp22(CpuContext *activeCpu);
static void cpOp23(CpuContext *activeCpu);
static void cpOp24(CpuContext *activeCpu);
static void cpOp25(CpuContext *activeCpu);
static void cpOp26(CpuContext *activeCpu);
static void cpOp27(CpuContext *activeCpu);
static void cpOp30(CpuContext *activeCpu);
static void cpOp31(CpuContext *activeCpu);
static void cpOp32(CpuContext *activeCpu);
static void cpOp33(CpuContext *activeCpu);
static void cpOp34(CpuContext *activeCpu);
static void cpOp35(CpuContext *activeCpu);
static void cpOp36(CpuContext *activeCpu);
static void cpOp37(CpuContext *activeCpu);
static void cpOp40(CpuContext *activeCpu);
static void cpOp41(CpuContext *activeCpu);
static void cpOp42(CpuContext *activeCpu);
static void cpOp43(CpuContext *activeCpu);
static void cpOp44(CpuContext *activeCpu);
static void cpOp45(CpuContext *activeCpu);
static void cpOp46(CpuContext *activeCpu);
static void cpOp47(CpuContext *activeCpu);
static void cpOp50(CpuContext *activeCpu);
static void cpOp51(CpuContext *activeCpu);
static void cpOp52(CpuContext *activeCpu);
static void cpOp53(CpuContext *activeCpu);
static void cpOp54(CpuContext *activeCpu);
static void cpOp55(CpuContext *activeCpu);
static void cpOp56(CpuContext *activeCpu);
static void cpOp57(CpuContext *activeCpu);
static void cpOp60(CpuContext *activeCpu);
static void cpOp61(CpuContext *activeCpu);
static void cpOp62(CpuContext *activeCpu);
static void cpOp63(CpuContext *activeCpu);
static void cpOp64(CpuContext *activeCpu);
static void cpOp65(CpuContext *activeCpu);
static void cpOp66(CpuContext *activeCpu);
static void cpOp67(CpuContext *activeCpu);
static void cpOp70(CpuContext *activeCpu);
static void cpOp71(CpuContext *activeCpu);
static void cpOp72(CpuContext *activeCpu);
static void cpOp73(CpuContext *activeCpu);
static void cpOp74(CpuContext *activeCpu);
static void cpOp75(CpuContext *activeCpu);
static void cpOp76(CpuContext *activeCpu);
static void cpOp77(CpuContext *activeCpu);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
CpWord *cpMem;
CpWord *ecsMem;
u32 ecsFlagRegister;
CpuContext *cpu;
u8 cpuCount;
volatile int monitorCpu;
volatile int exchangeCpu = -1;
u32 cpuMaxMemory;
u32 ecsMaxMemory;

/*
**  -----------------
**  Private Variables
**  -----------------
*/

volatile u32 exchangeTo;
#ifdef CPU_THREADS
#if !defined(_WIN32)
static pthread_t cpu_thread;
pthread_cond_t exchange_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t exchange_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flagreg_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif

/*
**  Opcode decode and dispatch table.
*/
static OpDispatch decodeCpuOpcode[] =
    {
    cpOp00, 15,
    cpOp01, 30,
    cpOp02, 30,
    cpOp03, 30,
    cpOp04, 30,
    cpOp05, 30,
    cpOp06, 30,
    cpOp07, 30,
    cpOp10, 15,
    cpOp11, 15,
    cpOp12, 15,
    cpOp13, 15,
    cpOp14, 15,
    cpOp15, 15,
    cpOp16, 15,
    cpOp17, 15,
    cpOp20, 15,
    cpOp21, 15,
    cpOp22, 15,
    cpOp23, 15,
    cpOp24, 15,
    cpOp25, 15,
    cpOp26, 15,
    cpOp27, 15,
    cpOp30, 15,
    cpOp31, 15,
    cpOp32, 15,
    cpOp33, 15,
    cpOp34, 15,
    cpOp35, 15,
    cpOp36, 15,
    cpOp37, 15,
    cpOp40, 15,
    cpOp41, 15,
    cpOp42, 15,
    cpOp43, 15,
    cpOp44, 15,
    cpOp45, 15,
    cpOp46, 15,
    cpOp47, 15,
    cpOp50, 30,
    cpOp51, 30,
    cpOp52, 30,
    cpOp53, 15,
    cpOp54, 15,
    cpOp55, 15,
    cpOp56, 15,
    cpOp57, 15,
    cpOp60, 30,
    cpOp61, 30,
    cpOp62, 30,
    cpOp63, 15,
    cpOp64, 15,
    cpOp65, 15,
    cpOp66, 15,
    cpOp67, 15,
    cpOp70, 30,
    cpOp71, 30,
    cpOp72, 30,
    cpOp73, 15,
    cpOp74, 15,
    cpOp75, 15,
    cpOp76, 15,
    cpOp77, 15
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
static INLINE bool cpuReadMem(CpuContext *activeCpu,
                              u32 address, CpWord *data)
    {
    if (address >= activeCpu->regFlCm || activeCpu->regRaCm + address >= cpuMaxMemory)
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
            address = 0;
            }
        }

    *data = cpMem[activeCpu->regRaCm + address] & Mask60;

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
static INLINE bool cpuWriteMem(CpuContext *activeCpu, 
                               u32 address, CpWord *data)
    {
    if (address >= activeCpu->regFlCm || activeCpu->regRaCm + address >= cpuMaxMemory)
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

    cpMem[activeCpu->regRaCm + address] = *data & Mask60;

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
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cpuInit(char *model, u32 count, u32 memory, u32 ecsBanks)
    {
    int cpuNum;
    
    /*
    **  Allocate cpu structures.
    */
    cpuCount = count;
    cpu = calloc(count, sizeof(CpuContext));
    if (cpu == NULL)
        {
        fprintf(stderr, "Failed to allocate cpu control blocks\n");
        exit(1);
        }

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
    **  Print a friendly message.
    */
    printf("CPU model %s initialised (CM: %o, ECS: %o)\n", model, cpuMaxMemory, ecsMaxMemory);
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
**      TRUE if the CPU is ready to exchange, FALSE to tell caller to
**      retry. 
**      The FALSE case happens for two reasons:
**      1. Another exchange is pending and this isn't a multi-threaded
**         flavor of DtCyber.  (The multi-threaded one just stalls
**         the thread in this case.)
**      2. monitor is 1 (enter monitor mode) but another CPU is currently
**         in monitor mode.  CPU XJ will stall for that case, PP MXN/MAN
**         turns into a pass.
**
**------------------------------------------------------------------------*/
FILE **cpuTF;
int foo;
bool cpuIssueExchange(u8 cp, u32 addr, bool monitor)
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
    for (;;)
        {
        if (monitor > 0 && monitorCpu != -1)
            {
            pthread_mutex_unlock (&exchange_mutex);
            return FALSE;
            }
        if (exchangeCpu == -1)
            {
            break;
            }
        pthread_cond_wait (&exchange_cond, &exchange_mutex);
        }
#else
    if (exchangeCpu != -1 || (monitor > 0 && monitorCpu != -1))
        {
        return FALSE;
        }
#endif
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
    return TRUE;
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
    *data = cpMem[address] & Mask60;
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
    cpMem[address] = data & Mask60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute next instruction in the CPUs.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
#ifndef CPU_THREADS
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
#if defined(_WIN32)
static void cpuThread(void *param)
#else
static void *cpuThread(void *param)
#endif
    {
    CpuContext *activeCpu = (CpuContext *) param;
    
    for (;;)
        {
        while (opActive)
            {
            opWait ();
            }
        cpuStep (activeCpu);
        }
    }
#endif // CPU_THREADS

/*--------------------------------------------------------------------------
**  Purpose:        Execute next instruction in some CPU.
**
**  Parameters:     Name        Description.
**                  cpuNum      Cpu number to step
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cpuStep(CpuContext *activeCpu)
    {
    u32 oldRegP;

    /*
    **  If this CPU needs to be exchanged, do that first.
    **  (It may not happen right now, if we're in the middle
    **  of the instruction word.)
    **  Note that this check must come BEFORE the "stopped" check.
    */
    if (activeCpu->id == exchangeCpu)
        {
        cpuExchangeJump (activeCpu);
        }
        
    if (activeCpu->cpuStopped)
        {
        return;
        }

    /*
    **  Execute one CM word atomically.
    */
    do
        {
        /*
        **  Decode based on type.
        */
        activeCpu->opFm = (u8)((activeCpu->opWord >> (activeCpu->opOffset - 6)) & Mask6);
        if (decodeCpuOpcode[activeCpu->opFm].length == 15)
            {
            activeCpu->opI       = (u8)((activeCpu->opWord >> (activeCpu->opOffset -  9)) & Mask3);
            activeCpu->opJ       = (u8)((activeCpu->opWord >> (activeCpu->opOffset - 12)) & Mask3);
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

            activeCpu->opI       = (u8)((activeCpu->opWord >> (activeCpu->opOffset -  9)) & Mask3);
            activeCpu->opJ       = (u8)((activeCpu->opWord >> (activeCpu->opOffset - 12)) & Mask3);
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
        decodeCpuOpcode[activeCpu->opFm].execute (activeCpu);

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
        **  Don't trace NOS's idle loop and CPUMTR.
        **  If control point tracing is set, act accordingly.
        */
        if (/*activeCpu->regRaCm != 0 && */ activeCpu->regP > 0100)
            {
            if (traceCp == 0 || 
                (activeCpu->id != monitorCpu &&
                 ((cpMem[060] >> 31) & 037) == traceCp))
                {
                traceCpu(activeCpu, oldRegP, activeCpu->opFm, activeCpu->opI, activeCpu->opJ, activeCpu->opK, activeCpu->opAddress);
                }
            if (activeCpu->opFm == 061 && activeCpu->opI == 0 && activeCpu->opJ != 0)
                {
                cpuTraceCtl (activeCpu);
                }
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
            activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
            activeCpu->opOffset = 60;
            }
        } while (activeCpu->opOffset != 60);
    }


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
void cpuExchangeJump(CpuContext *activeCpu)
    {
    CpuContext tmp;
    CpWord *mem;
    u32 addr;
    
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
    mem = cpMem + (addr % cpuMaxMemory);

    activeCpu->regP     = (u32)((*mem >> 36) & Mask18);
    activeCpu->regA[0]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[0]  = 0;

    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regRaCm  = (u32)((*mem >> 36) & Mask18);
    activeCpu->regA[1]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[1]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regFlCm  = (u32)((*mem >> 36) & Mask18);
    activeCpu->regA[2]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[2]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->exitMode = (u32)((*mem >> 36) & Mask24);
    activeCpu->regA[3]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[3]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regRaEcs = (u32)((*mem >> 36) & Mask24Ecs);
    activeCpu->regA[4]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[4]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regFlEcs = (u32)((*mem >> 36) & Mask24Ecs);
    activeCpu->regA[5]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[5]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regMa    = (u32)((*mem >> 36) & Mask18);
    activeCpu->regA[6]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[6]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regA[7]  = (u32)((*mem >> 18) & Mask18);
    activeCpu->regB[7]  = (u32)((*mem      ) & Mask18);

    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regX[0]  = *mem & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regX[1]  = *mem & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regX[2]  = *mem & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regX[3]  = *mem & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regX[4]  = *mem & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regX[5]  = *mem & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regX[6]  = *mem & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    activeCpu->regX[7]  = *mem & Mask60;

    activeCpu->exitCondition = EcNone;

#if CcDebug == 1
    traceExchange(activeCpu, exchangeTo, "New");
#endif

    /*
    **  Save old context.
    */
    addr = exchangeTo;
    mem = cpMem + (addr % cpuMaxMemory);
    *mem = ((CpWord)(tmp.regP     & Mask18) << 36) |
           ((CpWord)(tmp.regA[0] & Mask18) << 18);
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = ((CpWord)(tmp.regRaCm  & Mask24) << 36) |
           ((CpWord)(tmp.regA[1] & Mask18) << 18) |
           ((CpWord)(tmp.regB[1] & Mask18));
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = ((CpWord)(tmp.regFlCm  & Mask24) << 36) |
           ((CpWord)(tmp.regA[2] & Mask18) << 18) |
           ((CpWord)(tmp.regB[2] & Mask18));
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = ((CpWord)(tmp.exitMode & Mask24) << 36) |
           ((CpWord)(tmp.regA[3] & Mask18) << 18) |
           ((CpWord)(tmp.regB[3] & Mask18));
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = ((CpWord)(tmp.regRaEcs & Mask24) << 36) |
           ((CpWord)(tmp.regA[4] & Mask18) << 18) |
           ((CpWord)(tmp.regB[4] & Mask18));
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = ((CpWord)(tmp.regFlEcs & Mask24) << 36) |
           ((CpWord)(tmp.regA[5] & Mask18) << 18) |
           ((CpWord)(tmp.regB[5] & Mask18));
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = ((CpWord)(tmp.regMa    & Mask24) << 36) |
           ((CpWord)(tmp.regA[6] & Mask18) << 18) |
           ((CpWord)(tmp.regB[6] & Mask18));
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = ((CpWord)(tmp.regA[7] & Mask18) << 18) |
           ((CpWord)(tmp.regB[7] & Mask18));
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = tmp.regX[0] & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = tmp.regX[1] & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = tmp.regX[2] & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = tmp.regX[3] & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = tmp.regX[4] & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = tmp.regX[5] & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = tmp.regX[6] & Mask60;
    mem = cpMem + (++addr % cpuMaxMemory);
    *mem = tmp.regX[7] & Mask60;

    /*
    **  Exchange is done, wake up whoever is waiting for that.
    */
#ifdef CPU_THREADS
    pthread_mutex_lock (&exchange_mutex);
    exchangeCpu = -1;
    pthread_cond_broadcast (&exchange_cond);
    pthread_mutex_unlock (&exchange_mutex);
#else
    exchangeCpu = -1;
#endif

    /*
    **  Activate CPU.
    */
    activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
    if (activeCpu->cpuStopped)
        {
        printf ("huh?\n");
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
**------------------------------------------------------------------------
*/
#if CcDebug == 1
static void cpuTraceCtl (CpuContext *activeCpu)
    {
    if (activeCpu->opJ == 1 && activeCpu->opAddress == 000042)
        {
        // sb0 b1+42 is the magic word to turn on tracing
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
static INLINE void cpuRegASemantics(CpuContext *activeCpu)
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
        activeCpu->cpuStopped = cpuReadMem(activeCpu, activeCpu->regA[activeCpu->opI], activeCpu->regX + activeCpu->opI);
        }
    else
        {
        /*
        **  Write semantics.
        */
        activeCpu->cpuStopped = cpuWriteMem(activeCpu, activeCpu->regA[activeCpu->opI], activeCpu->regX + activeCpu->opI);
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
**  Purpose:        Transfer to/from ECS.
**
**  Parameters:     Name        Description.
**                  activeCpu   CPU context pointer
**                  writeToEcs  TRUE if this is a write to ECS, FALSE if
**                              this is a read.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void cpuEcsTransfer(CpuContext *activeCpu, bool writeToEcs)
    {
    u32 wordCount;
    u32 ecsAddress;
    u32 cmAddress;
    bool takeErrorExit = FALSE;

    /*
    **  ECS must exist and instruction must be located in the upper 30 bits.
    */
    if (ecsMaxMemory == 0 || activeCpu->opOffset != 30)
        {
        activeCpu->exitCondition |= EcAddressOutOfRange;       // <<<<<<<<<<<<< this may be wrong - manual does not specify
        activeCpu->regP = 0;
        activeCpu->cpuStopped = TRUE;
        return;
        }

    /*
    **  Calculate word count, source and destination addresses.
    */
    wordCount = cpuAdd18(activeCpu->regB[activeCpu->opJ], activeCpu->opAddress);
    ecsAddress = (u32)(activeCpu->regX[0] & Mask24);
    cmAddress = activeCpu->regA[0] & Mask18;

    /*
    **  Check if this is a flag register access.
    **
    **  The ECS book (60225100) says that a flag register reference occurs
    **  when bit 23 is set in the relative address AND in the ECS FL.
    **  But the ECS RA is NOT added to the relative address.
    */
    if ((ecsAddress & ((u32)1 << 23)) != 0 &&
        (activeCpu->regFlEcs & ((u32)1 << 23)) != 0)
        {
        if (cpuEcsAccess (ecsAddress, NULL, writeToEcs))
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
        activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
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
    **  (removed "and top ECS location reference." ??? -- gpk)
    */
    if (   (wordCount & Sign18) != 0
        || activeCpu->regFlCm  < cmAddress + wordCount
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
            activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
            activeCpu->opOffset = 60;
            }

        return;
        }

    /*
    **  Add base addresses.
    */
    cmAddress += activeCpu->regRaCm;
    ecsAddress += activeCpu->regRaEcs;

    /*
    **  Perform the transfer.
    */

    while (wordCount--)
        {
        cmAddress  %= cpuMaxMemory;
        takeErrorExit = cpuEcsAccess (ecsAddress, cpMem + cmAddress, writeToEcs);
        if (takeErrorExit && writeToEcs)
            {
            break;
            }
        cmAddress++;
        ecsAddress++;
        }
    
    if (takeErrorExit)
        {
        /*
        **  Error exit to lower 30 bits of instruction word.
        */
        return;
        }

    /*
    **  Normal exit to next instruction word.
    */
    activeCpu->regP = (activeCpu->regP + 1) & Mask18;
    activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
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
static bool cpuCmuGetByte(CpuContext *activeCpu, 
                          u32 address, u32 pos, u8 *byte)
    {
    CpWord data;

    /*
    **  Validate access.
    */
    if (address >= activeCpu->regFlCm || activeCpu->regRaCm + address >= cpuMaxMemory)
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
    **  Fetch the word.
    */
    data = cpMem[activeCpu->regRaCm + address] & Mask60;

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
static bool cpuCmuPutByte(CpuContext *activeCpu, u32 address, u32 pos, u8 byte)
    {
    CpWord data;

    /*
    **  Validate access.
    */
    if (address >= activeCpu->regFlCm || activeCpu->regRaCm + address >= cpuMaxMemory)
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
    **  Fetch the word.
    */
    data = cpMem[activeCpu->regRaCm + address] & Mask60;

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
    cpMem[activeCpu->regRaCm + address] = data & Mask60;

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
static void cpuCmuMoveIndirect(CpuContext *activeCpu)
    {
    CpWord descWord;
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
    activeCpu->cpuStopped = cpuReadMem(activeCpu, activeCpu->opAddress, &descWord);
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
        if (   cpuCmuGetByte(activeCpu, k1, c1, &byte)
            || cpuCmuPutByte(activeCpu, k2, c2, byte))
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
    activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
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
static void cpuCmuMoveDirect(CpuContext *activeCpu)
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
        if (   cpuCmuGetByte(activeCpu, k1, c1, &byte)
            || cpuCmuPutByte(activeCpu, k2, c2, byte))
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
    activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
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
static void cpuCmuCompareCollated(CpuContext *activeCpu)
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
        if (   cpuCmuGetByte(activeCpu, k1, c1, &byte1)
            || cpuCmuGetByte(activeCpu, k2, c2, &byte2))
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
            if (   cpuCmuGetByte(activeCpu, collTable + ((byte1 >> 3) & Mask3), byte1 & Mask3, &byte1)
                || cpuCmuGetByte(activeCpu, collTable + ((byte2 >> 3) & Mask3), byte2 & Mask3, &byte2))
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
    activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
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
static void cpuCmuCompareUncollated(CpuContext *activeCpu)
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
        if (   cpuCmuGetByte(activeCpu, k1, c1, &byte1)
            || cpuCmuGetByte(activeCpu, k2, c2, &byte2))
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
    activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
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

static void cpOp00(CpuContext *activeCpu)
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

static void cpOp01(CpuContext *activeCpu)
    {
    CpWord acc60;
    int monitor;
    u32 addr, oldP;
    u8 oldOffset;
    
    if (activeCpu->opI == 0)
        {
        /*
        **  RJ  K
        */
        acc60 = ((CpWord)0400 << 48) | ((CpWord)((activeCpu->regP + 1) & Mask18) << 30);
        activeCpu->cpuStopped = cpuWriteMem(activeCpu, activeCpu->opAddress, &acc60);
        activeCpu->regP = activeCpu->opAddress;
        activeCpu->opOffset = 0;
        }
    else if (activeCpu->opI == 1)
        {
        /*
        **  RE  Bj+K
        */
        cpuEcsTransfer(activeCpu, FALSE);
        }
    else if (activeCpu->opI == 2)
        {
        /*
        **  WE  Bj+K
        */
        cpuEcsTransfer(activeCpu, TRUE);
        }
    else if (activeCpu->opI == 3)
        {
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

      fprintf(cpuTF[0],"cpu issue exchange cpu%d to %06o monitor %d\n", (int) (activeCpu->id), addr, (int)monitor);
        if (!cpuIssueExchange (activeCpu->id, addr, monitor))
            {
            /*
            **  Need to retry until ready to exchange.
            **  In the multi-threaded version, we get here only
            **  because of a monitor mode conflict; if so,
            **  wait on the exchange condition variable.
            **  That variable is signaled at each exchange, so
            **  it's a good time to drive the retry.
            */
#ifdef CPU_THREADS
            pthread_mutex_lock (&exchange_mutex);
            if (monitorCpu != -1)
                    {
                    pthread_cond_wait (&exchange_cond, &exchange_mutex);
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
        }
    }

static void cpOp02(CpuContext *activeCpu)
    {
    /*
    **  JP  Bi+K
    */
    activeCpu->regP = cpuAdd18(activeCpu->regB[activeCpu->opI], activeCpu->opAddress);
    activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
    activeCpu->opOffset = 60;
    }

static void cpOp03(CpuContext *activeCpu)
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
        activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
        activeCpu->opOffset = 60;
        }
    }

static void cpOp04(CpuContext *activeCpu)
    {
    /*
    **  EQ  Bi Bj K
    */
    if (activeCpu->regB[activeCpu->opI] == activeCpu->regB[activeCpu->opJ])
        {
        activeCpu->regP = activeCpu->opAddress;
        activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
        activeCpu->opOffset = 60;
        }
    }

static void cpOp05(CpuContext *activeCpu)
    {
    /*
    **  NE  Bi Bj K
    */
    if (activeCpu->regB[activeCpu->opI] != activeCpu->regB[activeCpu->opJ])
        {
        activeCpu->regP = activeCpu->opAddress;
        activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
        activeCpu->opOffset = 60;
        }
    }

static void cpOp06(CpuContext *activeCpu)
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
    activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
    activeCpu->opOffset = 60;
    }

static void cpOp07(CpuContext *activeCpu)
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
    activeCpu->cpuStopped = cpuFetchOpWord(activeCpu, activeCpu->regP, &activeCpu->opWord);
    activeCpu->opOffset = 60;
    }

static void cpOp10(CpuContext *activeCpu)
    {
    /*
    **  BXi Xj
    */
    activeCpu->regX[activeCpu->opI] = activeCpu->regX[activeCpu->opJ] & Mask60;
    }

static void cpOp11(CpuContext *activeCpu)
    {
    /*
    **  BXi Xj*Xk
    */
    activeCpu->regX[activeCpu->opI] = (activeCpu->regX[activeCpu->opJ] & activeCpu->regX[activeCpu->opK]) & Mask60;
    }

static void cpOp12(CpuContext *activeCpu)
    {
    /*
    **  BXi Xj+Xk
    */
    activeCpu->regX[activeCpu->opI] = (activeCpu->regX[activeCpu->opJ] | activeCpu->regX[activeCpu->opK]) & Mask60;
    }

static void cpOp13(CpuContext *activeCpu)
    {
    /*
    **  BXi Xj-Xk
    */
    activeCpu->regX[activeCpu->opI] = (activeCpu->regX[activeCpu->opJ] ^ activeCpu->regX[activeCpu->opK]) & Mask60;
    }

static void cpOp14(CpuContext *activeCpu)
    {
    /*
    **  BXi -Xj
    */
    activeCpu->regX[activeCpu->opI] = ~activeCpu->regX[activeCpu->opK] & Mask60;
    }

static void cpOp15(CpuContext *activeCpu)
    {
    /*
    **  BXi -Xk*Xj
    */
    activeCpu->regX[activeCpu->opI] = (activeCpu->regX[activeCpu->opJ] & ~activeCpu->regX[activeCpu->opK]) & Mask60;
    }

static void cpOp16(CpuContext *activeCpu)
    {
    /*
    **  BXi -Xk+Xj
    */
    activeCpu->regX[activeCpu->opI] = (activeCpu->regX[activeCpu->opJ] | ~activeCpu->regX[activeCpu->opK]) & Mask60;
    }

static void cpOp17(CpuContext *activeCpu)
    {
    /*
    **  BXi -Xk-Xj
    */
    activeCpu->regX[activeCpu->opI] = (activeCpu->regX[activeCpu->opJ] ^ ~activeCpu->regX[activeCpu->opK]) & Mask60;
    }

static void cpOp20(CpuContext *activeCpu)
    {
    /*
    **  LXi jk
    */
    u8 jk;

    jk = (u8)((activeCpu->opJ << 3) | activeCpu->opK);
    activeCpu->regX[activeCpu->opI] = shiftLeftCircular(activeCpu->regX[activeCpu->opI] & Mask60, jk);
    }

static void cpOp21(CpuContext *activeCpu)
    {
    /*
    **  AXi jk
    */
    u8 jk;

    jk = (u8)((activeCpu->opJ << 3) | activeCpu->opK);
    activeCpu->regX[activeCpu->opI] = shiftRightArithmetic(activeCpu->regX[activeCpu->opI] & Mask60, jk);
    }

static void cpOp22(CpuContext *activeCpu)
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

static void cpOp23(CpuContext *activeCpu)
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

static void cpOp24(CpuContext *activeCpu)
    {
    /*
    **  NXi Bj Xk
    */
    activeCpu->regX[activeCpu->opI] = shiftNormalize(activeCpu->regX[activeCpu->opK], &activeCpu->regB[activeCpu->opJ], FALSE);
    }

static void cpOp25(CpuContext *activeCpu)
    {
    /*
    **  ZXi Bj Xk
    */
    activeCpu->regX[activeCpu->opI] = shiftNormalize(activeCpu->regX[activeCpu->opK], &activeCpu->regB[activeCpu->opJ], TRUE);
    }

static void cpOp26(CpuContext *activeCpu)
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

static void cpOp27(CpuContext *activeCpu)
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

static void cpOp30(CpuContext *activeCpu)
    {
    /*
    **  FXi Xj+Xk
    */
    activeCpu->regX[activeCpu->opI] = floatAdd(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], FALSE, FALSE);
    }

static void cpOp31(CpuContext *activeCpu)
    {
    /*
    **  FXi Xj-Xk
    */
    activeCpu->regX[activeCpu->opI] = floatAdd(activeCpu->regX[activeCpu->opJ], (~activeCpu->regX[activeCpu->opK] & Mask60), FALSE, FALSE);
    }

static void cpOp32(CpuContext *activeCpu)
    {
    /*
    **  DXi Xj+Xk
    */
    activeCpu->regX[activeCpu->opI] = floatAdd(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], FALSE, TRUE);
    }

static void cpOp33(CpuContext *activeCpu)
    {
    /*
    **  DXi Xj-Xk
    */
    activeCpu->regX[activeCpu->opI] = floatAdd(activeCpu->regX[activeCpu->opJ], (~activeCpu->regX[activeCpu->opK] & Mask60), FALSE, TRUE);
    }

static void cpOp34(CpuContext *activeCpu)
    {
    /*
    **  RXi Xj+Xk
    */
    activeCpu->regX[activeCpu->opI] = floatAdd(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], TRUE, FALSE);
    }

static void cpOp35(CpuContext *activeCpu)
    {
    /*
    **  RXi Xj-Xk
    */
    activeCpu->regX[activeCpu->opI] = floatAdd(activeCpu->regX[activeCpu->opJ], (~activeCpu->regX[activeCpu->opK] & Mask60), TRUE, FALSE);
    }

static void cpOp36(CpuContext *activeCpu)
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

static void cpOp37(CpuContext *activeCpu)
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

static void cpOp40(CpuContext *activeCpu)
    {
    /*
    **  FXi Xj*Xk
    */
    activeCpu->regX[activeCpu->opI] = floatMultiply(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], FALSE, FALSE);
    }

static void cpOp41(CpuContext *activeCpu)
    {
    /*
    **  RXi Xj*Xk
    */
    activeCpu->regX[activeCpu->opI] = floatMultiply(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], TRUE, FALSE);
    }

static void cpOp42(CpuContext *activeCpu)
    {
    /*
    **  DXi Xj*Xk
    */
    activeCpu->regX[activeCpu->opI] = floatMultiply(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], FALSE, TRUE);
    }

static void cpOp43(CpuContext *activeCpu)
    {
    /*
    **  MXi jk
    */
    u8 jk;

    jk = (u8)((activeCpu->opJ << 3) | activeCpu->opK);
    activeCpu->regX[activeCpu->opI] = shiftMask(jk);
    }

static void cpOp44(CpuContext *activeCpu)
    {
    /*
    **  FXi Xj/Xk
    */
    activeCpu->regX[activeCpu->opI] = floatDivide(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], FALSE);
    }

static void cpOp45(CpuContext *activeCpu)
    {
    /*
    **  RXi Xj/Xk
    */
    activeCpu->regX[activeCpu->opI] = floatDivide(activeCpu->regX[activeCpu->opJ], activeCpu->regX[activeCpu->opK], TRUE);
    }

static void cpOp46(CpuContext *activeCpu)
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
        cpuCmuMoveIndirect(activeCpu);
        break;

    case 5:
        /*
        **  Move direct.
        */
        cpuCmuMoveDirect(activeCpu);
        break;

    case 6:
        /*
        **  Compare collated.
        */
        cpuCmuCompareCollated(activeCpu);
        break;

    case 7:
        /*
        **  Compare uncollated.
        */
        cpuCmuCompareUncollated(activeCpu);
        break;
        }
#endif
    }

static void cpOp47(CpuContext *activeCpu)
    {
    /*
    **  CXi Xk
    */
    u8 i;
    u8 count = 0;
    CpWord acc60;

    acc60 = activeCpu->regX[activeCpu->opK] & Mask60;

    for (i = 60; i > 0; i--)
        {
        count += (u8)(acc60 & 1);
        acc60 >>= 1;
        }

    activeCpu->regX[activeCpu->opI] = count;
    }

static void cpOp50(CpuContext *activeCpu)
    {
    /*
    **  SAi Aj+K
    */
    activeCpu->regA[activeCpu->opI] = cpuAdd18(activeCpu->regA[activeCpu->opJ], activeCpu->opAddress);

    cpuRegASemantics (activeCpu);
    }

static void cpOp51(CpuContext *activeCpu)
    {
    /*
    **  SAi Bj+K
    */
    activeCpu->regA[activeCpu->opI] = cpuAdd18(activeCpu->regB[activeCpu->opJ], activeCpu->opAddress);

    cpuRegASemantics (activeCpu);
    }

static void cpOp52(CpuContext *activeCpu)
    {
    /*
    **  SAi Xj+K
    */
    activeCpu->regA[activeCpu->opI] = cpuAdd18((u32)activeCpu->regX[activeCpu->opJ], activeCpu->opAddress);

    cpuRegASemantics (activeCpu);
    }

static void cpOp53(CpuContext *activeCpu)
    {
    /*
    **  SAi Xj+Bk
    */
    activeCpu->regA[activeCpu->opI] = cpuAdd18((u32)activeCpu->regX[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    cpuRegASemantics (activeCpu);
    }

static void cpOp54(CpuContext *activeCpu)
    {
    /*
    **  SAi Aj+Bk
    */
    activeCpu->regA[activeCpu->opI] = cpuAdd18(activeCpu->regA[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    cpuRegASemantics (activeCpu);
    }

static void cpOp55(CpuContext *activeCpu)
    {
    /*
    **  SAi Aj-Bk
    */
    activeCpu->regA[activeCpu->opI] = cpuSubtract18(activeCpu->regA[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    cpuRegASemantics (activeCpu);
    }

static void cpOp56(CpuContext *activeCpu)
    {
    /*
    **  SAi Bj+Bk
    */
    activeCpu->regA[activeCpu->opI] = cpuAdd18(activeCpu->regB[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    cpuRegASemantics (activeCpu);
    }

static void cpOp57(CpuContext *activeCpu)
    {
    /*
    **  SAi Bj-Bk
    */
    activeCpu->regA[activeCpu->opI] = cpuSubtract18(activeCpu->regB[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);

    cpuRegASemantics (activeCpu);
    }

static void cpOp60(CpuContext *activeCpu)
    {
    /*
    **  SBi Aj+K
    */
    activeCpu->regB[activeCpu->opI] = cpuAdd18(activeCpu->regA[activeCpu->opJ], activeCpu->opAddress);
    }

static void cpOp61(CpuContext *activeCpu)
    {
    /*
    **  SBi Bj+K
    */
    activeCpu->regB[activeCpu->opI] = cpuAdd18(activeCpu->regB[activeCpu->opJ], activeCpu->opAddress);
    }

static void cpOp62(CpuContext *activeCpu)
    {
    /*
    **  SBi Xj+K
    */
    activeCpu->regB[activeCpu->opI] = cpuAdd18((u32)activeCpu->regX[activeCpu->opJ], activeCpu->opAddress);
    }

static void cpOp63(CpuContext *activeCpu)
    {
    /*
    **  SBi Xj+Bk
    */
    activeCpu->regB[activeCpu->opI] = cpuAdd18((u32)activeCpu->regX[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);
    }

static void cpOp64(CpuContext *activeCpu)
    {
    /*
    **  SBi Aj+Bk
    */
    activeCpu->regB[activeCpu->opI] = cpuAdd18(activeCpu->regA[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);
    }

static void cpOp65(CpuContext *activeCpu)
    {
    /*
    **  SBi Aj-Bk
    */
    activeCpu->regB[activeCpu->opI] = cpuSubtract18(activeCpu->regA[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);
    }

static void cpOp66(CpuContext *activeCpu)
    {
    /*
    **  SBi Bj+Bk
    */
    activeCpu->regB[activeCpu->opI] = cpuAdd18(activeCpu->regB[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);
    }

static void cpOp67(CpuContext *activeCpu)
    {
    /*
    **  SBi Bj-Bk
    */
    activeCpu->regB[activeCpu->opI] = cpuSubtract18(activeCpu->regB[activeCpu->opJ], activeCpu->regB[activeCpu->opK]);
    }

static void cpOp70(CpuContext *activeCpu)
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

static void cpOp71(CpuContext *activeCpu)
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

static void cpOp72(CpuContext *activeCpu)
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

static void cpOp73(CpuContext *activeCpu)
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

static void cpOp74(CpuContext *activeCpu)
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

static void cpOp75(CpuContext *activeCpu)
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

static void cpOp76(CpuContext *activeCpu)
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

static void cpOp77(CpuContext *activeCpu)
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
