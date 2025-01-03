/*
** riff.h
**
** RIFF (WAV) file-format header file.
*/
#ifndef riff_h
#define riff_h

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define WAVE_FORMAT_PCM        1U
#define WAVE_FORMAT_IEEE_FLOAT 3U

/*
** Structures (the RIFF headers must be packed).
*/
#pragma pack(push)

/*
** Good WAV/RIFF documentation:
** https://web.archive.org/web/20240921143601/https://soundfile.sapp.org/doc/WaveFormat/
** https://www.videoproc.com/resource/wav-file.htm
** https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
*/

#pragma pack(1)
struct RIFF_HEADER {
    uint32_t ChunkID;       /* "RIFF" (0x52494646) */
    uint32_t ChunkSize;     /* 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size) */
    uint32_t FormatTag;     /* "WAVE" (0x57415645) */
};

#pragma pack(1)
struct RIFF_FMT_CHUNK {
    uint32_t ChunkID;       /* "fmt " (0x666d7420) */
    uint32_t ChunkSize;     /* 16 for PCM (18 or 40 for others) */
    uint16_t AudioFormat;   /* 1 for PCM, 3 for IEEE float */
    uint16_t NumChannels;   /* Mono = 1, Stereo = 2, etc. */
    uint32_t SampleRate;    /* e.g. 48000 */
    uint32_t ByteRate;      /* SampleRate * NumChannels * BitsPerSample/8 */
    uint16_t BlockAlign;    /* NumChannels * BitsPerSample/8 */
    uint16_t BitsPerSample; /* 8-bits = 8, 16-bits = 16, etc. */
};

#pragma pack(1)
struct RIFF_EXT_FMT_CHUNK {
    uint32_t ChunkID;       /* "fact" (0x66616374) */
    uint32_t ChunkSize;     /* Always 12 for us.   */
    uint32_t NumSamples;    /* The number of samples (per channel) */
};

#pragma pack(1)
struct RIFF_PEAK_CHUNK {
    uint32_t ChunkID;       /* "PEAK" (0x????????) */
    uint32_t ChunkSize;     /* The size of the following fields */
    uint32_t Version;
    uint32_t Timestamp;     /* UNIX timestamp (secs since 1/1/1970) */
    /*Peak data follows*/
};

#pragma pack(1)
struct RIFF_DATA_CHUNK {
    uint32_t ChunkID;       /* 0x64617461 : "data" */
    uint32_t ChunkSize;     /* NumSamples * NumChannels * BitsPerSample/8 */
    /*Sound data follows*/
};

#pragma pack(1)
struct RIFF_WAVE {
    struct RIFF_HEADER        hdr;
    struct RIFF_FMT_CHUNK     fmt;
    struct RIFF_EXT_FMT_CHUNK fact;
    struct RIFF_DATA_CHUNK    data;
};

#pragma pack(pop)

/*
** Function declarations.
*/

/* From riff.c */
void riff_init_header(struct  RIFF_HEADER *chunk, uint64_t num_data_bytes, bool is_float_format);
bool riff_write_header(struct RIFF_HEADER *chunk, FILE *file);

void riff_init_format(struct RIFF_FMT_CHUNK *chunk, uint32_t sample_rate, bool is_float,
                      uint16_t num_channels, uint16_t bytes_per_sample, uint16_t bits_per_sample);
bool riff_write_format(struct RIFF_FMT_CHUNK *chunk, FILE *file);

void riff_init_fact(struct  RIFF_EXT_FMT_CHUNK *chunk, uint64_t num_samples);
bool riff_write_fact(struct RIFF_EXT_FMT_CHUNK *chunk, FILE *file);

void riff_init_data_hdr(struct RIFF_DATA_CHUNK *chunk, uint64_t num_data_bytes);
bool riff_write_data_hdr(struct RIFF_DATA_CHUNK *chunk, FILE *file);

#endif
