#include <stdio.h>

#define BlockSize               3840

FILE *inf;
FILE *outf;
unsigned char buf[5000];

void usage (void)
{
    printf ("usage: gettap <infile> [<outfile>]\n");
}

int main (int argc, char **argv)
{
    int bs, ds;
    int start = 1;
    int fn = 0;
    char name[40];
    
    if (argc < 2)
    {
        usage ();
        exit (1);
    }
    inf = fopen (argv[1], "rb");
    if (inf == 0)
    {
        perror ("input file open");
        exit (1);
    }
    if (argc > 2)
    {
        outf = fopen (argv[2], "wb");
        if (outf == 0)
        {
            perror ("output file open");
            exit (1);
        }
        start = 0;
    }
    
    while (!feof (inf))
    {
        fread (&bs, 1, sizeof (bs), inf);
        if (bs == 0)
        {
            printf ("tape mark seen\n");
            break;
        }
        fread (buf, bs, 1, inf);
        ds = bs - 6;
        if (ds % 15 == 9)
            ds -= 2;
        else if (ds % 15 != 0)
            printf ("unexpected data size %d (%d mod 15)\n",
                    ds, ds % 15);
        
        if (start)
        {
            sprintf (name, "file%04d", fn++);
            outf = fopen (name, "wb");
            if (outf == 0)
            {
                perror ("output file open");
                exit (1);
            }
            start = 0;
        }
        
        fwrite (buf, bs - 6, 1, outf);
        fread (&bs, 1, sizeof (bs), inf);
        if (ds < BlockSize)
        {
            if (argc > 2)
                break;
            fclose (outf);
            start = 1;
        }
    }
    fclose (inf);
    if (start == 0)
        fclose (outf);
    return 0;
}
