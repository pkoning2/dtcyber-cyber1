#include <stdio.h>

char line[100];
int row[16];
int col[8];
int count;

int main(int c, char **v)
{
    int i, j, k;
    
    for (;;)
    {
        fgets (line, 100, stdin);
        if (feof (stdin)) break;
        sscanf (line, " %o, %o, %o, %o, %o, %o, %o, %o, %o, %o, %o, %o, %o, %o, %o, %o ;%n",
                &row[0],&row[1],&row[2],&row[3],&row[4],&row[5],&row[6],&row[7],
                &row[8],&row[9],&row[10],&row[11],&row[12],&row[13],&row[14],&row[15],
                &count);
#if 0
        for (i = 0; i < 16; i++)
            printf (" %03o", row[i]);
        putchar ('\n');
#endif
        // bottom row == lsb of column value
        // leftmost column (msb of row value) == first column
        for (i = 0; i < 8; i++)
        {
            k = 0;
            for (j = 0; j < 16; j++)
            {
                if (row[j] & (0200 >> i))
                    k |= 0x8000 >> j;
            }
            col[i] = k;
        }
        for (i = 0; i < 8; i++)
            printf (" 0x%04x,", col[i]);
        printf (" // %s", line + count);
    }
}
