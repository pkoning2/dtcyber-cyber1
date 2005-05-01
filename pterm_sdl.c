/*--------------------------------------------------------------------------
**
**  Copyright (c) 2005, Paul Koning (see license.txt)
**
**  Name: pterm_sdl.c
**
**  Description:
**      This is the interface from pterm to the SDL sound libraries,
**      for GSW emulation
**
**--------------------------------------------------------------------------
*/

#include <stdio.h>
#include <SDL.h>
#include "const.h"
#include "types.h"

#define FREQ        22050           /* desired sound system data rate */
#define CRYSTAL     3872000         /* GSW clock crystal frequency */
#define SAMPLES     4096            /* number of samples to give to SDL each time */

#define C_NODATA        -1

struct gswState_t
{
    double phaseStep;
    void *user;
    u32 phase[4];
    u32 step[4];
    int vol[4];
    int voices;
    int voice;
    int clocksLeft;
    int clocksPerWord;
    bool playing;
    bool cis;
};

struct gswState_t gswState;
SDL_AudioSpec audioSpec;
bool audioOpened;

static void gswCallback (void *userdata, u8  *stream, int len);
extern int ptermNextGswWord (void *connection, int catchup);

/*
**  This function assigns the GSW to the specified "user" (a pointer to some
**  appropriate handle, e.g., a Connection object).
**
**  Return value:
**      -1  Error (GSW unavailable for any reason; it may be disabled or
**          some other connection may be using it).  You'll also get this
**          if you call twice.
**       0  Success (caller now owns the GSW)
*/
int ptermOpenGsw (void *user)
{
    SDL_AudioSpec *req;

    if (gswState.user != NULL)
    {
        return -1;
    }

    // GSW was free, so initialize the SDL sound machinery
    // First clear out all the current GSW state
    memset (&gswState, 0, sizeof (gswState));
        
    if (!audioOpened)
    {
        // We only open the SDL audio system the first time we come here,
        // because it doesn't seem to appreciate being opened and closed
        // multiple times, at least not on Linux.
        req = (SDL_AudioSpec *) calloc (1, sizeof (SDL_AudioSpec));
        req->freq = FREQ;
        req->format = AUDIO_U8;
        req->channels = 1;
        req->samples = SAMPLES;
        req->callback = gswCallback;
        if (SDL_OpenAudio (req, &audioSpec) < 0)
        {
            fprintf (stderr, "can't open sound: %s\n", SDL_GetError ());
            free (req);
            return -1;
        }
        free (req);
        audioOpened = TRUE;
    }
        
    // We're all set up, mark the GSW as in-use
    gswState.user = user;
    gswState.phaseStep = (double) CRYSTAL / audioSpec.freq;
    gswState.clocksPerWord = audioSpec.freq / 60;
#ifdef DEBUG
    printf ("sound opened, freq %d format %d channels %d samples %d\n",
            audioSpec.freq, audioSpec.format,
            audioSpec.channels, audioSpec.samples);
#endif
    return 0;
}

/*
**  Close the GSW (done with the sound machinery)
*/
void ptermCloseGsw (void)
{
    gswState.playing = FALSE;
    SDL_PauseAudio (1);
    gswState.user = NULL;
#ifdef DEBUG
    printf ("sound stopped\n");
#endif
}

/*
**  Start playing the GSW, if not already started.
**
**  The main emulation calls this when there is enough data in its buffer.
*/
void ptermStartGsw (void)
{
    if (!gswState.playing)
    {
#ifdef DEBUG
        printf ("sound started\n");
#endif
        gswState.playing = TRUE;
        SDL_PauseAudio (0);
    }
}

// Since there are four channels and we do 8 bit audio, we want +/- 127 max,
// which means +/- 31 max for each channel volume.
// We'll use a map tabel so we can accommodate any transfer function.
// For now (pending data from sjg) we'll assume a linear mapping.
static const volmap[8] = {
    4, 8, 12, 16, 20, 24, 28, 31
};

static const int mapvol (int volume)
{
    return volmap[volume & 7];
}

static void gswCallback (void *userdata, u8 *stream, int len)
{
    int i, word, voice;
    int catchup = 1;
    unsigned int audio;
    double dph;
    
#ifdef DEBUG
//    printf ("callback %d bytes\n", len);
#endif
    while (len > 0)
    {
        if (gswState.clocksLeft == 0)
        {
            // Finished processing the current word worth of data
            // (1/60th of a second), get the next word from the main emulation.
            // If we have no more data, supply silence.
            word = ptermNextGswWord (gswState.user, catchup);
            catchup = 0;
            if (word == C_NODATA)
            {
                while (len > 0)
                {
                    *stream++ = audioSpec.silence;
                    --len;
                }
                return;
            }
            
            // We have a word; figure out what it means.  In all cases,
            // it means we have 1/60th second more data.  
            gswState.clocksLeft = gswState.clocksPerWord;
            if ((word >> 16) == 3)
            {
                // -extout- word, update the GSW state
                if (word & 0100000)
                {
                    // voice word
                    word &= 077777;
                    voice = gswState.voice;
                    if (word < 2)
                    {
                        // rest, set step to zero to indicate silence
                        gswState.step[voice] = 0;
#ifdef DEBUG
                        printf ("voice %d rest\n", voice);
#endif
                    }
                    else
                    {
                        dph = gswState.phaseStep / (4 * word + 2);
                        if (dph > 0.5)
                        {
                            gswState.step[voice] = 0;
#ifdef DEBUG
                            printf ("voice %d step out of range: %f\n",
                                    voice, dph);
#endif
                        }
                        else
                        {
                            // form delta phase per audio clock, as a scaled
                            // integer, binary point to the right of the top bit.
//                            gswState.step[voice] = ldexp (dph, 31);
                            gswState.step[voice] = dph * 2147483648.0;
                            
#ifdef DEBUG
                            printf ("voice %d word %06o step %08x (%g) dph %g (%g Hz)\n",
                                    voice, word, gswState.step[voice],
//                                    ldexp (dph, 31),
                                    dph * 2147483648.0,
                                    dph, (double) audioSpec.freq * dph);
#endif
                        }
                    }
                    if (!gswState.cis)
                    {
                        if (--voice < 0)
                        {
                            voice = gswState.voices;
                        }
                        gswState.voice = voice;
                    }
                }
                else
                {
                    // Mode word
                    gswState.cis = (word & 040000) != 0;
                    gswState.voice = gswState.voices = (word >> 12) & 3;
                    gswState.vol[0] = mapvol (word >> 9);
                    gswState.vol[1] = mapvol (word >> 6);
                    gswState.vol[2] = mapvol (word >> 3);
                    gswState.vol[3] = mapvol (word);
#ifdef DEBUG
                    printf ("gsw mode %06o\n", word);
#endif
                }
            }
        }

        // Now generate one sample, from the current phase and volume
        // settings, then update the phase to reflect that one audio
        // clock has elapsed.

        audio = audioSpec.silence;
        for (i = 0; i < 4; i++)
        {
            if (gswState.step[i] != 0)
            {
                audio += (((gswState.phase[i] >> 30) & 1) ? -1 : 1) * gswState.vol[i];
                gswState.phase[i] += gswState.step[i];
            }
        }
        if (audio > 255)
        {
            printf ("audio out of range: %x\n", audio);
        }
        
       --gswState.clocksLeft;
        *stream++ = audio;
        --len;
    }
}

    
