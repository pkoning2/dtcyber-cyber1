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

// Station bank
struct stbank
{
    CpWord cmbank;      // framer output pointers                     
                        //         ist  12 bits = error correction / queue info
                        //                 1st   1 bits = error correction in process
                        //                 next  1      = in cont output queue
                        //                 next  1      = erase abort (doelz net)
                        //                 next  1      = drop parcels flag (signoff)
                        //                 next  1      = plato waiting for minimum pend parcels
                        //                 next  4      = unused
                        //                 next  3      = error correction retry cnt.
                        //         next 12      = terminal word count
                        //         next 12      = 1st parcel number
                        //         next 12      = output parcel number
                        //         next 12      = output parcel word count
    CpWord cmbank1;     // 
                        //         ist  12 bits = number parcels
                        //         next 01      = fast/long framer queue routing bit
                        //                        = 0 = current output in fast queue
                        //                        = 1 = current output in long queue
                        //         next 05      = number framer requests pending
                        //         next 06      = unused
                        //         next 12      = output error count
                        //         next 12      = fill parcel number
                        //         next 12      = fill parcel word count
    CpWord tfrmpnt;     // last frame for this station                
                        //
                        // fsysrln  equ    *-cmbank    formatter read only length
                        //
                        //
    CpWord tmode;       // terminal mode 00=dot 10=line 20=mem 30=cha 
                        //         also, o77 = undefined mode
    CpWord wexfunc;     // we and external functions                  
                        // (wexfunc initially says -mode write-, enable external)
    CpWord colors;      // white on black                             
    CpWord altmem;      // alternate char memory switch  0=norm 1=alt 
    CpWord fullscr;     // minus one if a full screen erase occurred  
    CpWord margins;     // left margin = 0, right margin = 511        
    CpWord xxxmem;      // current x and memory            
    CpWord yyy;         // current y                       
    CpWord ignrers;     // ignor reset                                
    CpWord dangle;      // non-zero if dangling 77b                   
                        //
                        // poutlng  equ    *-tmode     length of terminal parameters
                        //
    CpWord bckflg;      // normally forward                
                        //
    CpWord fstflgs;     // formatter read/write flag bits             
                        //
                        //         the following bits are used by the enable/disable
                        //         commands
                        //
                        // enablbs  equ    60          bit 59  dont use it
                        //
                        //         bits 58 - 56       not used
                        //
                        // enablo   equ    56          bit 57 - set by enable oriental
                        // enablt   equ    55          bit 54 - set by enable  touch
                        // enablx   equ    54          bit 53 - set by enable  ext
                        // enablpt  equ    53          bit 52 - set by pause key=touch
                        // enablpx  equ    52          bit 51 - set by pause key=exts
                        // enabltu  equ    51          bit 50 - if a lesson has a
                        //                            xmit command in a tunit
                        // sizbold  equ    50          bit 49 - if -size bold-

                        // enabst   equ    49          bit 48 - set by enable stream
                        //
                        // abtoput  equ    48          bit 47 - abort output (1=yes)
                        // fsedone  equ    47          bit 46 - full screen erase done
                        //
                        //         bits 43-45         unused.

                        //         bits 36-42         error word count.

                        // ewcbts   equ    43          error word ccount high bit + 1
                        // ewcbtn   equ    7           error word count bits

                        //
                        //         bits 35 - 18';  terminal is presently initialized
                        //                to this station bank
                        //
                        //         bits 17 - 00       reserved for tstmod and ldm
                        //
    CpWord outmout;     // total number of mouts formated             
    CpWord formtim;     // milliseconds formatting time               
    CpWord cset;        // wrap flag/overlay               
                        //
                        // psysrln  equ    *-cmbank    plato read only length
                        //
    CpWord stflags;     //         used for various flags.

                        //         if any of the top 9 bits are set, normal key
                        //         processing is inhibited for this station.

                        //                            bit 59-set when station has
                        //                                   old disk i/o pending.
                        // note that these shift counts are for a ',mx  1',, ',lx  shift',
                        // a bit is moved to the sign bit for testing by ',lx  60-symbol',
                        // ssbbit   equ    59          bit 58 marks station backout
                        // ptdbit   equ    ssbbit-1    bit 57 set when -pdpstart- key
                        //                                   has been received while
                        //                                   terminal signed in--
                        //                                   special backout bit for
                        //                                   programmable terminals.
                        // offbit   equ    ptdbit-1    bit 56 set when sign-off key
                        //                                   has been received.
                        // typbit   equ    offbit-1    bit 55 set when echo sent to
                        //                                   terminal in order to
                        //                                   determine terminal type
                        // clrbit   equ    typbit-1    bit 54 set when clear key
                        //                                   received from signed out
                        //                                   terminal.

                        // misbit   equ    clrbit-1    bit 53 marks mist query in progress
                        // sscbit   equ    misbit-1    bit 52 set when ssb complete
                        // sslbit   equ    sscbit-1    bit 51 set to mark log-out process

                        // oribit   equ    51          bit 50 - oriental module loaded

                        // misix1   equ    50          bits 48-49 - mist query index
                        // misix0   equ    misix1-1

                        // tpfqs    equ    48          bit  47 = plato-format que num
                        //                            = 0 = last output in fast que
                        //                            = 1 = last output in long que
                        // istlbs   equ    41          bits 40-46 = ist-2 down-loading
                        //                            bit  39 0=ext inputs allowed,
                        //                                    1=ignore ext inputs.
                        // st1bit   equ    39          bit 38  1=stop1 key pressed and lesson
                        //                                         processing should be ended
                        //                                    0=no stop1 yet, normal processing
                        //                            bits 20-37 indicate -echoto-
                        //                                   station number.
                        // finbit   equ    20          bit 19 set while in finish unit
                        //         the following flag is set to mark that the system
                        //         (or a site lesson) has pressed stop1 on a station.
                        //         this information is used to set the backout flag
                        //         --it is cleared on entry to the next non-router
                        //         lesson.
                        // ss1bit   equ    19          bit 18  1=system stop1 pressed on station
                        //                            bits 0-17 - station number for
                        //                                        this station.
                        //
    CpWord stflag1;     // must immediately follow stflags            
                        // asmpbit  equ    60          bit 59 - assembly language
                        //                            program loaded into terminal
                        //
                        // extmbit  equ    59          bit 58 - *extmap* selected
                        //
                        // mtnobit  equ    58          bit 57 - set to force re-load
                        //                            of ppt-tutor interpreter
                        // reskbit  equ    57          bit 56 - set on -reset- key
                        // newkbit  equ    56          bit 55 - set to enable ',new',-
                        //                            type key processing
                        // pst1bit  equ    55          bit 54 - priority stop1 (exit even
                        //                            if in stop1-protected lesson)
                        // adelbit  equ    54          bit 53 - set if author deletion
                        //
                        // etedf    equ    53          enhanced error correction flag, bit 52

                        // brflags  equ    52          backout request flag, bit 51.

                        // tekbit   equ    51          ascii ist in tektronix mode
                        //
                        // langbtn  equ    3           number bits in language field
                        // langbts  equ    50          bits 49,48,47 hold language num
                        //         (absolute shift value 50 used in lesson plato)
                        //         0=english, 1=french, 7=user chooses
                        //
                        //
                        //         bits 29-46 = fwa of charset area in terminal ram
                        //         bits 11-28 = fwa of terminal resident ram area
                        //         bits 08-10 = terminal memory configuration code
                        //                      ascii terminal type

                        // tcbtn    equ    3           number of configuration bits
                        // tcbts    equ    11          high order configuration bit + 1

                        //         bits 04-07 = terminal type code

                        // ttbtn    equ    4           number of terminal type bits
                        // ttbts    equ    8           high order terminal bit + 1

                        //         bits 00-03 = ppt-tutor version number
    CpWord slastky;     // time last key processed for                
                        //                            this station.
    CpWord bankadd;     // bits 0-17 - relative address               
                        //                                        of student bank.

                        //                            bits 18-24 resident load file number

                        // lfbtn    equ    7           number of load file bits
                        // lfbts    equ    25          high order load file bit + 1

                        //                            bits 25-31 terminal subtype

                        // tsbtn    equ    7           number of terminal subtype bits
                        // tsbts    equ    32          high order subtype bit + 1

                        //                            bits 32-38 terminal configuration

                        // cfbtn    equ    7           number of configuration bits
                        // cfbts    equ    39          high order configuration bit + 1

                        //                            bits 39-41 ascii terminal type

                        // atbtn    equ    3           number of ascii type bits
                        // atbts    equ    42          high order ascii type bit + 1
                        // xofbit   equ    58          bit 57 = output off
                        // ss2bit   equ    59          system *d3* stop1 bit + 1

                        //                            bits 42-56 unused
                        //                            bit 59 - set when station is
                        //                                     signed-out.
    CpWord sdinfo;      // save area for disk request info            
                        //                            used when disk error occurs.
                        //
    CpWord pslave;      // 1/set if being monitored, 59/0             
    CpWord pmaster;     // should be unused (who knows)               
                        //
    CpWord inbgnd;      // top bit set if a key waiting in            
                        //         backbnd queue, 2nd bit set set if in backbnd mode
                        //         lower bits contain background queue number
    CpWord submout;     // total numb of mouts submitted              
    CpWord cwsinfo;     // cybis workstation software info            
                        //
                        // fntinfo  equ    59          start info at bit 58
                        //
                        // vfd   1/format font bit (0=default font)
                        //       6/font code
                        //       6/font size
                        //       6/font mode
                        //       1/*zfastk*
                        //       1/*zscroll*
                        //       1/*zhomer*
                        //       1/*zledit*
                        //      12/left text margin for x wrapping (in pixels)
                        //       1/*zmenu*
                        //       1/*zwindow*
                        //       1/*zkermit*
                        //       1/*zpckeys*
                        //       1/*zfgt*
#define ZFGT (1 << 20)
                        //      10/fine grid touch 'x (*zfgtx*)
                        //      10/fine grid touch 'y (*zfgty*)
};
