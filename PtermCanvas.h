#ifndef __PtermCanvas_H__
#define __PtermCanvas_H__ 1

#include "CommonHeader.h"
class PtermFrame;

// define a scrollable canvas for drawing onto
class PtermCanvas : public wxScrolledCanvas
{
public:
    PtermCanvas(PtermFrame *parent);

    void ptermTouchPanel(bool enable);

    void OnDraw(wxDC &dc);
    void OnCharHook(wxKeyEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnMouseDown(wxMouseEvent &event);
    void OnMouseUp(wxMouseEvent &event);
    void OnMouseContextMenu(wxMouseEvent &event);
    void OnMouseMotion(wxMouseEvent &event);
    void OnMouseWheel(wxMouseEvent &event);

#if defined (__WXMSW__)
    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif

    void Unadjust(int x, int y, int *xx, int *yy) const;

    int m_mouseX;
    int m_mouseY;

private:
    PtermFrame * m_owner;
    bool        m_touchEnabled;

    DECLARE_EVENT_TABLE()
};

#endif  // __PtermCanvas_H__

