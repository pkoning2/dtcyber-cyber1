#ifndef PTERM_H
#define PTERM_H
/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Paul Koning (see license.txt)
**
**  Name: pterm.h
**
**  Description:
**      This file defines external function prototypes for pterm.
**
**--------------------------------------------------------------------------
*/

/*
**  --------------------
**  Function Prototypes.
**  --------------------
*/

#ifdef PTERM_DEFINE_X
extern bool platoKeypress (XKeyEvent *kp);
extern bool platoTouch (XButtonPressedEvent *bp);
extern void ptermInput(XEvent *event);
extern void dtXinit(void);
#endif
extern void ptermComInit(void);
extern void ptermSetName (const char *winName);
extern void ptermSetStation (const char *hostName, int station);
extern void ptermLoadChar (int snum, int cnum, const u16 *data);
extern void ptermSetWeMode (u8 we);
extern void ptermSetClip (bool enable);
extern void ptermSetTrace (bool fileaction);
extern void ptermShowTrace (bool enable);
extern void ptermSendKey(int key);
extern int procNiuWord (u32 d);
extern void ptermDrawChar (int x, int y, int snum, int cnum);
extern void ptermDrawPoint (int x, int y);
extern void ptermDrawLine(int x1, int y1, int x2, int y2);
extern void ptermFullErase (void);
extern void ptermTouchPanel(bool enable);

#endif  /* PTERM_H */
