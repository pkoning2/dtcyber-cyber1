/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2006, Tom Hunter, Gerard van der Grinten,
**  and Paul Koning.
**  (see license.txt)
**
**  Name: dtdisksubs.c
**
**  Description:
**      Disk I/O subroutines
**
**--------------------------------------------------------------------------
*/

#define DEBUG 0

/*
**  -------------
**  Include Files
**  -------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#if defined(_WIN32)
#include <io.h>
#define open _open
#define close _close
#define lseek _lseek
#define read _read
#define write _write
#define O_RDWR _O_RDWR
#define O_CREAT _O_CREAT
#define O_RDONLY _O_RDONLY
#else
#include <sys/file.h>
#include <unistd.h>
#endif
#include <errno.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define SIG_AIO_DONE SIGRTMIN

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void ddOpenSetup (DiskIO *io, int fd);

/*
**  ----------------
**  Public Variables
**  ----------------
*/

/*
**  -----------------
**  Private Variables
**  -----------------
*/

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Open an existing disk file for I/O
**
**  Parameters:     Name        Description.
**                  io          DiskIO descriptor
**                  name        File name
**                  writable    TRUE for read/write access
**
**  Returns:        TRUE if successful, FALSE if not.
**
**------------------------------------------------------------------------*/
bool ddOpen (DiskIO *io, const char *name, bool writable)
    {
    int fd;
    int mode;
    
    io->fd = -1;
    if (writable)
        {
        mode = O_RDWR;
        }
    else
        {
        mode = O_RDONLY;
        }
    
    fd = open (name, mode);
    if (fd < 0)
        {
        return FALSE;
        }
    ddOpenSetup (io, fd);
    return TRUE;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Open a new disk file for I/O
**
**  Parameters:     Name        Description.
**                  io          DiskIO descriptor
**                  name        File name
**
**  Returns:        TRUE if successful, FALSE if not.
**
**------------------------------------------------------------------------*/
bool ddCreate (DiskIO *io, const char *name)
    {
    int fd;
    
    io->fd = -1;
    fd = open (name, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
        {
        return FALSE;
        }
    ddOpenSetup (io, fd);
    return TRUE;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Test if I/O is still pending
**
**  Parameters:     Name        Description.
**                  io          DiskIO descriptor
**
**  Returns:        TRUE if busy, FALSE if done.
**
**------------------------------------------------------------------------*/
bool ddIOPending (DiskIO *io)
    {
    return FALSE;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Lock a disk file for exclusive access
**
**  Parameters:     Name        Description.
**                  io          DiskIO descriptor
**
**  Returns:        TRUE if successful, FALSE if not.
**
**------------------------------------------------------------------------*/
bool ddLock (DiskIO *io)
    {
#if !defined(_WIN32)
    int e;

    e = flock (io->fd, LOCK_EX | LOCK_NB);
    if (e != 0)
        {
        return FALSE;
        }
    return TRUE;
#endif
    }

/*--------------------------------------------------------------------------
**  Purpose:        Close a disk file
**
**  Parameters:     Name        Description.
**                  io          DiskIO descriptor
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
void ddClose (DiskIO *io)
    {
    if (ddOpened (io))
        {
        ddWaitIO (io);
        close (io->fd);
        io->fd = -1;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Start a disk read
**
**  Parameters:     Name        Description.
**                  io          DiskIO descriptor
**                  buf         Buffer pointer
**                  bytes       Byte count
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
void ddQueueRead (DiskIO *io, void *buf, int bytes)
    {
    int ret;
    
    /*
    **  Start reading a sector.
    **
    **  Make sure any prior I/O on this unit is done.  Then mark I/O
    **  as in progress, and start the actual operation.
    */
    lseek (io->fd, io->pos, SEEK_SET);
    ret = read (io->fd, buf, bytes);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Start a disk write
**
**  Parameters:     Name        Description.
**                  io          DiskIO descriptor
**                  buf         Buffer pointer
**                  bytes       Byte count
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
void ddQueueWrite (DiskIO *io, const void *buf, int bytes)
    {
    int ret;
    
    /*
    **  Start writing a sector.
    **
    **  Make sure any prior I/O on this unit is done.  Then mark I/O
    **  as in progress, and start the actual operation.
    */
    lseek (io->fd, io->pos, SEEK_SET);
    ret = write (io->fd, buf, bytes);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set the position for the next I/O
**
**  Parameters:     Name        Description.
**                  io          DiskIO descriptor
**                  pos         Byte position
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
void ddSeek (DiskIO *io, off_t pos)
    {
    io->pos = pos;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Wait for I/O completion
**
**  Parameters:     Name        Description.
**                  io          DiskIO descriptor
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
void ddWaitIO (DiskIO *io)
    {
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Common setup code for open and create
**
**  Parameters:     Name        Description.
**                  io          DiskIO struct
**                  fd          File descriptor number
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void ddOpenSetup (DiskIO *io, int fd)
    {
    io->ioPending = FALSE;
    io->fd = fd;
    }
