/*
** opts.c
**
** Parse command-line options into various structs.
*/
#include <math.h>
#include <getopt.h>
#include "wavgen.h"

void parse_duration(const char* arg_str, uint32_t *duration_ms)
{
    int items;
    char units[2];

    items = sscanf(arg_str, "%u%c%c", duration_ms, &units[0], &units[1]);
    if (items > 1) {
        if (units[0] == 's') {
            *duration_ms *= 1000U; // Seconds to ms.
        }
        else if (units[0] == 'm') {
            if (units[1] == '\0') {
                *duration_ms *= 60000; // Minutes to ms.
            }
            else {
                if (units[1] != 's') { // A suffix of 'ms' is allowed.
                    printf("ERROR: Unknown units for duration (use s, m, h, ms or nothing)");
                    exit(EXIT_FAILURE);
                }
            }
        }
        else if (units[0] == 'h') {
            *duration_ms *= 3600000; // Hours to ms.
        }
        else if (units[0] != '\0') {
            printf("ERROR: Unknown units for duration (use h, m, s, ms/nothing)");
            exit(EXIT_FAILURE);
        }
    } // else ms is assumed.
}

void parse_frequency(const char* arg_str, uint32_t *freq_hz)
{
    int items;
    char units[2];

    items = sscanf(arg_str, "%u%c%c", freq_hz, &units[0], &units[1]);
    if (items > 1) {
        if (units[0] == 'k') {
            *freq_hz *= 1000U; // kHz to Hz.
        }
        else if (units[1] != 'h' && units[1] != 'H') {
            printf("ERROR: Unknown units for frequency (use kHz or Hz/nothing)");
            exit(EXIT_FAILURE);
        }
    } // else hz is assumed.
}

void parse_opts(int argc, char *argv[], struct FIXED_PARAMS *fixed, struct COMMON_USER_PARAMS *user, struct ADDITIONAL_USER_PARAMS *extra)
{
    int opt;
    int num_args;
    int opt_index;

    bool help_required  = false;
    bool context_help   = false;
    bool calculate_gain = false;

    /* Options that need some intermediate processing.*/
    unsigned int opt_s = 0U;
    unsigned int opt_d = 1U;

    /*
    ** Initialise options to default parameters.
    */
    fixed->verbose         = false;
    fixed->piping          = !isatty(STDOUT_FILENO); // Inhibit stdout logs if piping to another application.

    user->wf_type          = NUM_WAVEFORM_TYPES; // i.e. invalid.
    user->save_as_float    = false;
    user->sample_rate      = 48000U;
    user->bits_per_sample  = 32U;
    user->bytes_per_sample = 4U;
    user->num_channels     = 1U;
    user->duration_ms      = 1000U;
    user->num_samples      = (user->duration_ms * user->sample_rate * user->num_channels) / 1000U; // =duration.
    user->frequency_hz     = 440U;
    user->peak_level_dbfs  = 0.0f;
    user->align_level_dbfs = 0.0f;
    user->filename         = NULL;

    extra->power_fraction  = 1U;
    extra->period_ms       = 100U;
    extra->markers_on      = false;
    extra->markers_in_msb  = false;
    extra->uncorrelated    = false;

    /*
    ** Define the available options, both short and long.
    */
    static const char short_opts[] = "huva:x:b:c:d:f:l:m:n:p:r:s:t:w:";
    static struct option long_opts[] = {
       {"align",        required_argument, 0, 'a' },
       {"bitdepth",     required_argument, 0, 'b' },
       {"channels",     required_argument, 0, 'c' },
       {"duration",     required_argument, 0, 'd' },
       {"frequency",    required_argument, 0, 'f' },
       {"help",         no_argument,       0, 'h' },
       {"level",        required_argument, 0, 'l' },
       {"markers",      required_argument, 0, 'm' },
       {"numcycles",    required_argument, 0, 'n' },
       {"period",       required_argument, 0, 'p' },
       {"power",        required_argument, 0, 'w' },
       {"rate",         required_argument, 0, 'r' },
       {"samples",      required_argument, 0, 's' },
       {"type",         required_argument, 0, 't' },
       {"uncorrelated", no_argument,       0, 'u' },
       {"verbose",      no_argument,       0, 'v' },
       {"version",      no_argument,       0, 'x' },
       {0,              0,                 0,  0  }
    };

    /*
    ** Parse command-line arguments.
    */
    num_args  = 0;
    opt_index = 0;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, &opt_index)) != -1) {
        ++opt_index;
        switch (opt) {

        case 'a':
            log_extra(fixed, "Alignment level option is '%s'\n", optarg);
            sscanf(optarg, "%f", &user->align_level_dbfs);
            /* Alignment level is "absolute" and so cannot be +ve (more than -dBFS).*/
            if (user->align_level_dbfs > 0.0f) {
                user->align_level_dbfs = 0.0f;
            }
            calculate_gain = true;
            num_args += 2;
            break;

        case 'b':
            log_extra(fixed, "Bit-depth option is '%s'\n", optarg);
            sscanf(optarg, "%hhu", &user->bits_per_sample);

            if ((user->bits_per_sample != 32U ) && (user->bits_per_sample != 16U ) && (user->bits_per_sample != 0U)) {
                log_info(fixed, "This bit-width is not currently supported.\n");
                /* TODO: Add 24-bit support.*/
                exit (EXIT_FAILURE);
            }

            /* Passing -b 0 is a hacky way to ask for FLOAT_LE format.*/
            if (user->bits_per_sample == 0U) {
                user->save_as_float = true;
                user->bits_per_sample = 32U;
                user->bytes_per_sample = BYTES_32BIT;
                log_extra(fixed, "Floating-point format selected (FLOAT_LE).\n");
            }
            else {
                log_extra(fixed, "Fixed-point format selected.\n");
                if (user->bits_per_sample != 8U  &&
                    user->bits_per_sample != 16U &&
                    user->bits_per_sample != 24U &&
                    user->bits_per_sample != 32U) {
                    user->bits_per_sample = 32U;
                }
            }

            num_args += 2;
            break;

        case 'c':
            log_extra(fixed, "Channels option is '%s'\n", optarg);
            sscanf(optarg, "%hhu", &user->num_channels);
            if (user->num_channels > MAX_CHANNELS) {
                user->num_channels = MAX_CHANNELS;
            }
            num_args += 2;
            break;

        case 'd':
            log_extra(fixed, "Duration option is '%s'\n", optarg);
            parse_duration(optarg, &opt_d);
            num_args += 2;
            break;

        case 'f':
            log_extra(fixed, "Frequency option is '%s'\n", optarg);
            parse_frequency(optarg, &user->frequency_hz);
            /* Constrained later when the sample rate is known.*/
            num_args += 2;
            break;

        case 'h':
            help_required = true;
            break;

        case 'l':
            log_extra(fixed, "Peak level option is '%s' dB\n", optarg);
            sscanf(optarg, "%f", &user->peak_level_dbfs);
            /* Peak level is relative to the alignment, so can potentially be +ve.*/
            if (user->peak_level_dbfs > 20.0f) {
                user->peak_level_dbfs = 20.0f;
            }
            calculate_gain = true;
            num_args += 2;
            break;

        case 'm':
            log_extra(fixed, "Channel markers are ON (in '%s')\n", optarg);
            extra->markers_on = true;
            if ((strcmp(optarg, "tb") == 0) || (strcmp(optarg, "msb") == 0)) {
                extra->markers_in_msb = true;
            }
            else {
                extra->markers_in_msb = false;
            }
            num_args += 2;
            break;

        case 'n':
            log_extra(fixed, "Num cycles option is '%s'\n", optarg);
            sscanf(optarg, "%u", &extra->num_cycles);
            num_args += 2;
            break;

        case 'p':
            log_extra(fixed, "Period option is '%s' ms\n", optarg);
            parse_duration(optarg, &extra->period_ms);
            num_args += 2;
            break;

        case 'r':
            log_extra(fixed, "Sample Rate option is '%s'\n", optarg);
            parse_frequency(optarg, &user->sample_rate);
            if (user->sample_rate > MAX_SAMPLE_RATE_HZ) {
                user->sample_rate = MAX_SAMPLE_RATE_HZ;
            }
            num_args += 2;
            break;

        case 's':
            log_extra(fixed, "Sample count option is '%s'\n", optarg);
            sscanf(optarg, "%u", &opt_s);
            if (opt_s > MAX_SAMPLES_PER_CHNL) {
                opt_s = MAX_SAMPLES_PER_CHNL;
            }
            num_args += 2;
            break;

        case 't':
            log_extra(fixed, "Waveform requested: '%s'\n", optarg);
            context_help = true;

            if ((strcmp(optarg, "saw") == 0) || (strcmp(optarg, "sawtooth") == 0)) {
                user->wf_type = WAVEFORM_TYPE_SAW;
            }
            else if ((strcmp(optarg, "sine") == 0) || (strcmp(optarg, "sinewave") == 0)) {
                user->wf_type = WAVEFORM_TYPE_SINE;
            }
            else if ((strcmp(optarg, "step") == 0) || (strcmp(optarg, "steps") == 0)) {
                user->wf_type = WAVEFORM_TYPE_STEPS;
            }
            else if ((strcmp(optarg, "square") == 0) || (strcmp(optarg, "squarewave") == 0)) {
                user->wf_type = WAVEFORM_TYPE_SQUARE;
            }
            else if ((strcmp(optarg, "count") == 0) || (strcmp(optarg, "counter") == 0)) {
                user->wf_type = WAVEFORM_TYPE_COUNTER;
            }
            else if (strcmp(optarg, "silence") == 0) {
                user->wf_type = WAVEFORM_TYPE_SILENCE;
            }
            else if (strcmp(optarg, "pink") == 0) {
                user->wf_type = WAVEFORM_TYPE_PINK;
            }
            else if (strcmp(optarg, "burst") == 0) {
                user->wf_type = WAVEFORM_TYPE_BURST;
            }
            else if (strcmp(optarg, "white") == 0) {
                user->wf_type = WAVEFORM_TYPE_WHITE;
            }
            else {
                help_type_unknown();
                exit(EXIT_FAILURE);
            }
            num_args += 2;
            break;

        case 'u':
            log_info(fixed, "Uncorrelated mode ON\n");
            extra->uncorrelated = true;
            num_args += 1;
            break;

        case 'v':
            log_info(fixed, "Verbose mode ON\n");
            fixed->verbose = true;
            num_args += 1;
            break;

        case 'w':
            log_extra(fixed, "Power fraction option is '%s'\n", optarg);
            sscanf(optarg, "%hu", &extra->power_fraction);
            if (extra->power_fraction < 1U) {
                extra->power_fraction = 1U;
            }
            calculate_gain = true;
            num_args += 2;
            break;

        case 'x':
            help_version();
            exit(EXIT_SUCCESS);
            break;

        default:
            log_info(fixed, "Unrecognised command-line option\n");
            break;
        }
    }


    /*
    ** If the user has asked for help, do that now (including the context-sensitive option).
    */
    if (help_required) {
        if (context_help) {
            /* Help on the type specified by '-t'.*/
            waveform_type_help(user->wf_type);
        }
        else {
            /* Generic command help.*/
            help();
        }
        exit(EXIT_SUCCESS);
    }

    /*
    ** Stop now if the user didn't specify a waveform type. This is required.
    */
    if (user->wf_type == NUM_WAVEFORM_TYPES) {
        waveform_type_help(user->wf_type);
        exit(EXIT_FAILURE);
    }

    /*
    ** If any of the level/gain options have been specified, calculate the required gain
    ** to transform a 0dBFS signal to the required level.
    */
    if (calculate_gain) {
        fixed->gain = gain_from_params(fixed, user->align_level_dbfs, user->peak_level_dbfs, extra->power_fraction);
    }
    else {
        fixed->gain = 1.0;
    }

    /*
    ** Sanitise and constrain options that don't make sense.
    */
    if (user->frequency_hz < 1U) {
        /* 0 Hz would cause floating-point division exceptions.*/
        user->frequency_hz = 1U;
    }

    if (user->frequency_hz > user->sample_rate / 2) {
        log_info(fixed, "Frequency must be less than half the sample rate (%u).\n", user->sample_rate);
        exit(EXIT_FAILURE);
    }

    if (extra->period_ms > user->duration_ms) {
        extra->period_ms = user->duration_ms;
    }

    /*
    ** Because adding "Cx" in sample MSBs makes them very -ve, this cannot be used with
    ** proper symmetrical waveforms because the +ve/-ve half will be too similar.
	** It would also invalidate any level requirements which are likely to be vital
	** when using these types.
    */
    if (extra->markers_on && extra->markers_in_msb) {
        switch (user->wf_type) {
        case WAVEFORM_TYPE_SAW:
        case WAVEFORM_TYPE_SINE:
        case WAVEFORM_TYPE_SQUARE:
        case WAVEFORM_TYPE_PINK:
        case WAVEFORM_TYPE_WHITE:
            log_info(fixed, "Markers cannot be put in the MSB of this waveform type.\n");
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    /*
    ** The special "optind" is the index in argv of the first argv-element that is not an option.
    ** This should be a filename unless the user is piping the output to another application.
    */
    if (((argc - optind) == 1) && (!fixed->piping)) {
        user->filename = argv[argc-1];
        if (strlen(user->filename) < 5) {
            log_info(fixed, "Invalid output filename (length < 5 characters).");
            log_info(fixed, "Supply a filename as the last parameter - at least 'o.wav'.\n");
            exit(EXIT_FAILURE);
        }
        log_extra(fixed, "Output filename will be '%s'\n", user->filename);
    }
    else if (argc == optind) {
        if (!fixed->piping) {
            log_info(fixed, "Invalid arguments (try ./wavgen --help).\n");
            log_info(fixed, "Either provide an output filename or pipe to another application.\n");
            exit(EXIT_FAILURE);
        }
        else {
            // This is expected when piping to another application.
            // Logging must be suppressed in this case, and there is no filename.
        }
    }

    /*
    ** Calculate data quantities.
    */
    user->bytes_per_sample = user->bits_per_sample / 8U;
    if (opt_s != 0) {
        /* opt_s is the number of samples PER CHANNEL specified on the command-line.*/
        user->num_samples =  opt_s * user->num_channels;
        user->duration_ms = (opt_s * 1000U) / user->sample_rate; // This does not need to be accurate.
    }
    else {
        /*
        ** If no opt_s, assume that DURATION (in ms) was specified (opt_d).
        ** Constrain the duration to a maximum, although it will still be possible to
        ** overflow the uint32_t miliiseconds value by asking for a silly number of hours.
        */
        if (opt_d > MAX_DURATION_MS) {
            opt_d = MAX_DURATION_MS;
        }

        // The calculation could overflow so a cast to 64-bit is required.
        user->num_samples = ((uint64_t) opt_d * user->sample_rate * user->num_channels) / 1000U;
        user->duration_ms = opt_d;
    }

    return;
}

/*
** Helper function to calculate an absolute gain value to transform an otherwise
** full-scale (0dBFS) signal to the requested level.
**
** The "alignment level" is the main setting. This sets the required peak level
** of the signal for the environment in which it is being used. Very often though,
** the alignment level is simply "maximum", or 0dBFS.
**
** The peak level setting is relative to the alignment level. It allows a signal
** that is typically at a certain level to be set lower or higher than the alignment
** level.
**
** The "power fraction" is optional and allowed only for specific waveforms.
** If given, it represents the denominator of a fractional power requirement.
** Most common is the requirement for an "eighth-power" waveform, i.e. power_fraction == 8.
** Zero/null == unused.
*/
double gain_from_params(struct FIXED_PARAMS *fixed, float align_dbfs, float peak_dbfs, uint16_t power_fraction)
{
    double gain;
    double target_dbfs;

    /*
    ** Work out the overall dB target by combining the alignment level and peak target.
    ** Both of these parameters are VOLTAGE GAIN adjustments.
    */
    target_dbfs = (double) align_dbfs + (double) peak_dbfs;

    /*
    ** If a power fraction (e.g. 1/8th power) is required, modify the target dB value accordingly.
    ** Note that this is a POWER (not VOLTAGE) adjustment, hence the 10*log10 rather than 20*log10.
    */
    if (power_fraction != 1) {
        target_dbfs = (double) align_dbfs + 10.0 * log10(1.0 / (double) power_fraction);

        if (peak_dbfs != 0.0f) {
            log_info(fixed, "WARNING: Peak level is ignored when a power fraction is supplied.\n");
        }
    }

    /*
    ** Convert the target dB level to a gain value.
    ** This is an adjustment of VOLTAGE output (not POWER), hence it uses 20^x.
    */
    gain = pow(10, target_dbfs / 20.0);
    log_extra(fixed, "Overall gain factor is %.3f\n", gain);

    /* The OVERALL gain cannot be greater than one, otherwise clipping/wrapping would occur.*/
    if (gain > 1.0) {
        gain = 1.0;
    }

    return gain;
}
