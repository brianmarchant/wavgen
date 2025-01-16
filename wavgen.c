/*
** wavgen.c
**
** A simple utility to generate various waveforms for testing audio playback.
** The sample values can be "stamped" with permanent (obvious) channel indicators on top of
** the basic waveforms to make debugging multi-channel playback issues easer.
** Non-audio content such as the "count[er]" type are also very useful for debugging buffers
** or for verifying continuity of playback (provided they are not converted or filtered).
**
** There are no dependancies so on Linux it should build using CMake or just with:
** cc wavgen.c help.c log.c opts.c riff.c wf*.c -lm -o wavgen
**
** See the accompanying README.md for more help on compiling (and cross-compiling).
**
** To check the output on Linux run this to generate a VERY small test file:
** ./wavgen -b 32 -c 2 -s 4 -t counter -m msb /tmp/check.wav && hexdump -C -n 64 /tmp/check.wav
** or
** ./wavgen -b 32 -c 2 -s 4 -t counter -m msb | hexdump -C -n 64
**
** The output of the check above should look like this:
** 00000000  52 49 46 46 34 00 00 00  57 41 56 45 66 6d 74 20  |RIFF4...WAVEfmt |
** 00000010  10 00 00 00 01 00 02 00  80 bb 00 00 00 dc 05 00  |................|
** 00000020  08 00 20 00 64 61 74 61  10 00 00 00 00 00 00 c1  |.. .data........|
** 00000030  00 00 00 c2 01 00 00 c1  01 00 00 c2 02 00 00 c1  |................|
*/
#include "riff.h"
#include "wavgen.h"

/*
** The main application entry point.
*/
int main(int argc, char *argv[])
{
    FILE    *wavfile = NULL;
    bool     success = true;
    uint32_t num_data_bytes;

    /* Structs holding command-line parameters.*/
    struct FIXED_PARAMS           fixed;
    struct COMMON_USER_PARAMS     user;
    struct ADDITIONAL_USER_PARAMS extra;

    /* The header information that describes the WAV file.*/
    struct RIFF_HEADER        riff_header;
    struct RIFF_FMT_CHUNK     riff_fmt;
    struct RIFF_EXT_FMT_CHUNK riff_fact;
    struct RIFF_DATA_CHUNK    riff_data;

    /*
    ** Gather and parse command-line options.
    ** This function will EXIT (it won't return) if there are fatal errors.
    */
    parse_opts(argc, argv, &fixed, &user, &extra);

    /*
    ** Either write RIFF data to stdout (i.e. to another application) or create
    ** a WAV file on the filesystem. If writing to stdout then the log_xxx()
    ** functions will have their output suppressed.
    */
    if (fixed.piping) {
        wavfile = stdout;
    }
    else {
        log_extra(&fixed, "Output filename is '%s'\n", user.filename);
        wavfile = fopen(user.filename, "w+");
    }

    if (wavfile == NULL) {
        log_info(&fixed, "ERROR: Could not create or open output file '%s'\n", user.filename);
        exit(EXIT_FAILURE);
    }

    /*
    ** Work out how many bytes are required for the FINAL (not intermediate) waveform,
    ** in the format asked for through the command-line parameters.
    ** Note that num_samples is across ALL channels.
    */
    num_data_bytes = user.num_samples * user.bytes_per_sample;
    log_extra(&fixed, "Samples to generate (per channel) = %lu, duration ~%lu ms\n",
              user.num_samples, user.duration_ms);

    /*
    ** Initialise the RIFF header.
    */
    riff_init_header(&riff_header, num_data_bytes, user.save_as_float);
    log_extra(&fixed, "Total RIFF chunk size is %zu bytes.\n", riff_header.ChunkSize);

    /*
    ** Initialise the FORMAT chunk from the various user-defined parameters.
    */
    riff_init_format(&riff_fmt, user.sample_rate, user.save_as_float, user.num_channels,
                     user.bytes_per_sample, user.bits_per_sample);

    /*
    ** Only if floating-point samples are being written, an EXTENDED FORMAT chunk is required too.
    */
    if (user.save_as_float) {
        riff_init_fact(&riff_fact, user.num_samples);
    }

    /*
    ** Initialise the DATA chunk header.
    */
    riff_init_data_hdr(&riff_data, num_data_bytes);

    /*
    ** Write the RIFF header chunk to the file.
    */
    if (!riff_write_header(&riff_header, wavfile)) {
        log_info(&fixed, "Error: failed to write RIFF header.\n");
        success = false;
    }

    /*
    ** Followed by the format chunk.
    */
    if (!riff_write_format(&riff_fmt, wavfile)) {
        log_info(&fixed, "Error: failed to write FMT chunk.\n");
        success = false;
    }

    /*
    ** If this is a floating-point file, an EXTENDED FMT chunk is mandatory.
    */
    if (user.save_as_float) {
        if (!riff_write_fact(&riff_fact, wavfile)) {
            log_info(&fixed, "Error: failed to write EXTENDED FMT chunk.\n");
            success = false;
        }
    }

    /*
    ** Now the DATA chunk (HEADER ONLY), immediately preceeding the sample data itself.
    */
    if (!riff_write_data_hdr(&riff_data, wavfile)) {
        log_info(&fixed, "Error: failed to write DATA chunk.\n");
        success = false;
    }

    /*
    ** Finally, write the sample data to the file in the format requested,
    ** converting from the 32-bit generated data and adding markers if required.
    */
    /*
    ** Generate the requested waveform data into the intermediate buffer.
    */
    for (fixed.sample_number = 0; fixed.sample_number < user.num_samples; ++fixed.sample_number) {
        for (fixed.current_chnl = 0; fixed.current_chnl < user.num_channels; ++fixed.current_chnl) {

            switch (user.wf_type) {
            case WAVEFORM_TYPE_SILENCE:
                generate_silence(&fixed);
                break;

            case WAVEFORM_TYPE_SAW:
                generate_saw(&fixed, &user);
                break;

            case WAVEFORM_TYPE_SQUARE:
                generate_square(&fixed, &user);
                break;

            case WAVEFORM_TYPE_STEPS:
                generate_steps(&fixed, &user);
                break;

            case WAVEFORM_TYPE_COUNTER:
                generate_counter(&fixed, &user, &extra);
                break;

            case WAVEFORM_TYPE_SINE:
                generate_sine(&fixed, &user);
                break;

            case WAVEFORM_TYPE_BURST:
                generate_burst(&fixed, &user, &extra);
                break;

            case WAVEFORM_TYPE_PINK:
                generate_pink(&fixed, &extra);
                break;

            case WAVEFORM_TYPE_WHITE:
                generate_white(&fixed, &extra);
                break;

            default:
                /* Non-specified types are guarded against in the options parsing module.*/
                break;
            }

            if (!finalise_data(&fixed, &user, &extra, wavfile)) {
                success = false;
                break;
            }
        }
    }

    /*
    ** Clean up resources and exit.
    */
    fclose(wavfile);

    if (success) {
        log_extra(&fixed, "Success.\n");
        exit (EXIT_SUCCESS);
    }
    else {
        log_extra(&fixed, "FAILED.\n");
        exit (EXIT_FAILURE);
    }
}
