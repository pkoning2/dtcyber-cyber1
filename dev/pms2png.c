#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define dsblks  7           // blocks per lesson space
#define pdblks  16          // num blocks in pack directory
#define pdstart 12          // starting block num of directory
#define pdspace ((pdblks+pdstart+dsblks-1)/dsblks) // # parts taken up by pdir
#define mpdblks  7          // num blocks in master file pack directory
#define mpdspace 1          // # parts for pdir in master file

#define pdname  "4;;;;;;;;;" // pack dir "name"
#define master  "master   a" // pack type codes
#define backup  "backup   b"
#define general "general  c"
#define binary  "binary   d"

typedef unsigned char u8;
typedef unsigned long long u64;
typedef unsigned long u32;
typedef unsigned short u16;

typedef u8 cw[10];
typedef u8 w18[3];
typedef u8 w12[2];

#define setval(to,from) setbytes (to, from, sizeof (to))
#define setstr(to,from) atod (to, from, sizeof (to))

typedef struct 
{
    cw packname;
    cw packtype;
    u8 f[4];
    w18 stotal;
    w18 sused;
    u8 f2[4];
    w18 ftotal;
    w18 fused;
    cw inflth;
} packdir;

const char asciiToCdc[256] = {
    /* 000- */  0,      0,      0,      0,      0,      0,      0,      0,
    /* 010- */  0,      0,      0,      0,      0,      0,      0,      0,
    /* 020- */  0,      0,      0,      0,      0,      0,      0,      0,
    /* 030- */  0,      0,      0,      0,      0,      0,      0,      0,
    /* 040- */  055,    066,    064,    060,    053,    063,    067,    070,
    /* 050- */  051,    052,    047,    045,    056,    046,    057,    050,
    /* 060- */  033,    034,    035,    036,    037,    040,    041,    042,
    /* 070- */  043,    044,    0,      077,    072,    054,    073,    071,
    /* 100- */  074,    001,    002,    003,    004,    005,    006,    007,
    /* 110- */  010,    011,    012,    013,    014,    015,    016,    017,
    /* 120- */  020,    021,    022,    023,    024,    025,    026,    027,
    /* 130- */  030,    031,    032,    061,    075,    062,    076,    065,
    /* 140- */  0,      001,    002,    003,    004,    005,    006,    007,
    /* 150- */  010,    011,    012,    013,    014,    015,    016,    017,
    /* 160- */  020,    021,    022,    023,    024,    025,    026,    027,
    /* 170- */  030,    031,    032,    061,    0,      0,      0,      0
    };

const char cdcToAscii[64] =
    {
    /* 00-07 */ ' ',    'a',    'b',    'c',    'd',    'e',    'f',    'g',
    /* 10-17 */ 'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
    /* 20-27 */ 'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
    /* 30-37 */ 'x',    'y',    'z',    '0',    '1',    '2',    '3',    '4',
    /* 40-47 */ '5',    '6',    '7',    '8',    '9',    '+',    '-',    '*',
    /* 50-57 */ '/',    '(',    ')',    '$',    '=',    ' ',    ',',    '.',
    /* 60-67 */ '#',    '[',    ']',    '%',    '"',	'_',	'!',	'&',
    /* 70-77 */ '\'',   '?',    '<',    '>',    '@',    '\\',   '^',    ';'
    };

FILE *pd;
FILE *md;
u8 secbuf[512];
u8 blkbuf[3200];
u8 pdbuf[3200 * pdblks];

void atod (u8 *d, const u8 *a, int n)
{
    int i;
    
    for (i = 0; i < n; i++)
    {
        if (*a)
            *d++ = asciiToCdc[*a++];
        else
            *d++ = 0;
    }
}

void dtoa (u8 *a, const u8 *d, int n)
{
    int i;
    
    for (i = 0; i < n; i++)
        *a++ = cdcToAscii[*d++];
}

void setbytes (u8 *p, u64 val, int bytes)
{
    int i;
    
    for (i = bytes - 1; i >= 0 ; i--)
    {
        p[i] = val & 077;
        val >>= 6;
    }
}
#define set60(to,from) setbytes(to,from,10)

void getbytes (u64 *val, const u8 *p, int bytes)
{
    int i;
    u64 t = 0;
    
    for (i = 0; i < bytes ; i++)
    {
        t <<= 6;
        t |= *p++;
    }
    *val = t;
}
#define get60(to,from) getbytes(&to,from,10)

int readmsec (int sec, u8 *buf)
{
    int i;
    u16 p1, p2;
    u8 *p;
    
    fseek (md, sec * 480, SEEK_SET);
    i = fread (secbuf, 480, 1, md);
    if (i == 0)
        return 0;
    
    p = secbuf;
    for (i = 0; i < 480; i += 3)
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
    return 1;
}

void writesec (int sec, const u8 *buf)
{
    int i, j;
    const u8 *bb = buf;
    u16 p1, p2;
    u8 *p;
    
    p = secbuf;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    for (i = 2; i < 0502; i += 2)
    {
        if ((*buf | buf[1] | buf[2] | buf[3]) & 0300)
            printf ("buffer format error at %p\n", buf);
        
        p1 = *buf++ << 6;
        p1 += *buf++;
        p2 = *buf++ << 6;
        p2 += *buf++;
        *p++ = p1 >> 4;
        *p = p1 << 4;
        *p++ |= p2 >> 8;
        *p++ = p2;
    }
#if 0
    p1 = 0;
    for (i = 0; i < 640; i++)
        p1 |= bb[i];
    if (p1 != 0)
    {
        printf ("sector %d\n", sec);
        for (i = 0; i < 640; i += 20)
        {
            printf ("%04o/", i/2);
            for (j = 0; j < 20; j++)
            {
                if (j % 10 == 0)
                    putchar (' ');
                printf ("%02o", bb[i+j]);
                if (j & 1)
                    putchar (' ');
            }
            printf ("\n");
        }
    }
#endif
    fseek (pd, sec * 512, SEEK_SET);
    if (fwrite (secbuf, 512, 1, pd) != 1)
    {
        perror ("write");
    }
    
}

int readmblk (int blk, u8 *buf)
{
    int i;
    
    for (i = 0; i < 5; i++)
        if (!readmsec (blk * 5 + i, buf + (640 * i)))
            return 0;
    return 1;
}

void writeblk (int blk, const u8 *buf)
{
    int i;
    
    for (i = 0; i < 5; i++)
        writesec (blk * 5 + i, buf + (640 * i));
}


int main (int argc, char **argv)
{
    int type, i, f;
    char *p;
    int blk;
    packdir *pdir;
    u64 inflth;
    u64 ftotal;
    u64 dirblks;
    u8 *np, *ip;
    char fname[11];
    u64 finfo;
    int cyl;
    int part;
    
    if (argc < 3)
    {
        printf ("too few args\n");
        exit (1);
    }
    md = fopen(argv[1], "rb");
    if (md == NULL)
    {
        perror ("master open");
        exit (1);
    }
    pd = fopen(argv[2], "wb");
    if (pd == NULL)
    {
        perror ("plato create");
        exit (1);
    }
    for (i = 0; i < mpdblks; i++)
        readmblk (i, pdbuf + (3200 * i));
    pdir = (packdir *) pdbuf;
    atod (pdir->packtype, backup, 10);
    getbytes (&ftotal, pdir->ftotal, 3);
    get60 (inflth, pdir->inflth);
    np = pdbuf + (10 * inflth);
    ip = np + (10 * ftotal);
    for (;;)
    {
        get60 (finfo, ip);
        memset (fname, 0, sizeof (fname));
        dtoa (fname, np, 10);
        cyl = (finfo >> 12) & 07777;
        blk = finfo & 07777;
        part = ((cyl * 91) + blk) / 7;
        part += pdspace - mpdspace;
        finfo = (finfo & ~077777777ULL) | part;
        set60 (ip, finfo);
        printf ("%10.10s %020llo (%d %d)\n", fname, finfo, cyl, blk);
        if (cyl == 0 && blk == 0)
            break;
        np += 10;
        ip += 10;
    }
    for (i = 0; i < mpdblks; i++)
        writeblk (pdstart + i, pdbuf + (3200 * i));
    
    blk = mpdspace * dsblks;
    while (readmblk (blk, blkbuf))
    {
        writeblk ((pdspace - mpdspace) * dsblks + blk, blkbuf);
        blk++;
    }

    fclose (pd);
    fclose (md);
}
