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
    void (*execute)(void);
    u8   length;
    } OpDispatch;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
//static bool cpuFetchOpWord(u32 address, CpWord *data);
#define cpuFetchOpWord cpuReadMem
static INLINE bool cpuReadMem(u32 address, CpWord *data);
static INLINE bool cpuWriteMem(u32 address, CpWord *data);
static INLINE void cpuRegASemantics(void);
static INLINE u32 cpuAdd18(u32 op1, u32 op2);
static INLINE u32 cpuSubtract18(u32 op1, u32 op2);
static void cpuEcsTransfer(bool writeToEcs);
static bool cpuCmuGetByte(u32 address, u32 pos, u8 *byte);
static bool cpuCmuPutByte(u32 address, u32 pos, u8 byte);
static void cpuCmuMoveIndirect(void);
static void cpuCmuMoveDirect(void);
static void cpuCmuCompareCollated(void);
static void cpuCmuCompareUncollated(void);
static void cpuTraceCtl(void);

static void cpOp00(void);
static void cpOp01(void);
static void cpOp02(void);
static void cpOp03(void);
static void cpOp04(void);
static void cpOp05(void);
static void cpOp06(void);
static void cpOp07(void);
static void cpOp10(void);
static void cpOp11(void);
static void cpOp12(void);
static void cpOp13(void);
static void cpOp14(void);
static void cpOp15(void);
static void cpOp16(void);
static void cpOp17(void);
static void cpOp20(void);
static void cpOp21(void);
static void cpOp22(void);
static void cpOp23(void);
static void cpOp24(void);
static void cpOp25(void);
static void cpOp26(void);
static void cpOp27(void);
static void cpOp30(void);
static void cpOp31(void);
static void cpOp32(void);
static void cpOp33(void);
static void cpOp34(void);
static void cpOp35(void);
static void cpOp36(void);
static void cpOp37(void);
static void cpOp40(void);
static void cpOp41(void);
static void cpOp42(void);
static void cpOp43(void);
static void cpOp44(void);
static void cpOp45(void);
static void cpOp46(void);
static void cpOp47(void);
static void cpOp50(void);
static void cpOp51(void);
static void cpOp52(void);
static void cpOp53(void);
static void cpOp54(void);
static void cpOp55(void);
static void cpOp56(void);
static void cpOp57(void);
static void cpOp60(void);
static void cpOp61(void);
static void cpOp62(void);
static void cpOp63(void);
static void cpOp64(void);
static void cpOp65(void);
static void cpOp66(void);
static void cpOp67(void);
static void cpOp70(void);
static void cpOp71(void);
static void cpOp72(void);
static void cpOp73(void);
static void cpOp74(void);
static void cpOp75(void);
static void cpOp76(void);
static void cpOp77(void);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
CpWord *cpMem;
CpWord *ecsMem;
u32 ecsFlagRegister;
CpuContext cpu;
bool cpuStopped = TRUE;
u32 cpuMaxMemory;
u32 ecsMaxMemory;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static u8 opOffset;
static CpWord opWord;
static u8 opFm;
static u8 opI;
static u8 opJ;
static u8 opK;
static u32 opAddress;
static u32 oldRegP;
static CpWord acc60;
static u32 acc18;

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
static INLINE bool cpuReadMem(u32 address, CpWord *data)
    {
    if (address >= cpu.regFlCm || cpu.regRaCm + address >= cpuMaxMemory)
        {
        cpu.exitCondition |= EcAddressOutOfRange;
        if ((cpu.exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (cpu.regRaCm < cpuMaxMemory)
                {
                cpMem[cpu.regRaCm] = ((CpWord)cpu.exitCondition << 48) | ((CpWord)(cpu.regP + 1) << 30);
                }

            cpu.regP = 0;
            *data = cpMem[0] & Mask60;
            // ????????????? jump to monitor address ??????????????
            return(TRUE);
            }
        else
            {
            address = 0;
            }
        }

    *data = cpMem[cpu.regRaCm + address] & Mask60;

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
static INLINE bool cpuWriteMem(u32 address, CpWord *data)
    {
    if (address >= cpu.regFlCm || cpu.regRaCm + address >= cpuMaxMemory)
        {
        cpu.exitCondition |= EcAddressOutOfRange;
        if ((cpu.exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (cpu.regRaCm < cpuMaxMemory)
                {
                cpMem[cpu.regRaCm] = ((CpWord)cpu.exitCondition << 48) | ((CpWord)(cpu.regP + 1) << 30);
                }

            cpu.regP = 0;
            // ????????????? jump to monitor address ??????????????
            return(TRUE);
            }

        return(FALSE);
        }

    cpMem[cpu.regRaCm + address] = *data & Mask60;

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
**                  memory      configured central memory
**                  ecsBanks    configured number of ECS banks
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cpuInit(char *model, u32 memory, u32 ecsBanks)
    {
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
    **  Print a friendly message.
    */
    printf("CPU model %s initialised (CM: %o, ECS: %o)\n", model, cpuMaxMemory, ecsMaxMemory);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Return CPU P register.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
u32 cpuGetP(void)
    {
    return((cpu.regP) & Mask18);
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
**  Purpose:        Perform exchange jump.
**
**  Parameters:     Name        Description.
**                  addr        Exchange jump address.
**
**  Returns:        TRUE if exchange jump can be performed, FALSE otherwise.
**
**------------------------------------------------------------------------*/
bool cpuExchangeJump(u32 addr)
    {
    CpuContext tmp;
    CpWord *mem;


    /*
    **  Only perform exchange jump on instruction boundary or when stopped.
    */
    if (opOffset != 60 && !cpuStopped)
        {
        return(FALSE);
        }

#if CcDebug == 1
    traceExchange(&cpu, addr, "Old");
#endif

    /*
    **  Clear any spurious address bits.
    */
    addr &= Mask18;

    /*
    **  Verify if exchange package is within configured memory.
    **  <<<<<<<<<<<<<<<<<<< not quite right - maybe need to wrap  >>>>>>>>>>>>>>>>>>>>>>>>
    */
    if (addr + 020 >= cpuMaxMemory)
        {
        /*
        **  Pretend that exchange worked, but the address is bad.
        */
        return(TRUE);
        }

    /*
    **  Save current context.
    */
    tmp = cpu;

    /*
    **  Setup new context.
    */
    mem = cpMem + addr;

    cpu.regP     = (u32)((*mem >> 36) & Mask18);
    cpu.regA[0]  = (u32)((*mem >> 18) & Mask18);
    cpu.regB[0]  = 0;

    mem += 1;
    cpu.regRaCm  = (u32)((*mem >> 36) & Mask24);
    cpu.regA[1]  = (u32)((*mem >> 18) & Mask18);
    cpu.regB[1]  = (u32)((*mem      ) & Mask18);

    mem += 1;
    cpu.regFlCm  = (u32)((*mem >> 36) & Mask24);
    cpu.regA[2]  = (u32)((*mem >> 18) & Mask18);
    cpu.regB[2]  = (u32)((*mem      ) & Mask18);

    mem += 1;
    cpu.exitMode = (u32)((*mem >> 36) & Mask24);
    cpu.regA[3]  = (u32)((*mem >> 18) & Mask18);
    cpu.regB[3]  = (u32)((*mem      ) & Mask18);

    mem += 1;
    cpu.regRaEcs = (u32)((*mem >> 36) & Mask24Ecs);
    cpu.regA[4]  = (u32)((*mem >> 18) & Mask18);
    cpu.regB[4]  = (u32)((*mem      ) & Mask18);

    mem += 1;
    cpu.regFlEcs = (u32)((*mem >> 36) & Mask24Ecs);
    cpu.regA[5]  = (u32)((*mem >> 18) & Mask18);
    cpu.regB[5]  = (u32)((*mem      ) & Mask18);

    mem += 1;
    cpu.regMa    = (u32)((*mem >> 36) & Mask24);
    cpu.regA[6]  = (u32)((*mem >> 18) & Mask18);
    cpu.regB[6]  = (u32)((*mem      ) & Mask18);

    mem += 1;
    cpu.regSpare = (u32)((*mem >> 36) & Mask24);
    cpu.regA[7]  = (u32)((*mem >> 18) & Mask18);
    cpu.regB[7]  = (u32)((*mem      ) & Mask18);

    mem += 1;
    cpu.regX[0]  = *mem++ & Mask60;
    cpu.regX[1]  = *mem++ & Mask60;
    cpu.regX[2]  = *mem++ & Mask60;
    cpu.regX[3]  = *mem++ & Mask60;
    cpu.regX[4]  = *mem++ & Mask60;
    cpu.regX[5]  = *mem++ & Mask60;
    cpu.regX[6]  = *mem++ & Mask60;
    cpu.regX[7]  = *mem++ & Mask60;

    cpu.exitCondition = EcNone;

#if CcDebug == 1
    traceExchange(&cpu, addr, "New");
#endif

    /*
    **  Save old context.
    */
    mem = cpMem + addr;

    *mem++ = ((CpWord)(tmp.regP     & Mask18) << 36) | ((CpWord)(tmp.regA[0] & Mask18) << 18);
    *mem++ = ((CpWord)(tmp.regRaCm  & Mask24) << 36) | ((CpWord)(tmp.regA[1] & Mask18) << 18) | ((CpWord)(tmp.regB[1] & Mask18));
    *mem++ = ((CpWord)(tmp.regFlCm  & Mask24) << 36) | ((CpWord)(tmp.regA[2] & Mask18) << 18) | ((CpWord)(tmp.regB[2] & Mask18));
    *mem++ = ((CpWord)(tmp.exitMode & Mask24) << 36) | ((CpWord)(tmp.regA[3] & Mask18) << 18) | ((CpWord)(tmp.regB[3] & Mask18));
    *mem++ = ((CpWord)(tmp.regRaEcs & Mask24) << 36) | ((CpWord)(tmp.regA[4] & Mask18) << 18) | ((CpWord)(tmp.regB[4] & Mask18));
    *mem++ = ((CpWord)(tmp.regFlEcs & Mask24) << 36) | ((CpWord)(tmp.regA[5] & Mask18) << 18) | ((CpWord)(tmp.regB[5] & Mask18));
    *mem++ = ((CpWord)(tmp.regMa    & Mask24) << 36) | ((CpWord)(tmp.regA[6] & Mask18) << 18) | ((CpWord)(tmp.regB[6] & Mask18));
    *mem++ = ((CpWord)(tmp.regSpare & Mask24) << 36) | ((CpWord)(tmp.regA[7] & Mask18) << 18) | ((CpWord)(tmp.regB[7] & Mask18));
    *mem++ = tmp.regX[0] & Mask60;
    *mem++ = tmp.regX[1] & Mask60;
    *mem++ = tmp.regX[2] & Mask60;
    *mem++ = tmp.regX[3] & Mask60;
    *mem++ = tmp.regX[4] & Mask60;
    *mem++ = tmp.regX[5] & Mask60;
    *mem++ = tmp.regX[6] & Mask60;
    *mem++ = tmp.regX[7] & Mask60;

    /*
    **  Activate CPU.
    */
    cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
    opOffset = 60;

    return(TRUE);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute next instruction in the CPU.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cpuStep(void)
    {
    if (cpuStopped)
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
        opFm = (u8)((opWord >> (opOffset - 6)) & Mask6);
        if (decodeCpuOpcode[opFm].length == 15)
            {
            opI       = (u8)((opWord >> (opOffset -  9)) & Mask3);
            opJ       = (u8)((opWord >> (opOffset - 12)) & Mask3);
            opK       = (u8)((opWord >> (opOffset - 15)) & Mask3);
            opAddress = 0;

            opOffset -= 15;
            }
        else
            {
            if (opOffset == 15)
                {
                /*
                **  Stop when packing is invalid - this is the
                **  behaviour of the 6400 and 6500. 
                */
#if CcDebug == 1
                traceCpuPrint("Invalid packing\n");
#endif
                cpuStopped = TRUE;
                return;
                }

            opI       = (u8)((opWord >> (opOffset -  9)) & Mask3);
            opJ       = (u8)((opWord >> (opOffset - 12)) & Mask3);
            opK       = 0;
            opAddress = (u32)((opWord >> (opOffset - 30)) & Mask18);

            opOffset -= 30;
            }

        oldRegP = cpu.regP;

        /*
        **  Force B0 to 0.
        */
        cpu.regB[0] = 0;

        /*
        **  Execute instruction.
        */
        decodeCpuOpcode[opFm].execute();

        /*
        **  Force B0 to 0.
        */
        cpu.regB[0] = 0;

#if CcDebug == 1
        /*
        **  Don't trace COS's idle loop.
        */
    //    if ((cpu.regRaCm + cpu.regP) > 02062)

        /*
        **  Don't trace NOS's idle loop and CPUMTR.
        **  If control point tracing is set, act accordingly.
        */
        if (/*cpu.regRaCm != 0 && */ cpu.regP > 0100)
            {
            if (traceCp == 0 || 
                (!cpu.monitorMode &&
                 ((cpMem[060] >> 31) & 037) == traceCp))
                {
                traceCpu(oldRegP, opFm, opI, opJ, opK, opAddress);
                }
            if (opFm == 061 && opI == 0 && opJ != 0)
                {
                cpuTraceCtl ();
                }
            }
#endif

        if (cpuStopped)
            {
            if (opOffset == 0)
                {
                cpu.regP = (cpu.regP + 1) & Mask18;
                }
#if CcDebug == 1
            traceCpuPrint("Stopped\n");
#endif
            return;
            }

        /*
        **  Fetch next instruction word if necessary.
        */
        if (opOffset == 0)
            {
            cpu.regP = (cpu.regP + 1) & Mask18;
            cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
            opOffset = 60;
            }
        } while (opOffset != 60);
    }

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
                return TRUE;
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
                return TRUE;
                }

            break;

        case 7:
            /*
            **  Selective clear,
            */
            ecsFlagRegister = (ecsFlagRegister & ~flagWord) & Mask18;
            break;
            }

        return FALSE;
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

/*--------------------------------------------------------------------------
**  Purpose:        Turn on/off tracing under program control.
**
**  Parameters:     none.
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
static void cpuTraceCtl (void)
    {
    if (opJ == 1 && opAddress == 000042)
        {
        // sb0 b1+42 is the magic word to turn on tracing
        traceCp = (cpMem[060] >> 31) & 037;
        traceMask |= TraceCpu;
        }
    else if (opJ == 1 && opAddress == 000076)
        {
        traceMask &= ~TraceCpu;
        }
    else if (opJ == 2)
        {
        traceMask |= TraceCpu;
        traceCycles = opAddress;
        }
    }
 
/*--------------------------------------------------------------------------
**  Purpose:        Read CPU instruction word and verify that address is
**                  within limits.
**
**  Parameters:     Name        Description.
**                  address     RA relative address to read.
**                  data        Pointer to 60 bit word which gets the data.
**
**  Returns:        TRUE if access failed, FALSE otherwise;
**
**------------------------------------------------------------------------*/
#if 0
static bool cpuFetchOpWord(u32 address, CpWord *data)
    {
    u32 location;

    location = cpu.regRaCm + address;
    if ((location & Overflow18) != 0)
        {
        location += 1;
        location &= Mask18;
        }


    if (address >= cpu.regFlCm || location >= cpuMaxMemory)
        {
        cpu.exitCondition |= EcAddressOutOfRange;
        if ((cpu.exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (cpu.regRaCm < cpuMaxMemory)
                {
                cpMem[cpu.regRaCm] = ((CpWord)cpu.exitCondition << 48) | ((CpWord)(cpu.regP) << 30);
                }

            cpu.regP = 0;
            // ????????????? jump to monitor address ??????????????
            }

        return(TRUE);
        }

    *data = cpMem[location] & Mask60;

    return(FALSE);
    }
#endif
/*--------------------------------------------------------------------------
**  Purpose:        Implement A register sematics.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static INLINE void cpuRegASemantics(void)
    {
    if (opI == 0)
        {
        return;
        }

    if (opI <= 5)
        {
        /*
        **  Read semantics.
        */
        cpuStopped = cpuReadMem(cpu.regA[opI], cpu.regX + opI);
        }
    else
        {
        /*
        **  Write semantics.
        */
        cpuStopped = cpuWriteMem(cpu.regA[opI], cpu.regX + opI);
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
**                  writeToEcs  TRUE if this is a write to ECS, FALSE if
**                              this is a read.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void cpuEcsTransfer(bool writeToEcs)
    {
    u32 wordCount;
    u32 ecsAddress;
    u32 cmAddress;
    bool takeErrorExit = FALSE;

    /*
    **  ECS must exist and instruction must be located in the upper 30 bits.
    */
    if (ecsMaxMemory == 0 || opOffset != 30)
        {
        cpu.exitCondition |= EcAddressOutOfRange;       // <<<<<<<<<<<<< this may be wrong - manual does not specify
        cpu.regP = 0;
        cpuStopped = TRUE;
        return;
        }

    /*
    **  Calculate word count, source and destination addresses.
    */
    wordCount = cpuAdd18(cpu.regB[opJ], opAddress);
    ecsAddress = (u32)(cpu.regX[0] & Mask24);
    cmAddress = cpu.regA[0] & Mask18;

    /*
    **  Check if this is a flag register access.
    **
    **  The ECS book (60225100) says that a flag register reference occurs
    **  when bit 23 is set in the relative address AND in the ECS FL.
    **  But the ECS RA is NOT added to the relative address.
    */
    if ((ecsAddress & ((u32)1 << 23)) != 0 &&
        (cpu.regFlEcs & ((u32)1 << 23)) != 0)
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
        cpu.regP = (cpu.regP + 1) & Mask18;
        cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
        opOffset = 60;
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
        || cpu.regFlCm  < cmAddress + wordCount
        || cpu.regFlEcs < ecsAddress + wordCount)
        {
        cpu.exitCondition |= EcAddressOutOfRange;
        if ((cpu.exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            cpu.regP = 0;
            cpuStopped = TRUE;
            }
        else
            {
            cpu.regP = (cpu.regP + 1) & Mask18;
            cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
            opOffset = 60;
            }

        return;
        }

    /*
    **  Add base addresses.
    */
    cmAddress += cpu.regRaCm;
    ecsAddress += cpu.regRaEcs;

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
    cpu.regP = (cpu.regP + 1) & Mask18;
    cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
    opOffset = 60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        CMU get a byte.
**
**  Parameters:     Name        Description.
**                  address     CM word address
**                  pos         character position
**                  byte        pointer to byte
**
**  Returns:        TRUE if access failed, FALSE otherwise.
**
**------------------------------------------------------------------------*/
static bool cpuCmuGetByte(u32 address, u32 pos, u8 *byte)
    {
    CpWord data;

    /*
    **  Validate access.
    */
    if (address >= cpu.regFlCm || cpu.regRaCm + address >= cpuMaxMemory)
        {
        cpu.exitCondition |= EcAddressOutOfRange;
        if ((cpu.exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (cpu.regRaCm < cpuMaxMemory)
                {
                cpMem[cpu.regRaCm] = ((CpWord)cpu.exitCondition << 48) | ((CpWord)(cpu.regP + 1) << 30);
                }

            cpu.regP = 0;
            cpuStopped = TRUE;
            // ????????????? jump to monitor address ??????????????
            }

        return(TRUE);
        }

    /*
    **  Fetch the word.
    */
    data = cpMem[cpu.regRaCm + address] & Mask60;

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
**                  address     CM word address
**                  pos         character position
**                  byte        data byte to put
**
**  Returns:        TRUE if access failed, FALSE otherwise.
**
**------------------------------------------------------------------------*/
static bool cpuCmuPutByte(u32 address, u32 pos, u8 byte)
    {
    CpWord data;

    /*
    **  Validate access.
    */
    if (address >= cpu.regFlCm || cpu.regRaCm + address >= cpuMaxMemory)
        {
        cpu.exitCondition |= EcAddressOutOfRange;
        if ((cpu.exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (cpu.regRaCm < cpuMaxMemory)
                {
                cpMem[cpu.regRaCm] = ((CpWord)cpu.exitCondition << 48) | ((CpWord)(cpu.regP + 1) << 30);
                }

            cpu.regP = 0;
            cpuStopped = TRUE;
            // ????????????? jump to monitor address ??????????????
            }

        return(TRUE);
        }

    /*
    **  Fetch the word.
    */
    data = cpMem[cpu.regRaCm + address] & Mask60;

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
    cpMem[cpu.regRaCm + address] = data & Mask60;

    return(FALSE);
    }

/*--------------------------------------------------------------------------
**  Purpose:        CMU move indirect.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void cpuCmuMoveIndirect(void)
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
    opAddress = (u32)((opWord >> 30) & Mask18);
    opAddress = cpuAdd18(cpu.regB[opJ], opAddress);
    cpuStopped = cpuReadMem(opAddress, &descWord);
    if (cpuStopped)
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
        cpu.exitCondition |= EcAddressOutOfRange;
        if ((cpu.exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (cpu.regRaCm < cpuMaxMemory)
                {
                cpMem[cpu.regRaCm] = ((CpWord)cpu.exitCondition << 48) | ((CpWord)(cpu.regP + 1) << 30);
                }

            cpu.regP = 0;
            // ????????????? jump to monitor address ??????????????
            cpuStopped = TRUE;
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
        if (   cpuCmuGetByte(k1, c1, &byte)
            || cpuCmuPutByte(k2, c2, byte))
            {
            if (cpuStopped)
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
    cpu.regX[0] = 0;

    /*
    **  Normal exit to next instruction word.
    */
    cpu.regP = (cpu.regP + 1) & Mask18;
    cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
    opOffset = 60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        CMU move direct.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void cpuCmuMoveDirect(void)
    {
    u32 k1, k2;
    u32 c1, c2;
    u32 ll;
    u8 byte;

    //<<<<<<<<<<<<<<<<<<<<<<<< don't forget to optimise c1 == c2 cases.

    /*
    **  Decode opcode word.
    */
    k1 = (u32)(opWord >> 30) & Mask18;
    k2 = (u32)(opWord >>  0) & Mask18;
    c1 = (u32)(opWord >> 22) & Mask4;
    c2 = (u32)(opWord >> 18) & Mask4;
    ll = (u32)((opWord >> 26) & Mask4) | (u32)((opWord >> (48 - 4)) & (Mask3 << 4));

    /*
    **  Check for address out of range.
    */
    if (c1 > 9 || c2 > 9)
        {
        cpu.exitCondition |= EcAddressOutOfRange;
        if ((cpu.exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (cpu.regRaCm < cpuMaxMemory)
                {
                cpMem[cpu.regRaCm] = ((CpWord)cpu.exitCondition << 48) | ((CpWord)(cpu.regP + 1) << 30);
                }

            cpu.regP = 0;
            // ????????????? jump to monitor address ??????????????
            cpuStopped = TRUE;
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
        if (   cpuCmuGetByte(k1, c1, &byte)
            || cpuCmuPutByte(k2, c2, byte))
            {
            if (cpuStopped)
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
    cpu.regX[0] = 0;

    /*
    **  Normal exit to next instruction word.
    */
    cpu.regP = (cpu.regP + 1) & Mask18;
    cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
    opOffset = 60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        CMU compare collated.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void cpuCmuCompareCollated(void)
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
    k1 = (u32)(opWord >> 30) & Mask18;
    k2 = (u32)(opWord >>  0) & Mask18;
    c1 = (u32)(opWord >> 22) & Mask4;
    c2 = (u32)(opWord >> 18) & Mask4;
    ll = (u32)((opWord >> 26) & Mask4) | (u32)((opWord >> (48 - 4)) & (Mask3 << 4));

    /*
    **  Setup collating table.
    */
    collTable = cpu.regA[0];

    /*
    **  Check for addresses and collTable out of range.
    */
    if (c1 > 9 || c2 > 9 || collTable >= cpu.regFlCm || cpu.regRaCm + collTable >= cpuMaxMemory)
        {
        cpu.exitCondition |= EcAddressOutOfRange;
        if ((cpu.exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (cpu.regRaCm < cpuMaxMemory)
                {
                cpMem[cpu.regRaCm] = ((CpWord)cpu.exitCondition << 48) | ((CpWord)(cpu.regP + 1) << 30);
                }

            cpu.regP = 0;
            // ????????????? jump to monitor address ??????????????
            cpuStopped = TRUE;
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
        if (   cpuCmuGetByte(k1, c1, &byte1)
            || cpuCmuGetByte(k2, c2, &byte2))
            {
            if (cpuStopped)
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
            if (   cpuCmuGetByte(collTable + ((byte1 >> 3) & Mask3), byte1 & Mask3, &byte1)
                || cpuCmuGetByte(collTable + ((byte2 >> 3) & Mask3), byte2 & Mask3, &byte2))
                {
                if (cpuStopped)
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
    cpu.regX[0] = result;

    /*
    **  Normal exit to next instruction word.
    */
    cpu.regP = (cpu.regP + 1) & Mask18;
    cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
    opOffset = 60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        CMU compare uncollated.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void cpuCmuCompareUncollated(void)
    {
    CpWord result = 0;
    u32 k1, k2;
    u32 c1, c2;
    u32 ll;
    u8 byte1, byte2;

    /*
    **  Decode opcode word.
    */
    k1 = (u32)(opWord >> 30) & Mask18;
    k2 = (u32)(opWord >>  0) & Mask18;
    c1 = (u32)(opWord >> 22) & Mask4;
    c2 = (u32)(opWord >> 18) & Mask4;
    ll = (u32)((opWord >> 26) & Mask4) | (u32)((opWord >> (48 - 4)) & (Mask3 << 4));

    /*
    **  Check for address out of range.
    */
    if (c1 > 9 || c2 > 9)
        {
        cpu.exitCondition |= EcAddressOutOfRange;
        if ((cpu.exitMode & EmAddressOutOfRange) != 0)
            {
            /*
            **  Exit mode selected.
            */
            if (cpu.regRaCm < cpuMaxMemory)
                {
                cpMem[cpu.regRaCm] = ((CpWord)cpu.exitCondition << 48) | ((CpWord)(cpu.regP + 1) << 30);
                }

            cpu.regP = 0;
            // ????????????? jump to monitor address ??????????????
            cpuStopped = TRUE;
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
        if (   cpuCmuGetByte(k1, c1, &byte1)
            || cpuCmuGetByte(k2, c2, &byte2))
            {
            if (cpuStopped)
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
    cpu.regX[0] = result;

    /*
    **  Normal exit to next instruction word.
    */
    cpu.regP = (cpu.regP + 1) & Mask18;
    cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
    opOffset = 60;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Functions to implement all opcodes
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/

static void cpOp00(void)
    {
    /*
    **  PS.
    */
    cpuStopped = TRUE;

#if NeedToUnderstandPSBetter

??? how should this behave if CEJ/MEJ is enabled ???

cpu.regP = (cpu.regP + 1) & Mask18;
cpu.monitorMode = TRUE;
cpuExchangeJump(cpu.regMa);

#endif
    }

static void cpOp01(void)
    {
    if (opI == 0)
        {
        /*
        **  RJ  K
        */
        acc60 = ((CpWord)0400 << 48) | ((CpWord)((cpu.regP + 1) & Mask18) << 30);
        cpuStopped = cpuWriteMem(opAddress, &acc60);
        cpu.regP = opAddress;
        opOffset = 0;
        }
    else if (opI == 1)
        {
        /*
        **  RE  Bj+K
        */
        cpuEcsTransfer(FALSE);
        }
    else if (opI == 2)
        {
        /*
        **  WE  Bj+K
        */
        cpuEcsTransfer(TRUE);
        }
    else if (opI == 3)
        {
        /*
        **  XJ  K
        */
        cpu.regP = (cpu.regP + 1) & Mask18;
        cpuStopped = TRUE;

        if (cpu.monitorMode)
            {
            cpu.monitorMode = FALSE;
            cpuExchangeJump(opAddress + cpu.regB[opJ]);
            }
        else
            {
            cpu.monitorMode = TRUE;
            cpuExchangeJump(cpu.regMa);
            }
        }
    }

static void cpOp02(void)
    {
    /*
    **  JP  Bi+K
    */
    cpu.regP = cpuAdd18(cpu.regB[opI], opAddress);
    cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
    opOffset = 60;
    }

static void cpOp03(void)
    {
    bool jump = FALSE;

    switch (opI)
        {
    case 0:
        /*
        **  ZR  Xj K
        */
        jump = cpu.regX[opJ] == 0 || cpu.regX[opJ] == NegativeZero;
        break;

    case 1:
        /*
        **  NZ  Xj K
        */
        jump = cpu.regX[opJ] != 0 && cpu.regX[opJ] != NegativeZero;
        break;

    case 2:
        /*
        **  PL  Xj K
        */
        jump = (cpu.regX[opJ] & Sign60) == 0;
        break;

    case 3:
        /*
        **  NG  Xj K
        */
        jump = (cpu.regX[opJ] & Sign60) != 0;
        break;

    case 4:
        /*
        **  IR  Xj K
        */
        acc60 = cpu.regX[opJ] >> 48;
        jump = acc60 != 03777 && acc60 != 04000;
        break;

    case 5:
        /*
        **  OR  Xj K
        */
        acc60 = cpu.regX[opJ] >> 48;
        jump = acc60 == 03777 || acc60 == 04000;
        break;

    case 6:
        /*
        **  DF  Xj K
        */
        acc60 = cpu.regX[opJ] >> 48;
        jump = acc60 != 01777 && acc60 != 06000;
        break;

    case 7:
        /*
        **  ID  Xj K
        */
        acc60 = cpu.regX[opJ] >> 48;
        jump = acc60 == 01777 || acc60 == 06000;
        break;
        }

    if (jump)
        {
        cpu.regP = opAddress;
        cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
        opOffset = 60;
        }
    }

static void cpOp04(void)
    {
    /*
    **  EQ  Bi Bj K
    */
    if (cpu.regB[opI] == cpu.regB[opJ])
        {
        cpu.regP = opAddress;
        cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
        opOffset = 60;
        }
    }

static void cpOp05(void)
    {
    /*
    **  NE  Bi Bj K
    */
    if (cpu.regB[opI] != cpu.regB[opJ])
        {
        cpu.regP = opAddress;
        cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
        opOffset = 60;
        }
    }

static void cpOp06(void)
    {
    /*
    **  GE  Bi Bj K
    */
    i32 signDiff = (cpu.regB[opI] & Sign18) - (cpu.regB[opJ] & Sign18);
    if (signDiff > 0)
        {
        return;
        }
    if (signDiff == 0)
        {
        acc18 = (cpu.regB[opI] & Mask18) - (cpu.regB[opJ] & Mask18);
        if ((acc18 & Overflow18) != 0 && (acc18 & Mask18) != 0)
            {
            acc18 -= 1;
            }

        if ((acc18 & Sign18) != 0)
            {
            return;
            }
        }

    cpu.regP = opAddress;
    cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
    opOffset = 60;
    }

static void cpOp07(void)
    {
    /*
    **  LT  Bi Bj K
    */
    i32 signDiff = (cpu.regB[opI] & Sign18) - (cpu.regB[opJ] & Sign18);
    if (signDiff < 0)
        {
        return;
        }

    if (signDiff == 0)
        {
        acc18 = (cpu.regB[opI] & Mask18) - (cpu.regB[opJ] & Mask18);
        if ((acc18 & Overflow18) != 0 && (acc18 & Mask18) != 0)
            {
            acc18 -= 1;
            }

        if ((acc18 & Sign18) == 0 || acc18 == 0)
            {
            return;
            }
        }

    cpu.regP = opAddress;
    cpuStopped = cpuFetchOpWord(cpu.regP, &opWord);
    opOffset = 60;
    }

static void cpOp10(void)
    {
    /*
    **  BXi Xj
    */
    cpu.regX[opI] = cpu.regX[opJ] & Mask60;
    }

static void cpOp11(void)
    {
    /*
    **  BXi Xj*Xk
    */
    cpu.regX[opI] = (cpu.regX[opJ] & cpu.regX[opK]) & Mask60;
    }

static void cpOp12(void)
    {
    /*
    **  BXi Xj+Xk
    */
    cpu.regX[opI] = (cpu.regX[opJ] | cpu.regX[opK]) & Mask60;
    }

static void cpOp13(void)
    {
    /*
    **  BXi Xj-Xk
    */
    cpu.regX[opI] = (cpu.regX[opJ] ^ cpu.regX[opK]) & Mask60;
    }

static void cpOp14(void)
    {
    /*
    **  BXi -Xj
    */
    cpu.regX[opI] = ~cpu.regX[opK] & Mask60;
    }

static void cpOp15(void)
    {
    /*
    **  BXi -Xk*Xj
    */
    cpu.regX[opI] = (cpu.regX[opJ] & ~cpu.regX[opK]) & Mask60;
    }

static void cpOp16(void)
    {
    /*
    **  BXi -Xk+Xj
    */
    cpu.regX[opI] = (cpu.regX[opJ] | ~cpu.regX[opK]) & Mask60;
    }

static void cpOp17(void)
    {
    /*
    **  BXi -Xk-Xj
    */
    cpu.regX[opI] = (cpu.regX[opJ] ^ ~cpu.regX[opK]) & Mask60;
    }

static void cpOp20(void)
    {
    /*
    **  LXi jk
    */
    u8 jk;

    jk = (u8)((opJ << 3) | opK);
    cpu.regX[opI] = shiftLeftCircular(cpu.regX[opI] & Mask60, jk);
    }

static void cpOp21(void)
    {
    /*
    **  AXi jk
    */
    u8 jk;

    jk = (u8)((opJ << 3) | opK);
    cpu.regX[opI] = shiftRightArithmetic(cpu.regX[opI] & Mask60, jk);
    }

static void cpOp22(void)
    {
    /*
    **  LXi Bj Xk
    */
    u32 count;

    count = cpu.regB[opJ] & Mask18;
    acc60 = cpu.regX[opK] & Mask60;

    if ((count & Sign18) == 0)
        {
        count &= Mask6;
        cpu.regX[opI] = shiftLeftCircular(acc60, count);
        }
    else
        {
        count = ~count;
        count &= Mask11;
        if ((count & ~Mask6) != 0)
            {
            cpu.regX[opI] = 0;
            }
        else
            {
            cpu.regX[opI] = shiftRightArithmetic(acc60, count);
            }
        }
    }

static void cpOp23(void)
    {
    /*
    **  AXi Bj Xk
    */
    u32 count;

    count = cpu.regB[opJ] & Mask18;
    acc60 = cpu.regX[opK] & Mask60;

    if ((count & Sign18) == 0)
        {
        count &= Mask11;
        if ((count & ~Mask6) != 0)
            {
            cpu.regX[opI] = 0;
            }
        else
            {
            cpu.regX[opI] = shiftRightArithmetic(acc60, count);
            }
        }
    else
        {
        count = ~count;
        count &= Mask6;
        cpu.regX[opI] = shiftLeftCircular(acc60, count);
        }
    }

static void cpOp24(void)
    {
    /*
    **  NXi Bj Xk
    */
    cpu.regX[opI] = shiftNormalize(cpu.regX[opK], &cpu.regB[opJ], FALSE);
    }

static void cpOp25(void)
    {
    /*
    **  ZXi Bj Xk
    */
    cpu.regX[opI] = shiftNormalize(cpu.regX[opK], &cpu.regB[opJ], TRUE);
    }

static void cpOp26(void)
    {
    /*
    **  UXi Bj Xk
    */
    if (opJ == 0)
        {
        cpu.regX[opI] = shiftUnpack(cpu.regX[opK], NULL);
        }
    else
        {
        cpu.regX[opI] = shiftUnpack(cpu.regX[opK], &cpu.regB[opJ]);
        }
    }

static void cpOp27(void)
    {
    /*
    **  PXi Bj Xk
    */
    if (opJ == 0)
        {
        cpu.regX[opI] = shiftPack(cpu.regX[opK], 0);
        }
    else
        {
        cpu.regX[opI] = shiftPack(cpu.regX[opK], cpu.regB[opJ]);
        }
    }

static void cpOp30(void)
    {
    /*
    **  FXi Xj+Xk
    */
    cpu.regX[opI] = floatAdd(cpu.regX[opJ], cpu.regX[opK], FALSE, FALSE);
    }

static void cpOp31(void)
    {
    /*
    **  FXi Xj-Xk
    */
    cpu.regX[opI] = floatAdd(cpu.regX[opJ], (~cpu.regX[opK] & Mask60), FALSE, FALSE);
    }

static void cpOp32(void)
    {
    /*
    **  DXi Xj+Xk
    */
    cpu.regX[opI] = floatAdd(cpu.regX[opJ], cpu.regX[opK], FALSE, TRUE);
    }

static void cpOp33(void)
    {
    /*
    **  DXi Xj-Xk
    */
    cpu.regX[opI] = floatAdd(cpu.regX[opJ], (~cpu.regX[opK] & Mask60), FALSE, TRUE);
    }

static void cpOp34(void)
    {
    /*
    **  RXi Xj+Xk
    */
    cpu.regX[opI] = floatAdd(cpu.regX[opJ], cpu.regX[opK], TRUE, FALSE);
    }

static void cpOp35(void)
    {
    /*
    **  RXi Xj-Xk
    */
    cpu.regX[opI] = floatAdd(cpu.regX[opJ], (~cpu.regX[opK] & Mask60), TRUE, FALSE);
    }

static void cpOp36(void)
    {
    /*
    **  IXi Xj+Xk
    */
    acc60 = (cpu.regX[opJ] & Mask60) - (~cpu.regX[opK] & Mask60);
    if ((acc60 & Overflow60) != 0)
        {
        acc60 -= 1;
        }

    cpu.regX[opI] = acc60 & Mask60;
    }

static void cpOp37(void)
    {
    /*
    **  IXi Xj-Xk
    */
    acc60 = (cpu.regX[opJ] & Mask60) - (cpu.regX[opK] & Mask60);
    if ((acc60 & Overflow60) != 0)
        {
        acc60 -= 1;
        }

    cpu.regX[opI] = acc60 & Mask60;
    }

static void cpOp40(void)
    {
    /*
    **  FXi Xj*Xk
    */
    cpu.regX[opI] = floatMultiply(cpu.regX[opJ], cpu.regX[opK], FALSE, FALSE);
    }

static void cpOp41(void)
    {
    /*
    **  RXi Xj*Xk
    */
    cpu.regX[opI] = floatMultiply(cpu.regX[opJ], cpu.regX[opK], TRUE, FALSE);
    }

static void cpOp42(void)
    {
    /*
    **  DXi Xj*Xk
    */
    cpu.regX[opI] = floatMultiply(cpu.regX[opJ], cpu.regX[opK], FALSE, TRUE);
    }

static void cpOp43(void)
    {
    /*
    **  MXi jk
    */
    u8 jk;

    jk = (u8)((opJ << 3) | opK);
    cpu.regX[opI] = shiftMask(jk);
    }

static void cpOp44(void)
    {
    /*
    **  FXi Xj/Xk
    */
    cpu.regX[opI] = floatDivide(cpu.regX[opJ], cpu.regX[opK], FALSE);
    }

static void cpOp45(void)
    {
    /*
    **  RXi Xj/Xk
    */
    cpu.regX[opI] = floatDivide(cpu.regX[opJ], cpu.regX[opK], TRUE);
    }

static void cpOp46(void)
    {
    switch (opI)
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
        if (opOffset != 45)
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
    switch (opI)
        {
    case 4:
        /*
        **  Move indirect.
        */
        cpuCmuMoveIndirect();
        break;

    case 5:
        /*
        **  Move direct.
        */
        cpuCmuMoveDirect();
        break;

    case 6:
        /*
        **  Compare collated.
        */
        cpuCmuCompareCollated();
        break;

    case 7:
        /*
        **  Compare uncollated.
        */
        cpuCmuCompareUncollated();
        break;
        }
#endif
    }

static void cpOp47(void)
    {
    /*
    **  CXi Xk
    */
    u8 i;
    u8 count = 0;

    acc60 = cpu.regX[opK] & Mask60;

    for (i = 60; i > 0; i--)
        {
        count += (u8)(acc60 & 1);
        acc60 >>= 1;
        }

    cpu.regX[opI] = count;
    }

static void cpOp50(void)
    {
    /*
    **  SAi Aj+K
    */
    cpu.regA[opI] = cpuAdd18(cpu.regA[opJ], opAddress);

    cpuRegASemantics();
    }

static void cpOp51(void)
    {
    /*
    **  SAi Bj+K
    */
    cpu.regA[opI] = cpuAdd18(cpu.regB[opJ], opAddress);

    cpuRegASemantics();
    }

static void cpOp52(void)
    {
    /*
    **  SAi Xj+K
    */
    cpu.regA[opI] = cpuAdd18((u32)cpu.regX[opJ], opAddress);

    cpuRegASemantics();
    }

static void cpOp53(void)
    {
    /*
    **  SAi Xj+Bk
    */
    cpu.regA[opI] = cpuAdd18((u32)cpu.regX[opJ], cpu.regB[opK]);

    cpuRegASemantics();
    }

static void cpOp54(void)
    {
    /*
    **  SAi Aj+Bk
    */
    cpu.regA[opI] = cpuAdd18(cpu.regA[opJ], cpu.regB[opK]);

    cpuRegASemantics();
    }

static void cpOp55(void)
    {
    /*
    **  SAi Aj-Bk
    */
    cpu.regA[opI] = cpuSubtract18(cpu.regA[opJ], cpu.regB[opK]);

    cpuRegASemantics();
    }

static void cpOp56(void)
    {
    /*
    **  SAi Bj+Bk
    */
    cpu.regA[opI] = cpuAdd18(cpu.regB[opJ], cpu.regB[opK]);

    cpuRegASemantics();
    }

static void cpOp57(void)
    {
    /*
    **  SAi Bj-Bk
    */
    cpu.regA[opI] = cpuSubtract18(cpu.regB[opJ], cpu.regB[opK]);

    cpuRegASemantics();
    }

static void cpOp60(void)
    {
    /*
    **  SBi Aj+K
    */
    cpu.regB[opI] = cpuAdd18(cpu.regA[opJ], opAddress);
    }

static void cpOp61(void)
    {
    /*
    **  SBi Bj+K
    */
    cpu.regB[opI] = cpuAdd18(cpu.regB[opJ], opAddress);
    }

static void cpOp62(void)
    {
    /*
    **  SBi Xj+K
    */
    cpu.regB[opI] = cpuAdd18((u32)cpu.regX[opJ], opAddress);
    }

static void cpOp63(void)
    {
    /*
    **  SBi Xj+Bk
    */
    cpu.regB[opI] = cpuAdd18((u32)cpu.regX[opJ], cpu.regB[opK]);
    }

static void cpOp64(void)
    {
    /*
    **  SBi Aj+Bk
    */
    cpu.regB[opI] = cpuAdd18(cpu.regA[opJ], cpu.regB[opK]);
    }

static void cpOp65(void)
    {
    /*
    **  SBi Aj-Bk
    */
    cpu.regB[opI] = cpuSubtract18(cpu.regA[opJ], cpu.regB[opK]);
    }

static void cpOp66(void)
    {
    /*
    **  SBi Bj+Bk
    */
    cpu.regB[opI] = cpuAdd18(cpu.regB[opJ], cpu.regB[opK]);
    }

static void cpOp67(void)
    {
    /*
    **  SBi Bj-Bk
    */
    cpu.regB[opI] = cpuSubtract18(cpu.regB[opJ], cpu.regB[opK]);
    }

static void cpOp70(void)
    {
    /*
    **  SXi Aj+K
    */
    acc60 = (CpWord)cpuAdd18(cpu.regA[opJ], opAddress);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    cpu.regX[opI] = acc60 & Mask60;
    }

static void cpOp71(void)
    {
    /*
    **  SXi Bj+K
    */
    acc60 = (CpWord)cpuAdd18(cpu.regB[opJ], opAddress);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    cpu.regX[opI] = acc60 & Mask60;
    }

static void cpOp72(void)
    {
    /*
    **  SXi Xj+K
    */
    acc60 = (CpWord)cpuAdd18((u32)cpu.regX[opJ], opAddress);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    cpu.regX[opI] = acc60 & Mask60;
    }

static void cpOp73(void)
    {
    /*
    **  SXi Xj+Bk
    */
    acc60 = (CpWord)cpuAdd18((u32)cpu.regX[opJ], cpu.regB[opK]);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    cpu.regX[opI] = acc60 & Mask60;
    }

static void cpOp74(void)
    {
    /*
    **  SXi Aj+Bk
    */
    acc60 = (CpWord)cpuAdd18(cpu.regA[opJ], cpu.regB[opK]);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    cpu.regX[opI] = acc60 & Mask60;
    }

static void cpOp75(void)
    {
    /*
    **  SXi Aj-Bk
    */
    acc60 = (CpWord)cpuSubtract18(cpu.regA[opJ], cpu.regB[opK]);


    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    cpu.regX[opI] = acc60 & Mask60;
    }

static void cpOp76(void)
    {
    /*
    **  SXi Bj+Bk
    */
    acc60 = (CpWord)cpuAdd18(cpu.regB[opJ], cpu.regB[opK]);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    cpu.regX[opI] = acc60 & Mask60;
    }

static void cpOp77(void)
    {
    /*
    **  SXi Bj-Bk
    */
    acc60 = (CpWord)cpuSubtract18(cpu.regB[opJ], cpu.regB[opK]);

    if ((acc60 & 0400000) != 0)
        {
        acc60 |= SignExtend18To60;
        }

    cpu.regX[opI] = acc60 & Mask60;
    }

/*---------------------------  End Of File  ------------------------------*/
