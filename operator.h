#ifndef OPERATOR_H
#define OPERATOR_H
/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: const.h
**
**  Description:
**      This file defines constants for communication between the
**      operator interface in DtCyber and the operator UI programs
**
**--------------------------------------------------------------------------
*/

/*
**  ----------------
**  Public Constants
**  ----------------
*/

/*
**  Command codes
**
**  Each of these is the TAG field of a TLV encoded message element
*/
#define OpCommand       0           /* Operator command from UI */
#define OpReply         1           /* Command reply */
#define OpText          2           /* Left screen (static text) data */
#define OpSyntax        3           /* Command parse pattern data */
#define OpStatus        4           /* Right screen (status text) data */
#define OpInitialized   5           /* Initial data all done */

#endif  /* OPERATOR_H */
