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
the SetGlobals function.  If this assumption becomes untrue, then this file
should contain a copy of the appropriate documentation for the SetGlobals
function.


Contents:
--------
  1.	This file contains the definitions for TRUE and FALSE.  It also
  	contains the global variables USE_SOUND, which lets you enable and
	disable the sound system, and FATAL_ERROR, which logs what type of
	fatal error just occurred.

*******************************************************************************/

#ifndef __GLOBAL_H__
#define __GLOBAL_H__



/*******************************************************************************
* Included files:
* --------------
*
*******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "const.h"
#include "types.h"

/******************************************************************************
*
* Type names used in the 8080a emulation
*
******************************************************************************/

typedef u8 Uint8;
typedef u16 Uint16;

/******************************************************************************
* Global Definitions:
* ------------------
*
*   TRUE - Defined as true, which is an integer that is not zero.
*   
*   FALSE - Defined as false, which is an integer that is zero.
*   
*   FPS - Defined as 60 by default.  Indicated the number of frames to draw
*	per second.
******************************************************************************/
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FPS
#define FPS 60
#endif


#endif

