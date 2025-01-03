/*
** wf_counter.c
**
** Generate a series of samples that increase by "one bit" (one
** quantisation level) at a time. Hence, the frequency is fixed
** and cannot be set on the command-line. The peak level (if set)
** will set the point at which the counter resets to zero.
**
** The count/sample value is positive-only to make recognising
** values easier when tracing code or looking at the WAV file in
** a hex editor.
**
** If you need a symetrical (no DC content) count-like waveform,
** use the "saw" option instead and set a suitably low frequency.
*/
#include "wavgen.h"

/*
** Create a buffer consisting of an integer count (essentially a slow saw-tooth).
*/
void generate_counter(struct FIXED_PARAMS *fixed,
                      struct COMMON_USER_PARAMS *user,
                      struct ADDITIONAL_USER_PARAMS *extra)
{
    /*
    ** Samples are Little-Endian:
    **               v LSB
    **  sample = 0x 00 00 00 00;
    **                    MSB ^
    */

    uint32_t counter_value;

    /*
    ** The counter value increaments independently of CHANNEL, so multi-channel
    ** waveforms will have the same counter value across all sample in the frame.
    */
    counter_value = fixed->sample_number;

    /*
    ** If the user has asked for channel markers in the LSB, make room
    ** for them by placing the counter higher up. If placed in the MSB
    ** then the markers will just overwrite the upper counter bits.
    */
    if (extra->markers_on && !extra->markers_in_msb) {
        counter_value = counter_value << 8;
    }

    /*
    ** Because the counter should count "LSBs" the generated data must be
    ** different for different sample widths, even though they are saved
    ** in 32-bit format for now.
    */
    if ((user->bytes_per_sample == BYTES_32BIT) || (user->save_as_float)) {
        fixed->sample_value.i = (int32_t) counter_value;
    }
    else if (user->bytes_per_sample == BYTES_24BIT) {
        fixed->sample_value.i = (int32_t) (counter_value << 8U);
    }
    else if (user->bytes_per_sample == BYTES_16BIT) {
        fixed->sample_value.i = (int32_t) (counter_value << 16U);
    }
    else {
        fixed->sample_value.i = 0U; // Unsupported formats.
    }
}
