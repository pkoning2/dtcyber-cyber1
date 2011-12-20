/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: channel.c
**
**  Description:
**      Perform simulation of CDC 6600 channels.
**
**--------------------------------------------------------------------------
*/

/*
**  -------------
**  Include Files
**  -------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/

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

/*
**  ----------------
**  Public Variables
**  ----------------
*/
ChSlot *channel;
DevSlot *activeDevice;
u8 channelCount;
u32 channelDelayMask;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static u8 ch = 0;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialise channels.
**
**  Parameters:     Name        Description.
**                  count       channel count
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void channelInit(u8 count)
    {
    /*
    **  Allocate channel structures.
    */
    channelCount = count;
    channel = calloc(count, sizeof(ChSlot));
    if (channel == NULL)
        {
        fprintf(stderr, "Failed to allocate channel control blocks\n");
        exit(1);
        }

    /*
    **  Initialise all channels.
    */
    for (ch = 0; ch < channelCount; ch++)
        {
        channel[ch].id = ch;
#ifdef USE_THREADS
        pthread_cond_init (&channel[ch].cond, NULL);
        pthread_mutex_init (&channel[ch].mutex, NULL);
#endif
        }

    /*
    **  Print a friendly message.
    */
    printf("Channels initialised (number of channels %o)\n", channelCount);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Terminate channels.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void channelTerminate(void)
    {
    DevSlot *dp;
    DevSlot *fp;
    u8 i;

    /*
    **  Free allocated memory of all devices hanging of this channel.
    */
    for (ch = 0; ch < channelCount; ch++)
        {
        for (dp = channel[ch].firstDevice; dp != NULL; dp = dp->next)
            {
            if (dp->devType == DtDcc6681)
                {
                dcc6681Terminate(dp);
                }

            /*
            **  Free all unit contexts and close all open files.
            */
            for (i = 0; i < MaxUnits; i++)
                {
                if (dp->context[i] != NULL)
                    {
                    free(dp->context[i]);
                    }

                if (dp->fcb[i] != NULL)
                    {
                    fclose(dp->fcb[i]);
                    }
                }
            }

        for (dp = channel[ch].firstDevice; dp != NULL;)
            {
            /*
            **  Free all device control blocks.
            */
            fp = dp;
            dp = dp->next;
            free(fp);
            }
        }

    /*
    **  Free all channel control blocks.
    */
    free(channel);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Return device control block attached to a channel.
**
**  Parameters:     Name        Description.
**                  channelNo   channel number to attach to.
**                  devType     device type
**
**  Returns:        Pointer to device slot.
**
**------------------------------------------------------------------------*/
DevSlot *channelFindDevice(u8 channelNo, u8 devType)
    {
    DevSlot *device;
    ChSlot *cp;

    cp = channel + channelNo;
    device = cp->firstDevice;

    /*
    **  Try to locate device control block.
    */
    while (device != NULL) 
        {
        if (device->devType == devType)
            {
            return(device);
            }

        device = device->next;
        }

    return(NULL);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Attach device to channel.
**
**  Parameters:     Name        Description.
**                  channelNo   channel number to attach to
**                  eqNo        equipment number
**                  devType     device type
**
**  Returns:        Pointer to device slot.
**
**------------------------------------------------------------------------*/
DevSlot *channelAttach(u8 channelNo, u8 eqNo, u8 devType)
    {
    DevSlot *device;
    ChSlot *activeChannel;
    
    activeChannel = channel + channelNo;
    device = activeChannel->firstDevice;

    /*
    **  Try to locate existing device control block.
    */
    while (device != NULL) 
        {
        if (   device->devType == devType
            && device->eqNo    == eqNo)
            {
            return(device);
            }

        device = device->next;
        }

    /*
    **  No device control block of this type found, allocate new one.
    */
    device = calloc(1, sizeof(DevSlot));
    if (device == NULL)
        {
        fprintf(stderr, "Failed to allocate control block for Channel %d\n",channelNo);
        exit(1);
        }

    /*
    **  Link device control block into the chain of devices hanging of this channel.
    */
    device->next = activeChannel->firstDevice;
    activeChannel->firstDevice = device;
    device->channel = activeChannel;
    device->devType = devType;
    device->eqNo    = eqNo;

    return(device);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Issue a function code to all attached devices.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void channelFunction(ChSlot *activeChannel, PpWord funcCode)
    {
    DevSlot *activeDevice;
    FcStatus status = FcDeclined;

    activeChannel->full = FALSE;
    for (activeDevice = activeChannel->firstDevice; activeDevice != NULL; activeDevice = activeDevice->next)
        {
        status = activeDevice->func (activeChannel, activeDevice, funcCode);
        if (status == FcAccepted)
            {
            /*
            **  Device has claimed function code - select it for I/O.
            */
            activeChannel->ioDevice = activeDevice;
            break;
            }

        if (status == FcProcessed)
            {
            /*
            **  Device has processed function code - no need for I/O.
            */
            break;
            }
        }

    if (activeDevice == NULL || status == FcDeclined)
        {
        /*
        **  No device has claimed the function code - keep channel active
        **  and full, but disconnect device.
        */
        activeChannel->ioDevice = NULL;
        activeChannel->full = TRUE;
        activeChannel->active = TRUE;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Activate a channel and let attached device know.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void channelActivate(ChSlot *activeChannel)
    {
    DevSlot *activeDevice;

    activeChannel->active = TRUE;

    if (activeChannel->ioDevice != NULL)
        {
        activeDevice = activeChannel->ioDevice;
        activeDevice->activate (activeChannel, activeDevice);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Disconnect a channel and let active device know.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void channelDisconnect(ChSlot *activeChannel)
    {
    DevSlot *activeDevice;

    activeChannel->active = FALSE;

    if (activeChannel->ioDevice != NULL)
        {
        activeDevice = activeChannel->ioDevice;
        activeDevice->disconnect (activeChannel, activeDevice);
        }
    else
        {
        activeChannel->full = FALSE;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Probe channel status.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
void channelProbe(ChSlot *activeChannel)
    {
    /*
    **  The following gives individual drivers an opportunity to delay
    **  too rapid changes of the channel status when probed via FJM and
    **  EJM PP opcodes. Initially this will be used by mt669.c and mt679.c
    **  only.
    */
    if (activeChannel->delayStatus == 0)
        {
        activeChannel->delayStatus = 5;
        }

    channelIo (activeChannel);
    }

/*--------------------------------------------------------------------------
**  Purpose:        IO on a channel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
void channelIo(ChSlot *activeChannel)
    {
    DevSlot *activeDevice;

    /*
    **  Perform request.
    */
    if ((activeChannel->active || activeChannel->id == ChClock)
        && activeChannel->ioDevice != NULL)
        {
        activeDevice = activeChannel->ioDevice;
        activeDevice->io (activeChannel, activeDevice);
        }
    }

/*---------------------------  End Of File  ------------------------------*/
