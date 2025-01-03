/*
** wf_square.c
**
** Generate a square-wave at the desired frequency.
**
** Channel markers may be added to this waveform in which case
** the peak level is limited to make room for the markers (which
** will cause the waveform to be entirely negative if put in the
** MSB). It is therefore recommended that markers be added in the
** LSB if possible.
*/
#include "wavgen.h"

/*
** Generate the sample data in "unified" 32-bit integer format.
*/
void generate_square(struct FIXED_PARAMS *fixed,
                     struct COMMON_USER_PARAMS *user)
{
    static int32_t sample_value = MAX_LEVEL_32BIT;

    uint32_t half_period_samples;

    /* Calculate the nearest samples-per-period for the requested frequency.*/
    half_period_samples = (user->sample_rate / user->frequency_hz) / 2U;

    /*
    ** The square-wave starts from a +ve peak (it is never zero).
    */
    fixed->sample_value.i = sample_value;

    /* If a 1/2-period has elapsed, flip the polarity, but only at the start of a new frame.*/
    if (fixed->current_chnl == 0) {
        if ((fixed->sample_number % half_period_samples) == (half_period_samples - 1)) {
            sample_value = -sample_value;
        }
    }
}
