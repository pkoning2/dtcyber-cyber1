/*--------------------------------------------------------------------------
**
**  Copyright (c) 2005-2010, Paul Koning (see pterm-license.txt)
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
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#if !defined(_WIN32)
#include <sys/time.h>
#include <unistd.h>
#endif
#include <SDL.h>
#include <math.h>

#include <sndfile.h>

#include "const.h"
#include "types.h"
#include "ptermx.h"

#define FREQ        22050           // desired sound system data rate
#define CRYSTAL     3872000         // GSW clock crystal frequency
#define SAMPLES     4096            // number of samples per callback
#define MAXIDLE     5               // time to stop if this many idle callbacks

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
    int nodata;
    bool playing;
    bool cis;
    /* IIR filter state for IIR based RC filter */
    double a;
    double z;
    /* Sound file state */
    SF_INFO sfinfo;
    SNDFILE	*sfile ;
};

struct gswState_t gswState;
SDL_AudioSpec audioSpec;
bool audioOpened;

static void gswCallback (void *userdata, uint8_t  *stream, int len);

/*
**  This function assigns the GSW to the specified "user" (a pointer to some
**  appropriate handle, e.g., a Connection object).
**
**  Arguments:
**      user    Pointer to a distinguishing handle.
**      fn      File name for audio output file, or NULL/empty string
**              if no output file.
**      fmt     libsndfile format code (the main format code; we'll
**              fill in the subcode that says 16 bit signed PCM)
**
**  Return value:
**      -1  Error (GSW unavailable for any reason; it may be disabled or
**          some other connection may be using it).  You'll also get this
**          if you call twice.
**       0  Success (caller now owns the GSW)
*/
int ptermOpenGsw (void *user, const char *fn, int fmt)
{
    SDL_AudioSpec *req;
    double t;

    // RC filter is the summing network resistor (100k) and the
    // associated capacitor (.01 uF).  We'll model that using an IIR.
    const double r = 100e3;
    const double c = .01e-6;
    
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
        req->format = AUDIO_S16;
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

    // We haven't seen any callbacks with no data available yet
    gswState.nodata = 0;

    // Initialize the RC filter state.  This is an IIR filter
    // as described in Wikipedia:
    //   http://en.wikipedia.org/wiki/Low-pass_filter

    t = 1.0 / audioSpec.freq;
    gswState.a = t / (t + r * c);
    gswState.z = 0.0;

    // If a sound file name was supplied, open it
    if (fn != NULL && fn[0] !='\0')
    {
        gswState.sfinfo.samplerate = audioSpec.freq;
        gswState.sfinfo.frames = -1;
        gswState.sfinfo.channels = 1;
        gswState.sfinfo.format = fmt | SF_FORMAT_PCM_16;
        
        gswState.sfile = sf_open (fn, SFM_WRITE, &gswState.sfinfo);
        if (gswState.sfile == NULL)
        {
            printf ("Error opening sound file\n");
        }
#ifdef DEBUG
        else
        {
            printf ("sound file opened to %s\n", fn);
        }
#endif
    }

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
    if (gswState.sfile != NULL)
    {
        sf_close (gswState.sfile);
        gswState.sfile = NULL;
#ifdef DEBUG
        printf ("sound file closed\n");
#endif
    }
    
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

// Since there are four channels and we do 16 bit audio, we want +/- 32767 max,
// which would mean +/- 8191 max for each channel volume.  But since the
// basic waveform is then run through an IIR filter, we'll use a slightly
// lower limit instead.  The filter in this case is an RC filter, which
// in its analog form has no overshoot, of course.  But the IIR form may
// have very slight overshoot due to rounding errors.  We have spare range,
// so we'll set the per-channel max at 8000.
// We'll use a map tabel so we can accommodate any transfer function.
// For now (pending data from sjg) we'll assume a linear mapping.
static const int volmap[8] = {
    1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000
};

static int mapvol (int volume)
{
    return volmap[volume & 7];
}

static void gswCallback (void *userdata, uint8_t *b, int len)
{
    int16_t *stream = (int16_t *) b;
    int i, word, voice;
    int audio;
    double dph;
    int items;
    void *buf = b;
    
#if defined(_WIN32)
    (void) userdata;        // No Operation; Suppresses Warning C4100
#endif
    len /= sizeof (*stream);
    items = len;            // Remember the total number requested
    
#ifdef DEBUG
    printf ("callback %d words\n", len);
#endif
    while (len > 0)
    {
        if (gswState.clocksLeft == 0)
        {
            // Finished processing the current word worth of data
            // (1/60th of a second), get the next word from the main emulation.
            // If we have no more data, supply silence.  Tell the
            // main code if we've been idle long; if yes, it will stop
            // playing.  "idle long" is defined by the number of callbacks
            // in which we had no data at all.
            word = ptermNextGswWord (gswState.user, gswState.nodata > MAXIDLE);
            if (word == C_NODATA || word == C_GSWEND)
            {
                if (len == items)
                {
                    // If this was no data right at the start of the callback,
                    // increment the idle count.
                    gswState.nodata++;
                }
                while (len > 0)
                {
                    *stream++ = audioSpec.silence;
                    --len;
                }
                break;
            }
            
            // We have a word; figure out what it means.  In all cases,
            // it means we have 1/60th second more data.  
            gswState.nodata = 0;
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
                            gswState.step[voice] = (int) (dph * 2147483648.0);
                            
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
        // clock has elapsed.  Note that the base waveform is a square
        // wave, so the wave generation simply produces -1 or +1 depending
        // on the current phase, multiplied by the volume.

        audio = audioSpec.silence;
        for (i = 0; i < 4; i++)
        {
            if (gswState.step[i] != 0)
            {
                audio += (((gswState.phase[i] >> 30) & 1) ? -1 : 1) * gswState.vol[i];
                gswState.phase[i] += gswState.step[i];
            }
        }
        
        --gswState.clocksLeft;

        // Feed the raw audio into the RC filter
        gswState.z = gswState.z * (1.0 - gswState.a) + audio * gswState.a;
        audio = (int) (gswState.z);

        if (audio < -32767 || audio > 32767)
        {
            printf ("audio out of range: %d\n", audio);
        }
        
        *stream++ = audio;
        --len;
    }

    if (gswState.sfile != NULL)
    {
        // Sound output file is open, write this burst to it
        if (sf_write_short (gswState.sfile, buf, items) != items)
        {
            puts (sf_strerror (gswState.sfile));
            gswState.sfile = NULL;
        }
#ifdef DEBUG
        else
        {
            printf ("%d words written to sound file\n", items);
        }
#endif
            
    }
}
