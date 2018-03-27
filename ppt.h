/////////////////////////////////////////////////////////////////////////////
// Name:        ppt.h
// Purpose:     PPT emulation for pterm
// Author:      Paul Koning
// Modified by:
// Created:     07/11/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     DtCyber license
/////////////////////////////////////////////////////////////////////////////

#ifndef __PPT_H__
#define __PPT_H__ 1

// PPT Resident entry points
#define R_MAIN  0x3d    // Fake return address for mode 5/6/7 call
#define R_INIT  0x40
#define R_DOT   0x43
#define R_LINE  0x46
#define R_CHARS 0x49
#define R_BLOCK 0x4c
#define R_INPX  0x4f
#define R_INPY  0x52
#define R_OUTX  0x55
#define R_OUTY  0x58
#define R_XMIT  0x5b
#define R_MODE  0x5e
#define R_STEPX 0x61
#define R_STEPY 0x64
#define R_WE    0x67
#define R_DIR   0x6a
#define R_INPUT 0x6d
#define R_SSF   0x70
#define R_CCR   0x73
#define R_EXTOUT 0x76
#define R_EXEC  0x79
#define R_GJOB  0x7c
#define R_XJOB  0x7f
#define R_RETURN 0x82  // obsolete
#define R_CHRCV  0x85
#define R_ALARM  0x88
#define R_PRINT  0x8b
#define R_FCOLOR 0x8e
#define R_BCOLOR 0x91
#define R_PAINT  0x94
#define R_WAIT16 0x97
#define R_DUMMY2 0x9a
#define R_DUMMY3 0x9d

// Interrupt mask bits
#define IM_SIR  0x80
#define IM_KST  0x40
#define IM_TP   0x20
#define IM_EXT0 0x08
#define IM_CON  0x02
#define IM_CAR  0x01

// Initial SP
#define INITSP  0xffff      /* To make microTutor not scribble on the stack */       //0x2200

// PPT Resident variables
#define M_FLAG0 0x22ea
#define M_TYPE  0x22eb
#define M_CLOCK 0x22ec
#define M_EXTPA 0x22ee
#define M_MARGIN 0x22f0
#define M_JOBS  0x22f2
#define M_CCR   0x22f4
#define M_MODE  0x22f6
#define M_DIR   0x22f8
#define M_KSW   0x22fa
#define M_ENAB  0x22fc

#define CSETS           8       // leave room for the PPT multiple sets

#define M2ADDR          0x2340  // PPT start address for set 2
#define M3ADDR          0x2740  // PPT start address for set 3

#define M5ORIGIN        (WORKRAM + 0x0300)  // Pointer to mode 5 program
#define M6ORIGIN        (WORKRAM + 0x0302)  // Pointer to mode 6 program
#define M7ORIGIN        (WORKRAM + 0x0304)  // Pointer to mode 7 program

#define C2ORIGIN        (WORKRAM + 0x0306)  // Pointer to M2 characters
#define C3ORIGIN        (WORKRAM + 0x0308)  // Pointer to M3 characters
#define C4ORIGIN        (WORKRAM + 0x030a)  // Pointer to M4 characters
#define C5ORIGIN        (WORKRAM + 0x030c)  // Pointer to M5 characters
#define C6ORIGIN        (WORKRAM + 0x030e)  // Pointer to M6 characters
#define C7ORIGIN        (WORKRAM + 0x0310)  // Pointer to M7 characters

#define TM_HALT     (WORKRAM + 0x0314)
#define TM_STATUS   (WORKRAM + 0x0316)

// PPT input port addresses
#define SIO     0x00
#define COMSTAT 0x01
#define INTVECT 0x02
#define KST     0x04
#define TP      0x05
#define XL      0x10
#define XU      0x11
#define YL      0x12
#define YU      0x13

// PPT output port addresses
#define SIO     0x00
#define COMSTAT 0x01
#define IMASK   0x03
#define XLONG   0x08
#define YLONG   0x09
#define SETXR   0x0a
#define SETXF   0x0b
#define SETYR   0x0c
#define SETYF   0x0d
#define SETABT  0x0e
#define CLRABT  0x0f
#define XL      0x10
#define XU      0x11
#define YL      0x12
#define YU      0x13
#define PDL     0x14
#define PDU     0x15
#define PDM     0x16
#define PDLU    0x17
#define CLOCKX  0x18
#define CLOCKY  0x19
#define CLOCKXY 0x1a
#define CLOCKL  0x1b
#define HCHAR   0x1c
#define VCHAR   0x1d
#define WE      0x1e
#define SCREEN  0x1f
#define SLIDEL  0x20
#define SLIDEU  0x21

#endif   // __PPT_H__
