/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: dt_x11.c
**
**  Description:
**      DtCyber common code relating to X windows.
**
**--------------------------------------------------------------------------
*/


/*
**  -------------
**  Include Files
**  -------------
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/cursorfont.h>
#include <sys/time.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/

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

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/

/*
**  ----------------
**  Public Variables
**  ----------------
*/
Display *disp;
XrmDatabase XrmDb;

/*
**  -----------------
**  Private Variables
**  -----------------
*/

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Common X initialization for DtCyber and Pterm
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dtXinit (void)
{
    char *home;
    char *xf;
    XrmDatabase xdef, appdef;
    
    /*
    **  Open the X11 display.
    */
    disp = XOpenDisplay (0);
    if (disp == NULL)
    {
        fprintf(stderr, "Could not open display\n");
        exit(1);
    }
    
    XrmInitialize ();
    appdef = XrmGetFileDatabase ("/usr/lib/X11/app-defaults/Dtcyber");
    if (appdef == NULL)
    {
//        fprintf (stderr, "no app default database for dtcyber\n");
    }
    home = getenv ("HOME");
    xf = malloc (strlen (home) + strlen ("/.Xdefaults") + 1);
    strcpy (xf, home);
    strcat (xf, "/.Xdefaults");
    xdef = XrmGetFileDatabase (xf);
    free (xf);
    if (xdef != NULL)
    {
        XrmCombineDatabase (appdef, &xdef, FALSE);
    }
    else
    {
        xdef = appdef;
    }
    XrmDb = xdef;
}

