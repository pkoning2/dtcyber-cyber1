/////////////////////////////////////////////////////////////////////////////
// Name:        iir.h
// Purpose:     IIR filter implementation
// Author:      Paul Koning
// Modified by:
// Created:     08/01/2005
// Copyright:   (c) Paul Koning
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// Delay line -- used to align control signals (beam on/off)
// with analog data going through filters.
class Delay
{
public:
    Delay (int delay);
    ~Delay ();
    void SetFlag (bool flag);
    bool Flag (void) const
    {
        return m_flags[0];
    }
    
private:
    Delay (const Delay &);

    int m_delay;
    bool *m_flags;
};

// R/C lowpass filter, implemented as an IIR filter
class RClow
{
public:
    RClow (double r, double c, int freq);
    double Step (double in);
    void Reset (void)
    {
        z = 0.0;
    }
    
private:
    double a, b, z;
};

// R/C highpass filter, implemented as an IIR filter
class RChigh
{
public:
    RChigh (double r, double c, int freq);
    double Step (double in);
    void Reset (void)
    {
        z = 0.0;
    }

private:
    enum { delay = 5 };
    double a, b, z;
};

