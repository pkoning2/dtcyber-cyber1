#include <stdio.h>

typedef unsigned char u8;
typedef unsigned long long u64;
typedef unsigned long u32;
typedef unsigned short u16;

const u8 d2atable[] = ":abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.#[]%\"_!&'?<>@\\^;";

#define LBYT 20

u8 sect[400*2];
u8 secbuf[300];

FILE *pd;

void readsec (int sec, u8 *buf)
{
    int i;
    u16 p1, p2;
    u8 *p;
    
    fseek (pd, sec * 300, SEEK_SET);
    fread (secbuf, 300, 1, pd);
    p = secbuf;
    for (i = 0; i < 300; i += 3)
    {
        p1 = *p++ << 4;
        p1 += *p >> 4;
        p2 = *p++ << 8;
        p2 += *p++;
        *buf++ = (p1 >> 6) & 077;
        *buf++ = p1 & 077;
        *buf++ = (p2 >> 6) & 077;
        *buf++ = p2 & 077;
    }
}


int main (int argc, u8 *argv[])
{
	u8 out[200];
	int table = 0;
	int i, off;
    int sec = 0;
	u8 *p;
	u8 c;
	int addr = 0;
    u8 *d;
    
    if (argc < 2)
        pd = stdin;
    else
        pd = fopen (argv[1], "rb");
    
    for (;;)
	{
        readsec (sec++, sect);
        if (feof (pd))
            break;
        d = sect;
		table = 0;
        for (off = 0; off < 400; off += LBYT)
        {
            printf ("%08o/ ", (addr + off / 2) / 5);
            p = out;
            for (i = 0; i < LBYT; i++)
            {
                printf ("%02o", d[i]);
                c = d2atable[d[i]];
                *p++ = c;
                if (((i + 1) % 5) == 0)
                {
                    putchar (' ');
//                    *p++ = ' ';
                }
                table = 0;
            }
            d += LBYT;
            *p = '\0';
            printf (" %s\n", out);
        };
        addr += 400 / 2;
	}
}

