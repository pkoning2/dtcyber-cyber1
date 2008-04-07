#include <stdio.h>
#include <stdlib.h>
#include "vhdl.h"
#include "const.h"
#include "types.h"
#include "proto.h"

#define DPRINTF(f...)
//#define DPRINTF printf

static bool pending_in[19][040];

coaxsigs foo = { w,x,w,w,one,zero,one,zero,one,zero
}
    ;

void synchro (int chnum, coaxsigs incable, coaxsigs outcable)
{
    int i;
    u32 dout, din;
    ChSlot prev_ch;
    bool ic[19], oc[19];
    enum std_logic z = zero;
    
    activeChannel = channel + chnum;
    if (activeChannel == NULL)
    {
        return;
    }

    // Convert between std_logic values and boolean.  
    // Note that coax cable signals are inverted logic so we
    // convert '0' to true.

    // Send pending input, then clear pending
    for (i = 0; i < 16; i++)
    {
        if (pending_in[i][chnum])
        {
            incable[i] = zero;
            DPRINTF ("1");
        }
        else 
        {
            incable[i] = one;
            DPRINTF ("0");
        }
        pending_in[i][chnum] = FALSE;
    }
    DPRINTF ("...  ");

    // Convert output
    for (i = 0; i < 19; i++)
    {
        if (outcable[i] == z)
        {
            oc[i] = TRUE;
            DPRINTF ("1");
        }
        else
        {
            oc[i] = FALSE;
            DPRINTF ("0");
        }
    }
    DPRINTF ("\n");
    
    // Convert output data to integer
    dout = 0;
    for (i = 11; i >= 0; i--)
    {
        dout <<= 1;
        if (oc[i])
        {
            dout++;
        }
    }

    // First process active and inactive pulses (those get no acknowledgment
    // so we check them before saving the channel state)
    if (oc[12])
    {
        DPRINTF ("connect\n");
        channelActivate ();
    }
    if (oc[13])
    {
        DPRINTF ("disconnect\n");
        channelDisconnect ();
    }
    
    // Process function
    if (oc[16])
    {
        DPRINTF ("function %04o", dout);
        channelFunction (dout);
        if (!(activeChannel->full))
        {
            // Function was accepted, send Inactive
            pending_in[13][chnum] = TRUE;
            DPRINTF (" accepted");
        }
        DPRINTF ("\n");
    }
    else
    {
        
        // Process data
        if (oc[15])
        {
            activeChannel->full = FALSE;
            DPRINTF ("empty\n");
        }
        if (oc[14])
        {
            activeChannel->full = TRUE;
            activeChannel->data = dout;
            DPRINTF ("output data %04o\n", dout);
        }
    
        // Now save the channel state so we can generate input pulses
        // corresponding to any state changes done by the I/O processing
        prev_ch = *activeChannel;
    
        // See what the device thinks of that
        channelIo ();
    
        // If there are any channel state changes from what we captured
        // before the data I/O, generate the appropriate response.

        // TODO: DiscAfterInput

        if (prev_ch.active && !activeChannel->active)
        {
            // Disconnect, send inactive
            pending_in[13][chnum] = TRUE;
            DPRINTF ("device disconnect\n");
        }
        else if (prev_ch.full && !activeChannel->full)
        {
            // Output accepted, send empty
            pending_in[15][chnum] = TRUE;
            DPRINTF ("device empty\n");
        }
        else if (!prev_ch.full && activeChannel->full)
        {
            // New input, send full and the data
            pending_in[14][chnum] = TRUE;
            din = activeChannel->data;
            for (i = 0; i < 12; i++)
            {
                if (din & (1 << i))
                {
                    pending_in[i][chnum] = TRUE;
                }
            }
            DPRINTF ("device input data %04o\n", din);
        }
    }
}
