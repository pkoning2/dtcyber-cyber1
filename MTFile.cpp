////////////////////////////////////////////////////////////////////////////
// Name:        MTFile.cpp
// Purpose:     Definition of class for micro-tutor floppies 
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#include "MTFile.h"

MTFile::MTFile()
{
#ifdef _WIN32
    ms_handle = NULL;
#else
    fileHandle = -1;
#endif
    position = 0;
    rcnt = 0;
    wcnt = 0;
}

bool MTFile::Open(const char *fn)
{
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
    wxString msg("Error opening \u00b5Tutor floppy file ");
    msg.Append(fn);
    msg.Append(":\n");
    msg.Append(wxSysErrorMsg());

    wxMessageBox(msg, "Floppy error", wxICON_ERROR | wxOK | wxCENTRE);
    return false;
}

bool MTFile::Test(const char *fn)
{
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

u8 MTFile::ReadByte()
{
    int retry = 0;

#ifdef _WIN32
    if (ms_handle != NULL)
    {
        CHECKOUT
    retry1:
        u8 mybyte = 0;
        DWORD length;
        bool result = ReadFile(ms_handle, &mybyte, 1, &length, NULL);
        if (result == 0) {
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

void MTFile::WriteByte(u8 val)
{
    int retry = 0;

    if (wcnt > 128)
    {
        wcnt++;
        return;
    }
    if (wcnt > 129)
    {
        wcnt = 1;
        return;
    }

#ifdef _WIN32
    if (ms_handle != NULL)
    {
    retry2:
        DWORD length;
        bool result = WriteFile(ms_handle, &val, 1, &length, NULL);

        if (result == 0)
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
            wcnt = 1;

        position++;
    }
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

