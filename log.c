/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: log.c
**
**  Description:
**      Perform logging of abnormal conditions.
**
**--------------------------------------------------------------------------
*/

/*
**  -------------
**  Include Files
**  -------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
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

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static FILE *logF;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialize logging.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void logInit(void)
    {
    logF = fopen("log.txt", "wt");
    if (logF == NULL)
        {
        fprintf(stderr, "can't open log file");
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Write a message to the error log.
**
**  Parameters:     Name        Description.
**                  file        file name where error occured
**                  line        line where error occured
**                  fmt         format string
**                  ...         variable length argument list
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void logError(char *file, int line, char *fmt, ...)
    {
    va_list param;

    va_start(param, fmt);
    fprintf(logF, "[%s:%d] ", file, line); 
    vfprintf(logF, fmt, param);
    va_end(param);
    fprintf(logF, NEWLINE);
    }


/*---------------------------  End Of File  ------------------------------*/
