/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: operator.c
**
**  Description:
**      Provide operator interface for CDC 6600 emulation. This is required
**      to enable a human "operator" to change tapes, remove paper from the
**      printer, shutdown etc.
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
#include <string.h>
#include <ctype.h>
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
typedef struct opCmd
    {
    char            *name;               /* command name */
    void            (*handler)(bool help, char *cmdParams);
    } OpCmd;


/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
char *opGetString(char *inStr, char *outStr, int outSize);
//static void opCmdLoadTape(bool help, char *cmdParams);
//static void opCmdRemovePaper(bool help, char *cmdParams);
static void opCmdHelp(bool help, char *cmdParams);
static void opHelpHelp(void);

static void opCmdEnd(bool help, char *cmdParams);
static void opHelpEnd(void);

static void opCmdLoadTape(bool help, char *cmdParams);
static void opHelpLoadTape(void);

static void opCmdRemovePaper(bool help, char *cmdParams);
static void opHelpRemovePaper(void);

static void opCmdShutdown(bool help, char *cmdParams);
static void opHelpShutdown(void);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
bool opActive = FALSE;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static OpCmd decode[] = 
    {
    "load_tape",                opCmdLoadTape,
    "remove_paper",             opCmdRemovePaper,
    "?",                        opCmdHelp,
    "help",                     opCmdHelp,
    "end",                      opCmdEnd,
    "bye",                      opCmdEnd,
    "shutdown",                 opCmdShutdown,
    NULL,                       NULL
    };

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Operator interface initialisation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void opInit(void)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        Operator interface activation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void opRequest(void)
    {
    OpCmd *cp;
    char cmd[80];
    char name[80];
    char *params;
    char *pos;

    printf("\nOperator interface (%s)\nPlease enter 'help' to get a list of commands\n", DtCyberVersion);

    while (opActive)
        {
        /*
        **  Prompt operator for a command.
        */
        printf("\nOperator> ");
        fflush(stdout);
        if (fgets(cmd, sizeof(cmd), stdin) == NULL || strlen(cmd) == 0)
            {
            continue;
            }

        /*
        **  Replace newline by zero terminator.
        */
        pos = strchr(cmd, '\n');
        if (pos != NULL)
            {
            *pos = 0;
            }

        /*
        **  Extract the command name.
        */
        params = opGetString(cmd, name, sizeof(name));
        if (*name == 0)
            {
            continue;
            }

        /*
        **  Find the command handler.
        */
        for (cp = decode; cp->name != NULL; cp++)
            {
            if (strcmp(cp->name, name) == 0)
                {
                cp->handler(FALSE, params);
                break;
                }
            }

        if (cp->name == NULL)
            {
            printf("Command not implemented: %s\n", name);
            }
        }
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Parse command string and return the first string
**                  terminated by whitespace
**
**  Parameters:     Name        Description.
**                  inStr       Input string
**                  outStr      Output string
**                  outSize     Size of output field
**
**  Returns:        Pointer to first character in input string after
**                  the extracted string..
**
**------------------------------------------------------------------------*/
char *opGetString(char *inStr, char *outStr, int outSize)
    {
    u8 pos = 0;
    u8 len = 0;

    /*
    **  Skip leading whitespace.
    */
    while (inStr[pos] != 0 && isspace(inStr[pos]))
        {
        pos += 1;
        }

    /*
    **  Return pointer to end of input string when there was only
    **  whitespace left.
    */
    if (inStr[pos] == 0)
        {
        *outStr = 0;
        return(inStr + pos);
        }

    /*
    **  Copy string to output buffer.
    */
    while (inStr[pos] != 0 && !isspace(inStr[pos]))
        {
        if (len >= outSize - 1)
            {
            return(NULL);
            }

        outStr[len++] = inStr[pos++];
        }

    outStr[len] = 0;

    /*
    **  Skip trailing whitespace.
    */
    while (inStr[pos] != 0 && isspace(inStr[pos]))
        {
        pos += 1;
        }

    return(inStr + pos);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Terminate emulation.
**
**  Parameters:     Name        Description.
**                  help        Request only help on this command.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opCmdShutdown(bool help, char *cmdParams)
    {
    /*
    **  Process help request.
    */
    if (help)
        {
        opHelpShutdown();
        return;
        }

    /*
    **  Check parameters.
    */
    if (strlen(cmdParams) != 0)
        {
        printf("no parameters expected\n");
        opHelpShutdown();
        return;
        }

    /*
    **  Process command.
    */
    opActive = FALSE;
    emulationActive = FALSE;

    printf("\nThanks for using %s - Goodbye for now.\n\n", DtCyberVersion);
    }

static void opHelpShutdown(void)
    {
    printf("'shutdown' terminates emulation.\n");
    }

/*--------------------------------------------------------------------------
**  Purpose:        Terminate operator interface.
**
**  Parameters:     Name        Description.
**                  help        Request only help on this command.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opCmdEnd(bool help, char *cmdParams)
    {
    /*
    **  Process help request.
    */
    if (help)
        {
        opHelpEnd();
        return;
        }

    /*
    **  Check parameters.
    */
    if (strlen(cmdParams) != 0)
        {
        printf("no parameters expected\n");
        opHelpEnd();
        return;
        }

    /*
    **  Process commands.
    */
    opActive = FALSE;

    printf("\nTerminating operator interface - type ALT-O to re-enter.\n\n");
    }

static void opHelpEnd(void)
    {
    printf("'end' terminates operator interface - type ALT-O to re-enter.\n");
    }

/*--------------------------------------------------------------------------
**  Purpose:        Provide command help.
**
**  Parameters:     Name        Description.
**                  help        Request only help on this command.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opCmdHelp(bool help, char *cmdParams)
    {
    OpCmd *cp;

    /*
    **  Process help request.
    */
    if (help)
        {
        opHelpHelp();
        return;
        }

    /*
    **  Check parameters and process command.
    */
    if (strlen(cmdParams) == 0)
        {
        /*
        **  List all available commands.
        */
        printf("\nList of available commands:\n\n");
        for (cp = decode; cp->name != NULL; cp++)
            {
            printf("%s\n", cp->name);
            }

        return;
        }
    else
        {
        /*
        **  Provide help for specified command.
        */
        for (cp = decode; cp->name != NULL; cp++)
            {
            if (strcmp(cp->name, cmdParams) == 0)
                {
                printf("\n");
                cp->handler(TRUE, NULL);
                return;
                }
            }

        printf("Command not implemented: %s\n", cmdParams);
        }
    }

static void opHelpHelp(void)
    {
    printf("'help'       list all available commands.\n");
    printf("'help <cmd>' provide help for <cmd>.\n");
    }

/*--------------------------------------------------------------------------
**  Purpose:        Load a new tape
**
**  Parameters:     Name        Description.
**                  help        Request only help on this command.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opCmdLoadTape(bool help, char *cmdParams)
    {
    /*
    **  Process help request.
    */
    if (help)
        {
        opHelpLoadTape();
        return;
        }

    /*
    **  Check parameters and process command.
    */
    if (strlen(cmdParams) == 0)
        {
        printf("parameters expected\n");
        opHelpLoadTape();
        return;
        }

    mt669LoadTape(cmdParams);
    }

static void opHelpLoadTape(void)
    {
    printf("'load_tape <channel>,<unit>,<r|w>,<filename>' load specified tape on unit.\n");
    }

/*--------------------------------------------------------------------------
**  Purpose:        Remove paper from printer.
**
**  Parameters:     Name        Description.
**                  help        Request only help on this command.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opCmdRemovePaper(bool help, char *cmdParams)
    {
    /*
    **  Process help request.
    */
    if (help)
        {
        opHelpRemovePaper();
        return;
        }

    /*
    **  Check parameters and process command.
    */
    if (strlen(cmdParams) == 0)
        {
        printf("parameters expected\n");
        opHelpRemovePaper();
        return;
        }

    lp501RemovePaper(cmdParams);
    lp512RemovePaper(cmdParams);
    }

static void opHelpRemovePaper(void)
    {
    printf("'remove_paper <channel>,<equipment>' remover paper from printer.\n");
    }

/*---------------------------  End Of File  ------------------------------*/
