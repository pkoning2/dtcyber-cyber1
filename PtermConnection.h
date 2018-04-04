////////////////////////////////////////////////////////////////////////////
// Name:        PTermConnection.h
// Purpose:     Definition of the a connection to a host
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __PTermConnection_H__
#define __PTermConnection_H__ 1

#include "CommonHeader.h"

class PtermFrame;

enum ConnType_e
{
    LOCAL,
    HOST,
    HELP,
    TEST
};

// Pterm connection state
class PtermConnection
{
public:
    PtermConnection ();
    virtual ~PtermConnection ();

    virtual ConnType_e ConnType (void) const = 0;
    virtual int NextWord (void) = 0;
    virtual void SendData (const void *data, int len);

    void SetOwner (PtermFrame *owner) { m_owner = owner; }
    virtual int RingCount (void) const;
    bool Ascii (void) const
    {
        return (m_connMode == ascii);
    }
    bool Classic (void) const
    {
        return (m_connMode == niu);
    }
    bool GswActive (void) const
    {
        return m_gswActive;
    }
    virtual void StoreWord (int word);
    virtual void Connect (void);

    bool        m_connActive;

protected:
    PtermFrame  *m_owner;
    bool        m_gswActive;
    connMode    m_connMode;
};

class PtermLocalConnection : public PtermConnection
{
    ConnType_e ConnType (void) const { return LOCAL; }
    int NextWord (void);
};

class PtermTestConnection : public PtermConnection
{
public:
    PtermTestConnection (FILE *testdata);
    ~PtermTestConnection ();
    ConnType_e ConnType (void) const { return TEST; }
    
    FILE *m_testdata;
    u32 m_pseq;
    int NextWord (void);
};

class PtermHostConnection : public PtermConnection
{
public:
    PtermHostConnection (const wxString &host, int port);
    virtual ~PtermHostConnection ();
    ConnType_e ConnType (void) const { return HOST; }
    
    int NextWord (void);
    
    void SendData (const void *data, int len);
    void StoreWord (int word);
    void Connect (void);
    int RingCount (void) const;

    int NextGswWord (bool idle);
    
private:
    NetPortSet  m_portset;
    NetFet      *m_fet;
    u32         m_displayRing[RINGSIZE];
    volatile int m_displayIn, m_displayOut;
    u32         m_gswRing[GSWRINGSIZE];
    volatile int m_gswIn, m_gswOut;
    wxString    m_hostName;
    int         m_port;
    wxCriticalSection m_pointerLock;
    bool        m_gswStarted;
    int         m_savedGswMode;
    int         m_gswWord2;
    int         m_pending;
    in_addr_t   m_hostAddr;
    
    // Callback handler
    static void s_dataCallback (NetFet *np, int bytes, void *arg);
    void dataCallback (void);
    void endGsw (void);
    
    int AssembleNiuWord (void);
    int AssembleAsciiWord (void);
    int AssembleAutoWord (void);
    int NextRingWord (void);

    bool IsEmpty (void) const
    {
        return (m_displayIn == m_displayOut);
    }
    bool IsFull (void) const
    {
        volatile int next;
    
        next = m_displayIn + 1;
        if (next == RINGSIZE)
        {
            next = 0;
        }
        return (next == m_displayOut);
    }
};

#endif   // __PTermConnection_H__
