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
extern void ptermSetName (const char *winName);
extern void ptermLoadChar (int snum, int cnum, const u16 *data);
extern void ptermSetWeMode (u8 we);
extern void ptermSetClip (bool enable);
extern void ptermSetTrace (bool fileaction);
extern void ptermSendKey(int key);
extern int procNiuWord (u32 d);

#endif  /* PTERM_H */
