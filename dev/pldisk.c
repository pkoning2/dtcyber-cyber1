#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define hasflawtb 0

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
#define pdstart ((hasflawtb) ? 12 : 1)          // starting block num of directory
#define pdspace ((pdblks+pdstart+dsblks-1)/dsblks) // # parts taken up by pdir
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

#define ttabs   015253545556575000000ULL // tutor tabs--9 + 8
#define ctabs   017264200000000000000ULL // compass tabs--11,18,30

#define ttype "tutor    a"
#define rtype "router   a"
#define ctype "compass  e"

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
    /* 00-07 */ ':',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
    /* 10-17 */ 'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
    /* 20-27 */ 'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
    /* 30-37 */ 'X',    'Y',    'Z',    '0',    '1',    '2',    '3',    '4',
    /* 40-47 */ '5',    '6',    '7',    '8',    '9',    '+',    '-',    '*',
    /* 50-57 */ '/',    '(',    ')',    '$',    '=',    ' ',    ',',    '.',
    /* 60-67 */ '#',    '[',    ']',    '%',    '"',	'_',	'!',	'&',
    /* 70-77 */ '\'',   '?',    '<',    '>',    '@',    '\\',   '^',    ';'
    };

const char date[] =   " 09/02/03.";
const char *ptype;
char pname[10];
int aidsflag = 0;

cw ccode;
cw icode;
cw account;
cw useles;
cw ftype;
cw ftabs;

const char types[][10] = { master, backup, general, binary };

FILE *pd;
u8 secbuf[512];
u8 blkbuf[3200];
u8 pdbuf[3200 * pdblks];
int fused, sused;
int nextpart = pdspace;     // next space number to allocate

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

void setcode (u8 *to, const char *from)
{
    memset (to, 0, 10);
    to += 2;
    if (*from == '0')
        return;
    atod (to, from, 8);
}

void readsec (int sec, u8 *buf)
{
    int i;
    u16 p1, p2;
    u8 *p;
    
    fseek (pd, sec * 512, SEEK_SET);
    fread (secbuf, 512, 1, pd);
    p = secbuf;
    for (i = 0; i < 0502; i += 2)
    {
        p1 = *p++ << 4;
        p1 += *p >> 4;
        p2 = *p++ << 8;
        p2 += *p++;
        if (i < 2)
            continue;
        *buf++ = p1 >> 6;
        *buf++ = p1 & 077;
        *buf++ = p2 >> 6;
        *buf++ = p2 & 077;
    }
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

void readblk (int blk, u8 *buf)
{
    int i;
    
    for (i = 0; i < 5; i++)
        readsec (blk * 5 + i, buf + (640 * i));
}

void writeblk (int blk, const u8 *buf)
{
    int i;
    
    for (i = 0; i < 5; i++)
        writesec (blk * 5 + i, buf + (640 * i));
}

void initflaw (void)
{
    memset (blkbuf, 0, sizeof (blkbuf));
    atod (blkbuf, fflag, sizeof (fflag));
    set60 (blkbuf + 10, SERIAL);
    atod (blkbuf + 20, date, 10);
    writeblk (flawtb, blkbuf);
}

static int curspart;
static char curname[11];

void extpf (int parts, int nparts)
{
    int bwrd, bbyt, bit, m;
    int i;
    int entry;
    u64 finfo;
    packdir *pd;
    int spart = curspart + parts;
    
    entry = pinflth + fused - 1 + pfiles;
    pd = (packdir *) pdbuf;
    bwrd = spart / pnbits;
    bit = spart % pnbits;
    bbyt = 2 + bit / 6;
    bit = bit % 6;
    m = 040 >> bit;
    for (i = parts; i < nparts; i++)
    {
        if ((pdbuf[50 + (bwrd * 10) + bbyt] & m) == 0)
        {
            printf ("space already in use, file %s, part %d\n",
                    curname, i);
            exit (1);
        }
        pdbuf[50 + (bwrd * 10) + bbyt] &= ~m;
        m >>= 1;
        bit++;
        if (m == 0)
        {
            m = 040;
            bit = 0;
            bbyt++;
            if (bbyt == 10)
            {
                bwrd++;
                bbyt = 2;
            }
        }
    }
    get60 (finfo, pdbuf + (entry * 10));
    finfo = (finfo & ~(077ULL << 24)) | (nparts << 24);
#if 0
    printf ("new finfo %020llo at %d\n", finfo, entry);
#endif
    set60 (pdbuf + (entry * 10), finfo);
    sused += nparts - parts;
    setval (pd->sused, sused);
}

void cpf (const char *name, int type, int parts, int spart)
{
    int entry;
    u64 finfo;
    packdir *pd;
    
    strcpy (curname, name);
    curspart = spart;
    pd = (packdir *) pdbuf;
    entry = pinflth + fused;
    fused++;
    atod (pdbuf + (entry * 10), name, 10);
    if (fused > 1 &&
        memcmp (pdbuf + (entry * 10), pdbuf + ((entry - 1) * 10), 10) <= 0)
        printf ("file %s is out of alphabetical order\n", name);
    entry += pfiles;
    finfo = ((u64) type << 30) + spart;
#if 0
    printf ("initial finfo %020llo at %d\n", finfo, entry);
#endif
    set60 (pdbuf + (entry * 10), finfo);
    extpf (0, parts);
    setval (pd->fused, fused);
}

void initpd (void)
{
    int i;
    u64 b = 020607777777777777777ULL;
    packdir *pd;
    
    memset (pdbuf, 0, sizeof (pdbuf));
    pd = (packdir *) pdbuf;
    setstr (pd->packname, pname);
    setstr (pd->packtype, ptype);
    setval (pd->stotal, pspaces);
    setval (pd->ftotal, pfiles);
    setval (pd->inflth, pinflth);
    for (i = 0; i < pbitlth - 1; i++)
        set60 (pdbuf + 50 + (i * 10), b);
    b &= ~((1ULL << (pnbits * (pbitlth - 1)) - pspaces) - 1);
    set60 (pdbuf + 50 + (pbitlth - 1) * 10, b);
    atod (pdbuf + 60 + 10 * pbitlth, date, 10);
    set60 (pdbuf + 70 + 10 * pbitlth, SERIAL);
}

void finishpd (void)
{
    int i;
    
    // This one must be last (for alphabetic order)
    cpf (pdname, 077, pdspace, 0);
    
    for (i = 0; i < pdblks; i++)
        writeblk (pdstart + i, pdbuf + 3200 * i);
}

static char line[400];
static leshdr lh;
#define WMAX 300
#define UMIN 250
#define MAXPARTS 18
#define MAXCOM 20

void copyfile (const char *fn)
{
    FILE *inf;
    int blk = 1, words = 0, unitw = -1, unitpos = -1;
    int lastaunit = -2;
    int units = 0, aunits = 0;
    int i, j, llen, lw;
    int parts = 1;
    char blkname[11] = "x";
    char endblk = 0;
    char *p;
    int partial = 0;
    int startblk;
    int commons = 0;
    int comlth[MAXCOM];
    cw comname[MAXCOM];
    char cname[100];
    char uname[10];
    
    inf = fopen (fn, "r");
    if (inf == NULL)
    {
        perror ("input file open");
        return;
    }
    startblk = nextpart * dsblks;
    cpf (fn, ftype[9], parts, nextpart);
    nextpart += parts;
    memset (&lh, 0, sizeof (lh));
    setstr (lh.lname, fn);
    memcpy (lh.ftype, ftype, 10);
    setstr (lh.bname[0], "*directory");
    memset (blkbuf, 0, sizeof (blkbuf));
    memcpy (lh.ei.ccode, ccode, 10);
    memcpy (lh.ei.comcode, ccode, 10);
    memcpy (lh.ei.ucode, ccode, 10);
    memcpy (lh.ei.jcode, ccode, 10);
    memcpy (lh.ei.icode, icode, 10);
    memcpy (lh.ei.useles, useles, 10);
    memcpy (lh.ei.acct, account, 10);
    memcpy (lh.ei.tabset, ftabs, 10);
    for (;;)
    {
        fgets (line, sizeof (line), inf);
        if (feof (inf))
            break;
        llen = strlen (line) - 1;
        if (line[llen] != '\n')
        {
            printf ("line too long in %s:\n %s\n", fn, line);
            continue;
        }
        line[llen] = '\0';
        lw = (llen + 11) / 10;  // number of words -- note that 9 chars is 2 words
        if (line[0] && line[0] < ' ')
        {
            // control character, see what this tells us
            p = &line[1];
            switch (line[0] + 96)
            {
            case 'l':
                endblk = 1;
                lw = llen = unitw = 0;
                break;
            case 'n':
                if (*p == ('n' & 31))
                {
                    partial = 1;
                    p++;
                }
                else
                    partial = 0;
                strncpy (blkname, p, 10);
                continue;
            case 'u':
                setstr (lh.ei.useles, p);
                continue;
            case 'c':
                if (commons == MAXCOM + 1)
                {
                    printf ("too many commons at -c%s\n", p);
                    break;
                }
                sscanf (p, "%s %d", cname, &i);
#if 0
                printf ("common %s %d words\n", cname, i);
#endif
                strncpy (comname[commons], cname, 10);
                comlth[commons] = i;
                commons++;
                continue;
            default:
                printf ("unrecognized control char ctrl/%c, line:\n %s\n", 
                        line[0] + 96, line);
                continue;
            }
        }
        if (blk >= MAXPARTS * dsblks)
        {
            printf ("ran out of space, file %s, line is:\n %s\n", fn, line);
            words = 0;
            break;
        }
        if (blk >= parts * dsblks)
        {
            parts++;
            nextpart++;
            extpf (parts - 1, parts);
        }
        if (strncmp (line, "unit    ", 8) == 0)
        {
            units++;
            unitpos = words;
            memset (uname, 0, sizeof (uname));
            strncpy (uname, line + 8, sizeof (uname) - 1);
            if (words > UMIN)
                unitw = words;
        }
        if (aidsflag)
        {
            if (strncmp (line, "*end ", 5) == 0 ||
                strcmp (line, "*end") == 0)
            {
                endblk = 1;
                unitw = 0;
#if 0
                if (unitpos < 0)
                    printf ("*end but no unit\n");
#endif
            }
            else if (lastaunit != unitpos &&
                     (strncmp (line, "*titlet ", 8) == 0 ||
                      strncmp (line, "*back1t ", 8) == 0 ||
                      strncmp (line, "*backt ",  7) == 0 ||
                      strncmp (line, "*next1t ", 8) == 0 ||
                      strncmp (line, "*nextt ",  7) == 0))
            {
                aunits++;
                lastaunit = unitpos;
                if (units > aunits)
                {
                    endblk = 2;
                    unitw = unitpos;
                }
                if (aunits == 1)
                    strcpy (blkname, uname);
                partial = 1;
            }
        }
        atod (blkbuf + (10 * words), line, llen);
#if 0
        printf ("%d(%d) %s\n", words, lw, line);
#endif
        words += lw;
        if (words > WMAX || endblk)
        {
            if (unitw > 0)
            {
                if (aunits > 0) 
                    aunits = 1;
                units = 1;
                lw = unitw;
                unitpos = 0;
                lastaunit = 0;
            }
            else
            {
                units = aunits = 0;
                lw = words;
                unitpos = -1;
                lastaunit = -2;
            }
            setval (lh.binfo[blk], 
                    (((u64) partial) << 59) |
                    (1 << 18) | (lw << 9) | blk);
            setstr (lh.bname[blk], blkname);
#if 0
            printf ("block %d (%s), %d words\n", blk, blkname, lw);
#endif
            writeblk (startblk + blk, blkbuf);
            if (endblk == 2)
            {
                // special flag for aids case, set next block name
                strcpy (blkname, uname);
                partial = 1;
            }
            endblk = 0;
            unitw = -1;
            blk++;
            if (lw < words)
            {
                memmove (blkbuf, blkbuf + (10 * lw), (words - lw) * 10);
                words -= lw;
                memset (blkbuf + (10 * words), 0, sizeof (blkbuf) - (10 * words));
            }
            else
            {
                words = 0;
                memset (blkbuf, 0, sizeof (blkbuf));
            }
        }
    }
    if (words > 0)
    {
#if 0
        printf ("block %d (%s), %d words\n", blk, blkname, words);
#endif
        setval (lh.binfo[blk], 
                (((u64) partial) << 59) |
                (1 << 18) | (words << 9) | blk);
        setstr (lh.bname[blk], blkname);
        writeblk (startblk + blk, blkbuf);
        blk++;
    }
    memset (blkbuf, 0, sizeof (blkbuf));
    for (i = 0; i < commons; i++)
    {
        lw = (comlth[i] + blklth - 1) / blklth;
        for (j = 0; j < lw; j++)
        {
            setval (lh.binfo[blk], 
                    (041ULL << 54) |
                    (((j == 0) ? lw : 0) << 18) |
                    (blklth << 9) | blk);
            setstr (lh.bname[blk], comname[i]);
#if 0
            printf ("common block %s, %d words, %d blks\n",
                    comname[i], comlth[i], lw);
#endif
            if (blk >= MAXPARTS * dsblks)
            {
                printf ("ran out of space, file %s, common %s\n", fn, comname[i]);
                i = MAXCOM;
                break;
            }
            if (blk >= parts * dsblks)
            {
                parts++;
                nextpart++;
                extpf (parts - 1, parts);
            }
            writeblk (startblk + blk, blkbuf);
            blk++;
        }
    }
    while (blk % 7 != 0)
    {
        setval (lh.binfo[blk], blk);
        blk++;
    }
    setval (lh.blocks, blk);
    setval (lh.lastblk, (1ULL << 59) | (blk - 1));
    writeblk (startblk, (char *) &lh);
    fclose (inf);
#if 1
    printf ("file %s, %d blocks\n", fn, blk);
#endif
}

int main (int argc, char **argv)
{
    int type, i, f;
    char *p;
    
    if (argc < 4)
    {
        printf ("too few args\n");
        exit (1);
    }
    type = tolower(*argv[3]) - 'a';
    if (type > 3)
    {
        printf ("invalid pack type %s\n", argv[3]);
        exit (1);
    }
    ptype = types[type];
    memset (pname, ':', 10);
    i = strlen (argv[2]);
    if (i > 10)
    {
        printf ("invalid pack name %s\n", argv[2]);
        exit (1);
    }
    memcpy (pname, argv[2], i);
    
    pd = fopen(argv[1], "r+b");
    if (pd == NULL)
    {
        perror ("open");
        exit (1);
    }
    initflaw ();
    initpd ();
    // set all the defaults
    setcode (ccode, "s");
    setcode (icode, "0");
    setstr (account, "system");
    setstr (ftype, ttype);
    setval (ftabs, ttabs);
    
    for (f = 4; f < argc; f++)
    {
        p = argv[f];
        if (*p == '-')
        {
            p++;
            switch (tolower (*p++))
            {
            case 'c': 
                setcode (ccode, p);
                break;
            case 'd':
                aidsflag = 1;
                break;
            case 'i':
                setcode (icode, p);
                break;
            case 'a':
                setstr (account, p);
                break;
            case 'u':
                setstr (useles, p);
                break;
            case 't':
                aidsflag = 0;
                switch (tolower (*p))
                {
                case 't':
                    setstr (ftype, ttype);
                    setval (ftabs, ttabs);
                    break;
                case 'r':
                    setstr (ftype, rtype);
                    setval (ftabs, ttabs);
                    break;
                case 'c':
                    setstr (ftype, ctype);
                    setval (ftabs, ctabs);
                    break;
                default:
                    printf ("unrecognized switch -t%c ignored\n", *p);
                }
                break;
            default:
                printf ("unrecognized switch %s ignored\n", p - 2);
            }
        }
        else
            copyfile (argv[f]);
    }
    finishpd ();
    fclose (pd);
}
