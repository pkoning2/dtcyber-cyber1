/*--------------------------------------------------------------------------
**  Purpose:        Process byte of PLATO output data
**
**  Parameters:     Name        Description.
**                  d           8-bit byte
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermAscii::procPlatoByte (u8 d)
{
    mptr mp;
    const char *msg = "";
    int i, j, n = 0;
    AscState    asmState;

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
#ifdef DEBUGASCII
    if (!m_dumbTty)
    {
        int c = ((d & 127) >= 32 && (d & 127) < 127) ? d & 127 : '.';
        printf ("%04x  %c\n", d, c);
    }
#endif

    switch (m_asmState)
    {
    case pni_rs:
        // We just want to ignore 3 command codes.  Note that escape
        // sequences count for one, not two.
        if (++m_asmBytes == 3)
        {
            m_asmBytes = 0;
            m_asmState = none;
        }
        break;
    case ldc:
        if (AssembleCoord (d))
        {
            currentX = lastX;
            currentY = lastY;
            TRACE2 ("load coordinate %d %d", currentX, currentY);
        }
        break;
    case paint:
        n = AssemblePaint (d);
        if (n != -1)
        {
            TRACE ("paint %03o", n);
            ptermPaint (n);
        }
        break;
    case lde:
        n = AssembleData (d);
        if (n != -1)
        {
            n &= 0177;
            switch (n)
            {
            case 0160:
                // 160 is terminal type query
                TRACE ("load echo termtype %d", ASCTYPE);
                n = 0160 + ASCTYPE;
                break;
            case 0x71:
                TRACE ("load echo subtype %d", SUBTYPE);
                n = SUBTYPE;
                break;
            case 0x72:
                TRACEN ("load echo loadfile (unused)");
                n = 0;
                break;
            case 0x73:
                // hex 73 is report terminal config
                TRACEN ("load echo termdata");
                n = TERMCONFIG;
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
                n = memaddr;
                break;
            case 0x52:
                // hex 52 is enable flow control
                TRACEN ("enable flow control");
                m_flowCtrl = true;
                n = 0x53;
                break;
            case 0x60:
                // hex 60 is inquire features
                TRACE ("report features 0x%02x", ASCFEATURES);
                n += ASCFEATURES;
                m_sendFgt = true;
                break;
            default:
                TRACE2 ("load echo %d (0x%02x)", n, n);
            }
            if (n == 0x7b)
                break;          // -beep- does NOT send an echo code in reply
            n += 0200;
            if (m_conn->RingCount () > RINGXOFF1)
                m_pendingEcho = n;
            else
            {
                ptermSendKey (n);
                m_pendingEcho = -1;
            }
        }
        break;
    case lda:
        n = AssembleData (d);
        if (n != -1)
        {
            TRACE ("load memory address %04x", n);
            memaddr = n & 077777;
        }
        break;
    case ext:
        TRACE ("ext %04x", d);
        n = AssembleData (d);
        switch (n)
        {
        case -1:
            break;
            // check for special TERM area save/restore
        case CWS_TERMSAVE:
            TRACE ("ext completed %04x", n);
            cwswindow[0].data[0] = 0;
            cwswindow[0].data[1] = 48;
            cwswindow[0].data[2] = 511;
            cwswindow[0].data[3] = 0;
            ptermSaveWindow(0);
            break;
        case CWS_TERMRESTORE:
            TRACE ("ext completed %04x", n);
            ptermRestoreWindow(0);
            break;
        default:
            TRACE ("ext completed %04x", n);
            // check if in cws mode
            switch (cwsmode)
            {
                // not in cws; check for font data
            case 0:
                cwscnt = 0;
                switch (n & 07700)
                {
                case 05000:		// font face name and family
                    SetFontFaceAndFamily(n & 077);
                    break;
                case 05100:		// font size
                    SetFontSize(n & 077);
                    break;
                case 05200:		// font flags
                    SetFontFlags(n & 077);
                    SetFontActive();
                    break;
                }
                break;
                // check for cws data mode
            case 1:
                cwscnt++;
                if      (cwscnt==1 && n==CWS_SAVE)			
                {
                    TRACEN ("CWS: specify save function");
                    cwsfun = CWS_SAVE;
                }
                else if (cwscnt==1 && n==CWS_RESTORE)	
                {
                    TRACEN ("CWS: specify restore function");
                    cwsfun = CWS_RESTORE;
                }
                else if (cwscnt==1 || cwscnt>6)
                {
                    TRACE ("CWS: invalid function; %d", n);
                    cwsmode=0, cwsfun=0, cwscnt=0;	// unknown function; terminate cws mode
                }
                else if (cwscnt==2)
                {
                    TRACE ("CWS: specify window; %d", n);
                    cwswin = n;
                }
                else if (cwscnt<7)
                {
                    TRACE ("CWS: data; %d", n);
                    cwswindow[cwswin].data[cwscnt-3] = n;
                }
                break;
                // check for cws execute mode
            case 2:
                cwscnt = 0;
                if (n==CWS_EXEC)
                {
                    TRACEN ("CWS: process exec");
                    switch (cwsfun)
                    {
                    case CWS_SAVE:
                        ptermSaveWindow(cwswin);
                        break;
                    case CWS_RESTORE:
                        ptermRestoreWindow(cwswin);
                        break;
                    }
                }
                else
                {
                    cwsmode=0, cwsfun=0, cwscnt=0;	// unknown function; terminate cws mode
                }
                break;
            }
        }
        break;
    case ssf:
        n = AssembleData (d);
        if (n != -1)
        {
            TRACE ("ssf %04x", n);
            m_canvas->ptermTouchPanel ((n & 0x20) != 0);
        }
        switch (n)
        {
        case 0x1f00:	// xin 7; means start CWS functions
            TRACE ("ssf; start cws mode; %04x", n);
            cwsmode = 1;
            break;
        case 0x1d00:	// xout 7; means stop CWS functions
            TRACE ("ssf; stop cws mode; %04x", n);
            cwsmode = 2;
            break;
        case -1:
            break;
        default:
            TRACE ("ssf %04x", n);
            m_canvas->ptermTouchPanel ((n & 0x20) != 0);
            break;
        }
        break;
    case fg:
    case bg:
        asmState = m_asmState;
        n = AssembleColor (d);
        if (n != -1 && !ptermApp->m_noColor)
        {
            wxColour c ((n >> 16) & 0xff, (n >> 8) & 0xff, n & 0xff);
            if (asmState == fg)
            {
                //TRACE ("set foreground color %06x", n);
                m_currentFg = c;
            }
            else
            {
                //TRACE ("set background color %06x", n);
                m_currentBg = c;
            }
            SetColors (m_currentFg, m_currentBg, m_scale);
        }
        break;
    case gsfg:
        asmState = m_asmState;
        n = AssembleGrayScale (d);
        if (n != -1 && !ptermApp->m_noColor)
        {
            wxColour c (n & 0xff, n & 0xff, n & 0xff);
            if (asmState == gsfg)
            {
                //TRACE ("set gray-scale foreground color %06x", n);
                m_currentFg = c;
            }
            SetColors (m_currentFg, m_currentBg, m_scale);
        }
        break;
    case pmd:
        n = AssembleAsciiPlatoMetaData (d);
        if (n == 0)
        {
            if (m_fontPMD)
            {
                TRACEN ("plato meta data complete: font data accepted");
                TRACE3 ("Font selected: %s,%d,%d", m_fontface.c_str(), m_fontsize, m_fontbold|m_fontitalic|m_fontstrike|m_fontunderln);
                m_fontPMD = false;
            }
            else if (m_fontinfo)
            {
                TRACEN ("plato meta data complete: get font data accepted and sent");
                m_fontinfo = false;
            }
            else if (m_osinfo)
            {
                TRACEN ("plato meta data complete: get operating system info accepted and sent");
                m_osinfo = false;
            }
            else
            {
                TRACE ("plato meta data complete: %s", m_PMD.c_str());
                ProcessPlatoMetaData();
            }
            m_PMD = wxT("");
        }
        break;
    case esc:
        // Escape sequence
        m_asmState = norm;          // default to normal when done
        switch (d)
        {
        case 002:   // ESC STX
            if (m_dumbTty)
            {
                TRACEN ("Entering PLATO terminal mode");
                m_dumbTty = false;
                mode = (3 << 2) + 1;    // set character mode, rewrite
                ptermSetStation (-1, true, ptermApp->m_showStatusBar);   // Show connected in ASCII mode
            }
            else
            {
                TRACEN ("Still in PLATO terminal mode");
            }
            break;
        case 003:   // ESC ETX
            TRACEN ("Leaving PLATO terminal mode");
            m_dumbTty = true;
            m_flowCtrl = false;
            m_sendFgt = false;
            currentX = 0;
            currentY = 496;
            break;
        case 014:   // ESC FF
            TRACEN ("Full screen erase");
            ptermFullErase ();
            break;
        case 026:
            // mode xor (also sets mode write for off-screen DC operations)
            TRACEN ("load mode xor");
            modexor = true;
            mode = (mode & ~3) + 2;
            break;
        case 021:   // ESC DC1
        case 022:   // ESC DC2
        case 023:   // ESC DC3
        case 024:   // ESC DC4
            // modes inverse, write, erase, rewrite
            modexor = false;
            mode = (mode & ~3) + ascmode[d - 021];
            //mode = 017;
            TRACE ("load mode %d", mode);
            break;
        case '2':
            // Load coordinate
            TRACEN ("Start load coordinate");
            m_asmState = ldc;
            m_asmBytes = 0;
            break;
        case '@':
            // superscript
            TRACEN ("Superscript");
            cy = (cy + supdelta) & 0777;
            break;
        case 'A':
            // subscript
            TRACEN ("Subscript");
            cy = (cy - supdelta) & 0777;
            break;
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
            TRACE ("select memory M%d", d - 'B');
            setCmem (d - 'B');
            break;
        case 'J':
            TRACEN ("Horizontal writing mode");
            setVertical (false);
            break;
        case 'K':
            TRACEN ("Vertical writing mode");
            setVertical (true);
            break;
        case 'L':
            TRACEN ("Forward writing mode");
            setReverse (false);
            break;
        case 'M':
            TRACEN ("Reverse writing mode");
            setReverse (true);
            break;
        case 'N':
            TRACEN ("Normal size writing mode");
            setLarge (false);
            break;
        case 'O':
            TRACEN ("Double size writing mode");
            setLarge (true);
            break;
        case 'P':
            modexor = false;
            mode = (mode & 3) + (2 << 2);
            TRACE ("load mode %d", mode);
            break;
        case 'Q':
            m_asmState = ssf;
            m_asmBytes = 0;
            TRACEN ("Start SSF");
            break;
        case 'R':
            // external data
            m_asmState = ext;
            m_asmBytes = 0;
            TRACEN ("Start ext");
            break;
        case 'S':
            modexor = false;
            mode = (mode & 3) + (2 << 2);
            TRACE ("load mode %d", mode);
            break;
        case 'T':
            modexor = false;
            mode = (mode & 3) + (5 << 2);
            TRACE ("load mode %d", mode);
            break;
        case 'U':
            modexor = false;
            mode = (mode & 3) + (6 << 2);
            TRACE ("load mode %d", mode);
            break;
        case 'V':
            modexor = false;
            mode = (mode & 3) + (7 << 2);
            TRACE ("load mode %d", mode);
            break;
        case 'W':
            // Load memory address
            TRACEN ("Start LDA");
            m_asmState = lda;
            m_asmBytes = 0;
            break;
        case 'X':
            TRACEN ("Start load plato meta data");
            m_asmState = pmd;
            m_asmBytes = 0;
            break;
        case 'Y':
            // load echo
            TRACEN ("Start LDE");
            m_asmState = lde;
            m_asmBytes = 0;
            break;
        case 'Z':
            // set margin
            TRACE ("set margin %d", currentX);
            setMargin (cx);
            break;
        case 'a':
            // set foreground color
            TRACEN ("Start foreground color");
            m_asmState = fg;
            m_asmBytes = 0;
            break;
        case 'b':
            // set background color
            TRACEN ("Start background color");
            m_asmState = bg;
            m_asmBytes = 0;
            break;
        case 'c':
            // paint
            TRACEN ("Start paint");
            m_asmState = paint;
            m_asmBytes = 0;
            break;
        case 'g':
            // set gray-scale foreground color
            TRACEN ("Start grayscale foreground color");
            m_asmState = gsfg;
            m_asmBytes = 0;
            break;
        default:
            TRACE ("Other unknown ESCAPE sequence: %d", d);
            break;
        }
        break;
    case norm:
        if (m_dumbTty)
        {
            if (d >= 32 && d < 127)
            {
                d = asciiM0[d];
                if ((d & 0xf0) != 0xf0)
                {
                    // Force mode rewrite
                    mode = (3 << 2) + 1;
                    ptermDrawChar (currentX, currentY, (d & 0x80) >> 7, d & 0x7f);
                    currentX = (currentX + 8) & 0777;
                }
            }
            else if (d == 015)
            {
                currentX = 0;
            }
            else if (d == 012)
            {
#if 0
                wxClientDC dc(m_canvas);
                PrepareDC (dc);
                m_memDC->SetBackground (m_backgroundBrush);
                if (currentY != 0)
                {
                    currentY -= 16;
                }
                else
                {
                    // On the bottom line... scroll.
                    dc.Blit (XTOP, YTOP, vScreenSize(m_scale), vScreenSize(m_scale) - (16 * ((m_stretch) ? 1 : m_scale)), m_memDC, XMTOP, YMADJUST (496), wxCOPY);
                    m_memDC->Blit (XMTOP, YMTOP, vScreenSize(m_scale), vScreenSize(m_scale) - (16 * ((m_stretch) ? 1 : m_scale)), &dc, XTOP, YTOP, wxCOPY);
                }
                // Erase the line we just moved to.
                m_memDC->Blit (XMTOP, YMADJUST (16) + 1, vScreenSize(m_scale), 16 * ((m_stretch) ? 1 : m_scale), m_memDC, 0, 0, wxCLEAR);
                dc.Blit (XTOP, YADJUST (16) + 1, vScreenSize(m_scale), 16 * ((m_stretch) ? 1 : m_scale), &dc, 0, 0, wxCLEAR);
#endif
            }
            break;
        }

        // PLATO mode.  First look for control characters
        switch (d)
        {
        case 010:   // backspace
            cx = (cx - deltax) & 0777;
            TRACEN ("backspace");
            break;
        case 011:   // tab
            TRACEN ("tab");
            cx = (cx + deltax) & 0777;
            break;
        case 012:   // linefeed
            TRACEN ("linefeed");
            cy = (cy - deltay) & 0777;
            break;
        case 013:   // vertical tab
            TRACEN ("vertical tab");
            cy = (cy - deltay) & 0777;
            break;
        case 014:   // form feed
            TRACEN ("form feed");
            if (vertical)
            {
                cx = deltay - 1;
                cy = reverse ? 512 - deltax : 0;
            }
            else
            {
                cy = 512 - deltay;
                cx = reverse ? 512 - deltax : 0;
            }
            break;
        case 015:   // carriage return
            cx = margin;
            cy = (cy - deltay) & 0777;
            TRACE2 ("CR to %d %d", currentX, currentY);
            break;
        case 031:   // EM
            mode = (mode & 3) + (4 << 2);
            modewords = 0;              // words since entering mode
            TRACE ("load mode %d", mode);
            break;
        case 033:   // ESC
            m_asmState = esc;
            break;
        case 034:   // FS
            mode = (mode & 3) + (0 << 2);
            TRACE ("load mode %d", mode);
            break;
        case 035:   // FS
            mode = (mode & 3) + (1 << 2);
            m_asmState = ldc;// to have first coordinate be "dark"
            TRACE ("load mode %d", mode);
            break;
        case 036:   // RS -- used by PNI in connect handshake
            m_asmState = pni_rs;
            TRACEN ("pni start download, ignoring next 3 commands");
            break;
        case 037:   // FS
            mode = (mode & 3) + (3 << 2);
            TRACE ("load mode %d", mode);
            break;
        }
        if (d >= 040)
        {
            switch (mode >> 2)
            {
            case 0:
                if (AssembleCoord (d))
                {
                    mode0 ((lastX << 9) + lastY);
                }
                break;
            case 1:
                if (AssembleCoord (d))
                {
                    mode1 ((lastX << 9) + lastY);
                }
                break;
            case 2:
                n = AssembleData (d);
                if (n != -1)
                {
                    mode2 (n);
                }
                break;
            case 3: // text mode
                TRACE2 ("char %03o (%c)", d, d);
                m_asmState = none;
                m_asmBytes = 0;
                i = currentCharset;
                if (m_usefont && i == 0)
                    drawFontChar(currentX, currentY, d);
                else
                {
                    if (i == 0)
                    {
                        d = asciiM0[d];
                        // The ROM vs. RAM choice is given by the
                        // current character set.  
                        // For the ROM characters, the even vs. odd
                        // (M0 vs. M1) choice is given by the top bit
                        // of the ASCII translation table.
                        i = (d & 0x80) >> 7;
                    }
                    else if (i == 1)
                    {
                        d = asciiM1[d];
                        i = (d & 0x80) >> 7;
                    }
                    else
                    {
                        // RAM characters are indexed by printable
                        // ASCII characters; the RAM character offset
                        // is simply the character code - 32.
                        // The set choice is simply what the host sent.
                        d = (d - 040) & 077;
                    }
                    if ((d & 0xf0) == 0xf0)
                    {
                        if (d != 0xff)
                        {
                            // A builtin composite
                            int savemode = mode;
                            int sy = cy;
                                
                            d -= 0xf0;
                            for (i = 0; i < 8; i += 2)
                            {
                                n = M1specials[d][i];
                                if (n == 0)
                                {
                                    break;
                                }
                                j = (i8) M1specials[d][i + 1];
                                cy += j;
                                ptermDrawChar (currentX, currentY, n >> 7, n & 0x7f);
                                cy = sy;
                                mode |= 2;
                            }
                            mode = savemode;
                            cx = (cx + deltax) & 0777;
                        }
                    }   
                    else 
                    {
                        ptermDrawChar (currentX, currentY, i, d & 0x7f);
                        cx = (cx + deltax) & 0777;
                    }
                }
                break;
            case 4:
                if (AssembleCoord (d))
                {
                    modewords++;
                    mode4 ((lastX << 9) + lastY);
                }
                break;
            case 5:
                n = AssembleData (d);
                if (n != -1)
                    mode5 (n);
                break;
            case 6:
                n = AssembleData (d);
                if (n != -1)
                    mode6 (n);
                break;
            case 7:
                n = AssembleData (d);
                if (n != -1)
                    mode7 (n);
                break;
            }
            break;
        }
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a 9 bit data word for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise the word
**
**------------------------------------------------------------------------*/
int PtermAscii::AssemblePaint (int d)
{
    if (m_asmBytes == 0)
    {
        m_assembler = 0;
    }    
    m_assembler |= ((d & 077) << (m_asmBytes * 6));
    if (++m_asmBytes == 2)
    {
        m_asmBytes = 0;
        m_asmState = none;
        TRACE2 ("paint %03o (0x%04x)", m_assembler, m_assembler);
        return m_assembler;
    }
    else
    {
        TRACE2 ("paint byte %d: %d", m_asmBytes, d & 077);
    }
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble an 18 bit data word for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise the word
**
**------------------------------------------------------------------------*/
int PtermAscii::AssembleData (int d)
{
    if (m_asmBytes == 0)
    {
        m_assembler = 0;
    }    
    m_assembler |= ((d & 077) << (m_asmBytes * 6));
    if (++m_asmBytes == 3)
    {
        m_asmBytes = 0;
        m_asmState = none;
        TRACE2 ("data %07o (0x%04x)", m_assembler, m_assembler);
        return m_assembler;
    }
    else
    {
        TRACE2 ("data byte %d: %d", m_asmBytes, d & 077);
    }
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a 24 bit color word for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise the color word
**
**------------------------------------------------------------------------*/
int PtermAscii::AssembleColor (int d)
{
    if (m_asmBytes == 0)
    {
        m_assembler = 0;
    }    
    m_assembler |= ((d & 077) << (m_asmBytes * 6));
    if (++m_asmBytes == 4)
    {
        m_asmBytes = 0;
        m_asmState = none;
        //TRACE ("color 0x%06x)", m_assembler);
        return m_assembler;
    }
    else
    {
        TRACE2 ("color byte %d: %d", m_asmBytes, d & 077);
    }
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble an 7 bit grayscale word for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise the color word
**
**------------------------------------------------------------------------*/
int PtermAscii::AssembleGrayScale (int d)
{
    if (m_asmBytes == 0)
    {
        m_assembler = 0;
    }    
    m_assembler = (d & 077) << 2;
    if (++m_asmBytes == 1)
    {
        m_asmBytes = 0;
        m_asmState = none;
        TRACE ("gray-scale color 0x%06x", m_assembler);
        return m_assembler;
    }
    else
    {
        TRACE2 ("gray-scale color byte %d: %d", m_asmBytes, d & 0177);
    }
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a coordinate pair for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        true if word is complete, false if not
**                  lastX and lastY are the x/y coordinate received
**
**------------------------------------------------------------------------*/
bool PtermAscii::AssembleCoord (int d)
{
    int c = d & 037;
    
    switch (d >> 5)
    {
    case 1: // High X or high Y
        if (m_asmBytes == 0)
        {
            // High Y
            TRACE ("high Y %d", c);
            lastY = (lastY & 037) | (c << 5);
            m_asmBytes = 2;
        }
        else
        {
            TRACE ("high X %d", c);
            lastX = (lastX & 037) | (c << 5);
        }
        break;
    case 2:
        lastX = (lastX & 0740) | c;
        m_assembler = (lastX << 16) + lastY;
        m_asmBytes = 0;
        m_asmState = none;
        TRACE3 ("low X %d, coordinates %d %d", c, lastX, lastY);
        return true;
    case 3:
        TRACE ("low Y %d", c);
        lastY = (lastY & 0740) | c;
        m_asmBytes = 2;
        break;
    }
    return false;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a string for use as extended data from Plato
**                  sent in ASCII connection mode.  Could be meta data
**                  for setting window caption or could be font data, or
**                  other data someday.
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise 0 and get the
**                     data from m_PlatoMetaData
**
**------------------------------------------------------------------------*/
int PtermAscii::AssembleAsciiPlatoMetaData (int d)
{
	int od = d;
    const int chardelay = atoi(ptermApp->m_charDelay.mb_str());

    TRACE2 ("plato meta data: %d (counter=%d)", d, m_asmBytes+1);
	d &= 077;
    if (m_asmBytes==0)
		m_PMD = wxT("");
	m_asmBytes++;
	// check for start font mode
	if (od=='F' && m_asmBytes==1)
		m_fontPMD = true;
	// check for request font character info
	else if (od=='f' && m_asmBytes==1)
	{
		m_fontinfo = true;
		m_asmBytes = 0;
		m_asmState = none;
		ptermSendExt((int)m_fontwidth);
		wxMilliSleep(chardelay);
		ptermSendExt((int)m_fontheight);
		wxMilliSleep(chardelay);
		return 0;
	}
	// check for request operating system info
	else if (od=='o' && m_asmBytes==1)
	{
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
		m_osinfo = true;
		m_asmBytes = 0;
		m_asmState = none;
		return 0;
	}
	// check if in font mode
	else if (m_fontPMD && m_asmBytes==2)
	{
		SetFontFaceAndFamily(d);
		if (d==0)
		{
			m_asmBytes = 0;
			m_asmState = none;
			return 0;
		}
	}
	else if (m_fontPMD && m_asmBytes==3)
		SetFontSize(d);
	else if (m_fontPMD && m_asmBytes==4)
	{
		SetFontFlags(d);
		SetFontActive();
        m_asmBytes = 0;
        m_asmState = none;
		return 0;
	}
	// check if done / full
    else if (d==0 || m_asmBytes==1001)
    {
		if (m_asmBytes==1001)
		{
		    TRACE ("plato meta data limit reached: %d bytes",1000);
		}
        m_asmBytes = 0;
        m_asmState = none;
        return 0;
    }
	// otherwise keep assembling
    else
    {
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
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Change frame title to string specified in the assembled
**                  meta data.
**
**  Parameters:     none
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermAscii::ProcessPlatoMetaData ()
{
	int fnd;
	int len;
	wxString l_str;
	wxString l_name;
	wxString l_group;
	wxString l_system;
	wxString l_station;

	//special check for roster
	if (m_SendRoster)
		return;
	if ((fnd = m_PMD.Find(wxT("sendroster;"))) != -1)
	{
		SendRoster();
		return;
	}

	//initialize
	l_name = wxT("");
	l_group = wxT("");
	l_system = wxT("");
	l_station = wxT("");
	len = m_PMD.Len();

	//collect meta data items
	if ((fnd = m_PMD.Find(wxT("name="))) != -1)
	{
		l_name = m_PMD.Mid(fnd+5,len-fnd-5);
		l_name = l_name.BeforeFirst(wxT(';'));
	}

	if ((fnd = m_PMD.Find(wxT("group="))) != -1)
	{
		l_group = m_PMD.Mid(fnd+6,len-fnd-6);
		l_group = l_group.BeforeFirst(wxT(';'));
	}

	if ((fnd = m_PMD.Find(wxT("system="))) != -1)
	{
		l_system = m_PMD.Mid(fnd+7,len-fnd-7);
		l_system = l_system.BeforeFirst(wxT(';'));
	}

	if ((fnd = m_PMD.Find(wxT("station="))) != -1)
	{
		l_station = m_PMD.Mid(fnd+8,len-fnd-8);
		l_station = l_station.BeforeFirst(wxT(';'));
	}

	//make title string based on flags
	ptermApp->m_IsRegistrar = false;
	l_str = wxT("");
	if (ptermApp->m_showSignon)
	{
        l_str = l_name;
        l_str.Append (wxT ("/"));
        l_str += l_group;
		ptermApp->m_IsRegistrar = (l_str.Cmp(wxT("registrar/o")) == 0);
	}
	if (ptermApp->m_showSysName)
	{
        l_str.Append (wxT ("/"));
        l_str += l_system;
	}
	if (ptermApp->m_showHost)
	{
        l_str.Append (wxT (" "));
        l_str += m_hostName;
	}
	if (ptermApp->m_showStation)
	{
        l_str.Append (wxT (" ("));
        l_str += l_station;
        l_str.Append (wxT (")"));
	}
	l_str.Trim(true);
	l_str.Trim(false);
	if (l_str.IsEmpty())
	{
		l_str = wxT("Pterm");
	}
	SetTitle (l_str);
}


/*--------------------------------------------------------------------------
**  Purpose:        Set font face name and family flag.
**
**  Parameters:     font type code
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermAscii::SetFontFaceAndFamily (int n)
{
	switch (n)
	{
	case 2:
		m_fontface = wxT("Terminal");
		m_fontfamily = wxFONTFAMILY_TELETYPE;
		break;
	case 3:
		m_fontface = wxT("UOL8X14");
		m_fontfamily = wxFONTFAMILY_TELETYPE;
		break;
	case 4:
		m_fontface = wxT("UOL8X16");
		m_fontfamily = wxFONTFAMILY_TELETYPE;
		break;
	case 5:
		m_fontface = wxT("Courier");
		m_fontfamily = wxFONTFAMILY_TELETYPE;
		break;
	case 6:
		m_fontface = wxT("Courier New");
		m_fontfamily = wxFONTFAMILY_MODERN;
		break;
	case 16:
		m_fontface = wxT("Arial");
		m_fontfamily = wxFONTFAMILY_DECORATIVE;
		break;
	case 17:
		m_fontface = wxT("Times New Roman");
		m_fontfamily = wxFONTFAMILY_ROMAN;
		break;
	case 18:
		m_fontface = wxT("Script");
		m_fontfamily = wxFONTFAMILY_SCRIPT;
		break;
	case 19:
		m_fontface = wxT("MS Sans Serif");
		m_fontfamily = wxFONTFAMILY_SWISS;
		break;
	default:
		m_fontface = wxT("default");
		m_fontfamily = wxFONTFAMILY_TELETYPE;
	}
}

/*--------------------------------------------------------------------------
**  Purpose:        Set font size.
**
**  Parameters:     font size
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermAscii::SetFontSize (int n)
{
	m_fontsize = (n < 1 ? 1 : (n > 63 ? 63 : n)) * m_scale;
}

/*--------------------------------------------------------------------------
**  Purpose:        Set font flags.
**
**  Parameters:     font flag
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermAscii::SetFontFlags (int n)
{
	m_fontitalic = ((n & 0x01) != 0);
	m_fontbold = ((n & 0x02) != 0);
	m_fontstrike = ((n & 0x04) != 0);
	m_fontunderln = ((n & 0x08) != 0);
	TRACE3 ("Font selected: %s,%d,%d", m_fontface.c_str(), m_fontsize, n & 0x0f);
}

/*--------------------------------------------------------------------------
**  Purpose:        Set font.
**
**  Parameters:     none
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermAscii::SetFontActive ()
{
	m_usefont = (m_fontface.Cmp(wxT(""))!=0 && m_fontface.Cmp(wxT("default"))!=0);
	if (m_usefont)
	{
		m_font = new wxFont;
		//m_font->New(m_fontsize, m_fontfamily, wxFONTFLAG_NOT_ANTIALIASED, m_fontface);
		m_font->New(m_fontsize, m_fontfamily, wxFONTFLAG_ANTIALIASED | (m_fontstrike ? wxFONTFLAG_STRIKETHROUGH : 0), m_fontface);
		m_font->SetFaceName(m_fontface);
		m_font->SetFamily(m_fontfamily);
		m_font->SetPointSize(m_fontsize);
		m_font->SetStyle(m_fontitalic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);
		m_font->SetWeight(m_fontbold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
		m_font->SetUnderlined(m_fontunderln);
		m_memDC->SetFont(*m_font);
		m_memDC->GetTextExtent(wxT(" "), &m_fontwidth, &m_fontheight);
	}
}

