/*
** wavgen.h
**
** Main application header file.
*/
#ifndef wavgen_h
#define wavgen_h

#include <ctype.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for isatty
#include <stdbool.h>

/*
** Define some sensible option limits.
*/
#define MAX_SAMPLE_RATE_HZ   (192000U)                              // 192kHz.
#define MAX_DURATION_MS      (10U * 60U * 1000U)                    // 10 minutes maxumum duration.
#define MAX_SAMPLES_PER_CHNL (MAX_DURATION_MS * MAX_SAMPLE_RATE_HZ) // 10 minutes at 192kHz.
#define MAX_CHANNELS         (8U)                                   // 8 channels maximum.

#define MAX_LEVEL_32BIT      (0x7FFFFFFF)

/*
** Typedefs and enums.
*/
typedef enum {
    WAVEFORM_TYPE_BURST,
    WAVEFORM_TYPE_COUNTER,
    WAVEFORM_TYPE_SAW,
    WAVEFORM_TYPE_SILENCE,
    WAVEFORM_TYPE_SINE,
    WAVEFORM_TYPE_SQUARE,
    WAVEFORM_TYPE_STEPS,
    WAVEFORM_TYPE_PINK,
    WAVEFORM_TYPE_WHITE,
    NUM_WAVEFORM_TYPES
} WAVEFORM_TYPE;

enum BYTES_PER_SAMPLE {
    BYTES_NONE  = 0,
    BYTES_8BIT  = 1,
    BYTES_16BIT = 2,
    BYTES_24BIT = 3,
    BYTES_32BIT = 4
};

/* Mixed pointer to a sample/buffer that can hold either a 32-bit int or a float sample */
typedef union {
    int32_t i;
    float   f;
} SAMPLE;

/*
** General parameters that apply to all (or at least most) waveforms.
** These represent the command-line options "b:c:d:s:f:l:a:p:w:m:"
*/
struct COMMON_USER_PARAMS {
    bool     save_as_float;     // -b 0
    uint32_t sample_rate;       // -r 48000|44100 etc,
    uint32_t bits_per_sample;   // -b 32|24|16
    uint32_t bytes_per_sample;  //    =4 =4 =2
    uint32_t num_channels;      // -c 1:8
    uint32_t duration_ms;       // -d (or calculated from -s)
    uint32_t frequency_hz;      // -f (of the main waveform)
    uint64_t num_samples;       // -s (or calculated from -d)
    float    peak_level_dbfs;   // -l
    float    align_level_dbfs;  // -a

    WAVEFORM_TYPE wf_type;      // -t
    const char *filename;       // The final parameter (no prefix).
};

/*
** Extra options that only apply to some waveforms.
*/
struct ADDITIONAL_USER_PARAMS {
    uint32_t power_fraction;    // -w (e.g. '8' for 1/8th power)
    uint32_t period_ms;         // -p (for burst/impulse waveforms)
    uint32_t num_cycles;        // -n (for burst/impulse waveforms)
    bool     markers_on;        // -m
    bool     markers_in_msb;    // -m tb|msb (not bb|lsb)
    bool     uncorrelated;      // -u (for pink noise)
};

/*
** Fixed parameters that aren't DIRECTLY set by the user, or are internal only.
*/
struct FIXED_PARAMS {
    double gain;        // Gain value calculated from user params (align, level, power).
    bool   verbose;     // Output information to the console.
    bool   piping;      // True if piping the "wavfile" to another application.

    SAMPLE   sample_value;  // Holds the value of the current sample being generated.
    uint64_t sample_number; // Holds the offset of the current sample (i.e. the sample number)
    uint32_t current_chnl;  // Holds the channel number of the current sample being generated.
};

/*
** Function declarations.
*/

/* From help.c */
void help(void);
void waveform_type_help(WAVEFORM_TYPE type);
void help_type_unknown(void);
void help_version(void);

/* From log.c */
void log_info(struct FIXED_PARAMS *fixed, const char *format, ...);
void log_extra(struct FIXED_PARAMS *fixed, const char *format, ...);

/* From opts.c */
void   parse_opts(int argc, char *argv[], struct FIXED_PARAMS *fixed, struct COMMON_USER_PARAMS *user, struct ADDITIONAL_USER_PARAMS *extra);
double gain_from_params(struct FIXED_PARAMS *fixed, float align_dbfs, float peak_dbfs, uint32_t power_fraction);

/* From wf_xxx.c - these waveforms can have channel-markers overlaid.*/
void generate_saw(struct FIXED_PARAMS *fixed, struct COMMON_USER_PARAMS *user_params);
void generate_steps(struct FIXED_PARAMS *fixed, struct COMMON_USER_PARAMS *user_params);
void generate_silence(struct FIXED_PARAMS *fixed);
void generate_counter(struct FIXED_PARAMS *fixed, struct COMMON_USER_PARAMS *user_params, struct ADDITIONAL_USER_PARAMS *extra_params);

/* From wf_xxx.c - these waveforms cannot have markers, but their level can be specified by power fraction.*/
void generate_square(struct FIXED_PARAMS *fixed, struct COMMON_USER_PARAMS *user_params);
void generate_sine(struct FIXED_PARAMS *fixed, struct COMMON_USER_PARAMS *user_params);
void generate_burst(struct FIXED_PARAMS *fixed, struct COMMON_USER_PARAMS *user_params, struct ADDITIONAL_USER_PARAMS *extra_params);
void generate_white(struct FIXED_PARAMS *fixed, struct ADDITIONAL_USER_PARAMS *extra_params);
void generate_pink(struct FIXED_PARAMS *fixed, struct ADDITIONAL_USER_PARAMS *extra_params);

/* From wf_markers.c */
bool check_markers(struct FIXED_PARAMS *fixed, struct COMMON_USER_PARAMS *user, struct ADDITIONAL_USER_PARAMS *extra);
bool add_markers(struct FIXED_PARAMS *fixed, bool markers_in_msb);

/* From wf_output.c */
bool finalise_data(struct FIXED_PARAMS *fixed, struct COMMON_USER_PARAMS *user, struct ADDITIONAL_USER_PARAMS *extra_params, FILE *wavfile);

#endif
