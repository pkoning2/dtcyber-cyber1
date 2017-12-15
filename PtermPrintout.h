////////////////////////////////////////////////////////////////////////////
// Name:        PTermPrintout.h
// Purpose:     Definition of the Screen Print
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __PTermPrintout_H__
#define __PTermPrintout_H__ 1

#include "CommonHeader.h"

class PtermFrame;

// Pterm screen printout
class PtermPrintout : public wxPrintout
{
public:
    PtermPrintout(PtermFrame *owner,
        const wxString &title = _("Pterm printout"))
        : wxPrintout(title),
        m_owner(owner)
    {}
    bool OnPrintPage(int page);
    bool HasPage(int page);
    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom,
        int *selPageTo);
    void DrawPage(wxDC *dc);

private:
    PtermFrame * m_owner;
};

#endif  // __PTermPrintout_H__