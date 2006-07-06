/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2006, Tom Hunter, Gerard van der Grinten,
**  and Paul Koning.
**  (see license.txt)
**
**  Name: dtdisksubs.c
**
**  Description:
**      Disk I/O subroutines, with async I/O support
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
#include <sys/file.h>
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
#include <unistd.h>
#endif
#include <errno.h>
#include "const.h"
#include "types.h"
#include "proto.h"

#ifdef _POSIX_ASYNCHRONOUS_IO
#include <aio.h>
#ifndef _POSIX_REALTIME_SIGNALS
#error "no signals"
#endif
#include <signal.h>
#endif

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
#ifdef _POSIX_ASYNCHRONOUS_IO
#define ASYNC_IO 0
#else
#define ASYNC_IO 0
#endif

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
#if ASYNC_IO
static void ddAioDone (int signo, siginfo_t *info, void *ignored);
#endif
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
#if ASYNC_IO
static bool handlerSet = FALSE;
static sigset_t aio_completion_signals;
#endif

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
#if ASYNC_IO
    int err, ret;
    
    if (!io->ioPending)
        {
        return FALSE;
        }
    err = aio_error (&io->iocb);
    if (err == EINPROGRESS)
        {
        return TRUE;
        }
    ret = aio_return (&io->iocb);
#if DEBUG
    printf ("aio done fd %d err %d ret %d\n", io->fd, err, ret);
#endif
    io->ioPending = FALSE;
#endif
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
#if ASYNC_IO
    ddWaitIO (io);
    io->buf = buf;
    io->count = bytes;
    io->ioPending = TRUE;
    io->iocb.aio_fildes = io->fd;
    io->iocb.aio_offset = io->pos;
    io->iocb.aio_buf = buf;
    io->iocb.aio_nbytes = bytes;
#ifdef HOST_Linux
    io->iocb.aio_sigevent.sigev_notify = SIGEV_NONE;
#else
    io->iocb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    io->iocb.aio_sigevent.sigev_signo = SIGRTMIN;
    io->iocb.aio_sigevent.sigev_value.sival_ptr = io;
#endif
    ret = aio_read (&io->iocb);
#else
    lseek (io->fd, io->pos, SEEK_SET);
    ret = read (io->fd, buf, bytes);
#endif
#if DEBUG
    printf ("aio_read fd %d pos %d ret %d err %d\n", io->fd, io->pos, ret, errno);
#endif
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
#if ASYNC_IO
    ddWaitIO (io);
    io->buf = (void *) buf;
    io->count = bytes;
    io->ioPending = TRUE;
    io->iocb.aio_fildes = io->fd;
    io->iocb.aio_offset = io->pos;
    io->iocb.aio_buf = (void *) buf;
    io->iocb.aio_nbytes = bytes;
#ifdef HOST_Linux
    io->iocb.aio_sigevent.sigev_notify = SIGEV_NONE;
#else
    io->iocb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    io->iocb.aio_sigevent.sigev_signo = SIGRTMIN;
    io->iocb.aio_sigevent.sigev_value.sival_ptr = io;
#endif
    ret = aio_write (&io->iocb);
#else
    lseek (io->fd, io->pos, SEEK_SET);
    ret = write (io->fd, buf, bytes);
#endif
#if DEBUG
    printf ("aio_write fd %d pos %d ret %d err %d\n", io->fd, io->pos, ret, errno);
#endif
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
#if ASYNC_IO
    const struct aiocb *alist[1];
    int ret, err;
    
    if (!io->ioPending)
        {
        return;
        }
#if DEBUG
    printf ("aio waiting fd %d\n", io->fd);
#endif
    alist[0] = &io->iocb;
    aio_suspend (alist, 1, NULL);
    io->ioPending = FALSE;
#ifdef HOST_Linux
    err = aio_error (&io->iocb);
    ret = aio_return (&io->iocb);
#if DEBUG
    printf ("aio wait fd %d err %d ret %d\n", io->fd, err, ret);
#endif
#endif
#endif
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

#if ASYNC_IO
/*--------------------------------------------------------------------------
**  Purpose:        Signal handler for AIO completion
**
**  Parameters:     Name        Description.
**                  signo       Signal number
**                  info        Signal associated info
**                  ignored     not used
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void ddAioDone (int signo, siginfo_t *info, void *ignored)
    {
    DiskIO *io;
    int err, ret;
    
    io = (DiskIO *) info->si_value.sival_ptr;
#if DEBUG
    printf ("aio done fd %d\n", io->fd);
#endif
    
    if (signo != SIG_AIO_DONE ||
        info->si_code != SI_ASYNCIO)
        {
        fprintf (stderr, "spurious call to ddAioDone, signal %d\n", signo);
        return;
        }
    if (!io->ioPending)
        {
        fprintf (stderr, "Completion but I/O not marked as pending");
        }
    err = aio_error (&io->iocb);
    ret = aio_return (&io->iocb);
#if DEBUG
    printf ("aio done fd %d err %d ret %d\n", io->fd, err, ret);
#endif
    
    io->ioPending = FALSE;
    }
#endif

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
#if ASYNC_IO
#ifndef HOST_Linux
    struct sigaction sa;
    
    if (!handlerSet)
        {
        /*
        **  Declare the signal handler for async I/O
        */
        handlerSet = TRUE;
        sigemptyset (&aio_completion_signals);
        sigaddset (&aio_completion_signals, SIG_AIO_DONE);
        sa.sa_flags = SA_SIGINFO;
        sigemptyset (&sa.sa_mask);
        sa.sa_sigaction = ddAioDone;
        sigaction (SIG_AIO_DONE, &sa, NULL);
        }
#endif
#endif
    io->ioPending = FALSE;
    io->fd = fd;
    }
