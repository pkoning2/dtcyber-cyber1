////////////////////////////////////////////////////////////////////////////
// Name:        MTFile.h
// Purpose:     Declaration of class for micro-tutor floppies 
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __MTFile_H__
#define __MTFile_H__ 1

#include "CommonHeader.h"

class MTFile
{
public:
    MTFile();
    bool Open(const char *fn);
    bool Test(const char *fn);
    void Close(void);
    void Seek(long int loc);
    u8 ReadByte(void);
    void WriteByte(u8 val);
    void WriteReset (void);
    void ReadReset (void);
    void Format (void);
    void SetRamBased (const char *fn);
    bool Active(void) const
    {
#ifdef _WIN32
        return (ms_handle != NULL);
#else
        return (fileHandle != -1);

#endif
    }
    bool reportError(const char *fn);
    void SetHelpContext (u8 context);

    wxString rwflag;

private:
    bool _RamBased;
    u16 _chkSum;
    void CalcCheck (u8 b);

#ifdef _WIN32
    HANDLE ms_handle;
#else
    int fileHandle;
#endif
    long int position;
    int rcnt;
    int wcnt;
    u8 ReadRam (void);
};

#define CHECKOUT                                        \
          if (rcnt == 129)                              \
            {                                           \
                rcnt++;                                 \
                return (u8)(_chkSum & 0xff);            \
            }                                           \
          else if (rcnt == 130)                         \
            {                                           \
                u8 ret = (u8)((_chkSum >> 8) & 0xff);   \
                ReadReset ();                           \
                return ret;                             \
            }


#endif    // __MTFile_H__

