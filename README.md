# Wavfile Generator (wavgen)

This is a tool to generate simple WAV files designed to help test embedded audio systems.
Output is either written to the file system or piped to an application that expectes a .WAV file.


## Description

When designing, programming or testing embedded audio hardware and software that can play WAV-format audio files
it is usual to prepare sets of test-files manually to play through the system. On systems with limited disk space
or connectivity this can be time-consuming or awkward.

This application therefore allows some useful test waveforms to be generated locally on the system, or even
"on the fly" to simplify testing and debugging. It might also be useful in automated test systems for generating
test stimulus.

## Getting Started


### Dependencies

There are no particular dependancies. The code should compile on Linux or Windows, but is really
targetted at Linux systems running ALSA where **wavgen**'s output can be piped directly to **aplay**
or your own playback application.


### Building

Use CMake:

  ```
  cmake .
  make
  ```

Or just build directly:

```
cc wavgen.c help.c log.c opts.c riff.c wf*.c -o wavgen
```


### Cross-Compiling

Because this application is largely intended to run on small embedded systems, cross-compiling will be common.
I've been using the **ZIG** compiler for this recently (https://ziglang.org/learn/getting-started/), as it's
small and incredibly easy to get running.

For example, you can build a *musl* version of **wavgen** for an ARM-HF target like this *(once you've downloaded
and unpacked the tiny zig archive somewhere and put it in your path):*

```
zig cc --target=arm-linux-musleabihf wavgen.c help.c log.c opts.c riff.c wf_*.c -o wavgen-armhf
```

* WINDOWS64 : zig cc --target=x86_64-windows-gnu wavgen.c help.c log.c opts.c riff.c wf_*.c -o wavgen.exe
* LINUX-X64 : zig cc --target=x86_64-linux-musl wavgen.c help.c log.c opts.c riff.c wf_*.c -o wavgen
* ARM-HF    : zig cc --target=arm-linux-musleabihf wavgen.c help.c log.c opts.c riff.c wf_*.c -o wavgen-armhf

etc.


## Executing The Program

Either run **wavgen** with a filename as the last parameter to write a WAV file to disk:
  ```
  wavgen -t <type> [options] /tmp/output-filename.wav
  ```

Or pipe the output to something that can play WAV files, for example:
  ```
  wavgen -t <type> [options] | aplay -D sysdefault:1 -t wav
  ```
  *Note here that you must tell aplay to expect a WAV-format file, but it should work out the rest itself*.


## Help

Continue reading to see what waveforms can be generated, or view the help page as a reminder:

```
./wavgen -h
 or
./wavgen --help
 or
./wavgen -t <type> --help
```


### Channel Markers

For some test types, **channel markers** may be added to the sample values. These identify each sample by
'stamping' either the most-significant (msb) or least-significant (lsb) byte of each sample with the one-based
channel number added to the hex value '0xC0' (so channel 1 will have the byte value **0xC1**).

For example, a four-channel silence waveform (all zeros) with `-m lsb` might have the first few samples looking
like this in the RIFF data section (the S32LE format is always interleaved and data is *little-endian*):

```
0xC1000000 0xC2010000 0xC3020000 0xC4030000 (the first sample for each channel)
0xC1040000 0xC2050000 0xC3060000 0xC4070000 (the second sample for each channel)
```

A **counter** type with `-m msb` might look like this (the counter starts at zero, incrementing each sample):
```
0x000000C1 0x000001C2 0x000002C3 0x000003C4 (the first sample for each channel)
0x000004C1 0x000005C2 0x000006C3 0x000007C4 (the second sample for each channel)
```

Markers are added using the `-m msb` or `-m lsb` options. Note that if markers are added into the MSB then the
resulting waveform will have a large (negative) D.C. offset applied (due to the '0xCx'), which may cause a large
'pop' at the beginning of playback. Prefer putting markers into the LSB if possible.


## Common command-line Options

The following options may be applied to *all* the possible waveform types.
<dl>
    <dt>--channels (-c)</dt>
    <dd>The number of audio channels.</dd>
    <dt>--samples (-s)</dt>
    <dd>The number of samples per-channel (aka <i>frames</i>). Mutually exclusive with <b>\-\-duration (-d)</b>.</dd>
    <dt>--duration (-d)</dt>
    <dd>The duration in seconds. Mutually exclusive with <b>\-\-samples (-s)</b>.</dd>
    <dt>--bitdepth (-b)</dt>
    <dd>The bit-depth (width) of the samples (16, 24 or 32-bit) [default 32-bit].</dd>
</dl>


## Setting Levels

Audio-type waveforms can have their output level set in three layers, which can each be used independently or
not at all. In the absence of any command-line options, all waveforms will have their peak level at 0dBFS.

> Non-audio waveforms such as the **counter** type are just data and cannot be adjusted for 'level'.

The three levels of adjustment heirachy are:

* Alignment `-a [level]` in dBFS.
* Level `-l [level]` in dB.
* Power-fraction `-w [fraction denominator]`, e.g. '8' for 1/8th power.

The **alignment** level is the main way of setting a target level. This 'aligns' the waveform peak to the level
asked for, in dBFS. Typically, you would always keep the alignment level set the same for each set of waveforms.

The **level** setting allows adjustment *relative to the alignment level*. For instance, if you set your
alignment level to **-20dBFS** (using `-a -20.0`) and additionally set the level to **-6dB** (using `-l -6.0`)
then the peak level of the waveform will be **-26dBFS**.
The **level** setting may be +ve, i.e. you can set the peak level *above* the normal alignment level.

The third adjustment setting is **power fraction**. This is useful for making test waveforms that equate to
power-fractions such as eighth-power (1/8 power) which is common when testing amplifier output power, etc.
The power fraction is applied on top of any alignment and level settings.

> You would not normally use **level** and **power fraction** together.

## Supported Waveforms

The following types of waveform (test signal) can be created using **wavgen**:

```
counter, steps, saw, silence, sine, square, pink, burst, white.
```

The context-sensitive help describes each option (e.g. use `./wavgen sine --help` to show sinewave options).


### Limitations

Most of the waveform generators are deliberately simplistic and do not seek to generate the *exact* frequency
that is asked for if it does not divide neatly into the sample rate. For most test scenarios the purity of the
signal will be more important than the actual frequency, so the generators avoid artifacts or sidebands that
might result from trying to match "awkward" frequencies.

That being said, the ultimately fidelity (accuracy) of the test signals is not really the aim of this utility.
If you require a very accurrate waveform it should be obvious how to add a new type (or improve and existing
one). Contributions are certainly welcome!

The "unified" format for generating waveforms is integer 32-bit samples because it makes the precise integer
types such as the counter and channel markers easier than using floats.

Assembly of the waveform data is *not* intended to be efficient! It deliberately uses a simple sample-by-sample
pipeline to allow layered options such as format conversion and markers. The pipeline assembles sample data like
this:

 * Generate the next sample in the sequence for the requested waveform (`--type=x`).
 * Adjust its level according to user-supplied options (e.g. `--level=x`).
 * Add markers to the sample if requested and allowed (e.g. `--markers=lsb`).
 * Reduce the sample-depth or convert to float if required (e.g. `--bitdepth=16` or `--bitdepth=0` for float).
 * Write the sample to the output `filename` or to stdout (e.g. when piping to `aplay`).
 * Continue until the requested time (`-d=t`) or quantity of samples (`-s=n`) is exhausted.

The "channel markers" are chosen to be easily visible in HEX views such as memory or register lists presented by
real-time debuggers or emulators. These are only permitted in waveforms that are not expected to be "quality
dependant", and only in some cases can markers be placed in the most-significant byte (MSB) of the output
samples, because this can make the waveform both very large in amplitude (loud!) and also entirely negative.


## Authors

Brian Marchant (brian@brianmarchant.com)


### Version History

* 1.0.0 : Initial Release.
* 0.0.x : Minor bugfixes (not listed here).
* 0.0.1 : First Commit (for testing).


### License

This project is licensed under the MIT License - see the LICENSE.md file for details. Do whatever you want with it!


### Acknowledgments and References

Excellent information on the Microsoft WAV/RIFF format:

* [WAVE PCM soundfile format (PCM only)](https://web.archive.org/web/20240921143601/https://soundfile.sapp.org/doc/WaveFormat/)
* [Some extra info not included above](https://www.videoproc.com/resource/wav-file.htm)
* [WAVE Specifications (inc. non-PCM float format)](https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html)

Noise-Generator Algorithms:

* [Psudo random number generator](https://www.firstpr.com.au/dsp/rand31/)
* [Pink noise generation](https://www.firstpr.com.au/dsp/pink-noise/)
