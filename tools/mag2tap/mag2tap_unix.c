/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: mag2tap_unix.c
**
**  Description:
**      Convert 9-track mag tapes to TAP format.
**
**--------------------------------------------------------------------------
*/

/*
**  -------------
**  Include Files
**  -------------
*/
#include <stdio.h>

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static unsigned char buf[100000];

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Convert 9-track mag tapes to TAP format.
**
**  Parameters:     Name        Description.
**                  argc        argument count
**                  argv        array of argument values.
**
**  Returns:        exit code.
**
**------------------------------------------------------------------------*/
int main(int argc, char **argv)
    {
    FILE *of;
    int mt;
    int ic;
    int zero = 0;

    if (argc != 2)
        {
        fprintf(stderr, "Usage: mag2tap <TAP image file>\n");
        exit(1);
        }

    if ((mt = open("/dev/nrst1", 0)) < 0)
        {
        perror("open");
        exit(1);
        }

    if ((of = fopen(argv[1], "w")) == NULL)
        {
        perror("open");
        exit(1);
        }

    while ((ic = read(mt, buf, sizeof(buf))) >= 0)
        {
        fprintf(stdout, " %d", ic);
        fflush(stdout);

        if (ic == 0)
            {
            zero += 1;
            if (zero >= 2)
                {
                break;
                }
            }
        else
            {
            zero = 0;
            }

        if (fwrite(&ic, sizeof(ic), 1, of) != 1)
            {
            perror("fwrite1");
            exit(1);
            }

        if (ic > 0)
            {
            if (fwrite(&buf, 1, ic, of) != ic)
                {
                perror("fwrite2");
                exit(1);
                }

            if (fwrite(&ic, sizeof(ic), 1, of) != 1)
                {
                perror("fwrite3");
                exit(1);
                }
            }
        }

    fclose(of);
    close(mt);
    }

/*---------------------------  End Of File  ------------------------------*/
