#include <stdio.h>

#define rows 336
#define cols 512
#define colb (cols/8)
#define Chars 48

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef u8 Row[colb];
typedef Row Buf[rows];

Buf iobuf, data;
FILE *inf, *outf;

typedef int bool;
#define FALSE 0
#define TRUE 1

void dumpf (const char *fn, int dx, bool bold)
{
    int i, j, k;
	int x;
	int fontY, fy, boldoff, yshift, ymask;
    Row *rp;
    
    outf = fopen (fn, "wb");
    if (outf == NULL)
    {
        perror ("create");
        exit (1);
    }
    
	switch (dx)
    {
	case 8:
		fontY = 32;
		boldoff = 8;
		yshift = 6;
		ymask = 077;
		break;
	case 16:
		fontY = 48;
		boldoff = 32;
		yshift = 5;
		ymask = 037;
		break;
	case 32:
		fontY = 112;
		boldoff = 128;
		yshift = 4;
		ymask = 017;
		break;
    }
    if (bold)
        fontY += boldoff;
    fprintf (outf, "CHARS %d\n", Chars);
    
    for (i = 0; i < Chars; i++)
    {
        fprintf (outf, "STARTCHAR char%d\nENCODING %d\n"
                 "SWIDTH 720 0\nDWIDTH %d 0\n"
                 "BBX %d %d 0 0\nBITMAP\n",
                 i, i, dx, dx, dx);
        rp = &data[fontY + (dx * ((i & ~ymask) >> yshift))];
        x = dx * (i & ymask) / 8;
        for (j = 0; j < dx; j++)
        {
            for (k = 0; k < dx / 8; k++)
                fprintf (outf, "%02X", (*rp)[x + k]);
            fprintf (outf, "\n");
            rp++;
        }
        fprintf (outf, "ENDCHAR\n");
    }
    fprintf (outf, "ENDFONT\n");
    fclose (outf);
}

int main (int c, char **v)
{
    int i;
    
    inf = fopen ("fonts.bmp", "rb");
    fseek (inf, 0x3e, SEEK_SET);
    if (fread (iobuf, sizeof (iobuf), 1, inf) != 1)
    {
        perror ("read");
        exit (1);
    }
    fclose (inf);
    for (i = 0; i < rows; i++)
        memcpy (data[i], iobuf[rows - 1 - i], colb);
    dumpf ("8m", 8, FALSE);
    dumpf ("8b", 8, TRUE);
    dumpf ("16m", 16, FALSE);
    dumpf ("16b", 16, TRUE);
    dumpf ("32m", 32, FALSE);
    dumpf ("32b", 32, TRUE);
    return 0;
}
