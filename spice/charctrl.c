#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;

#include "../chargen6612.h"

void sig (FILE *f, int ch, int xy)
{
    int i, v, delta;
    char coord;
    int prev, cur;
    
    for (v = 1; v <= 6; v++)
    {
        prev = -1;
        coord = xy ? 'y' : 'x';
        fprintf (f, "v%c%d v%c%d 0 pwl (", coord, v, coord, v);
        for (i = 0; i < 28; i++)
        {
            if (xy)
            {
                cur = dd60chars[ch][i].y;
            }
            else
            {
                cur = dd60chars[ch][i].x;
            }
                
            cur = (cur < v) ? 0 : 1;
            if (cur != prev)
            {
                if (prev != -1)
                {
                    fprintf (f, "%dns %d ", i * 100, prev);
                    delta = 5;
                }
                else
                {
                    delta = 0;
                }
                fprintf (f, "%dns %d ", i * 100 + delta, cur);
                prev = cur;
            }
        }
        fprintf (f, "%dns %d)\n", 2800, prev);
    }
}

int main (int argc, char **argv)
{
    int i, ch;
    FILE *f;
    
    if (argc < 2)
    {
        printf ("usage: %s charnum\n", argv[0]);
        exit (1);
    }
    ch = strtol (argv[1], NULL, 8);
    if (ch < 0 || ch >= 060)
    {
        printf ("char code %o out of range\n", ch);
        exit (2);
    }
    f = fopen ("vctrl.mod", "w");
    if (f == NULL)
    {
        perror ("file open");
        exit (3);
    }
    fprintf (f, "* include file for dd60 control signals to AF modules\n");
    sig (f, ch, 0);
    sig (f, ch, 1);
    fclose (f);
    return 0;
}
