////////////////////////////////////////////////////////////////////////////
// Name:        PTermPrintout.cpp
// Purpose:     Implementation of the Screen Print
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#include "PtermPrintout.h"
#include "PtermConnDialog.h"
#include "PtermFrame.h"

// ----------------------------------------------------------------------------
// Pterm printing helper class
// ----------------------------------------------------------------------------

bool PtermPrintout::OnPrintPage (int page)
{
    wxDC *dc = GetDC ();

    if (dc)
    {
        if (page == 1)
        {
            DrawPage (dc);
        }

        return true;
    }

    return false;
}

void PtermPrintout::GetPageInfo (int *minPage, int *maxPage,
                                 int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool PtermPrintout::HasPage (int pageNum)
{
    return (pageNum == 1);
}

void PtermPrintout::DrawPage (wxDC *dc)
{
    int obr, obg, obb;
    // Get the size of the DC in pixels
    int w, h;
    dc->GetSize (&w, &h);

    // Calculate a suitable scaling factor.  "Suitable" means chosen so
    // that the page size corresponds to 600 screen pixels, so we get
    // about 45 pixels worth of margin all around.
    double scaleX = w / 600.0;
    double scaleY = h / 600.0;

    // Use x or y scaling factor, whichever fits on the DC
    double actualScale = wxMin (scaleX, scaleY);

    // Calculate the position on the DC for centring the graphic
    double posX = (w - 512. * actualScale) / 2.0;
    double posY = (h - 512. * actualScale) / 2.0;

    // Set the scale and origin
    dc->SetUserScale (actualScale, actualScale);
    dc->SetDeviceOrigin ((long) posX, (long) posY);

    // Re-color the image
    wxImage screenImage = m_owner->m_bitmap->ConvertToImage ();

    unsigned char *data = screenImage.GetData ();
    
    w = screenImage.GetWidth ();
    h = screenImage.GetHeight ();
    obr = m_owner->m_profile->m_bgColor.Red ();
    obg = m_owner->m_profile->m_bgColor.Green ();
    obb = m_owner->m_profile->m_bgColor.Blue ();

    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            if (data[0] == obr &&
                data[1] == obg &&
                data[2] == obb)
            {
                // Background, make it white
                data[0] = data[1] = data[2] = 255;
            }
            else
            {
                // Foreground, make it black
                data[0] = data[1] = data[2] = 0;
            }
            data += 3;
        }
    }

    wxBitmap printmap (screenImage);

    dc->DrawBitmap (printmap, 0, 0);
}



