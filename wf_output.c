/*
** wf_output.c
**
** Functions to output an output buffer of samples in "standard unified format" in the
** format chosen by the user (e.g. with the --bitdepth option). This is not intended to
** be a completely loss-less process, and it may introduce minor artifacts (no attempt
** to dither the output is made, for example).
**
** Some pre-processing functions are included here too (mostly level adjustment).
**
** The data is either written to file or to stdout if piping to another application.
*/
#include <limits.h>
#include "wavgen.h"

/*
** Scale the sample to the level requested through the --align, --level and --power options (where allowed).
*/
void set_level(struct FIXED_PARAMS *fixed)
{
    /*
    ** Gain is specified as a double so try to keep precision by converting the INTEGER
    ** samples to/from double-precision floating-point.
    */
    fixed->sample_value.i = (int32_t) (((double) fixed->sample_value.i * fixed->gain) + 0.5);
}

/*
** Check whether any level scaling is allowed and/or called for and apply it if so.
** Returns true if a level adjustment was made.
*/
void check_level(struct FIXED_PARAMS *fixed,
                 struct COMMON_USER_PARAMS *user)
{
    switch (user->wf_type) {
        /* Level adjustment is not allowed for these non-audio types.*/
        case WAVEFORM_TYPE_COUNTER:
        case WAVEFORM_TYPE_SILENCE:
        case WAVEFORM_TYPE_STEPS:
        break;

        /* These types all peak at 0dBFS and may have their level changed.*/
        case WAVEFORM_TYPE_SAW:
        case WAVEFORM_TYPE_SINE:
        case WAVEFORM_TYPE_SQUARE:
        case WAVEFORM_TYPE_BURST:
        /*
        ** TODO: White/Pink noise will measure an RMS level of approx. -4.6dBFS/-15dbFS if "aligned" to 0dBFS.
        **       Although the alignment level is used to set the PEAK level in this utility, that makes less
        **       sense for white or pink noise. Perhaps there's a better way of doing this?
        */
        case WAVEFORM_TYPE_PINK:
        case WAVEFORM_TYPE_WHITE:

        if ((fixed->gain > 1.0001) || (fixed->gain < 0.9999)) {
            set_level(fixed);
        }
        break;

        /* Ignore unknown or unsupported types.*/
        default:
        break;
    }
}

/*
** Check whether the sample needs converting between integer and floating-point,
** which depends on the user's choice of output format.
*/
void check_format(struct FIXED_PARAMS *fixed,
                  struct COMMON_USER_PARAMS *user)
{
    if (user->save_as_float) {
        /*
        ** Float32 WAV format has samples aligned to 1.0f, so convert to float then
        ** scale by the maximum integer value that the waveform generators produce.
        */
        fixed->sample_value.f = (float) fixed->sample_value.i;
        fixed->sample_value.f /= MAX_LEVEL_32BIT;
    }
}

/*
** Write the finalised sample data to the given output file in the appropriate word size.
** Returns true if the samples were written out successfully.
*/
bool save_sample(struct   FIXED_PARAMS *fixed,
                  struct  COMMON_USER_PARAMS *user,
                  FILE   *wavfile)
{
    size_t num_bytes;
    size_t bytes_written;

    /*
    ** Remember here that the data will already be in the required format,
    ** but the word length (sample depth) will still be 32-bit.
    */

    if (user->save_as_float == true) {
        /*
        ** The first case is where no conversion is required because samples
        ** have already been converted to floats in check_format().
        */
        num_bytes     = sizeof(float);
        bytes_written = fwrite(&fixed->sample_value.f, 1, num_bytes, wavfile);
    }
    else if (user->bytes_per_sample == BYTES_32BIT) {
        /*
        ** The second case where no conversion is required (samples are already S32LE).
        */
        num_bytes     = sizeof(int32_t);
        bytes_written = fwrite(&fixed->sample_value.i, 1, num_bytes, wavfile);
    }
    else if (user->bytes_per_sample == BYTES_16BIT) {
        /*
        ** Now we deal with the 16-bit sample format S16LE.
        */
        int16_t sample_s16;
        sample_s16 = (int16_t) (fixed->sample_value.i >> 16);

        num_bytes     = sizeof(int16_t);
        bytes_written = fwrite(&sample_s16, 1, num_bytes, wavfile);
    }
    else {
        /*
        ** There are plenty of other formats that are NOT supported here yet,
        ** notibly S8LE and big-endian ones.
        */
        return false;
    }

    /*
    ** Check for errors in writing the file.
    */
    return (bytes_written == num_bytes);
}

/*
** Perform final tasks on the generated waveform data to ready it for writing out.
*/
bool finalise_data(struct  FIXED_PARAMS *fixed,
                   struct  COMMON_USER_PARAMS *user,
                   struct  ADDITIONAL_USER_PARAMS *extra,
                   FILE   *wavfile)
{
    /*
    ** Check whether level adjustment is required and apply it if so.
    ** Must be done before markers are applied to avoid changing them.
    */
    check_level(fixed, user);

    /*
    ** Convert between integer and floating-point format if required.
    ** (must be done before markers can be added to integer formats).
    */
    check_format(fixed, user);

    /*
    ** Check whether channel markers have been asked for and add them if so.
    */
    check_markers(fixed, user, extra);

    /*
    ** The buffer has been converted to float or integer, so just write it out,
    ** truncating the word-length if required.
    */
    return save_sample(fixed, user, wavfile);
}
