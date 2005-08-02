/////////////////////////////////////////////////////////////////////////////
// Name:        knob.cpp
// Purpose:     rotary knob control
// Author:      Paul Koning
// Modified by:
// Created:     08/01/2005
// Copyright:   (c) Paul Koning
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "knob.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/control.h"
    #include "wx/settings.h"
    #include "wx/dc.h"
    #include "wx/dcclient.h"
#endif // WX_PRECOMP

#include "knob.h"
#include <math.h>

#define PI 3.14159265358969323846
#define irnd(x) (int (round (x)))

// ----------------------------------------------------------------------------
// the wxKnob class
// ----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(wxKnob,wxControl)
    EVT_PAINT       (wxKnob::OnPaint)
    EVT_LEFT_DOWN   (wxKnob::OnMouse)
    EVT_LEFT_UP     (wxKnob::OnMouse)
    EVT_MOTION      (wxKnob::OnMouse)
END_EVENT_TABLE()

// Constructor
void wxKnob::Create (wxWindow* parent, wxWindowID id, int value,
                     int minValue, int maxValue,
                     unsigned int minAngle, unsigned int range,
                     const wxPoint &pos, 
                     const wxSize &size, 
                     long style,
                     const wxValidator &validator,
                     const wxString &name)
{
    wxControl::Create (parent, id);
    SetInitialBestSize (size);
//    SetClientSize (size);
    
    m_min = minValue;
    m_max = maxValue;
    range %= 360;
    minAngle %= 360;
    m_maxAngle = (minAngle + 360 - range) % 360;
    
    m_range = range;
    SetValue (value);
}

void wxKnob::SetRange (int minValue, int maxValue)
{
    if (minValue < maxValue)
    {
        m_min = minValue;
        m_max = maxValue;
        SetValue (m_setting);
    }
}
    
int wxKnob::SetValue (int value)
{
    if (value < m_min)
        value = m_min;
    if (value > m_max)
        value = m_max;
    
    if (value != m_setting)
    {
        m_setting = value;
        Refresh ();
        Update ();
        
    }
}

void wxKnob::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    int sizeX, sizeY;
    double theta;
    double dx, dy;
    int cx, cy;
    int r, r2, lw;
    wxSize s = GetSize ();
    wxPen p = *wxWHITE_PEN;
    
    theta = (PI / 180.) * (m_maxAngle +
                           (((double) m_max - m_setting) / (m_max - m_min))
                           * m_range);
    dx = cos (theta);
    dy = -sin (theta);      // - because of upside down coordinates
    
    wxPaintDC dc(this);

    GetCenter (cx, cy);
    r = irnd (((s.x < s.y) ? s.x : s.y) * .48);
    r2 = irnd (r * .45);
    lw = int (ceil (r / 15.));
    
    dc.BeginDrawing();

    p.SetWidth (lw);
    dc.SetPen (p);
    dc.SetBrush (*wxBLACK_BRUSH);
    dc.DrawCircle (cx, cy, r);
    dc.DrawLine (cx + irnd (r * dx), cy + irnd (r * dy),
                 cx + irnd (r2 * dx), cy + irnd (r2 * dy));
    
    dc.EndDrawing();

}

void wxKnob::OnMouse (wxMouseEvent &event)
{
    int cx, cy;
    double dx, dy, theta, dt;
    int newval;
    wxEventType scrollEvent = wxEVT_NULL ;
    
    if (event.Moving ())
    {
        event.Skip ();
        return;
    }
    
    GetCenter (cx, cy);
    dx = event.m_x - cx;
    dy = cy - event.m_y;
    if (dx == 0. && dy == 0.)
        return;
    
    theta = atan2 (dy, dx) * 180. / PI;
    if (theta < 0.)
        theta += 360.;
    
    dt = theta - m_maxAngle;
    if (dt < 0.)
        dt += 360;
    if (dt > m_range)
        return;
    newval = int (m_max - (dt / m_range) * (m_max - m_min));

    SetValue (newval);
    if (event.Dragging () || event.ButtonUp ())
    {
        if (event.ButtonUp ())
            scrollEvent = wxEVT_SCROLL_THUMBRELEASE;
        else
            scrollEvent = wxEVT_SCROLL_THUMBTRACK;
            
        wxScrollEvent event (scrollEvent, m_windowId);
        event.SetPosition (newval);
        event.SetEventObject (this);
        GetEventHandler()->ProcessEvent (event);

        wxCommandEvent cevent (wxEVT_COMMAND_SLIDER_UPDATED, m_windowId);
        cevent.SetInt (newval);
        cevent.SetEventObject (this);
        GetEventHandler()->ProcessEvent (cevent);
    }
}

void wxKnob::GetCenter (int &x, int &y) const
{
    wxSize s = GetSize ();
    x = s.x / 2;
    y = s.y / 2;
}
