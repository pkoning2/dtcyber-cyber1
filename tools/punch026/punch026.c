/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Walter Spector and Tom Hunter (see license.txt)
**
**  Name: punch026.c
**
**  Description:
**      Convert ASCII text to Hollerith card deck.
**
**--------------------------------------------------------------------------
*/

/*
**  -------------
**  Include Files
**  -------------
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define CARDLEN 80

/*
**  -----------------
**  Private Variables
**  -----------------
*/
unsigned short asc2pnch_table[256] =
    {
    /*                                                                         */
    /* 000- */  0,      0,      0,      0,      0,      0,      0,      0,
    /*                                                                         */
    /* 010- */  0,      0,      0,      0,      0,      0,      0,      0,
    /*                                                                         */
    /* 020- */  0,      0,      0,      0,      0,      0,      0,      0,
    /*                                                                         */
    /* 030- */  0,      0,      0,      0,      0,      0,      0,      0,
    /*          space   !       "  	    #	    $	    %	    &	    '      */
    /* 040- */  0,      0,      0,      0,      02102,  0,      0,      0,
    /*          (       )       *       +       ,       -       .       /      */
    /* 050- */  01042,  04042,  02042,  04000,  01102,  02000,  04102,  01400,
    /*          0       1       2       3       4       5       6       7      */
    /* 060- */  01000,  00400,  00200,  00100,  00040,  00020,  00010,  00004,
    /*          8       9       :       ;       <       =       >       ?      */
    /* 070- */  00002,  00001,  0,      0,      0,      00204,  0,      0,
    /*          @       A       B       C       D       E       F       G      */
    /* 100- */  0,      04400,  04200,  04100,  04040,  04020,  04010,  04004,
    /*          H       I       J       K       L       M       N       O      */
    /* 110- */  04002,  04001,  02400,  02200,  02100,  02040,  02020,  02010,
    /*          P       Q       R       S       T       U       V       W      */
    /* 120- */  02004,  02002,  02001,  01200,  01100,  01040,  01020,  01010,
    /*          X       Y       Z       [       \       ]       ^       _      */
    /* 130- */  01004,  01002,  01001,  0,      0,      0,      0,      00042,
    /*          `       a       b       c       d       e       f       g      */
    /* 140- */  0,      04400,  04200,  04100,  04040,  04020,  04010,  04004,
    /*          h       i       j       k       l       m       n       o      */
    /* 150- */  04002,  04001,  02400,  02200,  02100,  02040,  02020,  02010,
    /*          p       q       r       s       t       u       v       w      */
    /* 160- */  02004,  02002,  02001,  01200,  01100,  01040,  01020,  01010,
    /*          x       y       z       {       |       }       ~              */
    /* 170- */  01004,  01002 , 01001,  0,      0,      0,      00007,  0
    };


/*--------------------------------------------------------------------------
**  Purpose:        Convert ASCII text to Hollerith card deck.
**
**  Parameters:     Name        Description.
**                  argc        argument count
**                  argv        array of argument values.
**
**  Returns:        exit code.
**
**------------------------------------------------------------------------*/
int main (int argc, char *argv[])
    {
    unsigned short pc;
    unsigned short obuf[CARDLEN];
    int c;
    int i, j;
    short blank = asc2pnch_table[' '];
    FILE *ifd, *ofd;

    if (argc < 2)
        {
        fprintf(stderr,"usage: punch026 <ascii cardfile>\n");
        exit(EXIT_FAILURE);
        }

    if ((ifd = fopen (argv[1], "rt")) < 0)
        {
        perror (argv[1]);
        exit(EXIT_FAILURE);
        }

    if ((ofd = fopen("CR405_C12", "wb")) < 0)
        {
        perror ("CR405_12");
        exit(EXIT_FAILURE);
        }

    i = 0;
    for (j=0; j < CARDLEN; j++)
        {
        obuf[j] = blank;
        }

    while ((c = fgetc(ifd)) != EOF)
        {
        pc = asc2pnch_table[c & 0x7F];
        if (c != '\n')
            {
            if (i < CARDLEN)
                {
                obuf[i++] = pc;
                }
            }
        else
            {
            fwrite (obuf, sizeof (short), CARDLEN, ofd);
            i = 0;
            for (j = 0; j < CARDLEN; j++)
                {
                obuf[j] = blank;
                }
            }
        }

    /*
    **  Write a 6/7/8/9 card at end of the file.
    */
    for (j = 0; j < CARDLEN; j++)
        {
        obuf[j] = blank;
        }

    obuf[0] = 017;
    fwrite(obuf, sizeof (short), CARDLEN, ofd);

    fclose(ofd);
    fclose(ifd);
    }

/*---------------------------  End Of File  ------------------------------*/

