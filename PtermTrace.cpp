////////////////////////////////////////////////////////////////////////////
// Name:        PtermTrace.cpp
// Purpose:     Implementation of class Tracing execution 
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#include "PtermTrace.h"

Trace::Trace ()
{
    tp = NULL;
    fd = NULL;
}

void Trace::Open (const char *fn)
{
    Close ();
    if (fn == NULL || fn[0] == '\0')
    {
        fd = stdout;
    }
    else
    {
        fd = fopen (fn, "w");
        if (fd == NULL)
        {
            fprintf (stderr, "Failure opening trace file %s\n", fn);
            return;
        }
    }

    tp = new wxMessageOutputStderr (fd);
    if (tp == NULL)
    {
        fprintf (stderr, "Failure opening trace message object\n");
        fclose (fd);
        fd = NULL;
    }
}

void Trace::Close (void)
{
    if (fd != NULL)
    {
        delete tp;
        tp = NULL;
        if (fd != stdout)
        {
            fclose (fd);
        }
        fd = NULL;
    }
}

void Trace::Log (const char *fmt, ...)
{
    va_list v;
    wxString msg;
    
    if (Active ())
    {
        va_start (v, fmt);
        msg.PrintfV (fmt, v);
        va_end (v);
        Log (msg);
    }
}

void Trace::Log (const wxString &s)
{
    char tbuf[10];
    wxString hdr;

    if (Active ())
    {
#ifdef _WIN32
        SYSTEMTIME tv;

        GetLocalTime (&tv);
        GetTimeFormatA (LOCALE_SYSTEM_DEFAULT, 0, &tv, 
                        "HH':'mm':'ss", &tbuf[0], 10);
        hdr.Printf ("%s.%03d: ", tbuf, tv.wMilliseconds);
#else
        struct timeval tv;
        struct tm tmbuf;
        
        gettimeofday (&tv, NULL);
        strftime (tbuf, 10, "%T", localtime_r (&tv.tv_sec, &tmbuf));
        hdr.Printf ("%s.%03ld: ", tbuf, (long) tv.tv_usec / 1000);
#endif
        hdr.Append (s);
        hdr.Append ("\n");
        tp->Output (hdr);
    }
}

