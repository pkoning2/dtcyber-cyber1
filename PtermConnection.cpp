////////////////////////////////////////////////////////////////////////////
// Name:        PTermConnection.cpp
// Purpose:     Implementation of the a connection to a host
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#include "PtermConnection.h"
#include "PtermConnDialog.h"
#include "PtermFrame.h"
#include "DebugPterm.h"

// ----------------------------------------------------------------------------
// PtermConnection
// ----------------------------------------------------------------------------

PtermConnection::PtermConnection ()
    : m_connActive (false),
      m_gswActive (false),
      m_connMode (both)
{
}

PtermConnection::~PtermConnection ()
{
}

void PtermConnection::SendData (const void *, int)
{
}

int PtermConnection::RingCount (void) const
{
    return 0;
}

void PtermConnection::StoreWord (int)
{
}

void PtermConnection::Connect (void)
{
    m_owner->ptermSetConnected ();
}

// ----------------------------------------------------------------------------
// PtermLocalConnection
// ----------------------------------------------------------------------------

int PtermLocalConnection::NextWord (void)
{
    return C_NODATA;
}

// ----------------------------------------------------------------------------
// PtermTestConnection
// ----------------------------------------------------------------------------

PtermTestConnection::PtermTestConnection (FILE *testdata)
    : m_testdata (testdata),
      m_pseq (~(0U))
{
}

PtermTestConnection::~PtermTestConnection ()
{
    fclose (m_testdata);
}

int PtermTestConnection::NextWord (void)
{
    char *fret, *p;
    char tline[200];
    u32 w, key, seq;

    for (;;)
    {
        fret = fgets (tline, 199, m_testdata);
        if (fret == NULL)
        {
            return C_NODATA;
        }
        if (m_connMode == both)
        {
            // See if we can figure out the connection mode
            if (strstr (tline, "ascii") != NULL ||
                strstr (tline, "ASCII") != NULL)
            {
                m_connMode = ascii;
            }
        }
                
        p = tline;
        seq = m_pseq ^ 1;
        if (p[2] == ':')
        {
            // Timestamp at start of line, skip it
            p += 14;
        }
        if (sscanf (p, "key to plato %o", &key))// != 0 && key < 0200)
        {
            // Key stroke in the trace, so this is a pause point.  But
            // simply pausing isn't correct because that doesn't
            // update the screen.  Need a better answer.
        }
        else if (sscanf (p, "%o seq %d", &w, &seq) != 0 ||
                 sscanf (p, "%o  wc", &w) != 0)
        {
            // Successful conversion, process the word, provided it is
            // new (different sequence number than before), or it's a
            // trace without sequence numbers.
            if (seq == m_pseq)
                continue;
            
            m_pseq = seq;
            if (m_connMode == both && w > 2)
            {
                // See if the value of the word gives a clue about
                // the protocol used
                if (w <= 0377 || (w >> 8) == 033)
                {
                    m_connMode = ascii;
                }
                else
                {
                    m_connMode = niu;
                }
            }
            return w;
        }
    }
}




// ----------------------------------------------------------------------------
// PtermHostConnection
// ----------------------------------------------------------------------------

PtermHostConnection::PtermHostConnection (const wxString &host, int port)
    : m_fet (NULL),
      m_displayIn (0),
      m_displayOut (0),
      m_gswIn (0),
      m_gswOut (0),
      m_port (port),
      m_gswStarted (false),
      m_savedGswMode (0),
      m_gswWord2 (0),
      m_pending (0)
{
    m_hostName = host;

    m_portset.callBack = NULL;
    m_portset.dataCallBack = s_dataCallback;
    m_portset.callArg = m_portset.dataCallArg = this;
    m_portset.portNum = 0;      // No listening
    m_portset.maxPorts = 1;
    m_portset.ringSize = BufSiz;
    m_portset.sendRingSize = 1000;
    dtInitPortset (&m_portset);    
}

PtermHostConnection::~PtermHostConnection ()
{
    if (m_gswActive)
    {
        ptermCloseGsw ();
        m_owner->m_gswFile = wxString ();
    }
    dtClose (m_fet, TRUE);
    m_fet = NULL;
}

void PtermHostConnection::Connect (void)
{
    struct hostent *hp;
    in_addr_t hostaddr;
    int i, addrcount, r, conntries;
    in_addr_t *addresses = NULL;

    hp = gethostbyname (m_hostName.mb_str ());
    if (hp == NULL || hp->h_length == 0)
    {
        StoreWord (C_CONNFAIL1);
        wxWakeUpIdle ();
        return;
    }

    for (addrcount = 0; hp->h_addr_list[addrcount] != NULL; addrcount++) ;
    addresses = new in_addr_t[addrcount];
    for (i = 0; i < addrcount; i++)
    {
        memcpy (&addresses[i], hp->h_addr_list[i], sizeof (int));
    }
    for (conntries = 0; conntries < addrcount; conntries++)
    {
        while (addresses[(r = (rand () >> 10) % addrcount)] == 0) ;
        hostaddr = addresses[r];
        m_hostAddr = ntohl (hostaddr);
        addresses[r] = 0;
        StoreWord (C_CONNECTING);
        wxWakeUpIdle ();
        m_fet = dtConnect (&m_portset, hostaddr, m_port);
        if (m_fet != NULL)
        {
            break;
        }
    }
    if (conntries == addrcount)
    {
        // We ran out of addresses
        StoreWord (C_CONNFAIL2);
    }
    if (addresses != NULL)
    {
        delete [] addresses;
    }
}

void PtermHostConnection::s_dataCallback (NetFet *, int, void *arg)
{
    PtermHostConnection *self = (PtermHostConnection *) arg;
    
    tracex ("Data callback on %p", self);
    self->dataCallback ();
}

void PtermHostConnection::endGsw (void)
{
    // Turn GSW off
    m_savedGswMode = m_gswWord2 = 0;
    if (m_gswActive)
    {
        m_gswActive = m_gswStarted = false;
        ptermCloseGsw ();
        m_owner->m_gswFile = wxString ();
    }
}

void PtermHostConnection::dataCallback (void)
{
    u32 platowd = 0;
    int i;

    for (;;)
    {
        //printf ("ringcount: %d\n", RingCount ());
        /*
        **  Assemble words from the network buffer, all the
        **  while looking for "abort output" codes (word == 2).
        */
        if (IsFull ())
        {
            printf ("ring is full\n");
        }

        // Possible race condition: a connection may become alive and
        // data appear on it before m_fet is set from the return value
        // of the dtConnect call.
        if (m_fet == NULL)
        {
#if defined (_WIN32)
            Sleep (1000);
#else
            sleep (1);
#endif
            continue;
        }
            
        switch (m_connMode)
        {
        case niu:
            platowd = AssembleNiuWord ();
            break;
        case ascii:
            platowd = AssembleAsciiWord ();
            break;
        case both:
            platowd = AssembleAutoWord ();
            break;
        }
            
        if (platowd == C_NODATA)
        {
            // No more data right now
            break;
        }
        else if (m_connMode == niu && platowd == 2)
        {
            endGsw ();
                
            // erase abort marker -- reset the ring to be empty
            wxCriticalSectionLocker lock (m_pointerLock);

            m_displayOut = m_displayIn;
        }
        else if (platowd == C_DISCONNECT ||
                 platowd == C_CONNFAIL1 || platowd == C_CONNFAIL2)
        {
            endGsw ();
            StoreWord (platowd);
            break;
        }
        
        StoreWord (platowd);
        i = RingCount ();
        debug ("Stored %07o, ring count is %d", platowd, i);


        if (m_gswActive && !m_gswStarted && i >= GSWRINGSIZE / 2)
        {
            ptermStartGsw ();
            m_gswStarted = true;
        }
            
        if (i == RINGXOFF1 || i == RINGXOFF2)
        {
            m_owner->ptermSendKey1 (xofkey);
        }
    }
    if (!IsEmpty ())
    {
        // Send a do-nothing event to the frame; that will wake up
        // the main thread and cause it to process the words we 
        // buffered.
        wxWakeUpIdle ();
    }
}

int PtermHostConnection::AssembleNiuWord (void)
{
    int i, j, k;
    
    for (;;)
    {
        if (dtFetData (m_fet) < 3)
        {
            if (dtConnected (m_fet))
            {
                return C_NODATA;
            }
            m_connActive = false;
            dtClose (m_fet, TRUE);
            m_fet = NULL;
            return C_DISCONNECT;
        }
        i = dtReado (m_fet);
        if (i & 0200)
        {
            printf ("Plato output out of sync byte 0: %03o\n", i);
            continue;
        }
newj:
        j = dtReado (m_fet);
        if ((j & 0300) != 0200)
        {
            printf ("Plato output out of sync byte 1: %03o\n", j);
            if ((j & 0200) == 0)
            {
                i = j;
                goto newj;
            }
            continue;
        }
        k = dtReado (m_fet);
        if ((k & 0300) != 0300)
        {
            printf ("Plato output out of sync byte 2: %03o\n", k);
            if ((k & 0200) == 0)
            {
                i = k;
                goto newj;
            }
            continue;
        }
        return (i << 12) | ((j & 077) << 6) | (k & 077);
    }
}

int PtermHostConnection::AssembleAutoWord (void)
{
    u8 buf[3];
    
    if (dtPeekw (m_fet, buf, 3) < 0)
    {
        if (dtConnected (m_fet))
        {
            return C_NODATA;
        }
        m_connActive = false;
        dtClose (m_fet, TRUE);
        m_fet = NULL;
        return C_DISCONNECT;
    }

    // We received something so the connection is now active
    if (!m_connActive)
    {
        m_connActive = true;
        StoreWord (C_CONNECTED);
    }

    if ((buf[0] & 0200) == 0 &&
        (buf[1] & 0300) == 0200 &&
        (buf[2] & 0300) == 0300)
    {
        m_connMode = niu;
        m_owner->menuFile->Enable (Pterm_SaveAudio, true);
        return AssembleNiuWord ();
    }
    else
    {
        m_connMode = ascii;
        return AssembleAsciiWord ();
    }
}

int PtermHostConnection::AssembleAsciiWord (void)
{
    int i;
    
    for (;;)
    {
        i = dtReado (m_fet);
        if (i == -1)
        {
            if (dtConnected (m_fet))
            {
                return C_NODATA;
            }
            m_connActive = false;
            dtClose (m_fet, TRUE);
            m_fet = NULL;
            return C_DISCONNECT;
        }
        else if (m_pending == 0 && i == 0377)
        {
            // 0377 is used by Telnet to introduce commands (IAC).
            // We recognize only IAC IAC for now.
            // Note that the check has to be made before the sign
            // bit is stripped off.
            m_pending = 0377;
            continue;
        }

        i &= 0177;
        if (i == 033)
        {
            m_pending = 033;
            continue;
        }
        if (m_pending == 033)
        {
            m_pending = 0;
            return (033 << 8) + i;
        }
        else
        {
            m_pending = 0;
            if (i == 0)
            {
                // NUL is for -delay-
                i = 1 << 19;
            }
            return i;
        }
    }
}


int PtermHostConnection::NextRingWord (void)
{
    int word, next, i;

    {
        wxCriticalSectionLocker lock (m_pointerLock);
        
        if (m_displayIn == m_displayOut)
        {
            return C_NODATA;
        }
    
        i = RingCount ();
        word = m_displayRing[m_displayOut];
        next = m_displayOut + 1;
        if (next == RINGSIZE)
        {
            next = 0;
        }
        m_displayOut = next;
    }
    debug ("consumed word %07o, ring count now %d", word, i);
    if (i < RINGXOFF1 && m_owner->m_pendingEcho != -1)
    {
        m_owner->ptermSendKey1 (m_owner->m_pendingEcho);
        m_owner->m_pendingEcho = -1;
    }
    if (i == RINGXON1 || i == RINGXON2)
    {
        m_owner->ptermSendKey1 (xonkey);
    }

    return word;
}

int PtermHostConnection::NextWord (void)
{
    int next, word;
    int delay = 0;
    wxString msg;

    if (m_gswActive)
    {
        // Take data from the ring of words that have just been given
        // to the GSW emulation.  Note that the delay amount has already
        // been set in those entries.  Usually it is 1, but it may be 0
        // if the display is falling behind.
        if (m_gswIn == m_gswOut)
        {
            return C_NODATA;
        }
        
        word = m_gswRing[m_gswOut];
        next = m_gswOut + 1;
        if (next == GSWRINGSIZE)
        {
            next = 0;
        }
        m_gswOut = next;

        if (word == C_GSWEND)
        {
            m_gswActive = m_gswStarted = false;
            ptermCloseGsw ();
            m_owner->m_gswFile = wxString ();
            m_owner->ptermShowTrace ();
        }
        else
        {
            return word;
        }
    }

    // Take data from the main input ring
    word = NextRingWord ();

    if (!Ascii () && 
        (word >> 16) == 3 &&
        word != 0700001 &&
        !((word == 0770000 || word == 0730000) &&
          m_owner->m_station == wxT ("0-1")) &&
        m_owner->m_profile->m_gswEnable)
    {
        // It's an -extout- word, which means we'll want to start up
        // GSW emulation, if enabled.
        // However, we'll see these also when PLATO is turning OFF
        // the GSW (common entry code in the various gsw lessons).
        // We don't want to grab the GSW subsystem in that case.
        // The "turn off" sequence consists of a mode word followed
        // by voice words that specify "rest" (operand == 1).
        // The sound is silenced when the GSW is not active, so we'll
        // ignore "rest" voice words in that case.  We'll save the last
        // voice word, because it sets number of voices and volumes.
        // We have to do that because when the music actually starts,
        // we'll first see a mode word and then some non-rest voice
        // words.  The non-rest voice words trigger the GSW startup,
        // so we'll need to send the preceding mode word for correct
        // initialization.
        // Also, if we're connected to station 0-1, ignore the
        // operator box command words.
        if ((word >> 15) == 6)
        {
            // mode word, just save it
            m_savedGswMode = word;
        }
        else if (ptermOpenGsw (this, m_owner->m_gswFile, 
                               m_owner->m_gswFFmt) == 0)
        {
            m_gswActive = true;
            m_gswWord2 = word;
            m_owner->ptermShowTrace ();
                
            if (!m_gswStarted && RingCount () >= GSWRINGSIZE / 2)
            {
                ptermStartGsw ();
                m_gswStarted = true;
            }
        }
    }
            
    // See if emulating 1260 baud, or the -delay- NOP code
    if (m_owner->m_classicSpeed ||
        word == 1)
    {
        delay = 1;
    }
    
    // Pass the delay to the caller
    word |= (delay << 19);

    // The processing for the Connecting and Connected message
    // formatting is here because setting the statusbar out of
    // connection thread context doesn't work.  The usual story,
    // GUI actions belong in the GUI (main) thread.
    if (word == C_CONNECTING)
    {
        msg.Printf (_("Connecting to %d.%d.%d.%d"), 
                    (m_hostAddr >> 24) & 0xff,
                    (m_hostAddr >> 16) & 0xff,
                    (m_hostAddr >>  8) & 0xff,
                    m_hostAddr & 0xff);
        m_owner->ptermSetStatus (msg);
    }
    else if (word == C_CONNECTED)
    {
        m_owner->ptermSetConnected ();
    }
        
    return word;
}

// Get a word from the main data ring for the GSW emulation.  The
// "idle" flag is true if the GSW has seen several consecutive periods
// (adding up to about a second of play) in which there was no data
// for it to process.  We treat that as end of song, something that is
// not explicitly encoded in the GSW data stream.
//
// When a word is given to the GSW, it is stored in the "gsw ring" which
// holds words to be processed by the terminal emulation display machinery
// whenever GSW emulation is active.  That's a small ring, because entries
// are added to it at 60 per second and they are processed promptly.
// This approach means the GSW is doing the display pacing, rather than
// some other timer.  That ties the display directly to the audio stream,
// which we want so things like "watch music display while it is playing"
// work right.

int PtermHostConnection::NextGswWord (bool idle)
{
    int next, word;

    if (m_savedGswMode != 0)
    {
        word = m_savedGswMode;
        m_savedGswMode = 0;
    }
    else if (m_gswWord2 != 0)
    {
        word = m_gswWord2;
        m_gswWord2 = 0;
    }
    else
    {
        next = m_gswOut;
        next = m_gswIn + 1;
        if (next == GSWRINGSIZE)
        {
            next = 0;
        }
        // If there is no room to save this data for the display,
        // tell the sound that there isn't any more data.
        if (next == m_gswOut)
        {
            return C_NODATA;
        }
        word = NextRingWord ();
        if (word == C_NODATA && idle)
        {
            word = C_GSWEND;
        }
        
        m_gswRing[m_gswIn] = word;
        m_gswIn = next;
        wxWakeUpIdle ();
    }
    
    return word;
}

int ptermNextGswWord (void *connection, int idle)
{
    return ((PtermHostConnection *) connection)->NextGswWord (idle != 0);
}

int PtermHostConnection::RingCount (void) const
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

void PtermHostConnection::StoreWord (int word)
{
    int next;
    
    if (word < 0)
    {
        m_displayOut = m_displayIn;
    }
    next = m_displayIn + 1;
    if (next == RINGSIZE)
    {
        next = 0;
    }
    if (next == m_displayOut)
    {
        return;
    }
    m_displayRing[m_displayIn] = word;
    m_displayIn = next;
    
    debug ("data from plato %07o", word);
}

void PtermHostConnection::SendData (const void *data, int len)
{
    dtSend (m_fet, data, len);
}



