/*
** wf_steps.c
**
** Generate a series of very course steps that are useful for checking levels.
** The configured PEAK LEVEL is divided into four chunks to give FOUR levels,
** including zero and the maximum. The steps are deliberately POSITIVE ONLY as
** it makes visualising HEX sample values in memory easier.
*/
#include "wavgen.h"

#define NUM_STEPS 4 // Not including the zero level.

/*
** Generate the sample data in "unified" 32-bit integer format.
*/
void generate_steps(struct FIXED_PARAMS *fixed,
                    struct COMMON_USER_PARAMS *user)
{
    static uint32_t step = 0;

    /* Work out the step-size from the maximum (peak) level.*/
    uint32_t step_size = MAX_LEVEL_32BIT / NUM_STEPS;

    /* Set the current sample from the step value.*/
    fixed->sample_value.i = (int32_t) (step_size * step);

    /*
    ** The same value is used within each frame of a multi-channel waveform,
    ** so only increment the step after generating a sample for the last channel.
    */
    if (fixed->current_chnl == (user->num_channels - 1)) {
        step = step + 1;
        if (step > NUM_STEPS) {
            step = 0;
        }
    }
}
