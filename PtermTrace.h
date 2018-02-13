////////////////////////////////////////////////////////////////////////////
// Name:        PtermTrace.h
// Purpose:     Declaration of class Tracing execution 
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __Trace_H__
#define __Trace_H__ 1

#include "CommonHeader.h"

class Trace
{
public:
    Trace ();
    void Open (const char *fn = NULL);
    void Close (void);
    // Note that the "format" attribute gives the argument index counting
    // the implied "this" as argument #1.
#ifdef _MSC_VER
    void Log (const char *fmt, ...);
#else
    void Log (const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
#endif
    void Log (const wxString &s);
    bool Active (void) const
    {
        return (fd != NULL);
    }
    
private:
    wxMessageOutputStderr *tp;
    FILE *fd;
};

#endif  // __Trace_H__
