/*
** help.c
**
** Output help messages.
*/
#include <stdio.h>
#include <stdlib.h>
#include "wavgen.h"

static const char version_str[] = "0.0.1";

void help(void)
{
    printf("Waveform Generator (wavgen) utility version %s\n", version_str);
    printf("\n");
    printf("Usage: wavgen [opts] [filename]\n\n");
    printf("Where opts:\n");
    printf(" -a [--align]     Alignment level in dBFS that the peak level is relative to.\n");
    printf(" -b [--bitdepth]  Bit-depth of the samples (16, 24 or 32-bit) [32-bit].\n");
    printf(" -c [--channels]  Number of channels in the generated output file [1].\n");
    printf(" -d [--duration]  Duration of the file content in seconds [default 1s].\n");
    printf(" -f [--frequency] Frequency (does not effect the 'count' types) [440Hz].\n");
    printf(" -h [--help]      Show this help page.\n");
    printf(" -l [--level]     Peak level in dBFS (does not effect non-audio types) [0dBFS].\n");
    printf(" -m [--markers]   Add channel markers (top or bottom byte) into samples [OFF].\n");
    printf(" -n [--numcycles] Number of cycles for each burst or impulse waveform.\n");
    printf(" -p [--period]    The period for intermittent burst or impulse waveforms.\n");
    printf(" -w [--power]     Alternative to '-l', the 'power fraction' may be set instead.\n");
    printf(" -t [--type]      Type of waveform to be generated (see below for options).\n");
    printf(" -s [--samples]   Number of samples per-channel (an alternative to 'duration').\n");
    printf(" -v [--verbose]   Output data to stdout, if not piping to another application.\n");
    printf("    [--version]   Show the version number and exit.\n");
    printf("and:\n");
    printf(" filename is the output wavfile name, required unless piping to another program.\n");
    printf("\n");
    printf("The arguments for waveform type (-t/--type) are:\n");
    printf(" counter : +ve sample values incrementing by one LSB (a very slow saw-tooth).\n");
    printf(" steps   : A set of five levels useful for checking normalisation/conversion.\n");
    printf(" saw     : A symetrical saw-tooth waveform at the specified frequency.\n");
    printf(" sine    : A symetrical sinewave at the specified frequency.\n");
    printf(" square  : A symetrical square-wave at the specified frequency.\n");
    printf(" silence : Silence, apart from the added channel markers if selected with '-m'.\n");
    printf(" pink    : Pink noise generated by 1/f filtering the white noise source.\n");
    printf(" burst   : A periodic burst of sinewave cycles, useful for measuring latency.\n");
    printf(" white   : White noise generated using a fast psudo-random noise generator.\n");
    printf("\n");
    printf("e.g. wavgen -t counter -b 32 -c 2 -m msb /tmp/count-s32le-2ch-marked.wav\n");
    printf(" or  wavgen -t sine -b 32 -c 2 -d 1000 -f 1000 | aplay -D hw:default\n");
    printf("\n");
    printf("Use 'wavgen -t <type> --help' for context-sensitive help on each waveform type.\n");
}

void print_common_options()
{
    printf(" -v             : Verbose info on stdout if not piping to another app.\n");
    printf(" -r <rate>      : The sample-rate in Hz, e.g. 48000 for 48kHz.\n");
    printf(" -d <duration>  : The duration of the waveform in milliseconds (ms).\n");
    printf(" -s <samples>   : The number of samples per channel (an alternative to duration).\n");
    printf(" -c <channels>  : The number of channels to generate (1 - %u).\n", MAX_CHANNELS);
}

void help_type_burst(void)
{
    printf("PERIODIC BURST (-t burst):\n");
    printf("\n");
    printf("Example: ./wavgen -t burst -c 2 -d 1000 -p 100 -n 2 -f 1000 burst-x10-1khz.wav\n");
    printf("\n");
    printf("This type produces a 'burst' of sinewave cycles at a defined period, for example\n"
           "four cycles of 100Hz every 100ms, for 10s. This type of waveform is very useful\n"
           "for characterising the latency through a device, or the polarity of a signal as\n"
           "it progresses through the signal chain. It is also useful for time-aligning\n"
           "loudspeaker drivers in a multi-way system, or time-aligning a subwoofer to a\n"
           "main box.\n");
    printf("\n");
    printf("As with the pure sinewave option, an integer division of the requested burst\n"
           "frequency into the sample-rate will be made so that full cycles (from zero to\n"
           "zero) result for each burst.\n");
    printf("\n");
    printf("Channel markers are not allowed.\n");
    printf("\n");
    printf("Configure the burst waveform using these options:\n");
    print_common_options();
    printf(" -f <frequency> : The freqency in Hz of the periodic burst.\n");
    printf(" -p <period>    : The period in milliseconds (ms) between each set of bursts.\n");
    printf(" -n <cycles>    : The number of cycles that each burst consists of.\n");
    printf(" -l <level>     : The signal amplitude in dB relative to the alignment level.\n");
    printf(" -a <align>     : An optional alignment level (dBFS) that -l is relative to.\n");
}

void help_type_counter(void)
{
    printf("COUNTER (-t counter):\n");
    printf("\n");
    printf("Example: ./wavgen -t counter -c 2 -b 32 -m lsb -s 100 /tmp/count-s32le.wav\n");
    printf("\n");
    printf("This type produces an integer count increasing from zero upwards by one at each\n"
           "successive sample. This type is most useful for debugging buffer problems or for\n"
           "charactising signal loss (drop-outs) because the length of missing signal can be\n"
           "determined by the counter values at either side.\n"
           "It may also be useful for checking that playback starts and ends at the correct\n"
           "places and that all samples are played back.\n");
    printf("\n");
    printf("Channel markers may be added to this signal type, in which case the values of\n"
           "the count are restricted to the bytes *without* markers. Markers may be added\n"
           "into either the MSB or the LSB of each sample.\n");
    printf("\n");
    printf("Note that the counter will be the same for each channel within the same frame,\n"
           "making it easier to spot mis-aligned channels, glitches, bad interleaving etc.\n");
    printf("\n");
    printf("This is not a real audio waveform so no control of level (volume) is possible.\n"
           "Use caution when playing this signal back.\n");
    printf("\n");
    printf("Configure the counter using these options:\n");
    print_common_options();
    printf(" -m <lsb|msb>   : Place channel markers in the LSB or MSB.\n");
}

void help_type_steps(void)
{
    printf("STEPS (-t steps):\n");
    printf("\n");
    printf("Example: ./wavgen -t steps -c 2 -b 16 -m lsb -s 100 /tmp/steps-s16le.wav\n");
    printf("\n");
    printf("This type is a variation on the counter type that instead produces just a few\n"
           "descrete steps that are extremely easy to see when analysed or viewed as hex in\n"
           "memory.\n");
    printf("\n");
    printf("Channel markers may be added in either the LSB (-m lsb) or MSB (-m msb).\n");
    printf("\n");
    printf("This is not a real audio waveform so no control of level (volume) is possible.\n"
           "Use caution when playing this signal back.\n");
    printf("\n");
    printf("Configure the step waveform using these options:\n");
    print_common_options();
    printf(" -m <lsb|msb>   : Place channel markers in the LSB or MSB.\n");
}

void help_type_silence(void)
{
    printf("SILENCE (-t silence):\n");
    printf("\n");
    printf("Example: ./wavgen -t silence -c 2 -b 0 -m msb -d 1000 /tmp/silence-float.wav\n");
    printf("\n");
    printf("This type simply produces silence (zero-value samples). Use it to test whether\n"
           "your audio chain really is silent during playback.\n");
    printf("\n");
    printf("Channel markers may be added to the samples, into either the MSB (-m msb) or the\n"
           "LSB (-m lsb), and this is obviously one of the best uses of markers because they\n"
           "cannot be mistaken for audio data.\n");
    printf("\n");
    printf("CAUTION: Adding markers in the MSB will make the signal distinctly NON-silent,\n"
           "at least in D.C. terms.\n");
    printf("\n");
    printf("Configure the saw-tooth waveform using these options:\n");
    print_common_options();
    printf(" -m <lsb|msb>   : Place channel markers in the LSB or MSB.\n");
}

void help_type_saw(void)
{
    printf("SAW-TOOTH (-t saw):\n");
    printf("\n");
    printf("Example: ./wavgen -t saw -c 2 -d 1000 -f 440 -l -10.0 /tmp/saw-10dbfs.wav\n");
    printf("\n");
    printf("This type produces a saw-tooth (ramp) waveform at approximately the requested\n"
           "frequency. If the requested frequency does not divide nicely into the sample-\n"
           "rate then it will be adjusted to the nearest value to avoid adding frequency\n"
           "jitter to the generated tone. At the most usual test frequencies such as 1000Hz\n"
           "you will get exactly what you ask for.\n");
    printf("\n");
    printf("The ramp will start at the zero level before climbing to the requested peak\n"
           "level and 'wrapping around' to the negative peak level, in other words the\n"
           "waveform is symmetrical. Note that the wrap-around will result in a large pop\n"
           "at low frequencies (potentially damaging if played at high volume).\n");
    printf("\n");
    printf("Channel markers are not allowed (use the *counter* or *steps* types instead).\n");
    printf("\n");
    printf("Configure the saw-tooth waveform using these options:\n");
    print_common_options();
    printf(" -f <frequency> : The freqency in Hz of the generated waveform.\n");
    printf(" -l <level>     : The signal amplitude in dB relative to the alignment level.\n");
    printf(" -a <align>     : An optional alignment level (dBFS) that -l is relative to.\n");
}

void help_type_sine(void)
{
    printf("SINE-WAVE (-t sine):\n");
    printf("\n");
    printf("Example: ./wavgen -t sine -c 2 -d 1000 -f 440 -a -18 -l -3.0 sine-22dbfs.wav\n");
    printf("\n");
    printf("This type produces a pure sinewave at approximately the requested frequency.\n"
           "If the requested frequency does not divide nicely into the sample-rate then\n"
           "it will be adjusted to the nearest value to avoid adding frequency jitter\n"
           "to the generated tone. At the most usual test frequencies such as 1000Hz\n"
           "then you will get exactly what you ask for.\n");
    printf("\n");
    printf("Channel markers are not allowed.\n");
    printf("\n");
    printf("Configure the sine-wave using these options:\n");
    print_common_options();
    printf(" -f <frequency> : The freqency in Hz of the generated waveform.\n");
    printf(" -l <level>     : The signal amplitude in dB relative to the alignment level.\n");
    printf(" -a <align>     : An optional alignment level (dBFS) that -l is relative to.\n");
}

void help_type_square(void)
{
    printf("SQUARE-WAVE (-t square):\n");
    printf("\n");
    printf("Example: ./wavgen -t square -c 1 -d 1000 -f 20 square-1ch-0dbfs.wav\n");
    printf("\n");
    printf("This type produces a non-antialiased squarewave at approximately the requested\n"
           "frequency. If the requested frequency does not divide nicely into the sample-\n"
           "rate then it will be adjusted to the nearest value to avoid adding frequency\n"
           "jitter to the generated tone. At the most usual test frequencies such as 1000Hz\n"
           "then you will get exactly what you ask for.\n");
    printf("\n");
    printf("Channel markers are not allowed.\n");
    printf("\n");
    printf("Configure the square-wave using these options:\n");
    print_common_options();
    printf(" -f <frequency> : The freqency in Hz of the generated waveform.\n");
    printf(" -l <level>     : The signal amplitude in dB relative to the alignment level.\n");
    printf(" -a <align>     : An optional alignment level (dBFS) that -l is relative to.\n");
}

void help_type_pink(void)
{
    printf("PINK NOISE (-t pink):\n");
    printf("\n");
    printf("Example: ./wavgen -t pink -c 2 -d 5000 -w 8 -u pink-eighth-power.wav\n");
    printf("\n");
    printf("This type generates a fairly good approximation to a pink noise source, but is\n"
           "not intended to be used for very accurrate frequency measurements. It is quite\n"
           "common in professional audio testing to need an 'eighth-power' pink noise source\n"
           "which can be obtained here using the `--power 8` and `--level 0` options to give\n"
           "an amplitude appropriately lower than 0dBFS.\n"
           "Note that due to the nature of the noise spectrum, asking for an alignment level\n"
           "of 0dFS (the default) will give a pink-noise signal that measures an RMS level\n"
           "of approximately -15dBFS.\n");
    printf("\n");
    printf("Channel markers may be added in the LSB only (-m lsb), where they won't\n"
           "significantly affect the perceived sound.\n");
    printf("\n");
    printf("Configure the pink-noise waveform using these options:\n");
    print_common_options();
    printf(" -l <level>     : The signal amplitude in dB relative to the alignment level.\n");
    printf(" -a <align>     : An optional alignment level (dBFS) that -l is relative to.\n");
    printf(" -m lsb         : Place channel markers in the LSB.\n");
    printf(" -u             : Generate uncorrelated noise (different on each channel)\n");
}

void help_type_white(void)
{
    printf("WHITE NOISE (-t white):\n");
    printf("\n");
    printf("Example: ./wavgen -t white -c 2 -d 5000 -u white-5s-0dbfs.wav\n");
    printf("\n");
    printf("This type generates a fairly good approximation to a white noise source, but is\n"
           "not intended to be used for very accurrate frequency measurements.\n"
           "Note that due to the nature of the noise spectrum, asking for an alignment level\n"
           "of 0dFS (the default) will give a white-noise signal that measures an RMS level\n"
           "of approximately -4.8dBFS.\n");
    printf("\n");
    printf("Channel markers may be added in the LSB only (-m lsb), where they won't\n"
           "significantly affect the perceived sound.\n");
    printf("\n");
    printf("Configure the pink-noise waveform using these options:\n");
    print_common_options();
    printf(" -l <level>     : The signal amplitude in dB relative to the alignment level.\n");
    printf(" -a <align>     : An optional alignment level (dBFS) that -l is relative to.\n");
    printf(" -m lsb         : Place channel markers in the LSB.\n");
    printf(" -u             : Generate uncorrelated noise (different on each channel)\n");
}

void help_type_unknown(void)
{
    printf("UNRECOGNISED type:\n");
    printf("\n");
    printf("These waveform types are supported:\n");
    printf(" -t burst   : A periodic burst of -c sine-wave cycles every -p seconds.\n");
    printf(" -t counter : A non-audio increamental count in each sample position.\n");
    printf(" -t saw     : A saw-tooth waveform at frequency -f <freq>.\n");
    printf(" -t silence : Audio silence (zero-value samples) with optional channel markers.\n");
    printf(" -t sine    : A sine-wave at frequency -f <freq>.\n");
    printf(" -t steps   : A non-audio waveform consisting of large discrete steps.\n");
    printf(" -t square  : A square-wave at frequency -f <freq>.\n");
    printf(" -t pink    : A pink noise source (1/f filtered white noise).\n");
    printf(" -t white   : A white noise source.\n");
}

void help_version(void)
{
    printf("Waveform Generator (wavgen) utility version %s\n", version_str);
}

void waveform_type_help(WAVEFORM_TYPE type)
{
    /*
    ** Generate some specific help for each waveform type.
    */
    switch (type) {
    case WAVEFORM_TYPE_COUNTER:
        help_type_counter();
        break;

    case WAVEFORM_TYPE_STEPS:
        help_type_steps();
        break;

    case WAVEFORM_TYPE_SILENCE:
        help_type_silence();
        break;

    case WAVEFORM_TYPE_SAW:
        help_type_saw();
        break;

    case WAVEFORM_TYPE_SINE:
        help_type_sine();
        break;

    case WAVEFORM_TYPE_SQUARE:
        help_type_square();
        break;

    case WAVEFORM_TYPE_PINK:
        help_type_pink();
        break;

    case WAVEFORM_TYPE_BURST:
        help_type_burst();
        break;

    case WAVEFORM_TYPE_WHITE:
        help_type_white();
        break;

    default:
        help_type_unknown();
        break;
    }
}