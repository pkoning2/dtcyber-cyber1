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
    writeblks (pdstart, pdbuf, pdblks);
}

// size is in parts
void convertles (int *size)
{
    oldleshdr *o;
    leshdr n;
    u64 blks, t1;
    int tblks;
    u8 *p, *src;
    int srclen;
    int i, j;
    int newblks;
    int cblks, b;
    u64 binfo;
    
    o = (oldleshdr *) filebuf;
    memset (&n, 0, sizeof (n));
    memcpy (&n, o, sizeof (*o));
    get60 (blks, o->lastblk);
    tblks = *size * dsblks;
    if (++blks >= tblks)
    {
        if (*size == 18)
        {
            printf (" cannot be lengthened, not changed ");
            return;
        }
        printf (" lengthened... ");
        cblks = tblks;
        *size += 1;
        newblks = 6;
        t1 = *size * dsblks;
        set60 (n.blocks, t1);
    }
    else
    {
        cblks = tblks - 1;
        newblks = 0;
    }
    blks |= 1ULL << 59;
    set60 (n.lastblk, blks);
    p = ((u8 *) o) + 5 * sizeof (cw);
    memcpy (&n.binfo[1], p, cblks * sizeof (cw));
    j = 1;
    for (i = 0; i < cblks; i++)
    {
        get60 (binfo, p + (i * sizeof (cw)));
        b = binfo & 0777;
        if (i == 0)
        {
            if (b != 0)
            {
                printf ("\nstrange first binfo: %020o\n", binfo);
            }
        }
        if ((binfo & ~(1ULL << 59)) == cblks && newblks == 0)
            continue;
        if (b >= cblks)
        {
            printf ("\nstrange block number in binfo %020o\n", binfo);
        }
        binfo++;
        set60 (n.binfo[j++], binfo);
    }
    p += tblks * sizeof (cw);
    memcpy (&n.bname[1], p, cblks * sizeof (cw));
    setstr (n.bname[0], "*directory");
    p += tblks * sizeof (cw);
    memcpy (&n.ei, p, sizeof (extrai));
    // clear out account since that doesn't carry over well to another system!
    memset (n.ei.acct, 0, sizeof (n.ei.acct));
    p += 60 * sizeof (cw);
    get60 (binfo, n.binfo[1]);
    memmove (filebuf + 6400, filebuf + 3200, (cblks - 1) * 3200);
    memset (filebuf + 3200, 0, 3200);
    memcpy (filebuf + 3200, p, ((binfo >> 9) & 0777) * sizeof (cw));
    for (i = cblks + 1; i < cblks + 1 + newblks; i++)
    {
        binfo = i;
        set60 (n.binfo[i], binfo);
    }
    memset (filebuf, 0, 3200);
    memcpy (filebuf, &n, sizeof (n));
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
    readmblks (0, mpdbuf, mpdblks);
    pdir = (packdir *) mpdbuf;
    dtoa (pname, pdir->packname, 10);
    getbytes (&ftotal, pdir->ftotal, 3);
    get60 (inflth, pdir->inflth);
    np = mpdbuf + (10 * inflth);
    ip = np + (10 * ftotal);
    initflaw ();
    initpd ();
    pdir = (packdir *) pdbuf;
    for (;;)
    {
        get60 (finfo, ip);
        memset (fname, 0, sizeof (fname));
        dtoa (fname, np, 10);
        np += 10;
        ip += 10;
        cyl = (finfo >> 12) & 07777;
        blk = finfo & 07777;
        if (cyl == 0 && blk == 0)
            break;
        part = ((cyl * 91) + blk) / 7;
        size = (finfo >> 24) & 077;
        if (size > maxparts)
        {
            printf ("oversized file %s (%d parts)\n", fname, size);
            continue;
        }
        if (!readmblks (part * 7, filebuf, size * 7))
        {
            printf ("*** error reading %s\n", fname);
            continue;
        }
        olh = (oldleshdr *) filebuf;
        if (olh->ftype[9] == 1 &&
            (olh->lastblk[0] & 040) == 0)
        {
            printf ("converting %s... ", fname);
            convertles (&size);
            printf (" now %d parts\n", size);
        }
        else printf ("copying %s, %d parts\n", fname, size);
        startblk = nextpart * dsblks;
        cpf (fname, olh->ftype[9], size, nextpart);
        writeblks (startblk, filebuf, size * 7);
        nextpart += size;
    }
    finishpd ();
    fclose (pd);
    fclose (md);
}
