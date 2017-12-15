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

// Pterm connection state
class PtermConnection
{
public:
    // override base class virtuals
    // ----------------------------
    PtermConnection (PtermFrame *owner, const wxString &host, int port);
    ~PtermConnection ();
    
    // Callback handlers
    static void s_connCallback (NetFet *np, int portNum, void *arg);
    static void s_dataCallback (NetFet *np, int bytes, void *arg);
    void connCallback (void);
    void dataCallback (void);
    void endGsw (void);
    
    int AssembleNiuWord (void);
    int AssembleAsciiWord (void);
    int AssembleAutoWord (void);
    int NextWord (void);
    int NextGswWord (bool idle);
    
    void SendData (const void *data, int len);

    bool IsEmpty (void) const
    {
        return (m_displayIn == m_displayOut);
    }
    bool IsFull (void) const
    {
        int next;
    
        next = m_displayIn + 1;
        if (next == RINGSIZE)
        {
            next = 0;
        }
        return (next == m_displayOut);
    }
    int RingCount (void) const
    {
        if (m_displayIn >= m_displayOut)
        {
            return m_displayIn - m_displayOut;
        }
        else
        {
            return RINGSIZE + m_displayIn - m_displayOut;
        }
    }
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
    void StoreWord (int word);

private:
    NetPortSet  m_portset;
    NetFet      *m_fet;
    u32         m_displayRing[RINGSIZE];
    volatile int m_displayIn, m_displayOut;
    u32         m_gswRing[GSWRINGSIZE];
    volatile int m_gswIn, m_gswOut;
    PtermFrame  *m_owner;
    wxString    m_hostName;
    int         m_port;
    wxCriticalSection m_pointerLock;
    bool        m_gswActive;
    bool        m_gswStarted;
    int         m_savedGswMode;
    int         m_gswWord2;
    connMode    m_connMode;
    int         m_pending;
    in_addr_t   m_hostAddr;
    bool        m_connActive;
    
    int NextRingWord (void);
};

#endif   // __PTermConnection_H__
