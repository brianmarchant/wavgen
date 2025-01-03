/*
** wf_pulse.c
**
** A generally short pulse of sinewave cycles, repeated at a configurable period of time.
** This is very useful for testing the latency through a system, or the synchronisation
** between output channels, or the polarity of the signal as it travels along the signal chain.
** The period between pulses can be set, as well as the frequency of the sine-wave pulse and the
** number of cycles that each "pulse" contains.
**
** Example command : Four cycles of 60Hz every 200ms. Total length 1 second, -6dBFS.
** ./wavgen -t pulse -b 32 -c 2 -d 1000 -f 50 -n 4 -p 200 -l -6.0 ~/tmp/test-pulse.wav
*/
#include <math.h>
#include "wavgen.h"

static const double PI = 3.1415926536;

/*
** Generate the sample buffer with periodic sine-wave pulses.
*/
void generate_pulse(struct FIXED_PARAMS *fixed,
                    struct COMMON_USER_PARAMS *user,
                    struct ADDITIONAL_USER_PARAMS *extra_params)
{
    static uint64_t pulse_sample = 0;

    uint32_t pulse_length_samples;
    uint32_t period_length_samples;

    double sample_value_f;

    /*
    ** Sanitise input to avoid any potential floating-point exceptions etc.
    */
    if (extra_params->num_cycles < 1U) {
        extra_params->num_cycles = 1U;
    }

    pulse_length_samples  = user->sample_rate / user->frequency_hz;
    period_length_samples = (user->sample_rate / 1000U) * extra_params->period_ms;

    /*
    ** At the beginning of a the pulse duration (for the first channel), reset the counter
    ** that will cause the sinusiodal pulse to be generated.
    */
    if (((fixed->sample_number % period_length_samples) == 0) && (fixed->current_chnl == 0)) {
        pulse_sample = 0;
    }

    if (pulse_sample < (pulse_length_samples * extra_params->num_cycles)) {
        /* Generate a sinusoidal waveform ("pulse") during this period.*/
        sample_value_f  = sin(2.0 * PI * (double) pulse_sample / (double) pulse_length_samples);
        sample_value_f *= (double) MAX_LEVEL_32BIT;

        fixed->sample_value.i = (int32_t) (sample_value_f + 0.5);

        ++pulse_sample;
    }
    else {
        fixed->sample_value.i = 0;
    }
}
