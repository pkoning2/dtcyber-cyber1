#include <stdio.h>
#include "const.h"
#include "types.h"
#include "proto.h"


int main (int c, char **v)
{
    int i;
    for (i = 0; i < 64; i++)
    {
        if (asciiToCdc[cdcToAscii[i]] != i)
        {
            printf ("mismatch at display code  %02o ascii %c\n",
                    i, cdcToAscii[i]);
        }
        if (asciiToBcd[bcdToAscii[i]] != i)
        {
            printf ("mismatch at bcd %02o ascii %c\n", i, bcdToAscii[i]);
        }
    }
    return 0;    
}
