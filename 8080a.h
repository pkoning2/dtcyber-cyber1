/*******************************************************************************

8080A simulator
Copyright (C) 2002  Anoakie Ray Turner

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
USA.

        Contact:  Anoakie Turner
                  Anoakie.Turner@asu.edu

                  13240 N. 94th Pl.
                  Scottsdale, AZ 85260


Note:
----
  This file assumes that the user can access the source file for details about
the main8080a, LoadSet, and ResetProc functions.  If this assumption becomes
untrue, then this file should contain a copy of the appropriate documentation
for the main8080a, LoadSet, and ResetProc functions.


Contents:
--------
  1.	This file references header files which contain functions to get input,
	draw the contents of the 8080a's graphics buffer, and play sounds.
	This file also contains the 8080a's registers, the 8080a's interrupt
	information, the 8080a's default processor values (when reset),
	definitions for the PSW, memory map information, and the memory itself.

*******************************************************************************/


#ifndef __8080A_H__
#define __8080A_H__

/*******************************************************************************
* Included files:
* --------------
*
*   ctype.h - This file contains the following function definition needed
*	by 8080a.c: tolower.
*
*******************************************************************************/
#include "global.h"
#include <ctype.h>
#include "8080avar.h"


/*******************************************************************************
* 8080a Inline Function:
* ---------------------
* 
*    Parity8 - This function calculates the parity from the supplied 8-bits of
*	data.
*  
* Input:
* -----
*
*    byte - This is an 8-bit unsigned integer.  The content of this variable
*	byte for which the parity will be found.
*
* Output:
* ------
*
*    (return value) - This is an 8-bit unsigned integer that contains 1 (true)
*	if the byte is parity and 0 (false) if the byte is non-parity.
*
*******************************************************************************/
#define Parity8(byte) \
	(!!((byte)&0x80)) ^ \
	(!!((byte)&0x40)) ^ \
	(!!((byte)&0x20)) ^ \
	(!!((byte)&0x10)) ^ \
	(!!((byte)&0x08)) ^ \
	(!!((byte)&0x04)) ^ \
	(!!((byte)&0x02)) ^ \
	(!!((byte)&0x01))



#endif
