/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: trace.c
**
**  Description:
**      Trace execution.
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
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/

#define MAXDATANUM      8

/*
**  PPU command adressing modes.
*/
#define AN              1
#define Amd             2
#define Ar              3
#define Ad              4
#define Adm             5

/*
**  CPU command adressing modes.
*/
#define CN              1
#define CK              2
#define Ci              3
#define Cij             4
#define CiK             5
#define CjK             6
#define Cijk            7
#define Cik             8
#define Cikj            9
#define CijK            10
#define Cmv             11
#define CLINK           100

/*
**  CPU register set markers.
*/
#define R               1
#define RAA             2
#define RAAB            3
#define RAB             4
#define RABB            5
#define RAX             6
#define RAXB            7
#define RBA             8
#define RBAB            9
#define RBB             10
#define RBBB            11
#define RBX             12
#define RBXB            13
#define RX              14
#define RXA             15
#define RXAB            16
#define RXB             17
#define RXBB            18
#define RXBX            19
#define RXX             20
#define RXXB            21
#define RXXX            22
#define RZB             23
#define RZX             24
#define RXNX            25
#define RB              26
#define RBD             27

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
typedef struct decPpControl
    {
    u8          mode;
    char        *mnemonic;
    } DecPpControl;

typedef struct decCpControl
    {
    u8          mode;
    char        *mnemonic;
    u8          regSet;
    } DecCpControl;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static bool ppuTraced (void);
static void freset (FILE **f, const char *fn);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
u32 traceMask = 0;
u8 traceCp = 0;
u32 traceClearMask = 0;
u64 chTraceMask = 0;
u32 traceCycles = 0;
FILE *devF;
FILE **cpuTF;
FILE **ppuTF;
FILE *dataTF[MAXDATANUM];

/*
**  -----------------
**  Private Variables
**  -----------------
*/
//static FILE *devF;
u32 sequence;

static const char *monState[] = 
{ "none", "cpu 0", "cpu 1" };

static const DecPpControl ppDecode[] =
    {
    AN,           "PSN",    // 00
    Amd,          "LJM",    // 01
    Amd,          "RJM",    // 02
    Ar,           "UJN",    // 03
    Ar,           "ZJN",    // 04
    Ar,           "NJN",    // 05
    Ar,           "PJN",    // 06
    Ar,           "MJN",    // 07

    Ar,           "SHN",    // 10
    Ad,           "LMN",    // 11
    Ad,           "LPN",    // 12
    Ad,           "SCN",    // 13
    Ad,           "LDN",    // 14
    Ad,           "LCN",    // 15
    Ad,           "ADN",    // 16
    Ad,           "SBN",    // 17

    Adm,          "LDC",    // 20
    Adm,          "ADC",    // 21
    Adm,          "LPC",    // 22
    Adm,          "LMC",    // 23
    AN,           "PSN",    // 24
    AN,           "PSN",    // 25
    Ad,           "EXN",    // 26
    Ad,           "RPN",    // 27

    Ad,           "LDD",    // 30
    Ad,           "ADD",    // 31
    Ad,           "SBD",    // 32
    Ad,           "LMD",    // 33
    Ad,           "STD",    // 34
    Ad,           "RAD",    // 35
    Ad,           "AOD",    // 36
    Ad,           "SOD",    // 37

    Ad,           "LDI",    // 40
    Ad,           "ADI",    // 41
    Ad,           "SBI",    // 42
    Ad,           "LMI",    // 43
    Ad,           "STI",    // 44
    Ad,           "RAI",    // 45
    Ad,           "AOI",    // 46
    Ad,           "SOI",    // 47

    Amd,          "LDM",    // 50
    Amd,          "ADM",    // 51
    Amd,          "SBM",    // 52
    Amd,          "LMM",    // 53
    Amd,          "STM",    // 54
    Amd,          "RAM",    // 55
    Amd,          "AOM",    // 56
    Amd,          "SOM",    // 57

    Ad,           "CRD",    // 60
    Amd,          "CRM",    // 61
    Ad,           "CWD",    // 62
    Amd,          "CWM",    // 63
    Amd,          "AJM",    // 64
    Amd,          "IJM",    // 65
    Amd,          "FJM",    // 66
    Amd,          "EJM",    // 67

    Ad,           "IAN",    // 70
    Amd,          "IAM",    // 71
    Ad,           "OAN",    // 72
    Amd,          "OAM",    // 73
    Ad,           "ACN",    // 74
    Ad,           "DCN",    // 75
    Ad,           "FAN",    // 76
    Amd,          "FNC"     // 77
    };

static DecCpControl rjDecode[010] =
    {
    CK,         "RJ    %6.6o",          R,      // 0
    CjK,        "RE    B%o+%6.6o",      RZB,    // 1
    CjK,        "WE    B%o+%6.6o",      RZB,    // 2
    CK,         "XJ    %6.6o",          R,      // 3
    };

static DecCpControl cjDecode[010] =
    {
    CjK,        "ZR    X%o,%6.6o",      RZX,    // 0
    CjK,        "NZ    X%o,%6.6o",      RZX,    // 1
    CjK,        "PL    X%o,%6.6o",      RZX,    // 2
    CjK,        "NG    X%o,%6.6o",      RZX,    // 3
    CjK,        "IR    X%o,%6.6o",      RZX,    // 4
    CjK,        "OR    X%o,%6.6o",      RZX,    // 5
    CjK,        "DF    X%o,%6.6o",      RZX,    // 6
    CjK,        "ID    X%o,%6.6o",      RZX,    // 7
    };

static DecCpControl noDecode[010] = 
    {
    CN,         "NO",                                   R,      // 460
    CN,         "NO",                                   R,      // 461
    CN,         "NO",                                   R,      // 462
    CN,         "NO",                                   R,      // 463
    CjK,        "IM    B%o+%6.6o",                      RBD,    // 464
    Cmv,        "DM    %3.3o,%6.6o,%2.2o,%6.6o,%2.2o",  R,      // 465
    Cmv,        "CC    %3.3o,%6.6o,%2.2o,%6.6o,%2.2o",  R,      // 466
    Cmv,        "CU    %3.3o,%6.6o,%2.2o,%6.6o,%2.2o",  R,      // 467
    };

static DecCpControl cpDecode[0100] =
    {
    CN,         "PS",                   R,      // 00
    CLINK, (char *)rjDecode,            R,      // 01
    CiK,        "JP    B%o+%6.6o",      RB,     // 02
    CLINK, (char *)cjDecode,            R,      // 03
    CijK,       "EQ    B%o,B%o,%6.6o",  RBB,    // 04
    CijK,       "NE    B%o,B%o,%6.6o",  RBB,    // 05
    CijK,       "GE    B%o,B%o,%6.6o",  RBB,    // 06
    CijK,       "LT    B%o,B%o,%6.6o",  RBB,    // 07

    Cij,        "BX%o   X%o",           RXX,    // 10
    Cijk,       "BX%o   X%o*X%o",       RXXX,   // 11
    Cijk,       "BX%o   X%o+X%o",       RXXX,   // 12
    Cijk,       "BX%o   X%o-X%o",       RXXX,   // 13
    Cik,        "BX%o   -X%o",          RXXX,   // 14
    Cikj,       "BX%o   -X%o*X%o",      RXXX,   // 15
    Cikj,       "BX%o   -X%o+X%o",      RXXX,   // 16
    Cikj,       "BX%o   -X%o-X%o",      RXXX,   // 17

    Cijk,       "LX%o   %o%o",          RX,     // 20
    Cijk,       "AX%o   %o%o",          RX,     // 21
    Cijk,       "LX%o   B%o,X%o",       RXBX,   // 22
    Cijk,       "AX%o   B%o,X%o",       RXBX,   // 23
    Cijk,       "NX%o   B%o,X%o",       RXBX,   // 24
    Cijk,       "ZX%o   B%o,X%o",       RXBX,   // 25
    Cijk,       "UX%o   B%o,X%o",       RXBX,   // 26
    Cijk,       "PX%o   B%o,X%o",       RXBX,   // 27

    Cijk,       "FX%o   X%o+X%o",       RXXX,   // 30
    Cijk,       "FX%o   X%o-X%o",       RXXX,   // 31
    Cijk,       "DX%o   X%o+X%o",       RXXX,   // 32
    Cijk,       "DX%o   X%o-X%o",       RXXX,   // 33
    Cijk,       "RX%o   X%o+X%o",       RXXX,   // 34
    Cijk,       "RX%o   X%o-X%o",       RXXX,   // 35
    Cijk,       "IX%o   X%o+X%o",       RXXX,   // 36
    Cijk,       "IX%o   X%o-X%o",       RXXX,   // 37

    Cijk,       "FX%o   X%o*X%o",       RXXX,   // 40
    Cijk,       "RX%o   X%o*X%o",       RXXX,   // 41
    Cijk,       "DX%o   X%o*X%o",       RXXX,   // 42
    Cijk,       "MX%o   %o%o",          RX,     // 43
    Cijk,       "FX%o   X%o/X%o",       RXXX,   // 44
    Cijk,       "RX%o   X%o/X%o",       RXXX,   // 45
    CLINK, (char *)noDecode,            R,      // 46
    Cik,        "CX%o   X%o",           RXNX,   // 47

    CijK,       "SA%o   A%o+%6.6o",     RAA,    // 50
    CijK,       "SA%o   B%o+%6.6o",     RAB,    // 51
    CijK,       "SA%o   X%o+%6.6o",     RAX,    // 52
    Cijk,       "SA%o   X%o+B%o",       RAXB,   // 53
    Cijk,       "SA%o   A%o+B%o",       RAAB,   // 54
    Cijk,       "SA%o   A%o-B%o",       RAAB,   // 55
    Cijk,       "SA%o   B%o+B%o",       RABB,   // 56
    Cijk,       "SA%o   B%o-B%o",       RABB,   // 57
    
    CijK,       "SB%o   A%o+%6.6o",     RBA,    // 60
    CijK,       "SB%o   B%o+%6.6o",     RBB,    // 61
    CijK,       "SB%o   X%o+%6.6o",     RBX,    // 62
    Cijk,       "SB%o   X%o+B%o",       RBXB,   // 63
    Cijk,       "SB%o   A%o+B%o",       RBAB,   // 64
    Cijk,       "SB%o   A%o-B%o",       RBAB,   // 65
    Cijk,       "SB%o   B%o+B%o",       RBBB,   // 66
    Cijk,       "SB%o   B%o-B%o",       RBBB,   // 67
    
    CijK,       "SX%o   A%o+%6.6o",     RXA,    // 70
    CijK,       "SX%o   B%o+%6.6o",     RXB,    // 71
    CijK,       "SX%o   X%o+%6.6o",     RXX,    // 72
    Cijk,       "SX%o   X%o+B%o",       RXXB,   // 73
    Cijk,       "SX%o   A%o+B%o",       RXAB,   // 74
    Cijk,       "SX%o   A%o-B%o",       RXAB,   // 75
    Cijk,       "SX%o   B%o+B%o",       RXBB,   // 76
    Cijk,       "SX%o   B%o-B%o",       RXBB,   // 77
    };

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialise execution trace.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceInit(void)
    {
    u8 i;

    cpuTF = calloc(cpuCount, sizeof(FILE *));
    if (cpuTF == NULL)
        {
        fprintf(stderr, "Failed to allocate CPU trace FILE pointers\n");
        exit(1);
        }

    ppuTF = calloc(ppuCount, sizeof(FILE *));
    if (ppuTF == NULL)
        {
        fprintf(stderr, "Failed to allocate PP trace FILE pointers\n");
        exit(1);
        }

    sequence = 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Output CPU opcode.
**
**  Parameters:     Name        Description.
**                  opFm        Opcode
**                  opI         i
**                  opJ         j
**                  opK         k
**                  opAddress   jk
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceCpu(CpuContext *activeCpu, 
              u32 p, u8 opFm, u8 opI, u8 opJ, u8 opK, u32 opAddress)
    {
    u8 addrMode;
    bool link = TRUE;
    DecCpControl *decode = cpDecode;
    static char str[80];
    int i;
    int cpuNum = activeCpu ->id;
    CpWord data;
    char traceName[20];

    /*
    **  Bail out if no trace of the current CPU is requested.
    */
    if (!(((traceMask & TraceCpu (cpuNum)) != 0) ||
          ((traceMask & TraceXj) != 0 && opFm == 01 && opI == 3) ||
          ((traceMask & TraceEcs) != 0 && opFm == 01 && (opI == 1 || opI == 2))))
        {
        return;
        }

    /*
    **  If the trace file hasn't been opened yet, open it now.
    */
    if (cpuTF[cpuNum] == NULL)
        {
        sprintf(traceName, "cpu%d.trc", cpuNum);
        cpuTF[cpuNum] = fopen(traceName, "wt");
        if (cpuTF[cpuNum] == NULL)
            {
            logError(LogErrorLocation, "can't open cpu%d trace (%s)\n",
                     cpuNum, traceName);
            return;
            }
        }

    /*
    **  If trace cycle count set, count it down, cancel further
    **  tracing if count expired.
    */
    if (traceCycles != 0 && --traceCycles == 0)
        {
        for (i = 0; i < cpuCount; i++)
            {
            traceMask &= ~TraceCpu (i);
            fflush(cpuTF[i]);
            }
        }
    
#if 0
    for (i = 0; i < 8; i++)
        {
        data = activeCpu->regX[i];
        fprintf(cpuTF[cpuNum], "        A%d %06.6o  X%d %04.4o %04.4o %04.4o %04.4o %04.4o   B%d %06.6o\n",
            i, activeCpu->regA[i], i,
            (PpWord)((data >> 48) & Mask12),
            (PpWord)((data >> 36) & Mask12),
            (PpWord)((data >> 24) & Mask12),
            (PpWord)((data >> 12) & Mask12),
            (PpWord)((data      ) & Mask12),
            i, activeCpu->regB[i]);
        }
#endif

    fprintf(cpuTF[cpuNum], "%06d [%d] %6.6o  ", sequence, cpuNum, p);
    fprintf(cpuTF[cpuNum], "%02o%o%o%o   ", opFm, opI, opJ, opK);        // << not quite correct, but still nice for debugging

    /*
    **  Print opcode.
    */
    addrMode = decode[opFm].mode;

    while (link)
        {
        link = FALSE;

        switch (addrMode)
            {
        case CN:
            sprintf(str, decode[opFm].mnemonic);
            break;

        case CK:
            sprintf(str, decode[opFm].mnemonic, opAddress);
            break;

        case Ci:
            sprintf(str, decode[opFm].mnemonic, opI);
            break;

        case Cij:
            sprintf(str, decode[opFm].mnemonic, opI, opJ);
            break;

        case CiK:
            sprintf(str, decode[opFm].mnemonic, opI, opAddress);
            break;

        case CjK:
            sprintf(str, decode[opFm].mnemonic, opJ, opAddress);
            break;

        case Cijk:
            sprintf(str, decode[opFm].mnemonic, opI, opJ, opK);
            break;

        case Cik:
            sprintf(str, decode[opFm].mnemonic, opI, opK);
            break;

        case Cikj:
            sprintf(str, decode[opFm].mnemonic, opI, opK, opJ);
            break;

        case CijK:
            sprintf(str, decode[opFm].mnemonic, opI, opJ, opAddress);
            break;

        case Cmv:
            data = cpMem[p];
            sprintf(str, decode[opFm].mnemonic,
                    (opJ << 4) + ((data >> 26) & 017),
                    opAddress,
                    (data >> 22) & 017,
                    data & Mask18,
                    (data >> 18) & 017);
            break;

        case CLINK:
            decode = (DecCpControl *)decode[opFm].mnemonic;
            opFm = opI;
            addrMode = decode[opFm].mode;
            link = TRUE;
            break;

        default:
            sprintf(str,"unsupported mode %02o", opFm);
            break;
            }
        }

    fprintf(cpuTF[cpuNum], "%-30s", str);

    /*
    **  Dump relevant register set.
    */
    switch (decode[opFm].regSet)
        {
    case R:
        break;

    case RAA:
        fprintf(cpuTF[cpuNum], "A%d=%06o    ", opI, activeCpu->regA[opI]);
        fprintf(cpuTF[cpuNum], "A%d=%06o    ", opJ, activeCpu->regA[opJ]);
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        break;

    case RAAB:
        fprintf(cpuTF[cpuNum], "A%d=%06o    ", opI, activeCpu->regA[opI]);
        fprintf(cpuTF[cpuNum], "A%d=%06o    ", opJ, activeCpu->regA[opJ]);
        if (opK != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opK, activeCpu->regB[opK]);
            }
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        break;

    case RAB:
        fprintf(cpuTF[cpuNum], "A%d=%06o    ", opI, activeCpu->regA[opI]);
        if (opJ != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opJ, activeCpu->regB[opJ]);
            }
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        break;

    case RABB:
        fprintf(cpuTF[cpuNum], "A%d=%06o    ", opI, activeCpu->regA[opI]);
        if (opJ != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opJ, activeCpu->regB[opJ]);
            }
        if (opK != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opK, activeCpu->regB[opK]);
            }
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        break;

    case RAX:
        fprintf(cpuTF[cpuNum], "A%d=%06o    ", opI, activeCpu->regA[opI]);
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opJ, activeCpu->regX[opJ]);
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        break;

    case RAXB:
        fprintf(cpuTF[cpuNum], "A%d=%06o    ", opI, activeCpu->regA[opI]);
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opJ, activeCpu->regX[opJ]);
        if (opK != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opK, activeCpu->regB[opK]);
            }
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        break;

    case RB:
        if (opI != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opI, activeCpu->regB[opI]);
            }
        break;

    case RBA:
        if (opI != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opI, activeCpu->regB[opI]);
            }
        fprintf(cpuTF[cpuNum], "A%d=%06o    ", opJ, activeCpu->regA[opJ]);
        break;

    case RBAB:
        if (opI != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opI, activeCpu->regB[opI]);
            }
        fprintf(cpuTF[cpuNum], "A%d=%06o    ", opJ, activeCpu->regA[opJ]);
        if (opK != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opK, activeCpu->regB[opK]);
            }
        break;

    case RBB:
        if (opI != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opI, activeCpu->regB[opI]);
            }
        if (opJ != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opJ, activeCpu->regB[opJ]);
            }
        break;

    case RBBB:
        if (opI != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opI, activeCpu->regB[opI]);
            }
        if (opJ != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opJ, activeCpu->regB[opJ]);
            }
        if (opK != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opK, activeCpu->regB[opK]);
            }
        break;

    case RBD:
        if (opJ != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opJ, activeCpu->regB[opJ]);
            }
        data = cpMem[activeCpu->regB[opJ] + opAddress + activeCpu->regRaCm];
        fprintf(cpuTF[cpuNum], "MD=%5.5o,%6.6o,%2.2o,%6.6o,%2.2o    ",
                ((data >> 44) & 017760) + ((data >> 26) & 017),
                (data >> 30) & Mask18,
                (data >> 22) & 017,
                data & Mask18,
                (data >> 18) & 017);
        break;

    case RBX:
        if (opI != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opI, activeCpu->regB[opI]);
            }
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opJ, activeCpu->regX[opJ]);
        break;

    case RBXB:
        if (opI != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opI, activeCpu->regB[opI]);
            }
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opJ, activeCpu->regX[opJ]);
        if (opK != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opK, activeCpu->regB[opK]);
            }
        break;

    case RX:
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        break;

    case RXA: 
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        fprintf(cpuTF[cpuNum], "A%d=%06o    ", opJ, activeCpu->regA[opJ]);
        break;

    case RXAB:
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        fprintf(cpuTF[cpuNum], "A%d=%06o    ", opJ, activeCpu->regA[opJ]);
        if (opK != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opK, activeCpu->regB[opK]);
            }
        break;

    case RXB:
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        if (opJ != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opJ, activeCpu->regB[opJ]);
            }
        break;

    case RXBB:
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        if (opJ != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opJ, activeCpu->regB[opJ]);
            }
        if (opK != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opK, activeCpu->regB[opK]);
            }
        break;

    case RXBX:
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        if (opJ != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opJ, activeCpu->regB[opJ]);
            }
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opK, activeCpu->regX[opK]);
        break;

    case RXX:
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opJ, activeCpu->regX[opJ]);
        break;

    case RXXB:
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opJ, activeCpu->regX[opJ]);
        if (opK != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opK, activeCpu->regB[opK]);
            }
        break;

    case RXXX:
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opJ, activeCpu->regX[opJ]);
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opK, activeCpu->regX[opK]);
        break;

    case RZB:
        if (opJ != 0)
            {
            fprintf(cpuTF[cpuNum], "B%d=%06o    ", opJ, activeCpu->regB[opJ]);
            }
        break;

    case RZX:
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opJ, activeCpu->regX[opJ]);
        break;

    case RXNX:
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opI, activeCpu->regX[opI]);
        fprintf(cpuTF[cpuNum], "X%d=" FMT60_020o "   ", opK, activeCpu->regX[opK]);
        break;

    default:
        fprintf(cpuTF[cpuNum],"unsupported register set %d", decode[opFm].regSet);
        break;
        }

    fprintf(cpuTF[cpuNum], "\n");
    if (decode == rjDecode && (opI == 1 || opI == 2))
        {
        // ECS read/write, show what memory was touched
        fprintf(cpuTF[cpuNum], " ECS FWA %08llo RAX %08o FLX %08o\n", 
                activeCpu->regX[0] & 077777777,
                activeCpu->regRaEcs, activeCpu->regFlEcs);
        dumpCpuMem (cpuTF[cpuNum], activeCpu->regA[0], activeCpu->regA[0] + activeCpu->regB[opJ] + opAddress);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Trace a exchange jump.
**
**  Parameters:     Name        Description.
**                  cc          CPU context pointer
**                  addr        Address of exchange package
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceExchange(CpuContext *cc, u32 addr, char *title)
{
    CpWord data;
    u8 i;
    int cpn = cc->id;
    char traceName[20];
    
    /*
    **  Bail out if no trace of exchange jumps is requested.
    */
    if ((traceMask & TraceXj) == 0)
        {
        return;
        }

    /*
    **  If the trace file hasn't been opened yet, open it now.
    */
    if (cpuTF[cpn] == NULL)
        {
        sprintf(traceName, "cpu%d.trc", cpn);
        cpuTF[cpn] = fopen(traceName, "wt");
        if (cpuTF[cpn] == NULL)
            {
            logError(LogErrorLocation, "can't open cpu%d trace (%s)\n",
                     cpn, traceName);
            return;
            }
        }

    fprintf(cpuTF[cpn], "\n%6d: Exchange jump CPU %d with package address %06o (%s)\n\n", sequence, cpn, addr, title);
    fprintf(cpuTF[cpn], "P       %06o  ", cc->regP);
    fprintf(cpuTF[cpn], "A%d %06o  ", 0, cc->regA[0]);
    fprintf(cpuTF[cpn], "B%d %06o", 0, cc->regB[0]);
    fprintf(cpuTF[cpn], "\n");
                           
    fprintf(cpuTF[cpn], "RAcm    %06o  ", cc->regRaCm);
    fprintf(cpuTF[cpn], "A%d %06o  ", 1, cc->regA[1]);
    fprintf(cpuTF[cpn], "B%d %06o", 1, cc->regB[1]);
    fprintf(cpuTF[cpn], "\n");
                           
    fprintf(cpuTF[cpn], "FLcm    %06o  ", cc->regFlCm);
    fprintf(cpuTF[cpn], "A%d %06o  ", 2, cc->regA[2]);
    fprintf(cpuTF[cpn], "B%d %06o", 2, cc->regB[2]);
    fprintf(cpuTF[cpn], "\n");
                           
    fprintf(cpuTF[cpn], "RAecs %08o  ", cc->regRaEcs);
    fprintf(cpuTF[cpn], "A%d %06o  ", 3, cc->regA[3]);
    fprintf(cpuTF[cpn], "B%d %06o", 3, cc->regB[3]);
    fprintf(cpuTF[cpn], "\n");
                           
    fprintf(cpuTF[cpn], "FLecs %08o  ", cc->regFlEcs);
    fprintf(cpuTF[cpn], "A%d %06o  ", 4, cc->regA[4]);
    fprintf(cpuTF[cpn], "B%d %06o", 4, cc->regB[4]);
    fprintf(cpuTF[cpn], "\n");
                           
    fprintf(cpuTF[cpn], "EM    %08o  ", cc->exitMode);
    fprintf(cpuTF[cpn], "A%d %06o  ", 5, cc->regA[5]);
    fprintf(cpuTF[cpn], "B%d %06o", 5, cc->regB[5]);
    fprintf(cpuTF[cpn], "\n");
                           
    fprintf(cpuTF[cpn], "MA      %06.6o  ", cc->regMa); 
    fprintf(cpuTF[cpn], "A%d %06o  ", 6, cc->regA[6]);
    fprintf(cpuTF[cpn], "B%d %06o", 6, cc->regB[6]);
    fprintf(cpuTF[cpn], "\n");
                           
    fprintf(cpuTF[cpn], "STOP         %d  ", cc->cpuStopped ? 1 : 0);
    fprintf(cpuTF[cpn], "A%d %06o  ", 7, cc->regA[7]);
    fprintf(cpuTF[cpn], "B%d %06o  ", 7, cc->regB[7]);
    fprintf(cpuTF[cpn], "\n");
    fprintf(cpuTF[cpn], "ECOND       %02o  ", cc->exitCondition);
    fprintf(cpuTF[cpn], "\n");
    fprintf(cpuTF[cpn], "\n");

    for (i = 0; i < 8; i++)
        {
        fprintf(cpuTF[cpn], "X%d ", i);
        data = cc->regX[i];
        fprintf(cpuTF[cpn], "%04o %04o %04o %04o %04o   ",
            (PpWord)((data >> 48) & Mask12),
            (PpWord)((data >> 36) & Mask12),
            (PpWord)((data >> 24) & Mask12),
            (PpWord)((data >> 12) & Mask12),
            (PpWord)((data      ) & Mask12));
        fprintf(cpuTF[cpn], "\n");
        }

    fprintf(cpuTF[cpn], "\nMonitor mode: %s\n", monState[monitorCpu + 1]);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Finish execution trace.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceFinish(void)
    {
    u8 i;

    if (devF != NULL)
        {
        fclose (devF);
        devF = NULL;
        }
    
    for (i = 0; i < cpuCount; i++)
        {
        if (cpuTF[i] != NULL)
            {
            fclose(cpuTF[i]);
            cpuTF[i] = NULL;
            }
        }

    for (i = 0; i < ppuCount; i++)
        {
        if (ppuTF[i] != NULL)
            {
            fclose(ppuTF[i]);
            ppuTF[i] = NULL;
            }
        }

    for (i = 0; i < MAXDATANUM; i++)
        {
        if (dataTF[i] != NULL)
            {
            fclose(dataTF[i]);
            dataTF[i] = NULL;
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Stop execution trace for now.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceStop(void)
    {
    u8 i;

    for (i = 0; i < cpuCount; i++)
        {
        if (cpuTF[i] != NULL)
            {
            fflush(cpuTF[i]);
            }
        }

    if (devF != NULL)
        {
        fflush(devF);
        }
    
    for (i = 0; i < ppuCount; i++)
        {
        if (ppuTF[i] != NULL)
            {
            fflush(ppuTF[i]);
            }
        }

    for (i = 0; i < MAXDATANUM; i++)
        {
        if (dataTF[i] != NULL)
            {
            fflush (dataTF[i]);
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Reset trace data (truncate any trace files to 0 bytes)
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceReset(void)
    {
    u8 i;
    char traceName[20];

    for (i = 0; i < cpuCount; i++)
        {
        if (cpuTF[i] != NULL)
            {
            sprintf(traceName, "cpu%d.trc", i);
            freset (&cpuTF[i], traceName);
            }
        }

    if (devF != NULL)
        {
        freset (&devF, "device.trc");
        }
    
    for (i = 0; i < ppuCount; i++)
        {
        if (ppuTF[i] != NULL)
            {
            sprintf(traceName, "ppu%02o.trc", i);
            freset (&ppuTF[i], traceName);
            }
        }

    for (i = 0; i < MAXDATANUM; i++)
        {
        if (dataTF[i] != NULL)
            {
            sprintf(traceName, "data%d.trc", i);
            freset (&dataTF[i], traceName);
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Output sequence number.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceSequence(void)
    {
    char traceName[20];

    /*
    **  Increment sequence number here.
    */
    sequence += 1;

    /*
    **  Bail out if no trace of this PPU is requested.
    */
    activePpu->traceLine = ppuTraced();
    if (!activePpu->traceLine)
        {
        return;
        }

    /*
    **  If the trace file hasn't been opened yet, open it now.
    */
    if (ppuTF[activePpu->id] == NULL)
        {
        sprintf(traceName, "ppu%02o.trc", activePpu->id);
        ppuTF[activePpu->id] = fopen(traceName, "wt");
        if (ppuTF[activePpu->id] == NULL)
            {
            logError(LogErrorLocation, "can't open ppu[%02o] trace (%s)\n",
                     activePpu->id, traceName);
            return;
            }
        }

    /*
    **  Print sequence no and PPU number.
    */
    fprintf(ppuTF[activePpu->id], "%06d [%2o]    ", sequence, activePpu->id);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Output registers.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceRegisters(void)
    {
    /*
    **  Bail out if no trace of this PPU is requested.
    */
    if (!activePpu->traceLine)
        {
        return;
        }

    /*
    **  Print registers.
    */
    fprintf(ppuTF[activePpu->id], "P:%04o  ", activePpu->regP);
    fprintf(ppuTF[activePpu->id], "A:%06o", activePpu->regA);
    fprintf(ppuTF[activePpu->id], "    ");
    }

/*--------------------------------------------------------------------------
**  Purpose:        Output opcode.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceOpcode(void)
    {
    PpWord opCode;
    u8 addrMode;
    u8 opF;
    u8 opD;
    
    /*
    **  Bail out if no trace of this PPU is requested.
    */
    if (!activePpu->traceLine)
        {
        return;
        }

    /*
    **  Print opcode.
    */
    opCode = activePpu->mem[activePpu->regP];
    opF = opCode >> 6;
    opD = opCode & 077;
    addrMode = ppDecode[opF].mode;

    fprintf(ppuTF[activePpu->id], "O:%04o   %3.3s ", opCode, ppDecode[opF].mnemonic);

    switch (addrMode)
        {
    case AN:
        fprintf(ppuTF[activePpu->id], "        ");
        break;

    case Amd:
        fprintf(ppuTF[activePpu->id], "%04o,%02o ", activePpu->mem[activePpu->regP + 1], opD);
        break;

    case Ar:
        if (opD < 040)
            {
            fprintf(ppuTF[activePpu->id], "+%02o     ", opD);
            }
        else
            {
            fprintf(ppuTF[activePpu->id], "-%02o     ", 077 - opD);
            }
        break;

    case Ad:
        fprintf(ppuTF[activePpu->id], "%02o      ", opD);
        break;

    case Adm:
        fprintf(ppuTF[activePpu->id], "%02o%04o  ", opD, activePpu->mem[activePpu->regP + 1]);
        break;
        }

    fprintf(ppuTF[activePpu->id], "    ");
    }

/*--------------------------------------------------------------------------
**  Purpose:        Output opcode.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
u8 traceDisassembleOpcode(char *str, PpWord *pm)
    {
    u8 result = 1;
    PpWord opCode;
    u8 addrMode;
    u8 opF;
    u8 opD;
    
    /*
    **  Print opcode.
    */
    opCode = *pm++;
    opF = opCode >> 6;
    opD = opCode & 077;
    addrMode = ppDecode[opF].mode;

    str += sprintf(str, "%3.3s  ", ppDecode[opF].mnemonic);

    switch (addrMode)
        {
    case AN:
        sprintf(str, "        ");
        break;

    case Amd:
        sprintf(str, "%04o,%02o ", *pm, opD);
        result = 2;
        break;

    case Ar:
        if (opD < 040)
            {
            sprintf(str, "+%02o     ", opD);
            }
        else
            {
            sprintf(str, "-%02o     ", 077 - opD);
            }
        break;

    case Ad:
        sprintf(str, "%02o      ", opD);
        break;

    case Adm:
        sprintf(str, "%02o%04o  ", opD, *pm);
        result = 2;
        break;
        }

    return(result);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Output channel unclaimed function info.
**
**  Parameters:     Name        Description.
**                  funcCode    Function code.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceChannelFunction(PpWord funcCode)
    {
    if (devF == NULL)
        {
        devF = fopen("device.trc", "wt");
        if (devF == NULL)
            {
            logError(LogErrorLocation, "can't open dev trace");
            }
        return;
        }
    
    fprintf(devF, "%06d [%02o]    ", sequence, activePpu->id);
    fprintf(devF, "Unclaimed function code %04o on CH%02o\n", funcCode, activeChannel->id);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Output string for PPU.
**
**  Parameters:     Name        Description.
**                  str         String to output.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void tracePrint(char *str)
    {
    char traceName[20];

    /*
    **  If the trace file hasn't been opened yet, open it now.
    */
    if (ppuTF[activePpu->id] == NULL)
        {
        sprintf(traceName, "ppu%02o.trc", activePpu->id);
        ppuTF[activePpu->id] = fopen(traceName, "wt");
        if (ppuTF[activePpu->id] == NULL)
            {
            logError(LogErrorLocation, "can't open ppu[%02o] trace (%s)\n",
                     activePpu->id, traceName);
            return;
            }
        }

    fputs(str, ppuTF[activePpu->id]);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Output string for CPU.
**
**  Parameters:     Name        Description.
**                  str         String to output.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceCpuPrint(CpuContext *activeCpu, char *str)
    {
    int cpuNum = activeCpu->id;
    char traceName[20];
    
    /*
    **  If the trace file hasn't been opened yet, open it now.
    */
    if (cpuTF[cpuNum] == NULL)
        {
        sprintf(traceName, "cpu%d.trc", cpuNum);
        cpuTF[cpuNum] = fopen(traceName, "wt");
        if (cpuTF[cpuNum] == NULL)
            {
            logError(LogErrorLocation, "can't open cpu%d trace (%s)\n",
                     cpuNum, traceName);
            return;
            }
        }
    fputs(str, cpuTF[cpuNum]);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Output status of channel.
**
**  Parameters:     Name        Description.
**                  ch          channel number.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceChannel(u8 ch)
    {
    /*
    **  Bail out if no trace of this PPU is requested.
    */
    if (!activePpu->traceLine)
        {
        return;
        }

    fprintf(ppuTF[activePpu->id], "  CH:%c%c", channel[ch].active ? 'A' : 'D', channel[ch].full ? 'F' : 'E');
    }

/*--------------------------------------------------------------------------
**  Purpose:        Output CM touched by PPU CM read/write.
**
**  Parameters:     Name        Description.
**                  start       CM start address.
**                  len         CM area length.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceCM(u32 start, u32 len)
    {
    /*
    **  Bail out if no trace of this PPU is requested.
    **  This one only tests the PPU trace mask because it 
    **  cannot apply in the case of an I/O instruction.
    */
    if ((traceMask & (1 << activePpu->id)) == 0)
        {
        return;
        }
    // Note that this goes to the PPU trace file, not the CPU trace file!
    dumpCpuMem (ppuTF[activePpu->id], start, start + len);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Output PPU memory touched by PPU block I/O.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void tracePM(void)
    {
    /*
    **  Bail out if no trace of this PPU is requested.
    */
    if (!activePpu->traceLine)
        {
        return;
        }

    if (activePpu->ppMemLen > 0)
        {
        dumpPpuMem (ppuTF[activePpu->id],
                    activePpu->id,
                    activePpu->ppMemStart, 
                    activePpu->ppMemStart + activePpu->ppMemLen);
        }
    }


/*--------------------------------------------------------------------------
**  Purpose:        Output end-of-line.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceEnd(void)
    {
    /*
    **  Bail out if no trace of this PPU is requested.
    */
    if (!activePpu->traceLine)
        {
        return;
        }

    /*
    **  Print newline.
    */
    fprintf(ppuTF[activePpu->id], "\n");

    /*
    ** See if we just jumped to PPR and if so, whether it's time to turn off trace
    */
    if (activePpu->regP == 0111 &&
        (traceClearMask & (1 << activePpu->id)) != 0)
        {
        traceMask &= ~(1 << activePpu->id);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Write data from X register to a data trace file.
**
**  Parameters:     Name        Description.
**                  data        60-bit value
**                  stream      file number (0..7)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void traceData (CpWord data, int stream)
    {
    char traceName[20];
    
    if (stream >= MAXDATANUM)
        {
        return;
        }
    
    if (dataTF[stream] == NULL)
        {
        sprintf(traceName, "data%d.trc", stream);
        dataTF[stream] = fopen(traceName, "wt");
        if (dataTF[stream] == NULL)
            {
            logError(LogErrorLocation, "can't open data stream %d trace (%s)\n",
                     stream, traceName);
            return;
            }
        }
    
    fprintf(dataTF[stream], "%06d %20.20llo\n", sequence, data);
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Reset (truncate to 0) a file
**
**  Parameters:     Name        Description.
**                  file        Address of file descriptor pointer.
**                  fn          File name (needed only for Win32).
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
static void freset (FILE **f, const char *fn)
    {
    if (ftell (*f) != 0)
        {
#if defined(_WIN32)
        // Win32 doesn't have "ftruncate" -- so much for Posix...
        // So close and re-open the file instead.
        fclose (*f);
        *f = fopen (fn, "wt");
        if (*f == NULL)
            {
            fprintf (stderr, "error reopening %s\n", fn);
            }
#else
        rewind (*f);
        ftruncate (fileno (*f), 0);
#endif
        }
    }
    

/*--------------------------------------------------------------------------
**  Purpose:        Test if we want to trace the current PPU
**
**  Parameters:     Name        Description.
**                  None.
**
**  Returns:        TRUE if yes
**
**------------------------------------------------------------------------*/
static bool ppuTraced(void)
    {
    PpWord opCode;
    u8 id = activePpu->id;
    u8 opF, opD;
    
    // If the PPU is traced, the answer is yes
    if (traceMask & (1 << id))
        {
        return TRUE;
        }

    // Otherwise, see if the next instruction is an I/O instruction
    // and references a channel that's being traced.
    // Note that we don't trace ACN/DCN and the status test branches.
    // If we're tracing exchanges and the next instruction is an exchange,
    // the answer is also yes.
    // Finally, if we're tracing ECS, and the instruction is an I/O instruction
    // for a DDP channel, the answer is yes.
    opCode = activePpu->mem[activePpu->regP];
    opF = opCode >> 6;
    opD = opCode & 037;                     // ignore NoHang bit
    if ((traceMask & TraceXj) != 0 && opF == 026)
        {
        return TRUE;
        }
    else if ((opF >= 070 && opF <= 073) ||  // IAN, IAM, OAN, OAM
        (opF >= 076))                       // FAN, FNC
        {
        if (chTraceMask & (ULL(1) << opD))
            {
            return TRUE;
            }
        if ((traceMask & TraceEcs) != 0 &&
            opD < channelCount &&
            channel[opD].firstDevice != NULL &&
            channel[opD].firstDevice->devType == DtDdp)
            {
            return TRUE;
            }
        }
    return FALSE;
    }

/*---------------------------  End Of File  ------------------------------*/

