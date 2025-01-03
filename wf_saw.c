/*
** wf_saw.c
**
** Generate a smooth saw-tooth waveform at the desired frequency.
** The waveform will start at zero-level and rise to the defined
** peak amplitude before "wrapping around" to the negative peak
** amplitude and climbing back from there.
**
** Channel markers may be added to this waveform in which case
** the peak level is limited to make room for the markers (which
** will cause the waveform to be entirely negative if put in the
** MSB). It is therefore recommended that markers be added in the
** LSB if possible.
**
** If you want a saw-tooth that increases in single-bit steps,
** you may wish to use the COUNTER type instead (it's similar).
*/
#include "wavgen.h"

/*
** Generate the sample data in "unified" 32-bit integer format.
*/
void generate_saw(struct FIXED_PARAMS *fixed,
                  struct COMMON_USER_PARAMS *user)
{
    static int32_t sample_value = 0;

    uint32_t num_steps = (user->sample_rate / user->frequency_hz) * 2U;
    uint32_t step_size = (MAX_LEVEL_32BIT / num_steps) * 2U;

    /* Adjust the target peak level so that it fits with our step-size.*/
    int32_t peak_level = (step_size * num_steps) / 2U;

    /* The saw-tooth waveform starts from zero but is then symetrical about zero.*/
    fixed->sample_value.i = sample_value;

    if (sample_value == MAX_LEVEL_32BIT) {
        /* If the last sample value EQUALS the peak value (it may be INT_MAX), take special care.*/
        sample_value = MAX_LEVEL_32BIT * -1;
    }
    else {
        /* Otherwise ramp-up the saw-tooth level by an amount driven by the required frequency.*/
        sample_value = sample_value + (int32_t) step_size;

        /* Once the peak is EXCEEDED, flip to the negative peak level.*/
        if (sample_value > peak_level) {
            sample_value = ((int32_t) (step_size * num_steps)) * -1;
        }
    }
}
