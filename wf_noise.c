/*
** wf_noise.c
**
** Generate white/pink noise sources, typically used for frequency analysis or
** for measuring/setting output levels.
**/
#include <string.h>
#include <time.h>
#include "wavgen.h"

/*
** Generate a pseudo-random number for noise generation.
** Algorithm source : http://www.firstpr.com.au/dsp/rand31/
*/
uint32_t rand_31(int32_t *seed) {
    uint32_t hi, lo;

    lo = 16807UL * (*seed & 0xFFFFUL);
    hi = 16807UL * (*seed >> 16);

    lo += (hi & 0x7FFFUL) << 16;
    lo += hi >> 15;

    if (lo > 0x7FFFFFFFUL) {
        lo -= 0x7FFFFFFFUL;
    }

    return ( *seed = (int32_t) lo);
}

/*
** Generate white noise.
** Algorithm source : https://www.firstpr.com.au/dsp/rand31/
**
** Example command : One second of white noise at -20dBFS (to be safe).
** ./wavgen -t white -b 32 -c 2 -d 1000 -l -20.0 ~/tmp/test-white.wav
*/
void generate_white(struct FIXED_PARAMS *fixed,
                    struct ADDITIONAL_USER_PARAMS *extra)
{
    static int32_t seed = 1;
    static int32_t last_sample;

    /*
    ** For uncorrelated noise, generate one new sample per channel,
    ** otherwise just repeat the sample generated for channel 0.
    */
    if ((fixed->current_chnl == 0) || (extra->uncorrelated)) {
        /* Get a white noise sample and scale it to the +/- 32-bit audio sample range.*/
        last_sample = (rand_31(&seed) - (0x7FFFFFFFUL / 2)) * 2;
        fixed->sample_value.i = last_sample;
    }
    else {
        /* For CORRELATED noise, repeat channel 0.*/
        fixed->sample_value.i = last_sample;
    }
}

/*
** Generate pink noise.
** Algorithm source : https://www.firstpr.com.au/dsp/pink-noise/
**
** Example command : One second of pink noise at -10dBFS.
** ./wavgen -t white -b 32 -c 2 -d 1000 -l -10.0 ~/tmp/test-pink.wav
*/
void generate_pink(struct FIXED_PARAMS *fixed,
                   struct ADDITIONAL_USER_PARAMS *extra)
{
    static int32_t seed = 1;
    static int32_t last_sample;
    static double  b[7];

    double pink;
    double white;

    /* Initialise the filter taps before generating the very first sample.*/
    if ((fixed->sample_number == 0) && (fixed->current_chnl == 0)) {
        memset(b, 0, sizeof(b));
    }

    /*
    ** For uncorrelated noise, generate one new sample per channel,
    ** otherwise just repeat the sample generated for channel 0.
    */
    if ((fixed->current_chnl == 0) || (extra->uncorrelated)) {
        /* Get a white noise sample and scale it to a +/- audio sample.*/
        white = (((double) rand_31(&seed)) - ((double) 0x7FFFFFFFUL / 2.0));

        /* 1/f filter the white noise to make it pink.*/
        b[0] = 0.99886 * b[0] + white * 0.0555179;
        b[1] = 0.99332 * b[1] + white * 0.0750759;
        b[2] = 0.96900 * b[2] + white * 0.1538520;
        b[3] = 0.86650 * b[3] + white * 0.3104856;
        b[4] = 0.55000 * b[4] + white * 0.5329522;
        b[5] = -0.7616 * b[5] - white * 0.0168980;
        pink = b[0] + b[1] + b[2] + b[3] + b[4] + b[5] + b[6] + (white * 0.5362);
        b[6] = white * 0.115926;

        /*
        ** Pink noise PEAK level is approximately 5x that of the source white noise, so scale it.
        ** This results in noise that measures an RMS level of around -15dBFS.
        */
        last_sample = (int32_t) (pink / 5.0);
        fixed->sample_value.i = last_sample;
    }
    else {
        /* For CORRELATED noise, repeat channel 0.*/
        fixed->sample_value.i = last_sample;
    }
}
