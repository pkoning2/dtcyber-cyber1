/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: tap2mag_unix.c
**
**  Description:
**      Convert TAP format files to 9-track mag tapes.
**
**--------------------------------------------------------------------------
*/

/*
**  -------------
**  Include Files
**  -------------
*/
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/mtio.h>

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
**  Purpose:        Convert TAP format files to 9-track mag tapes.
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
    int recLen1;
    int recLen2;
    int len;
    int zero = 0;

    if (argc != 2)
        {
        fprintf(stderr, "Usage: tap2mag <TAP image file>\n");
        exit(1);
        }

    if ((mt = open("/dev/nrst1", O_RDWR)) < 0)
        {
        perror("open");
        exit(1);
        }

    if ((of = fopen(argv[1], "r")) == NULL)
        {
        perror("open");
        exit(1);
        }

    while(1)
        {
        len = fread(&recLen1, sizeof(recLen1), 1, of);
        if (len <= 0)
            {
            fprintf(stderr, "error1\n");
            break;
            }

        if (recLen1 > 0)
            {
            len = fread(buf, 1, recLen1, of);
            if (recLen1 != len)
                {
                fprintf(stderr, "error2\n");
                break;
                }

            len = fread(&recLen2, sizeof(recLen2), 1, of);
            if (len <= 0 || recLen2 != recLen1)
                {
                fprintf(stderr, "error3\n");
                break;
                }
            }

        if (recLen1 == 0)
            {
            int rc;
            struct mtop op;
            op.mt_op = MTWEOF;
            op.mt_count = 1;
            rc = ioctl(mt, MTIOCTOP, &op, sizeof(op));
            fprintf(stderr, "zero length %d\n", rc);
            }
        else
            {
            write(mt, buf, recLen1);
            }
        }

    fclose(of);
    close(mt);
    }

/*---------------------------  End Of File  ------------------------------*/
