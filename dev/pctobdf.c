#include <stdio.h>

char line[100];
int row[16];
int col[8];
int count;

int main(int c, char **v)
{
    int i, j, k;
    int ch=0;
    
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
        printf ("STARTCHAR char%d\nENCODING %d\n"
                "SWIDTH 720 0\nDWIDTH 8 0\n"
                "BBX 8 16 0 0\nBITMAP\n", ch, ch);
        for (i = 0; i < 16; i++)
            printf ("%02x\n", row[i]);
        printf ("ENDCHAR\n");
        ch++;
    }
}
