/*--------------------------------------------------------------------------
**
**  Copyright (c) 2009, Paul Koning (see license.txt)
**
**  Name: plato.h
**
**  Description:
**      Definitions of PLATO structures and values
**
**--------------------------------------------------------------------------
*/

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define PNII    016     // ECS offset to PNI initialization pointer
#define MXNAM   3       // Number of PNI instances supported
#define NKEYSHF 3       // Shift for key buffer size
#define OFFKY2  01753   // Station backout key
#define F_PTS   04000   // Permission to Send
#define F_ABT   06000   // Output abort
#define NKEYLTH (1 << NKEYSHF)

#define CKCPIDX (NKEYLTH - 1)       // word offset to word with CP index
#define CKPPIDX (NKEYLTH - 2)       // word offset to word with "PP" index
#define IDXLIM  ((NKEYLTH * 5) - 6) // max char index

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
struct Io
{
    CpWord in;
    CpWord out;
};

struct Keybuf
{
    CpWord buf[CKCPIDX];    // keys (12 bits each) and "PP" index
    CpWord cpidx;           // 36/0, 12/stop1, 12/CP index
};

struct PniSiteData
{
    CpWord fod;         // Framat output buffer data (18/len, 21/bufptr, 21/inoutptr)
    CpWord frb;         // Framat request buffer data (18/len, 21/bufptr, 21/inoutptr)
    CpWord site;        // First site and number of sites (36.0, 12/first, 12/sites)
};

// This collection of pointers lives in ECS; a pointer to it lives 
// in ECS at offset PNII.
struct PniPtr
{
    CpWord  fill[2];    // unused
    CpWord  akb;        // Address of key buffers
    CpWord  ast;        // Address of station banks
    CpWord  ail;        // address of inhibit bit list
    CpWord  aasccon;    // ASCII connection information
    CpWord  abt;        // Address of process bit list
    CpWord  namto;      // Timeout at Press NEXT to begin
    CpWord  nampd;      // Timeout for PNI to drop PLATO
    CpWord  fill2[7];   // unused
    struct PniSiteData sd[MXNAM];
};

