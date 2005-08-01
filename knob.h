/////////////////////////////////////////////////////////////////////////////
// Name:        knob.h
// Purpose:     rotary knob control
// Author:      Paul Koning
// Modified by:
// Created:     08/01/2005
// Copyright:   (c) Paul Koning
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// Define a custom control that looks like a rotary knob
class wxKnob : public wxControl
{
public:
    wxKnob () : wxControl () { }
    wxKnob (wxWindow* parent, wxWindowID id, int value,
            int minValue, int maxValue,
            unsigned int minAngle = 240, unsigned int range = 300,
            const wxPoint &pos = wxDefaultPosition, 
            const wxSize &size = wxDefaultSize, 
            long style = 0,
            const wxValidator &validator = wxDefaultValidator,
            const wxString &name = wxT ("knob"))
    {
        Create (parent, id, value, minValue, maxValue,
                minAngle, range, pos, size, style, validator, name );
    }
    void Create (wxWindow* parent, wxWindowID id, int value,
                 int minValue, int maxValue,
                 unsigned int minAngle = 240, unsigned int range = 300,
                 const wxPoint &pos = wxDefaultPosition, 
                 const wxSize &size = wxSize (30, 30), 
                 long style = 0,
                 const wxValidator &validator = wxDefaultValidator,
                 const wxString &name = wxT ("knob"));

    // retrieve/change the range
    void SetRange (int minValue, int maxValue);
    int GetMin() const { return m_min; }
    int GetMax() const { return m_max; }
    void SetMin (int minValue) { SetRange( minValue , GetMax() ) ; }
    void SetMax (int maxValue) { SetRange( GetMin() , maxValue ) ; }
    unsigned int GetMinAngle() const { return (m_maxAngle - m_range) % 360; }
    int GetMaxAngle() const { return m_maxAngle; }
    int GetValue () const { return m_setting; }
    int SetValue (int value);

private:
    int m_min;
    int m_max;
    int m_setting;
    unsigned int m_maxAngle;
    unsigned int m_range;

    void GetCenter (int &x, int &y) const;
    void OnPaint(wxPaintEvent &event);
    void OnMouse (wxMouseEvent &event);

    DECLARE_EVENT_TABLE()
};
