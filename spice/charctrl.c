#include <stdio.h>
#include <stdlib.h>

#define START 20

typedef unsigned char u8;

#include "../chargen6612.h"

const char consoleToAscii[64] =
    {
    /* 00-07 */ 0,      'A',    'B',    'C',    'D',    'E',    'F',    'G',
    /* 10-17 */ 'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
    /* 20-27 */ 'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
    /* 30-37 */ 'X',    'Y',    'Z',    '0',    '1',    '2',    '3',    '4',
    /* 40-47 */ '5',    '6',    '7',    '8',    '9',    '+',    '-',    '*',
    /* 50-57 */ '/',    '(',    ')',    ' ',    '=',    ' ',    ',',    '.',
    /* 60-67 */  0,      0,      0,      0,      0,      0,      0,      0,
    /* 70-77 */  0,      0,      0,      0,      0,      0,      0,      0
    };

void sig (FILE *f, int ch, int xy)
{
    int i, v, delta, mini, maxi;
    char coord;
    int prev, cur;
    
    for (v = 1; v <= 6; v++)
    {
        mini = maxi = -1;
        prev = 0;
        coord = xy ? 'y' : 'x';
        fprintf (f, "v%c%d v%c%d 0 pwl (0ns 0 ", coord, v, coord, v);
        for (i = 0; i < 28; i++)
        {
            if (dd60chars[ch][i].x == 0 &&
                dd60chars[ch][i].y == 0 && dd60chars[ch][i].unblank == 0)
            {
                if (mini < 0)
                {
                    continue;
                }
                
            }
            else
            {
                maxi = i;
            }
            if (mini < 0)
            {
                mini = i - 1;
            }
            
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
                fprintf (f, "%dns %d ", (i - mini) * 100 - 100 + START, prev);
                fprintf (f, "%dns %d ", (i - mini) * 100 - 100 + START + 5, cur);
                prev = cur;
            }
        }
        fprintf (f, "%dns %d)\n", 2800, prev);
    }
    if (xy)
    {
        fprintf (f, ".tran .1ns %dns\n", (maxi - mini) * 100 + 200);
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
    if (ch <= 0 || ch >= 060)
    {
        printf ("char code %o out of range\n", ch);
        exit (2);
    }
    if (consoleToAscii[ch] == ' ')
    {
        printf ("that's a space\n");
        exit (2);
    }
    f = fopen ("vctrl.mod", "w");
    if (f == NULL)
    {
        perror ("file open");
        exit (3);
    }
    fprintf (f, "* include file for dd60 control signals to AF modules\n");
    fprintf (f, "* char code %03o: %c\n", ch, consoleToAscii[ch]);
    sig (f, ch, 0);
    sig (f, ch, 1);
    fclose (f);
    return 0;
}
