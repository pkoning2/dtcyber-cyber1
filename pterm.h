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
extern bool platoKeypress (XKeyEvent *kp, int stat);
extern bool platoTouch (XButtonPressedEvent *bp, int stat);
extern void ptermInput(XEvent *event);
#endif
extern void dtXinit(void);
extern void ptermSetName (const char *winName);
extern void ptermLoadChar (int snum, int cnum, const u16 *data);
extern void ptermSetWeMode (u8 we);
extern void ptermSetClip (bool enable);
extern void ptermSetTrace (bool fileaction);

#endif  /* PTERM_H */
