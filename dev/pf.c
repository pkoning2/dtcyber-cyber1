/*--------------------------------------------------------------------------
**
**  Copyright (c) 2004, Paul Koning (see license.txt)
**
**  Name: pf.c
**
**  Description:
**      Roughly the equivalent of the PLATO *pf* control card.
**      In other words, this program will read and write PLATO
**      files on a CERL-format dd844-21 disk image.
**      Optionally, it will instead read/write new format master files.
**      Unlike pldisk.c, it will create or modify files on an
**      existing disk image -- pldisk only creates and populates
**      new disk images from scratch. 
**
**--------------------------------------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#define k_sb    5           // sectors per block     (5)
#define k_st    24          // sectors per track     (24)
#define k_tc    19          // tracks  per cylinder  (19)
#define k_sc    (k_st*k_tc) // sectors per cylinder  (456)
#define k_cp    410         // cylinders per pack    (410)
#define k_bc    91          // blocks  per cylinder  (91)
#define k_ws    64          // words per sector      (64)
#define blklth  320         // length of a disk block
#define dsblks  7           // blocks per lesson space
#define pdmax   40          // max blocks in pack directory
#define pdstart ((mf) ? 1 : 12)          // starting block num of directory
#define pdspace ((pdblks+pdstart+dsblks-1)/dsblks) // # parts taken up by pdir
#define flawtb  11          // starting block num of flaw table
#define pdname  "4;;;;;;;;;" // pack dir "name"
#define pnbits  48         // number of bits/word in table
//#define pfiles  2500       // number files allowed / pack
#define pspaces (k_sc*k_cp/(k_sb*dsblks))  // spaces/pack
#define pextra  2          // number extra info words
#define pbitlth ((pspaces/pnbits)+2)   // lth of bit table
//#define pinflth (5+pbitlth+pextra)    // length of pack info

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
typedef enum { false, true } bool;

typedef u8 cw[10];
typedef u8 w18[3];
typedef u8 w12[2];

#define getval(to,from) getbytes (&to, from, sizeof (from))
#define setval(to,from) setbytes (to, from, sizeof (to))
#define setstr(to,from) atod (to, from, sizeof (to))

typedef void infoparsefun (const char *, int, int);
typedef void infoprintfun (FILE *, const char *, int, int);

typedef struct 
{
    const char *label;
    infoparsefun *parsefun;
    infoprintfun *printfun;
    int arg1;
    int arg2;
} infoitem;

infoparsefun inforeadstr, inforeadedate, inforeadeby, inforeadeat;
infoparsefun inforeadcode, inforeadacc;
infoprintfun infoputstr, infoputedate, infoputeby, infoputeat;
infoprintfun infoputcode, infoputacc;

void usage (void);

#define OFFSET_OF(__CLASS__,__FIELD__) ((size_t)(&((__CLASS__*)0)->__FIELD__))

typedef enum { None, Read, Write, Create, Packinit, List } action;


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

#define OFF(x) OFFSET_OF (leshdr, x)
#define SEL(x,y) (((x) << 8) + (y))

const infoitem infolist[] = 
{ { "lesson name ------ ", inforeadstr, infoputstr, OFF(lname), 10 },
  { "account ---------- ", inforeadstr, infoputstr, OFF(ei.acct), 7 },
  { "last edited ------ ", inforeadedate, infoputedate, OFF(ei.mdate), OFF(ei.mtime) },
  { "         by ------ ", inforeadeby, infoputeby, OFF(ei.mauth), OFF(ei.mgrp) },
  { "         at ------ ", inforeadeat, infoputeat, OFF(ei.mauth)+10, 0 },
  { "change code ------ ", inforeadcode, infoputcode, OFF(ei.ccode), 0 },
  { "inspect code ----- ", inforeadcode, infoputcode, OFF(ei.icode), 0 },
  { "common code ------ ", inforeadcode, infoputcode, OFF(ei.comcode), 1 },
  { "file access code - ", inforeadcode, infoputcode, OFF(ei.comcode), 5 },
  { "-use- code ------- ", inforeadcode, infoputcode, OFF(ei.ucode), 1 },
  { "-jumpout- code --- ", inforeadcode, infoputcode, OFF(ei.jcode), 1 },
  { "-attach- code ---- ", inforeadcode, infoputcode, OFF(ei.datawrt), 1 },
  { "system access ---- ", inforeadacc, infoputacc, OFF(ei.flags), 10 },
  { "-use-d lesson ---- ", inforeadstr, infoputstr, OFF(ei.useles), SEL(1,10) },
  { "submit block ----- ", inforeadstr, infoputstr, OFF(ei.useles), SEL(5,10) },
  { "charset lesson --- ", inforeadstr, infoputstr, OFF(ei.charsetles), 10 },
  { "charset block ---- ", inforeadstr, infoputstr, OFF(ei.charset), 10 },
  { "micro lesson ----- ", inforeadstr, infoputstr, OFF(ei.microles), 10 },
  { "micro block ------ ", inforeadstr, infoputstr, OFF(ei.micro), 10 },
  { "notes file ------- ", inforeadstr, infoputstr, OFF(ei.studcomment), 10 },
  { "processor -------- ", inforeadstr, infoputstr, OFF(ei.procles), 10 },
  { "editor ----------- ", inforeadstr, infoputstr, OFF(ei.editles), 10 },
  { "author name ------ ", inforeadstr, infoputstr, OFF(ei.author), 30 },
  { "author dept. ----- ", inforeadstr, infoputstr, OFF(ei.dept), 20 },
  { "author phone ----- ", inforeadstr, infoputstr, OFF(ei.tel), 20 },
  { "subject ---------- ", inforeadstr, infoputstr, OFF(ei.subj), 20 },
  { "audience --------- ", inforeadstr, infoputstr, OFF(ei.aud), 20 },
  { "description ------ ", inforeadstr, infoputstr, OFF(ei.desc), 80 },
};


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
    /* 00-07 */ ':',    'a',    'b',    'c',    'd',    'e',    'f',    'g',
    /* 10-17 */ 'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
    /* 20-27 */ 'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
    /* 30-37 */ 'x',    'y',    'z',    '0',    '1',    '2',    '3',    '4',
    /* 40-47 */ '5',    '6',    '7',    '8',    '9',    '+',    '-',    '*',
    /* 50-57 */ '/',    '(',    ')',    '$',    '=',    ' ',    ',',    '.',
    /* 60-67 */ '#',    '[',    ']',    '%',    '"',	'_',	'!',	'&',
    /* 70-77 */ '\'',   '?',    '<',    '>',    '@',    '\\',   '^',    ';'
    };

char date[11];
const char *ptype;
char pname[10];
int aidsflag = 0;
bool force;
bool topipe;
bool sourceonly;
bool verbose;
int parts;
int mf = 0;
int moddel = 0;
int modword = 0;

cw ccode;
cw icode;
cw account;
cw useles;
cw ftype;
cw ftabs;

const char types[][10] = 
{ "master   a",
  "backup   b",
  "general  c",
  "binary   d" };

const char *btypes[] =
{ "source",
  "com", 
  "chars", 
  "micro", 
  "llist", 
  "vocab", 
  "lines", 
  "(7)",
  "binary",
  "list",
};

FILE *pdisk;
FILE *logfile;

u8 secbuf[512];
u8 blkbuf[3200];
u8 lhbuf[3200];
leshdr * const lh = (leshdr * const) lhbuf;
u8 pdbuf[3200 * pdmax];
packdir * const pd = (packdir * const) pdbuf;
u64 fused, ftotal, sused, pinflth;
int pdblks;
int nextpart;       // next space number to allocate

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


// Put a string value into the lesson directory.
// If second argument is x * 256 + n, x is the lesson type code
// to which this entry applies.
void inforeadstr (const char *value, int offset, int count)
{
    if (count >> 8 != 0 &&
        (count >> 8) != ftype[9])
        return;
    atod (lhbuf + offset, value, count & 0xff);
}

void inforeadedate (const char *value, int offsetdate, int offsettime)
{
}

void inforeadeby (const char *value, int offsetauth, int offsetgrp)
{
}

void inforeadeat (const char *value, int offset, int count)
{
}

void inforeadcode (const char *value, int offset, int type)
{
    if (type != 0 &&
        type != ftype[9])
        return;

}

void inforeadacc (const char *value, int offset, int x)
{
}

// Print a string value from the lesson directory
// If second argument is x * 256 + n, x is the lesson type code
// to which this entry applies.
void infoputstr (FILE *outf, const char *label, int offset, int count)
{
    char line[100];
    
    if (count >> 8 != 0 &&
        (count >> 8) != ftype[9])
        return;
    dtoa (line, lhbuf + offset, count & 0xff);
    line[count & 0xff] = '\0';
    fprintf (outf, "%s%s\n", label, line);
}

void infoputedate (FILE *outf, const char *label,
                   int offsetdate, int offsettime)
{
    char date[10], time[10];
    
    dtoa (date, lhbuf + offsetdate, 10);
    dtoa (time, lhbuf + offsettime, 10);
    date[9] = time[9] = '\0';
    fprintf (outf, "%s%s %s\n", label, date + 1, time);
}

void infoputeby (FILE *outf, const char *label, int offsetauth, int offsetgrp)
{
    char auth[19], grp[9];
    
    dtoa (auth, lhbuf + offsetauth, 18);
    dtoa (grp, lhbuf + offsetgrp, 8);
    auth[18] = grp[8] = '\0';
    fprintf (outf, "%s%s of %s\n", label, auth, grp);
}

void infoputeat (FILE *outf, const char *label, int offset, int count)
{
    u64 where;
    int stat;
    
    get60 (where, lhbuf + offset);
    stat = where & 07777;
    fprintf (outf, "%s%d-%d\n", label, stat >> 5, stat & 037);
}

void infoputcode (FILE *outf, const char *label, int offset, int type)
{
    char str[11];
    
    if (type != 0 &&
        type != ftype[9])
        return;

    dtoa (str, lhbuf + offset, 10);
    str[10] = '\0';
    if (str[0] == ':')
    {
        if (str[1] == ':')
        {
            if (str[2] == ':')
                fprintf (outf, "%saccount %s\n", label, str + 3);
            else
            {
                if (strcmp (str, "::'n'o'n'e") == 0)
                    fprintf (outf, "%snone\n", label);
                else
                    fprintf (outf, "%sgroup %s\n", label, str + 2);
            }
        }
        else
            fprintf (outf, "%s//////////\n", label);
    }
    else if (str[0] =='\0')
        fprintf (outf, "%sblank\n", label);
    else
        fprintf (outf, "%s**********\n", label);
}

void infoputacc (FILE *outf, const char *label, int offset, int x)
{
    u64 flags;
    
    get60 (flags, lhbuf + offset);
    if (flags & (1ULL << 59))
        fprintf (outf, "%srestricted\n", label);
    else
        fprintf (outf, "%sallowed\n", label);
}

void readsec (int sec, u8 *buf)
{
    int i;
    u16 p1, p2;
    u8 *p;
    
    if (mf)
    {
        fseek (pdisk, sec * 480, SEEK_SET);
        fread (secbuf, 480, 1, pdisk);
    }
    else
    {
        fseek (pdisk, sec * 512, SEEK_SET);
        fread (secbuf, 512, 1, pdisk);
    }
    p = secbuf;
    for (i = (mf) ? 2 : 0; i < 0502; i += 2)
    {
        p1 = *p++ << 4;
        p1 += *p >> 4;
        p2 = (*p++ & 0x0f) << 8;
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
    fseek (pdisk, sec * 512, SEEK_SET);
    if (fwrite (secbuf, 512, 1, pdisk) != 1)
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

void dumpblk (FILE *outf, int base)
{
    int w;
    u64 w1, w2;
    
    for (w = 0; w < blklth; w += 2)
    {
        get60 (w1, blkbuf + w * 10);
        get60 (w2, blkbuf + w * 10 + 10);
        if (w1 != 0 || w2 != 0)
        {
            if (base > 0)
                fprintf (outf, "%06o  %020llo     %020llo\n", w + base, w1, w2);
            else
                fprintf (outf, "%03o  %020llo     %020llo\n", w + base, w1, w2);
        }
    }
}

void printblk (FILE *outf, int len)
{
    int w, i, linelen;
    char line[400];
    u64 m;
    
    w = 0;
    while (w < len)
    {
        for (i = w; i < len; i++)
        {
            if (blkbuf[i * 10 + 8] == 0 &&
                blkbuf[i * 10 + 9] == 0)
                break;
        }
        linelen = (i - w) * 10 + 8;
        if (linelen > 399)
            linelen = 399;
        if (blkbuf[w * 10] == 0 &&
            blkbuf[w * 10 + 1] == 0 &&
            blkbuf[w * 10 + 9] != 0)
        {
            // Modword... decode it
            get60 (m, blkbuf + w * 10);
            if (((m >> 29) & 1) && !moddel)
            {
                // Deleted line, skip it
                w = i + 1;
                continue;
            }
            if (modword)
            {
                dtoa (line, blkbuf + w * 10 + 2, 5);
                line[5] = '\0';
                fprintf (outf, "%5s %c%02d.%02d.%02d ",
                         line, ((m >> 29) & 1) ? 'D' : ' ',
                         (((m >> 9) & 037) + 73) % 100,
                         (m >> 5) & 017, m & 037);
            }
            w++;
            linelen -= 10;
        }
        else if (modword)
            fprintf (outf, "                ");
        
        dtoa (line, blkbuf + w * 10, linelen);
        line[linelen] = '\0';
        fprintf (outf, "%s\n", line);
        w = i + 1;
    }
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
    int spart = curspart + parts;
    
    entry = pinflth + fused - 1 + ftotal;
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
    
    strcpy (curname, name);
    curspart = spart;
    entry = pinflth + fused;
    fused++;
    atod (pdbuf + (entry * 10), name, 10);
    if (fused > 1 &&
        memcmp (pdbuf + (entry * 10), pdbuf + ((entry - 1) * 10), 10) <= 0)
        printf ("file %s is out of alphabetical order\n", name);
    entry += ftotal;
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
    
    memset (pdbuf, 0, sizeof (pdbuf));
    setstr (pd->packname, pname);
    setstr (pd->packtype, ptype);
    setval (pd->stotal, pspaces);
    setval (pd->ftotal, ftotal);
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

void readpd (void)
{
    int i;
    
    readblk (pdstart, pdbuf);
    getval (fused, pd->fused);
    getval (ftotal, pd->ftotal);
    getval (sused, pd->sused);
    get60 (pinflth,pd->inflth);
    
    pdblks = (pinflth + (ftotal * 2) + 320 - 1) / 320;
    if (pdblks > pdmax)
    {
        fprintf (stderr, "pack directory size (%d) is greater than max (%d)\n",
                 pdblks, pdmax);
        exit (1);
    }
    
    for (i = 1; i < pdblks; i++)
        readblk (pdstart + i, pdbuf + 3200 * i);
}
    
static char line[400];
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
    memset (lhbuf, 0, sizeof (lhbuf));
    setstr (lh->lname, fn);
    memcpy (lh->ftype, ftype, 10);
    setstr (lh->bname[0], "*directory");
    memset (blkbuf, 0, sizeof (blkbuf));
    memcpy (lh->ei.ccode, ccode, 10);
    memcpy (lh->ei.comcode, ccode, 10);
    memcpy (lh->ei.ucode, ccode, 10);
    memcpy (lh->ei.jcode, ccode, 10);
    memcpy (lh->ei.icode, icode, 10);
    memcpy (lh->ei.useles, useles, 10);
    memcpy (lh->ei.acct, account, 10);
    memcpy (lh->ei.tabset, ftabs, 10);
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
                setstr (lh->ei.useles, p);
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
            setval (lh->binfo[blk], 
                    (((u64) partial) << 59) |
                    (1 << 18) | (lw << 9) | blk);
            setstr (lh->bname[blk], blkname);
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
        setval (lh->binfo[blk], 
                (((u64) partial) << 59) |
                (1 << 18) | (words << 9) | blk);
        setstr (lh->bname[blk], blkname);
        writeblk (startblk + blk, blkbuf);
        blk++;
    }
    memset (blkbuf, 0, sizeof (blkbuf));
    for (i = 0; i < commons; i++)
    {
        lw = (comlth[i] + blklth - 1) / blklth;
        for (j = 0; j < lw; j++)
        {
            setval (lh->binfo[blk], 
                    (041ULL << 54) |
                    (((j == 0) ? lw : 0) << 18) |
                    (blklth << 9) | blk);
            setstr (lh->bname[blk], comname[i]);
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
        setval (lh->binfo[blk], blk);
        blk++;
    }
    setval (lh->blocks, blk);
    setval (lh->lastblk, (1ULL << 59) | (blk - 1));
    writeblk (startblk, (char *) lhbuf);
    fclose (inf);
#if 1
    printf ("file %s, %d blocks\n", fn, blk);
#endif
}

void printblabel (FILE *outf, int blk, bool fileinfo)
{
    char blkname[11];
    u64 blkinfo;
    int type;
    int edate;
    const char *btype;
    char bodd[20];
    
    dtoa (blkname, lh->bname[blk], 10);
    blkname[10] = '\0';
    get60 (blkinfo, lh->binfo[blk]);
    type = (blkinfo >> 54) & 037;
    edate = (blkinfo >> 27) & 077777777;
    if (fileinfo)
        btype = "fileinfo";
    else if (type < sizeof (btypes) / sizeof (btypes[0]))
        btype = btypes[type];
    else
    {
        sprintf (bodd, "(%d)", type);
        btype = bodd;
    }
    
    fprintf (outf, "* /---   block=%02d-%c %s           %02d/%02d/%02d. %02d.%02d    %s %s\n",
             blk / 7 + 1, 'a' + blk % 7, blkname, 
             (edate >> 20) + 73, (edate >> 16) & 017,
             (edate >> 11) & 037, (edate >> 6) & 037, edate & 077,
             btype,
             (!fileinfo && (blkinfo & (1ULL << 59)) != 0) ? " out" : "");
    
}

void printinfo (FILE *outf)
{
    int i;
    
    printblabel (outf, 0, true);
    for (i = 0; i < sizeof (infolist) / sizeof (infolist[0]); i++)
        (*infolist[i].printfun) (outf, infolist[i].label,
                                 infolist[i].arg1, infolist[i].arg2);
}

void pfwrite (int argc, char **argv)
{
}

void pflist (int argc, char **argv)
{
    int entry, ep;
    u64 fiw;
    int startblk, blks, type;
    char fn[11];
    
    if (argc != 1)
    {
        printf ("wrong number of arguments\n");
        usage ();
        exit (1);
    }
    pdisk = fopen(argv[0], "rb");
    if (pdisk == NULL)
    {
        perror ("open");
        exit (1);
    }
    readpd ();
    fn[10] = '\0';
    ep = pinflth * 10;
    for (entry = 0; entry < fused; entry++)
    {
        if (pdbuf[ep] == 0)
            break;
        dtoa (fn, pdbuf + ep, 10);
        if (strcmp (fn, pdname) == 0)
            break;
        get60 (fiw, pdbuf + ep + ftotal * 10);
        startblk =  (fiw & 077777) * dsblks;
        blks = ((fiw >> 24) & 077) * dsblks;
        type = ((fiw >> 30) & 077);
        if (type < 040)
        {
            if (verbose)
                printf ("%-10s  %3d %c  %5d %020llo\n", fn, blks, 
                        'a' + type - 1, startblk, fiw);
            else
                printf ("%s\n", fn);
        }
        ep += 10;
    }
}

void pfread (int argc, char **argv)
{
    int i, b;
    char *fn;
    char pfn[11];
    cw pfnw;
    int entry, ep;
    u64 fiw, blkinfo;
    int startblk, blks, type, blen, cblks, fblk;
    FILE *outf;
    int wild;
    
    if (argc < 2)
    {
        printf ("too few arguments\n");
        usage ();
        exit (1);
    }
    pdisk = fopen(argv[0], "rb");
    if (pdisk == NULL)
    {
        perror ("open");
        exit (1);
    }
    readpd ();
    if (topipe)
        outf = stdout;
    
    for (i = 1; i < argc; i++)
    {
        fn = argv[i];
        wild = (strcmp (fn, "*") == 0);
        setstr (pfnw, fn);
        ep = pinflth * 10;
        for (entry = 0; entry <= fused; entry++)
        {
            if (pdbuf[ep] == 0 || pdbuf[ep + 9] == 077)
            {
                if (!wild)
                    fprintf (stderr, "plato file %s not found\n", fn);
                break;
            }
            if (wild || memcmp (pdbuf + ep, pfnw, 10) == 0)
            {
                dtoa (pfn, pdbuf + ep, 10);
                pfn[10] = '\0';
                get60 (fiw, pdbuf + ep + ftotal * 10);
                startblk =  (fiw & 077777) * dsblks;
                blks = ((fiw >> 24) & 077) * dsblks;
                type = ((fiw >> 30) & 077);
                if (type >= 040)
                {
                    // skip spares, flaws
                    if (!wild)
                        break;
                    ep += 10;
                    continue;
                }
                readblk (startblk, lhbuf);
                if (memcmp (lh->lname, pdbuf + ep, 10) != 0)
                {
                    fprintf (stderr, "file name mismatch in block directory for %s\n",
                             pfn);
                    if (!wild)
                        break;
                    ep += 10;
                    continue;
                }
                memcpy (ftype, lh->ftype, 10);
                if (type != ftype[9])
                {
                    fprintf (stderr, "wrong file type word in block directory for %s\n", fn);
                    if (!wild)
                        break;
                    ep += 10;
                    continue;
                }
                if ((lh->lastblk[0] & 040) == 0)
                {
                    fprintf (stderr, "old format block directory for %s\n", pfn);
                    if (!wild)
                        break;
                    ep += 10;
                    continue;
                }
                if (type == 1 || type == 5)     // lesson or compass file
                {
                    if (!topipe)
                        outf = fopen (pfn, "w");
                    if (outf == NULL)
                    {
                        perror ("fopen");
                        exit (1);
                    }
                    cblks = 0;
                    if (!sourceonly)
                        printinfo (outf);
                    for (b = 1; b < blks; b++)
                    {
                        get60 (blkinfo, lh->binfo[b]);
                        blen = (blkinfo >> 9) & 0777;
                        type = (blkinfo >> 54) & 037;
                        fblk = blkinfo & 0777;
                        
                        // Skip empty blocks
                        if (blen == 0)
                            continue;
                        // If source only, skip partialed-out or non-source blocks
                        if (sourceonly)
                        {
                            if (((blkinfo >> 59) & 1) != 0 || 
                                (type != 0 && type != 9))
                                continue;
                        }
                        else
                            printblabel (outf, b, false);
                        readblk (fblk + startblk, blkbuf);
                        if (type == 0 || type == 9)
                            printblk (outf, blen);
                        else
                            dumpblk (outf, 0);
                        cblks++;
                    }            
                }
                else
                {
                    if (sourceonly)
                    {
                        fprintf (stderr, "%s it not a source file\n", pfn);
                        continue;
                    }
                    if (!topipe)
                        outf = fopen (pfn, "w");
                    if (outf == NULL)
                    {
                        perror ("fopen");
                        exit (1);
                    }
                    cblks = blks;
                    for (b = 0; b < blks; b++)
                    {
                        readblk (b + startblk, blkbuf);
                        dumpblk (outf, b * blklth);
                    }
                }
                if (!topipe)
                {
                    fclose (outf);
                }
                if (verbose)
                {
                    fprintf (logfile, "%s, type %c, %d blocks\n",
                             pfn, 'a' + ftype[9] - 1, cblks);
                }
                if (!wild)
                    break;
            }
            ep += 10;
        }
    }
}

void pfinit (int argc, char **argv)
{
    int type, i;
    
    if (argc != 3)
    {
        printf ("wrong number of arguments\n");
        usage ();
        exit (1);
    }
    type = tolower(*argv[2]) - 'a';
    if (type > 3)
    {
        printf ("invalid pack type %s\n", argv[3]);
        exit (1);
    }
    ptype = types[type];
    memset (pname, ':', 10);
    i = strlen (argv[1]);
    if (i > 10)
    {
        printf ("invalid pack name %s\n", argv[2]);
        exit (1);
    }
    memcpy (pname, argv[1], i);
    
    pdisk = fopen(argv[0], "wb");
    if (pdisk == NULL)
    {
        perror ("open");
        exit (1);
    }
    initflaw ();
    initpd ();
    finishpd ();
    fclose (pdisk);
}

void usage (void)
{
    fprintf (stderr, "usage:\n"
             "pf [-pmMD] pdisk pfile ...        read files\n"
             "pf [-m] -l pdisk                  list directory\n"
             "pf [-m] -w pdisk pfile ...        write files\n"
             "pf [-m] -p -w pdisk pfile         write file from stdin\n"
             "pf [-mf] -c parts pdisk pfile ... create files\n"

             "pf [-mf] -p -c parts pdisk pfile  create file from stdin\n"
             "pf [-mf] -i pdisk plabel ptype    initialize plato disk\n"
             "   -p                             pipe output to stdout\n"
             "   -m                             pdisk is a masterfile\n"
             "   -M                             display modwords\n"
             "   -D                             include deleted lines\n");
}

int main (int argc, char **argv)
{
    int type, i, f;
    char opt;
    action act;
    time_t t;
    
    nextpart = pdstart;
    
    time (&t);
    strftime (date, sizeof (date), " %m/%d/%y.", localtime (&t));
    
    act = Read;
    force = topipe = verbose = false;
    parts = 0;
    for (;;)
    {
        opt = getopt (argc, argv, "w:cifpslvmMD");
        if (opt == (char) (-1))
            break;
        switch (opt)
        {
        case 'w':
            act = Write;
            break;
        case 'c':
            parts = atoi (optarg);
            if (parts < 1 || parts > 18)
            {
                fprintf (stderr, "-c argument out of range: %d\n", parts);
                act = None;
                break;
            }
            act = Create;            
            break;
        case 'i':
            act = Packinit;
            break;
        case 'f':
            force = true;
            break;
        case 'l':
            act = List;
            break;
        case 'p':
            topipe = true;
            break;
        case 's':
            sourceonly = true;
            break;
        case 'v':
            verbose = true;
            break;
        case 'm':
            mf = 1;
            break;
        case 'M':
            modword = 1;
            break;
        case 'D':
            moddel = 1;
            break;
        }
    }
    argc -= optind;
    argv += optind;
    
    if (argc < 1)
    {
        usage ();
        exit (1);
    }
    
    if (topipe)
        logfile = stderr;
    else
        logfile = stdout;
    
    switch (act)
    {
    case None:
        usage ();
        exit (1);
    case Read:
        pfread (argc, argv);
        break;
    case Write:
        pfwrite (argc, argv);
        break;
    case List:
        pflist (argc, argv);
        break;
    case Packinit:
        if (!force)
        {
            char line[100];

            printf ("Initialize new PLATO pack %s -- are you sure? ", argv[1]);
            fflush (stdout);
            fgets (line, sizeof (line), stdin);
            if (tolower (line[0]) != 'y')
            {
                printf ("not confirmed\n");
                exit (0);
            }
        }
        pfinit (argc, argv);
        break;
    default:
        fprintf (stderr, "oops, code is missing\n");
    }
    return 0;
}

