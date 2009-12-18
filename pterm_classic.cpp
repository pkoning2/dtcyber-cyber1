/*--------------------------------------------------------------------------
**  Purpose:        Process byte of PLATO output data
**
**  Parameters:     Name        Description.
**                  d           8-bit byte
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermClassic::procPlatoByte (u8 d)
{
    mptr mp;
    const char *msg = "";
    int i, j, n = 0;

    // used in load coordinate
    int &coord = (d & 01000) ? currentY : currentX;
    int &cx = (vertical) ? currentY : currentX;
    int &cy = (vertical) ? currentX : currentY;
    
    int deltax, deltay, supdelta;

	bool settitleflag = false;
    
	if (m_usefont && currentCharset <= 1)
	{
		supdelta = (m_fontheight/3);
		// Not going to support reverse/vertical in font mode until I get documentation
		// on what worked with -font- in the past.  JWS 5/27/2007
		deltax = 8;
		deltay = m_fontheight;
	}
	else
	{
		deltax = (reverse) ? -8 : 8;
		deltay = (vertical) ? -16 : 16;
		if (large)
		{
			deltax *= 2;
			deltay *= 2;
		}
		supdelta = (deltay / 16) * 5;
	}
    
    //seq++;
    if (tracePterm)
    {
        fprintf (traceF, "%07o ", d);
    }
    switch (m_asmBytes)
    {
    case 0:
        if (d & 0200)
        {
            printf ("Plato output out of sync byte 0: %03o\n", d);
        }
        else
        {
            m_assembler = d << 12;
            m_asmBytes++;
        }
        return;
    case 1:
        if ((d & 0300) != 0200)
        {
            printf ("Plato output out of sync byte 1: %03o\n", d);
            if ((j & 0200) == 0)
            {
                m_assembler = d << 12;
            }
        }
        else
        {
            m_assembler |= (d & 077) << 6;
            m_asmBytes++;
        }
        return;
    case 2:
        if ((d & 0300) != 0300)
        {
            printf ("Plato output out of sync byte 2: %03o\n", d);
            if ((d & 0200) == 0)
            {
                m_assembler = d << 12;
                m_asmBytes = 1;
            }
            return;
        }
        m_assembler |= k & 077;
        m_asmBytes = 0;
    }

    d = m_assembler;
    if ((d & NOP_MASK) == 0)
    {
        // NOP command...
        if (d & 1)
        {
            wc = (wc + 1) & 0177;
        }
    }
    else
    {
        wc = (wc + 1) & 0177;
    }
    
    if (d & 01000000)
    {
        modewords++;
        mp = modePtr[mode >> 2];
        (this->*mp) (d);
    }
    else
    {
        switch ((d >> 15) & 7)
        {
        case 0:     // nop
            settitleflag = false;
            // special code to tell pterm the station number
            if ((d & NOP_MASKDATA) == NOP_SETSTAT)
            {
                d &= 0777;
                ptermSetStation (d, true, ptermApp->m_showStatusBar);
            }
            // special code to get font type / size / flags
            else if ((d & NOP_MASKDATA) == NOP_FONTTYPE)
                SetFontFaceAndFamily(d & 077);
            else if ((d & NOP_MASKDATA) == NOP_FONTSIZE)
                SetFontSize(d & 077);
            else if ((d & NOP_MASKDATA) == NOP_FONTFLAG)
            {
                SetFontFlags(d & 077);
                SetFontActive();
            }
            else if ((d & NOP_MASKDATA) == NOP_FONTINFO)
            {
                const int chardelay = atoi(ptermApp->m_charDelay.mb_str());
                ptermSendExt((int)m_fontwidth);
                wxMilliSleep(chardelay);
                ptermSendExt((int)m_fontheight);
                wxMilliSleep(chardelay);
            }
            else if ((d & NOP_MASKDATA) == NOP_OSINFO)
            {
                const int chardelay = atoi(ptermApp->m_charDelay.mb_str());
                // sends 3 external keys, OS, major version, minor version
                int os,major,minor;
                os = wxGetOsVersion(&major,&minor);
                ptermSendExt(os);
                wxMilliSleep(chardelay);
                if (os==wxMAC || os==wxMAC_DARWIN)
                    ptermSendExt(10*(major>>4) + (major &0x0f));
                else
                    ptermSendExt(major);
                wxMilliSleep(chardelay);
                if (os==wxMAC || os==wxMAC_DARWIN)
                    ptermSendExt(10*(minor>>4) + (minor &0x0f));
                else
                    ptermSendExt(minor);
                wxMilliSleep(chardelay);
            }
            // otherwise check for plato meta data codes
            else
            {
                if ((d & NOP_MASKDATA) == NOP_PMDSTART && !m_loadingPMD)
                {
                    m_loadingPMD = true;
                    m_PMD = wxT("");
                    settitleflag = AssembleClassicPlatoMetaData(d & 077);
                }
                else if ((d & NOP_MASKDATA) == NOP_PMDSTREAM && m_loadingPMD) 
                {
                    settitleflag = AssembleClassicPlatoMetaData(d & 077);
                }
                else if ((d & NOP_MASKDATA) == NOP_PMDSTOP && m_loadingPMD)
                {
                    m_loadingPMD = false;
                    AssembleClassicPlatoMetaData(d & 077);
                    settitleflag = true;
                }
                if (settitleflag)
                    ProcessPlatoMetaData();
            }
            TRACEN ("nop");
            break;

        case 1:     // load mode
            modewords = 0;              // words since entering mode
            if ((d & 020000) != 0)
            {
                // load wc bit is set
                wc = (d >> 6) & 0177;
            }
            modexor = false;
            mode = (d >> 1) & 037;
            if (d & 1)
            {
                // full screen erase
                ptermFullErase ();
            }
            TRACE2 ("load mode %d screen %d", mode, (d & 1));
            break;
            
        case 2:     // load coordinate
            
            if (d & 04000)
            {
                // Add or subtract from current coordinate
                if (d & 02000)
                {
                    coord -= d & 0777;
                }
                else
                {
                    coord += d & 0777;
                }
            }
            else
            {
                coord = d & 0777;
            }
        
            if (d & 010000)
            {
                setMargin (coord);
                msg = "margin";
            }
            TRACE3 ("load coord %c %d %s", (d & 01000) ? 'Y' : 'X', d & 0777, msg);
            break;
        case 3:     // echo
            d &= 0177;
            switch (d)
            {
            case 0160:
                // 160 is terminal type query
                TRACE ("load echo termtype %d", TERMTYPE);
                d = 0160 + TERMTYPE;
                break;
            case 0x7b:
                // hex 7b is beep
                if (ptermApp->m_beepEnable)
                {
                    TRACEN ("beep");
                    wxBell ();
                    if (!IsActive())
                        RequestUserAttention(wxUSER_ATTENTION_INFO);
                }
                break;
            case 0x7d:
                // hex 7d is report MAR
                TRACE ("report MAR %o", memaddr);
                d = memaddr;
                break;
            default:
                TRACE ("load echo %d", d);
            }
            if (d == 0x7b)
            {
                break;          // -beep- does NOT send an echo code in reply
            }
            d += 0200;
            if (m_conn->RingCount () > RINGXOFF1)
            {
                m_pendingEcho = d;
            }
            else
            {
                ptermSendKey (d);
                m_pendingEcho = -1;
            }
            break;
            
        case 4:     // load address
            memaddr = d & 077777;
            TRACE2 ("load address %o (0x%x)", memaddr, memaddr);
            break;
            
        case 5:     // SSF on PPT
            switch ((d >> 10) & 037)
            {
            case 1: // Touch panel control ?
                TRACE ("ssf touch %o", d);
                m_canvas->ptermTouchPanel ((d & 040) != 0);
                break;
            default:
                TRACE ("ssf %o", d);
                break;  // ignore
            }
            break;

        case 6:
        case 7:
            d &= 0177777;
            TRACE ("Ext %07o", d);
            // Take no other action here -- it's been done already
            // when the word was fetched
            break;

        default:    // ignore
            TRACE ("ignored command word %07o", d);
            break;
        }
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a string for use as frame caption from data
**                  sent in Classic connection mode.
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 
**
**------------------------------------------------------------------------*/
bool PtermFrame::AssembleClassicPlatoMetaData (int d)
{
    TRACE ("plato meta data: %d", d);
	if (m_PMD.Len()==1000)
	{
		TRACE ("plato meta data limit reached: 1000 bytes","");
		m_loadingPMD = false;
		return true;
	}
	else
	{
		d &= 077;
		if (d >= 1 && d <= 26)
		{
			m_PMD.Append('a'+d-1,1);
		}
		else if (d >= 27 && d <= 36)
		{
			m_PMD.Append('0'+d-27,1);
		}
		else if (d == 38)
		{
			m_PMD.Append('-',1);
		}
		else if (d == 40)
		{
			m_PMD.Append('/',1);
		}
		else if (d == 44)
		{
			m_PMD.Append('=',1);
		}
		else if (d == 45)
		{
			m_PMD.Append(' ',1);
		}
		else if (d == 63)
		{
			m_PMD.Append(';',1);
		}
	}
	return false;
}

