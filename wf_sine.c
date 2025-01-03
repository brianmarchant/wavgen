/*
** wf_sine.c
**
** Generate a sinewave at the requested frequency.
** The peak level can be specified, as can "fractional power" as an alternative.
** Markers are not allowed.
**/
#include <math.h>
#include "wavgen.h"

static const double PI = 3.1415926536;

/*
** A simple "double-maths" version is used here rather than anything high-performance.
** As per all generators, the output is INTEGER samples, which will be converted back
** to floating-point in the WAV file if that's what the user asked for.
*/
void generate_sine(struct FIXED_PARAMS *fixed,
                   struct COMMON_USER_PARAMS *user)
{
    uint32_t cycle_length_samples;
    double   sample_value_f;

    cycle_length_samples  = user->sample_rate / user->frequency_hz;

    sample_value_f = sin(2.0 * PI * (double) fixed->sample_number / (double) cycle_length_samples);
    sample_value_f *= (double) MAX_LEVEL_32BIT;

    /* Double-check sample levels. This should probably be removed.*/
    if (sample_value_f > (double) MAX_LEVEL_32BIT) {
        sample_value_f = (double) MAX_LEVEL_32BIT;
    }

    fixed->sample_value.i = (int32_t) (sample_value_f + 0.5);
}
