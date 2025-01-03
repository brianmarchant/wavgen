/*
** wf_markers.c
**
** Functions to add "channel markers" to suitable waveforms.
** This is useful for testing audio applications, especially ones that allow multi-channel
** streams and interleaved playback formats. The markers allow mixed-up data streams to be
** quickly diagnosed.
*/
#include "wavgen.h"

/*
** Adds a channel marker to the current sample with no regard to the waveform type.
** Returns true if markers were added.
*/
bool add_markers(struct FIXED_PARAMS *fixed,
                 bool   markers_in_msb)
{
    uint32_t marker_value = fixed->current_chnl + 1;

    if (markers_in_msb) {
        fixed->sample_value.i &= 0x00FFFFFF;
        fixed->sample_value.i |= ((0xC0 + marker_value) << 24);
    }
    else {
        fixed->sample_value.i &= 0xFFFFFF00;
        fixed->sample_value.i |= (0xC0 + marker_value);
    }

    return true;
}

/*
** Check whether channel markers should be added, and add them if so.
** Returns true if markers were added.
*/
bool check_markers(struct FIXED_PARAMS *fixed,
                   struct COMMON_USER_PARAMS *user,
                   struct ADDITIONAL_USER_PARAMS *extra)
{
    bool added = false;

    /*
    ** If the user didn't ask for markers, they're not added.
    */
    if (!extra->markers_on) {
        return false;
    }

    /*
    ** Whether markers CAN be added depends on type and sample format.
    */
    switch (user->wf_type) {
        /* Markers are not allowed on these types.*/
        case WAVEFORM_TYPE_SAW:
        case WAVEFORM_TYPE_SINE:
        case WAVEFORM_TYPE_SQUARE:
        case WAVEFORM_TYPE_BURST:
        break;

        /* These types MAY have markers added if the output format is not float.*/
        case WAVEFORM_TYPE_COUNTER:
        case WAVEFORM_TYPE_SILENCE:
        case WAVEFORM_TYPE_STEPS:
        case WAVEFORM_TYPE_PINK:
        case WAVEFORM_TYPE_WHITE:
        if (!user->save_as_float) {
            added = add_markers(fixed, extra->markers_in_msb);
        }
        break;

        /* Ignore unknown or unsupported types.*/
        default:
        break;
    }

    return added;
}
