// ----------------------------------------------------------------------------
// PtermConnection
// ----------------------------------------------------------------------------

PtermConnection::PtermConnection (PtermFrame *owner, wxString &host, int port)
    : wxThread (wxTHREAD_JOINABLE),
      m_displayIn (0),
      m_displayOut (0),
      m_gswIn (0),
      m_gswOut (0),
      m_owner (owner),
      m_port (port),
      m_gswActive (false),
      m_gswStarted (false),
      m_savedGswMode (0),
      m_gswWord2 (0),
      m_connMode (both),
      m_pending (0)
{
    m_hostName = host;
}

PtermConnection::~PtermConnection ()
{
    if (m_gswActive)
    {
        ptermCloseGsw ();
    }
    dtClose (m_fet, &m_portset, TRUE);
}

PtermConnection::ExitCode PtermConnection::Entry (void)
{
    u32 platowd;
    int i;
    bool wasEmpty;
    struct hostent *hp;
    in_addr_t host;
    int true_opt = 1;
    int addrcount, r, conntries;
    in_addr_t *addresses = NULL;
    wxString msg;
    bool connActive = false;
    
    m_portset.callBack = NULL;
    m_portset.maxPorts = 1;
    dtInitPortset (&m_portset, BufSiz);
    m_fet = m_portset.portVec;
    
    hp = gethostbyname (m_hostName.mb_str());
    if (hp == NULL || hp->h_length == 0)
    {
        StoreWord (C_CONNFAIL);
        wxWakeUpIdle ();
        return (ExitCode) 1;
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
        host = addresses[r];
        m_hostAddr = ntohl (host);
        addresses[r] = 0;
        StoreWord (C_CONNECTING);
        wxWakeUpIdle ();
        if (dtConnect (m_fet, NULL, host, m_port) < 0)
        {
            continue;
        }

        while (true)
        {
            // The reason for waiting a limited time here rather than
            // using the more obvious -1 (wait forever) is to make sure
            // we come out of the network wait and call TestDestroy
            // reasonably often.  Otherwise, closing the window doesn't work.
            i = dtRead (m_fet, &m_portset, 200);
#ifdef DEBUG
            printf ("dtRead status %i\n", i);
#endif
            if (TestDestroy ())
            {
                connActive = true;
                break;
            }
            if (i < 0)
            {
                m_savedGswMode = m_gswWord2 = 0;
                if (m_gswActive)
                {
                    m_gswActive = m_gswStarted = false;
                    ptermCloseGsw ();
                }

                if (connActive)
                {
                    StoreWord (C_DISCONNECT);
                }
                else
                {
                    break;
                }
                wxWakeUpIdle ();
                break;
            }
            // We received something so the connection is now active
            if (!connActive)
            {
                connActive = true;
                StoreWord (C_CONNECTED);
            }

            wasEmpty = IsEmpty ();
        
            for (;;)
            {
                /*
                **  Assemble words from the network buffer, all the
                **  while looking for "abort output" codes (word == 2).
                */
                if (IsFull ())
                {
                    break;
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
                    break;
                }
                else if (m_connMode == niu && platowd == 2)
                {
                    m_savedGswMode = m_gswWord2 = 0;
                    if (m_gswActive)
                    {
                        m_gswActive = m_gswStarted = false;
                        ptermCloseGsw ();
                    }
                
                    // erase abort marker -- reset the ring to be empty
                    wxCriticalSectionLocker lock (m_pointerLock);

                    m_displayOut = m_displayIn;
                }

                StoreWord (platowd);
                i = RingCount ();
                if (m_gswActive && !m_gswStarted && i >= GSWRINGSIZE / 2)
                {
                    ptermStartGsw ();
                    m_gswStarted = true;
                }
            
                if (i == RINGXOFF1 || i == RINGXOFF2)
                {
                    m_owner->ptermSendKey (xofkey);
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
        if (!connActive)
        {
            // Error on receive before we received anything means
            // connect failure.  Try another address, if we have another.
            continue;
        }

        delete addresses;
        return (ExitCode) 0;
    }
        
    delete addresses;
    StoreWord (C_CONNFAIL);
    wxWakeUpIdle ();
    return (ExitCode) 1;
}

int PtermConnection::AssembleNiuWord (void)
{
    int i, j, k;
    
    for (;;)
    {
        if (dtFetData (m_fet) < 3)
        {
            return C_NODATA;
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

int PtermConnection::AssembleAutoWord (void)
{
    u8 buf[3];
    
    if (dtPeekw (m_fet, buf, 3) < 0)
    {
        return C_NODATA;
    }
    if ((buf[0] & 0200) == 0 &&
        (buf[1] & 0300) == 0200 &&
        (buf[2] & 0300) == 0300)
    {
        m_connMode = niu;
        return AssembleNiuWord ();
    }
    else
    {
        m_connMode = ascii;
        return AssembleAsciiWord ();
    }
}

int PtermConnection::AssembleAsciiWord (void)
{
    int i;
    
    for (;;)
    {
        i = dtReado (m_fet);
        if (i == -1)
        {
            return C_NODATA;
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


int PtermConnection::NextRingWord (void)
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

    if (i < RINGXOFF1 && m_owner->m_pendingEcho != -1)
    {
        m_owner->ptermSendKey (m_owner->m_pendingEcho);
        m_owner->m_pendingEcho = -1;
    }
    if (i == RINGXON1 || i == RINGXON2)
    {
        m_owner->ptermSendKey (xonkey);
    }

    return word;
}

int PtermConnection::NextWord (void)
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
        !(m_owner->m_station == 1 && 
          (word == 0770000 || word == 0730000)) &&
        ptermApp->m_gswEnable)
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
        else if (ptermOpenGsw (this) == 0)
        {
            m_gswActive = true;
            m_gswWord2 = word;
            delay = 1;
                
            if (!m_gswStarted && RingCount () >= GSWRINGSIZE / 2)
            {
                ptermStartGsw ();
                m_gswStarted = true;
            }
        }
    }
            
    // See if emulating 1200 baud, or the -delay- NOP code
    if (ptermApp->m_classicSpeed ||
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
        msg.Printf (_("Connected to %d.%d.%d.%d"), 
                    (m_hostAddr >> 24) & 0xff,
                    (m_hostAddr >> 16) & 0xff,
                    (m_hostAddr >>  8) & 0xff,
                    m_hostAddr & 0xff);
        m_owner->ptermSetStatus (msg);
    }
    else if (word == C_CONNFAIL || word == C_DISCONNECT)
    {
		if (m_owner->m_statusBar != NULL)
			m_owner->m_statusBar->SetStatusText (_(" Not connected"), STATUS_CONN);

		wxDateTime ldt;
		ldt.SetToCurrent();
        if (word == C_CONNFAIL)
            msg.Printf(_("Connection failed @ %s on "), ldt.FormatTime().c_str());
        else
            msg.Printf(_("Dropped connection @ %s on "), ldt.FormatTime().c_str());
		msg.Append(ldt.FormatDate());
		m_owner->WriteTraceMessage(msg);
        msg.Printf(_("%s on "), ldt.FormatTime().c_str());
		msg.Append(ldt.FormatDate().c_str());	// fits in dialog box title
		
		m_owner->Iconize(false);	// ensure window is visible when connection fails
		ptermApp->m_connError = word;
        PtermConnFailDialog dlg (wxID_ANY, msg, wxDefaultPosition, wxSize( 320,140 ));
        dlg.CenterOnScreen ();
        dlg.ShowModal ();

		switch (ptermApp->m_connAction)
		{
		case 1:			// prompt for a connection rather than just bailing out
			ptermApp->DoConnect(true);
			word = C_NODATA;
			break;
		case 2:			// stay in pterm in disconnected state
			if (word == C_DISCONNECT)
				word = C_NODATA;
			else
            {
				if (m_owner->m_statusBar != NULL)
					m_owner->m_statusBar->SetStatusText (_(" Retrying..."), STATUS_CONN);
            }
			break;
		default:		// cancel exits
			m_owner->Close (true);
			word = C_NODATA;
		}
    }
        
    return word;
}

// Get a word from the main data ring for the GSW emulation.  The 
// "catchup" flag is true if this is the first word for the current
// burst of sound data, meaning that the display should be made to
// catch up to this point.  We do that by walking back through the
// ring of data from GSW to the display (m_gswRing) clearing out the
// delay setting for any words that haven't been processed yet.
// This is necessary because the GSW paces the display.  The 1/60th
// second timer roughly does the same, but in case it is off the two
// could end up out of sync, so we force them to resync here.
int PtermConnection::NextGswWord (bool catchup)
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
        if (catchup)
        {
            while (next != m_gswIn)
            {
                // The display has some catching up to do.
                if (int (m_gswRing[next]) >= 0)
                {
                    m_gswRing[next] &= 01777777;
                }
                next++;
                if (next == GSWRINGSIZE)
                {
                    next = 0;
                }
            }
            wxWakeUpIdle ();
        }
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
        m_gswRing[m_gswIn] = word | (1 << 19);
        m_gswIn = next;
    }
    
    return word;
}

int ptermNextGswWord (void *connection, int catchup)
{
    return ((PtermConnection *) connection)->NextGswWord (catchup != 0);
}

void PtermConnection::StoreWord (int word)
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
    
#ifdef DEBUG
//    wxLogMessage ("data from plato %07o", word);
#endif
}

void PtermConnection::SendData (const void *data, int len)
{
    // Windows has the wrong type for the buffer pointer argument...
    send(m_fet->connFd, (const char *) data, len, 0);
}
