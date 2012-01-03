/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: init.c
**
**  Description:
**      Perform reading and validation of startup file and use configured
**      values to startup simulation.
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
#define MaxLine                 512

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#define isoctal(c) ((c) >= '0' && (c) <= '7')

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef struct 
    {
    const char  *name;
    long        *varp;
    long        defval;
    } intParam;

struct idleEntry
    {
    const char *name;
    int code;
    };
    
/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void initCyber(const char *config);
static void initEquipment(void);
static void initDeadstart(void);
static bool initGetOctal(const char *entry, int defValue, long *value);
static bool initGetInteger(const char *entry, int defValue, long *value);
static bool initGetString(const char *entry, char *defString, char *str, int strLen);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
bool bigEndian;
extern u16 deadstartPanel[];
extern u8 deadstartCount;
long cpuRatio;
char autoDateString[32];
char autoString[32];
char platoSection[40];
int idleMode;
long chCount;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static FILE *fcb;
static long sectionStart;
static char *startupFile = "cyber.ini";
static char deadstart[40];
static char equipment[40];
static union
    {
    u32 number;
    u8 bytes[4];
    } endianCheck;

/* Table for a bunch of simple integer parameters */
const intParam intParamList[] = 
{
    { "telnetport", &telnetPort, DefTelnetPort },
    { "telnetconns", &telnetConns, 4 },
#ifdef NPU_SUPPORT
    { "nputelnetconns", &npuNetTelnetConns, 3 },
    { "nputelnetport", &npuNetTelnetPort, 6610 },
#endif
    { "operport", &opPort, DefOpPort },
    { "operconns", &opConns, 4 },
    { "consoleport", &dd60Port, DefDd60Port },
    { "consoleconns", &dd60Conns, 4 },
    { "tpmuxport", &tpmuxPort, DefTpmuxPort },
    { "tpmuxconns", &tpmuxConns, 2 },
    { "sockets", &extSockets, 128 },
    { NULL, NULL, 0 }                   /* End marker */
};

const struct idleEntry idleTable[] =
{
    { "nos287", 287 },
    { "nos2.8.7", 287 },
    { NULL, 0 }
};

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Read and process startup file.
**
**  Parameters:     Name        Description.
**                  config      name of the section to run
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void initStartup(const char *config)
    {
    /*
    **  Open startup file.
    */
    fcb = fopen(startupFile, "rt");
    if (fcb == NULL)
        {
        perror(startupFile);
        exit(1);
        }

    /*
    **  Determine endianness of the host.
    */
    endianCheck.bytes[0] = 0;
    endianCheck.bytes[1] = 0;
    endianCheck.bytes[2] = 0;
    endianCheck.bytes[3] = 1;
    bigEndian = endianCheck.number == 1;

    /*
    **  Read and process cyber.ini file.
    */
    printf("Starting initialisation\n");
#if defined(__APPLE__) && defined (ARCHNAME)
    printf (" Built for " ARCHNAME "\n");
#endif
    initCyber(config);
    initDeadstart();
    initEquipment();
    initExt();
    initPni();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Convert endian-ness of 32 bit value,
**
**  Parameters:     Name        Description.
**
**  Returns:        Converted value.
**
**------------------------------------------------------------------------*/
u32 initConvertEndian(u32 value)
    {
    u32 result;

    result  = (value & 0xff000000) >> 24;
    result |= (value & 0x00ff0000) >>  8;
    result |= (value & 0x0000ff00) <<  8;
    result |= (value & 0x000000ff) << 24;

    return(result);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Save current startup file position
**
**  Parameters:     Name        Description.
**
**  Returns:        Current position
**
**------------------------------------------------------------------------*/
long initSavePosition (void)
    {
    return ftell (fcb);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Restore startup file position
**
**  Parameters:     Name        Description.
**                  pos         Saved position
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void initRestorePosition (long pos)
    {
    fseek(fcb, pos, SEEK_SET);
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Read and process [cyber] startup file section.
**
**  Parameters:     Name        Description.
**                  config      name of the section to run
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void initCyber(const char *config)
    {
    char model[40];
    char cmFile[256];
    char ecsFile[256];
    char idle[256];
    long memory;
    long ecsBanks;
    long cpus;
    long clockIncrement;
    long clockMHz;
    long pps;
    long mask;
    const intParam *paramp;
    const struct idleEntry *gp;
    
    if (!initOpenSection(config))
        {
        fprintf(stderr, "Required section [%s] not found in %s\n", config, startupFile);
        exit(1);
        }

    /*
    **  Determine CPU model, central memory, ECS banks and initialise CPU.
    */
    (void)initGetString("model", "6600", model, sizeof(model));

    (void)initGetOctal("memory", 01000000, &memory);
    if (memory < 040000)
        {
        fprintf(stderr, "Entry 'memory' less then 40000B in section [%s] in %s\n", config, startupFile);
        exit(1);
        }
    if ((memory & (-memory)) != memory)
      {
	fprintf(stderr, "Entry 'memory' is not a power of 2 in section [%s] in %s\n", config, startupFile);
      }

    (void)initGetInteger("ecsbanks", 0, &ecsBanks);
    switch(ecsBanks)
        {
    case 0:
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
        break;

    default:
        fprintf(stderr, "Entry 'ecsbanks' invalid in section [%s] in %s - correct values are 0, 1, 2, 4, 8 or 16\n", config, startupFile);
        exit(1);
        }

    initGetInteger ("cpus", 1, &cpus);
    if (cpus < 1 || cpus > 2)
        {
        fprintf (stderr, "Entry 'cpus' invalid in section [%s] in %s -- correct values are 1 or 2\n", config, startupFile);
        exit (1);
        }
#ifndef USE_THREADS
    if (cpus != 1)
        {
        fprintf (stderr, "Entry 'cpus=2' ignored, no dual CPU support\n");
        cpus = 1;
        }
#else
    printf ("Running with %ld CPUs\n", cpus);
#endif
    initGetInteger ("cpuratio", 1, &cpuRatio);
    if (cpuRatio < 1 || cpuRatio > 50)
        {
        fprintf (stderr, "Entry 'cpuratio' invalid in section [%s] in %s -- correct value is between 1 and 50\n", config, startupFile);
        exit (1);
        }
    else
        {
        printf ("Running with %ld CPU instructions per PPU instruction\n",
                cpuRatio);
        }
    
    (void)initGetString("cmFile", "", cmFile, sizeof(cmFile));
    (void)initGetString("ecsFile", "", ecsFile, sizeof(ecsFile));
    
    /*
    **  See if we want OS specific idle processing.
    */
    if (initGetString ("idleMode", "", idle, sizeof (idle)))
        {
        gp = idleTable;
        while (gp->name != NULL)
            {
            if (strcmp (gp->name, idle) == 0)
                {
                idleMode = gp->code;
                break;
                }
            }
        }
    
    cpuInit(model, cpus, memory, ecsBanks, cmFile, ecsFile);

    /*
    **  Determine number of PPs and initialise PP subsystem.
    */
    (void)initGetOctal("pps", 012, &pps);
    if (pps < 012)
        {
        fprintf(stderr, "Entry 'pps' less then 12B in section [cyber] in %s\n", startupFile);
        exit(1);
        }

    ppInit((u8)pps);

    /*
    **  Determine number of channels and initialise channel subsystem.
    */
    (void)initGetOctal("channels", 014, &chCount);

    if (chCount < 014)
        {
        fprintf(stderr, "Entry 'channels' less then 14B in section [cyber] in %s\n", startupFile);
        exit(1);
        }

    channelInit((u8)chCount);

    /*
    **  Get active deadstart switch section name.
    */
    if (!initGetString("deadstart", "", deadstart, sizeof(deadstart)))
        {
        fprintf(stderr, "Required entry 'deadstart' in section [cyber] not found in %s\n", startupFile);
        exit(1);
        }

    /*
    **  Get clock increment value and initialise clock.
    */
    (void)initGetOctal("clock", 3, &clockIncrement);
    (void)initGetInteger("megahertz", 0, &clockMHz);
    rtcInit(model, (u8)clockIncrement, clockMHz);

    /*
    **  Get active equipment section name.
    */
    if (!initGetString("equipment", "", equipment, sizeof(equipment)))
        {
        fprintf(stderr, "Required entry 'equipment' in section [cyber] not found in %s\n", startupFile);
        exit(1);
        }

    /*
    **  Get optional trace mask. If not specified, use compile time value.
    */
    if (initGetOctal("trace", 0, &mask))
        {
        traceMask = (u16)mask;
        }

    /*
    **  Get PLATO section name.
    */
    initGetString("plato", "", platoSection, sizeof(platoSection));

    /*
    **  Get a set of integer parameters
    */
    paramp = intParamList;
    while (paramp->name != NULL)
        {
        initGetInteger (paramp->name, paramp->defval, paramp->varp);
        paramp++;
        }
    /*
    **  Get auto date string (prompt used to trigger auto-entry of
    **  the date and time) and auto string (initial text entered
    **  into the console input buffer at deadstart).
    */
    (void)initGetString("autoDate", "", 
                        autoDateString, sizeof(autoDateString));
    (void)initGetString("autoEntry", "", autoString, sizeof(autoString));
    }

/*--------------------------------------------------------------------------
**  Purpose:        Read and process equipment definitions.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void initEquipment(void)
    {
    char *line;
    char *token;
    char *deviceName;
    int eqNo;
    int unitNo;
    int channelNo;
    u8 deviceIndex;
    int lineNo;


    if (!initOpenSection(equipment))
        {
        fprintf(stderr, "Required section [%s] not found in %s\n", equipment, startupFile);
        exit(1);
        }

    /*
    **  Process all equipment entries.
    */
    lineNo = -1;
    while  ((line = initGetNextLine()) != NULL)
        {
        lineNo += 1;

        /*
        **  Parse device type.
        */
        token = strtok(line, ",");
        if (token == NULL || strlen(token) < 2)
            {
            fprintf(stderr, "Section [%s], relative line %d, invalid device type %s in %s\n",
                equipment, lineNo, token, startupFile);
            exit(1);
            }

        for (deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++)
            {
            if (strcmp(token, deviceDesc[deviceIndex].id) == 0)
                {
                break;
                }
            }

        if (deviceIndex == deviceCount)
            {
            fprintf(stderr, "Section [%s], relative line %d, unknown device %s in %s\n",
                equipment, lineNo, token, startupFile);
            exit(1);
            }

        /*
        **  Parse equipment number.
        */
        token = strtok(NULL, ",");
        if (token == NULL || strlen(token) != 1 || !isoctal(token[0]))
            {
            fprintf(stderr, "Section [%s], relative line %d, invalid equipment no %s in %s\n",
                equipment, lineNo, token, startupFile);
            exit(1);
            }

        eqNo = strtol(token, NULL, 8);

        /*
        **  Parse unit number.
        */
        token = strtok(NULL, ",");
        if (token == NULL || !isoctal(token[0]))
            {
            fprintf(stderr, "Section [%s], relative line %d, invalid unit count %s in %s\n",
                equipment, lineNo, token, startupFile);
            exit(1);
            }

        unitNo = strtol(token, NULL, 8);

        /*
        **  Parse channel number.
        */
        token = strtok(NULL, ", ");
        if (token == NULL || strlen(token) != 2 || !isoctal(token[0]) || !isoctal(token[1]))
            {
            fprintf(stderr, "Section [%s], relative line %d, invalid channel no %s in %s\n",
                equipment, lineNo, token, startupFile);
            exit(1);
            }

        channelNo = strtol(token, NULL, 8);

        if (channelNo < 0 || channelNo >= chCount)
            {
            fprintf(stderr, "Section [%s], relative line %d, channel no %s not permitted in %s\n",
                equipment, lineNo, token, startupFile);
            exit(1);
            }

        /*
        **  Parse optinal file name.
        */
        deviceName = strtok(NULL, " ");

        /*
        **  Initialise device.
        */
        deviceDesc[deviceIndex].init((u8)eqNo, (u8)unitNo, (u8)channelNo, deviceName);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Read and process deadstart panel settings.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void initDeadstart(void)
    {
    char *line;
    char *token;
    u8 lineNo;


    if (!initOpenSection(deadstart))
        {
        fprintf(stderr, "Required section [%s] not found in %s\n", deadstart, startupFile);
        exit(1);
        }

    /*
    **  Process all deadstart panel switches.
    */
    lineNo = 0;
    while  ((line = initGetNextLine()) != NULL && lineNo < MaxDeadStart)
        {
        /*
        **  Parse switch settings.
        */
        token = strtok(line, " ;\n");
        if (   token == NULL || strlen(token) != 4
            || !isoctal(token[0]) || !isoctal(token[1])
            || !isoctal(token[2]) || !isoctal(token[3]))
            {
            fprintf(stderr, "Section [%s], relative line %d, invalid deadstart setting %s in %s\n",
                deadstart, lineNo, token, startupFile);
            exit(1);
            }

        deadstartPanel[lineNo++] = (u16)strtol(token, NULL, 8);
        }

    deadstartCount = lineNo + 1;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Locate section header and remember the start of data.
**
**  Parameters:     Name        Description.
**                  name        section name
**
**  Returns:        TRUE if section was found, FALSE otherwise.
**
**------------------------------------------------------------------------*/
bool initOpenSection(const char *name)
    {
    char lineBuffer[MaxLine];
    char section[40];
    u8 sectionLength = strlen(name) + 2;

    /*
    **  Build section label.
    */
    strcpy(section, "[");
    strcat(section, name);
    strcat(section, "]");

    /*
    **  Reset to beginning.
    */
    fseek(fcb, 0, SEEK_SET);

    /*
    **  Try to find section header.
    */
    do
        {
        if (fgets(lineBuffer, MaxLine, fcb) == NULL)
            {
            /*
            **  End-of-file - return failure.
            */
            return(FALSE);
            }
        } while (strncmp(lineBuffer, section, sectionLength) != 0);

    /*
    **  Remember start of section and return success.
    */
    sectionStart = ftell(fcb);
    return(TRUE);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Return next non-blank line in section
**
**  Parameters:     Name        Description.
**
**  Returns:        Pointer to line buffer
**
**------------------------------------------------------------------------*/
char *initGetNextLine(void)
    {
    static char lineBuffer[MaxLine];
    char *cp;
    bool blank;

    /*
    **  Get next lineBuffer.
    */
    do
        {
        if (   fgets(lineBuffer, MaxLine, fcb) == NULL
            || lineBuffer[0] == '[')
            {
            /*
            **  End-of-file or end-of-section - return failure.
            */
            return(NULL);
            }

        /*
        **  Determine if this line consists only of whitespace or comment and
        **  replace all whitespace by proper space.
        */
        blank = TRUE;
        for (cp = lineBuffer; *cp != 0; cp++)
            {
            if (blank && *cp == ';')
                {
                break;
                }

            if (isspace(*cp))
                {
                *cp = ' ';
                }
            else
                {
                blank = FALSE;
                }
            }

        } while (blank);

    /*
    **  Found a non-blank line - return to caller.
    */
    return(lineBuffer);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Locate octal entry within section and return value.
**
**  Parameters:     Name        Description.
**                  entry       entry name
**                  defValue    default value
**                  value       pointer to return value
**
**  Returns:        TRUE if entry was found, FALSE otherwise.
**
**------------------------------------------------------------------------*/
static bool initGetOctal(const char *entry, int defValue, long *value)
    {
    char buffer[40];

    if (   !initGetString(entry, "", buffer, sizeof(buffer))
        || buffer[0] < '0'
        || buffer[0] > '7')
        {
        /*
        **  Return default value.
        */
        *value = defValue;
        return(FALSE);
        }

    /*
    **  Convert octal string to value.
    */
    *value = strtol(buffer, NULL, 8);

    return(TRUE);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Locate integer entry within section and return value.
**
**  Parameters:     Name        Description.
**                  entry       entry name
**                  defValue    default value
**                  value       pointer to return value
**
**  Returns:        TRUE if entry was found, FALSE otherwise.
**
**------------------------------------------------------------------------*/
static bool initGetInteger(const char *entry, int defValue, long *value)
    {
    char buffer[40];

    if (   !initGetString(entry, "", buffer, sizeof(buffer))
        || buffer[0] < '0'
        || buffer[0] > '9')
        {
        /*
        **  Return default value.
        */
        *value = defValue;
        return(FALSE);
        }

    /*
    **  Convert integer string to value.
    */
    *value = strtol(buffer, NULL, 10);

    return(TRUE);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Locate string entry within section and return string
**
**  Parameters:     Name        Description.
**                  entry       entry name
**                  defString   default string
**                  str         pointer to string buffer (return value)
**                  strLen      length of string buffer
**
**  Returns:        TRUE if entry was found, FALSE otherwise.
**
**------------------------------------------------------------------------*/
static bool initGetString(const char *entryname, char *defString, char *str, int strLen)
    {
    u8 entryLength = strlen(entryname) + 1;
    char *line;
    char *pos;
    char entry[40];

    /*
    **  Build entry label.
    */
    strcpy (entry, entryname);
    strcat (entry, "=");

    /*
    **  Leave room for zero terminator.
    */
    strLen -= 1;

    /*
    **  Reset to begin of section.
    */
    fseek(fcb, sectionStart, SEEK_SET);

    /*
    **  Try to find entry.
    */
    do
        {
        if ((line = initGetNextLine()) == NULL)
            {
            /*
            **  Copy return value.
            */
            strncpy(str, defString, strLen);

            /*
            **  End-of-file or end-of-section - return failure.
            */
            return(FALSE);
            }
        } while (strncmp(line, entry, entryLength) != 0);

    /*
    **  Cut off any trailing comments.
    */
    pos = strchr(line, ';');
    if (pos != NULL)
        {
        *pos = 0;
        }

    /*
    **  Cut off any trailing whitespace.
    */
    pos = line + strlen(line) - 1;
    while (pos > line && isspace(*pos))
        {
        *pos-- = 0;
        }

    /*
    **  Locate start of value.
    */
    pos = strchr(line, '=');
    if (pos == NULL)
        {
        if (defString != NULL)
            {
            strncpy(str, defString, strLen);
            }

        /*
        **  No value specified.
        */
        return(FALSE);
        }

    /*
    **  Return value and success.
    */
    strncpy(str, pos + 1, strLen);
    return(TRUE);
    }

/*---------------------------  End Of File  ------------------------------*/


