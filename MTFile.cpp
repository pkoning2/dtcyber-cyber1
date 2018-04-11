////////////////////////////////////////////////////////////////////////////
// Name:        MTFile.cpp
// Purpose:     Definition of class for micro-tutor floppies 
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#include "PtermApp.h"
#include "MTFile.h"

// For GTK (Linux) the Microtutor floppy image for help is compiled in
// as initial content of this array; for Windows and Mac is it read
// from the actual floppy file which is packaged with the program.
u8 MTIMAGE[128L * 64L * 154L] = {
#if defined (__WXGTK__)
#include "ptermhelp.h"
#endif
};
static bool mtimage_loaded = false;

MTFile::MTFile()
{
    rwflag = wxT ("  ");

#ifdef _WIN32
    ms_handle = NULL;
#else
    fileHandle = -1;
#endif
    _chkSum = 0;
    position = 0;
    rcnt = 0;
    wcnt = 0;
    _RamBased = false;
}

void MTFile::SetRamBased (const char *fn)
{
#if !defined (__WXGTK__)
    if (!mtimage_loaded)
    {
        wxFileName filename (ptermApp->m_resourcedir, fn);
        FILE *f;
        int i;
        
        f = fopen (filename.GetFullPath (), "rb");
        if (f != NULL)
        {
            i = fread (MTIMAGE, 1, sizeof (MTIMAGE), f);
            if (i != sizeof (MTIMAGE))
            {
                perror ("help floppy image read");
                fprintf (stderr, "expected %ld bytes, got %d bytes\n",
                         sizeof (MTIMAGE), i);
            }
            fclose (f);
        }
    }
#endif
    mtimage_loaded = true;
    _RamBased = true;
}

bool MTFile::Open(const char *fn)
{
    if (_RamBased)
        return true;
    Close();

#ifdef _WIN32

    wchar_t filenam[256];
    const size_t cSize = strlen(fn) + 1;
    mbstowcs(filenam, fn, cSize);

    ms_handle = CreateFile(filenam, (GENERIC_READ | GENERIC_WRITE),
        0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (ms_handle == INVALID_HANDLE_VALUE)
    {
        ms_handle = NULL;
        return reportError(fn);
    }
#else
    fileHandle = open(fn, O_RDWR);

    if (fileHandle == -1)
        return reportError(fn);
#endif
#ifndef _WIN32
    // Attempt to lock the file, don't block if it's already locked
    int result = flock(fileHandle, LOCK_EX | LOCK_NB);
    if (result == -1)
    {
        close(fileHandle);
        return reportError(fn);
    }
#endif
    return true;
}

bool MTFile::reportError(const char *fn)
{
    wxString msg(_(L"Error opening \u00b5Tutor floppy file "));
    msg.Append(fn);
    msg.Append(":\n");
    msg.Append(wxSysErrorMsg());

    wxMessageBox(msg, "Floppy error", wxICON_ERROR | wxOK | wxCENTRE);
    return false;
}

bool MTFile::Test(const char *fn)
{
    if (_RamBased)
        return true;
#ifdef _WIN32
    struct _stat buf;
    int result = _stat(fn, &buf);
#else
    struct stat buf;
    int result = stat(fn, &buf);
#endif
    if (result != 0)
        return reportError(fn);
    return true;
}

void MTFile::Close(void)
{
    if (_RamBased)
        return;
#ifdef _WIN32
    CloseHandle(ms_handle);
    ms_handle = NULL;
#else
    if (fileHandle != -1)
    {
        close(fileHandle);
        fileHandle = -1;
    }
#endif
}

void MTFile::Seek(long int loc)
{
    if (_RamBased)
    {
        position = loc;
        rcnt = wcnt = 1;
        return;
    }

#ifdef _WIN32
    DWORD x;

    if (ms_handle != NULL)
    {
        x = SetFilePointer(ms_handle, loc, 0, FILE_BEGIN);
        if (x == INVALID_SET_FILE_POINTER)
#else
    off_t x;

    if (fileHandle != -1)
    {
        x = lseek(fileHandle, loc, SEEK_SET);
        if (x < 0)
#endif

        {
            printf("Floppy seek error!  loc = %06lx\n", loc);
        }
        //printf("Seek Sector = %04lx\n", (loc / 130));
        rcnt = wcnt = 1;
        position = loc;
    }
}

void MTFile::ReadReset (void)
{
    rcnt = 1;
    _chkSum = 0;
    rwflag = wxT ("R ");

}

void MTFile::SetHelpContext (u8 context)
{
    // pass OS Type as part of context
    u8 ostype = osOther;  // other/unknown = 0
#if defined (__WXMAC__)
    ostype = osMac;       // MAC = 1
#endif
#if defined (__WXMSW__)
    ostype = osWin;       // Windows = 2
#endif
#if defined (__WXGTK__)
    ostype = osLinux;     // Linux = 3
#endif
    MTIMAGE[124] = ostype;
    MTIMAGE[125] = context;
}

u8 MTFile::ReadRam ()
{
    CHECKOUT
    u8 mybyte = MTIMAGE[position++];
    rcnt++;
    CalcCheck (mybyte);
    return mybyte;
}

u8 MTFile::ReadByte()
{
    if (_RamBased)
    {
        return ReadRam ();
    }
    int retry = 0;

#ifdef _WIN32
    if (ms_handle != NULL)
    {
        CHECKOUT
    retry1:
        u8 mybyte = 0;
        DWORD length;
        BOOL result = ReadFile(ms_handle, &mybyte, 1, &length, NULL);
        if (!result) {
#else
    if (fileHandle != -1)
    {
        CHECKOUT
    retry1:
        u8 mybyte = 0;
        size_t x = read(fileHandle, &mybyte, 1);
        if (x != 1) {
#endif
            printf("Floppy read error!  position: %06lx\n", position);
            long int save = position;
            int save2 = rcnt;
            int save3 = wcnt;
            Seek(0);
            Seek(save);
            rcnt = save2;
            wcnt = save3;
            if (retry++ < 3)
                goto retry1;
            printf("FATAL floppy read error!  position: %06lx\n", position);
        }
        
        //printf("readcnt: %d data: %02x  position: %06lx\n", rcnt, mybyte, position);
        position++;
        rcnt++;
        //if (rcnt > 130)
        //{
        //    //printf("Completed Sector read. Sector = %04lx\n", (position/130)-1);
        //    rcnt = 1;
        //}

        CalcCheck (mybyte);
        return mybyte;
    }
    return 0;
        }

void MTFile::WriteReset (void)
{
    wcnt = 1;
    rwflag = wxT ("W ");
}

void MTFile::WriteByte(u8 val)
{
    if (_RamBased)
        return;
    int retry = 0;


    if (wcnt > 129)
    {
        WriteReset ();
        return;
    }
    if (wcnt > 128)
    {
        wcnt++;
        return;
    }

#ifdef _WIN32
    if (ms_handle != NULL)
    {
    retry2:
        DWORD length;
        BOOL result = WriteFile(ms_handle, &val, 1, &length, NULL);

        if (!result)
#else
    if (fileHandle != -1)
    {
    retry2:
        size_t x = write(fileHandle, &val, 1);
        if (x != 1)
#endif
        {
            printf("floppy write error!  data:  %02x  position: %06lx\n",
                val, position);
            long int save = position;
            int save2 = rcnt;
            int save3 = wcnt;
            Seek(0);
            Seek(save);
            rcnt = save2;
            wcnt = save3;
            if (retry++ < 3)
                goto retry2;
            printf("FATAL floppy write error!  position: %06lx\n", position);
        }

        //printf("writecnt: %d  data: %02x  position: %06lx\n", wcnt, val, position);

        wcnt++;
        if (wcnt > 130)
            WriteReset ();

        position++;
    }
}

void MTFile::Format (void)
{
    if (_RamBased)
        return;
    rwflag = wxT ("W ");
    Seek (0);
    long int i;
    for (i = 0; i < (128L * 64L * 154L); i++)
    {
        u8 val = 0;
#ifdef _WIN32
        DWORD length;
        WriteFile (ms_handle, &val, 1, &length, NULL);
#else
        write (fileHandle, &val, 1);
#endif    
    }
#ifdef _WIN32
    FlushFileBuffers (ms_handle);
#endif
}

void MTFile::CalcCheck (u8 b)
{
    u8 cupper = (u8)((_chkSum >> 8) & 0xff);
    u8 clower = (u8)(_chkSum & 0xff);
    cupper ^= b;
    int x = cupper << 1;
    if ((x & 0x100) > 0)
        x = (x | 1) & 0xff;
    cupper = (u8)x;
    clower ^= b;
    int y = 0;
    if ((clower & 1) == 1)
        y = 0x80;
    x = clower >> 1;
    x = (x | y) & 0xff;
    clower = (u8)x;
    _chkSum = (u16)(((cupper << 8) & 0xff00) | (u16)(clower & 0xff));
}

