#pragma once
#ifndef DTNETSUBS_H
#define DTNETSUBS_H
/*
**  -----------------
**  Global variables.
**  -----------------
*/

extern bool emulationActive;

/*
**  dtnetsubs.c
*/
void dtInit(void);
NetFet * dtConnect(NetPortSet *ps, in_addr_t host, int portnum);
void dtInitPortset(NetPortSet *ps);
void dtClose(NetFet *np, bool hard);

int dtCreateThread(dtThreadFunPtr(fp), void *param, pthread_t *id);

const char *dtNowString(void);
int dtSendTlv(NetFet *fet, int tag, int len, const void *value);
int dtSend(NetFet *fet, const void *buf, int len);
NetFet * dtBind(NetPortSet *ps, in_addr_t host, int port, int backlog);
NetFet * dtAccept(NetFet *listenFet, NetPortSet *ps);

int dtReadoi(NetFet *fet, int *outidx);
int dtReadw(NetFet *fet, void *buf, int len);
int dtPeekw(NetFet *fet, void *buf, int len);
int dtReadmax(NetFet *fet, void *buf, int len);
int dtReadtlv(NetFet *fet, void *buf, int len);

/* We could do these as functions but they are short, so... */
#define dtEmpty(fet) \
    ((fet)->in == (fet)->out)
#define dtFull(fet) \
    ((fet)->in + 1 == (fet)->out || \
     ((fet)->in + 1 == (fet)->end && (fet)->out == (fet)->first))
#define dtFetData(fet) \
    (((fet)->in >= (fet)->out) ? (fet)->in - (fet)->out \
     : (fet)->end - (fet)->out + (fet)->in - (fet)->first)
#define dtFetFree(fet) \
    ((fet)->end - (fet)->first - dtFetData (fet) - 1)

#define dtSendEmpty(fet) \
    ((fet)->sendin == (fet)->sendout)
#define dtSendFull(fet) \
    ((fet)->sendin + 1 == (fet)->sendout || \
     ((fet)->sendin + 1 == (fet)->sendend && (fet)->sendout == (fet)->sendfirst))
#define dtSendData(fet) \
    (((fet)->sendin >= (fet)->sendout) ? (fet)->sendin - (fet)->sendout \
     : (fet)->sendend - (fet)->sendout + (fet)->sendin - (fet)->sendfirst)
#define dtSendFree(fet) \
    ((fet)->sendend - (fet)->sendfirst - dtSendData (fet) - 1)

#define dtActive(fet) \
    ((fet) != NULL && (fet)->closing == 0)
#define dtConnected(fet) \
    (dtActive (fet) && (fet)->connected)

/* This goes with dtReadoi */
#define dtUpdateOut(fet,outidx) \
    (fet)->out = (outidx) + fet->first

/*--------------------------------------------------------------------------
**  Purpose:        Read one byte from the network buffer
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**
**  Returns:        -1 if no data available, or the next byte as
**                  an unsigned value.
**
**------------------------------------------------------------------------*/
static inline int dtReado (NetFet *fet)
    {
    u8 *in, *out, *nextout;
    u8 b;
    
    if (fet == NULL)
        return -1;
    /*
    **  Copy the pointers, since they are volatile.
    */
    in = (u8 *) (fet->in);
    out = (u8 *) (fet->out);

    if (out == in)
        {
        return -1;
        }
    nextout = out + 1;
    if (nextout == fet->end)
        {
        nextout = fet->first;
        }
    b = *out;
    fet->out = nextout;
    return b;
    }

#endif /* DTNETSUBS_H */
