/////////////////////////////////////////////////////////////////////////////
// Name:        iir.cpp
// Purpose:     IIR filter implementation
// Author:      Paul Koning
// Modified by:
// Created:     08/01/2005
// Copyright:   (c) Paul Koning
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <math.h>
#include "iir.h"

Delay::Delay (int delay)
{
    int i;
    
    m_delay = delay + 1;
    m_flags = new bool[m_delay];
    for (i = 0; i < m_delay; i++)
        m_flags[i] = false;
}

Delay::~Delay ()
{
    delete [] m_flags;
}

void Delay::SetFlag (bool flag)
{
    if (m_delay > 0)
    {
        memmove (&m_flags[0], &m_flags[1], (m_delay - 1) * sizeof (*m_flags));
    }
    m_flags[m_delay - 1] = flag;
}

// See Frerking, page 60
RClow::RClow (double r, double c, int freq)
{
    double t = 1.0 / freq;
    
    a = t / (r * c);
    b = exp (-a);
    z = 0.0;
}

double RClow::Step (double in)
{
    z = (z * b) + in;
    return z * a;
}

// See Frerking, page 350
RChigh::RChigh (double r, double c, int freq)
{
    double t = 1.0 / freq;
    
    a = 1 / (1 + t / (2. * r * c));
    b = a * (1 - t / (2. * r * c));
    z = 0.0;
}

double RChigh::Step (double in)
{
    double z1 = z;
    
    z = (z1 * b) + (in * a);
    return z - z1;
}
