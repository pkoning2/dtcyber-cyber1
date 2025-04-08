#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define k_sb    5           // sectors per block     (5)
#define k_st    24          // sectors per track     (24)
#define k_tc    19          // tracks  per cylinder  (19)
#define k_sc    (k_st*k_tc) // sectors per cylinder  (456)
#define k_cp    410         // cylinders per pack    (410)
#define k_bc    91          // blocks  per cylinder  (91)
#define k_ws    64          // words per sector      (64)
#define blklth  320         // length of a disk block
#define dsblks  7           // blocks per lesson space
#define pdblks  16          // num blocks in pack directory
#define pdstart 12          // starting block num of directory
#define pdspace ((pdblks+pdstart+dsblks-1)/dsblks) // # parts taken up by pdir
#define mpdblks  7          // num blocks in master file pack directory
#define mpdspace 1          // # parts for pdir in master file
#define maxparts 18         // maximum number of parts for a file
#define lextra   60         // number of extra info words in old format
#define flawtb  11          // starting block num of flaw table
#define pdname  "4;;;;;;;;;" // pack dir "name"
#define master  "master   a" // pack type codes
#define backup  "backup   b"
#define general "general  c"
#define binary  "binary   d"
#define pnbits  48         // number of bits/word in table
#define pfiles  2500       // number files allowed / pack
#define pspaces (k_sc*k_cp/(k_sb*dsblks))  // spaces/pack
#define pextra  2          // number extra info words
#define pbitlth ((pspaces/pnbits)+2)   // lth of bit table
#define pinflth (5+pbitlth+pextra)    // length of pack info

#define fflag   "flaws"     // flaw table identifier
#define SERIAL  1

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

typedef struct 
{
    cw cdate;
    cw mdate;
    cw ccode;
    cw icode;
    u8 author[30];
    u8 dept[20];
    u8 tel[20];
    u8 subj[20];
    u8 aud[20];
    u8 desc[80];
    cw mtime;
    u8 mauth[20];
    u8 mgrp[10];
    cw comcode;
    cw ucode;
    cw jcode;
    cw charsetles;
    cw charset;
    cw microles;
    cw micro;
    cw flags;
    cw useles;
    cw acct;
    cw tabset;
    cw studcomment;
    cw ltype;
    cw catalog;
    cw datawrt;
    cw dataread;
    cw mastercard;
    cw procles;
    cw runsite;
    cw lasteditles;
    cw cdcres;
    cw editles;
    cw uplevel;
    cw unused[7];
    cw nextdatafilename;
    cw namesize;
    cw recsize;
} extrai;

typedef struct 
{
    cw lname;
    cw ftype;
    cw blocks;
    cw lastblk; // and "new" flag set in sign bit
    extrai ei;
    cw binfo[128];
    cw bname[128];
} leshdr;

typedef struct 
{
    cw lname;
    cw ftype;
    cw blocks;
    cw lastblk; // and "new" flag set in sign bit
} oldleshdr;

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
    /* 00-07 */ 000,    'a',    'b',    'c',    'd',    'e',    'f',    'g',
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
u8 mpdbuf[3200 * mpdblks];
u8 pdbuf[3200 * pdblks];
u8 filebuf[3200 * dsblks * maxparts];

const char date[] =   " 10/15/03.";
const char ptype[] = backup;
char pname[10];
int fused, sused;
int nextpart = pdspace;     // next space number to allocate

// Convert ascii to display code, n character field.
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

// Convert display code to ascii, n character field.
// Trailing 0 (colon) characters are stripped.
void dtoa (u8 *a, const u8 *d, int n)
{
    int i;
    
    for (i = 0; i < n; i++)
        *a++ = cdcToAscii[*d++];
    while (*--a == ':')
        *a = '\0';
}

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

int readmblks (int sblk, u8 *buf, int cnt)
{
    int i;
    
    for (i = 0; i < cnt; i++)
        if (!readmblk (sblk + i, buf + (3200 * i)))
            return 0;
    return 1;
}

void writeblks (int sblk, const u8 *buf, int cnt)
{
    int i;
    
    for (i = 0; i < cnt; i++)
        writeblk (sblk + i, buf + (3200 * i));
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
    char pn[11], pt[11];
    u64 finfo;
    int cyl;
    int part;
    int size;
    oldleshdr *olh;
    int startblk;
    
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
    for (i = 0; ; i++)
    {
        if (!readmblk (i, blkbuf))
            break;
        if (i == 1)
        {
            pdir = (const packdir *) blkbuf;
            pn[10] = pt[10] = '\0';
            dtoa (pn, pdir->packname, 10);
            dtoa (pt, pdir->packtype, 10);
            printf ("pack %s type %s\n", pn, pt);
            setstr (pdir->packtype, ptype);
            printf ("pack type changed to %s\n", ptype);
        }
        
        writeblk (i, blkbuf);
    }
    
    fclose (pd);
    fclose (md);
}
