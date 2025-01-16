/*
** riff.c
**
** Functions to configure and write RIFF (WAV) format chunks.
*/
#include <string.h> // memset
#include "riff.h"

/*
** Initialise the main RIFF header.
**
** param  chunk           : A pointer to the struct to initialise.
** param  num_data_bytes  : The number of audio data BYTES to be appended.
** param  is_float_format : True if an extended format ("FACT") chuck is included (req'd for float format).
*/
void riff_init_header(struct RIFF_HEADER *chunk, uint64_t num_data_bytes, bool is_float_format)
{
    memset(chunk, 0, sizeof(struct RIFF_HEADER));

    /*
    ** The chunk identifiers are deliberately in BIG-ENDIAN format so that they are human-readable
    ** when viewed in a hex editor. They are therefore byte-swapped here to make it obvious.
    ** All other data is LITTLE-ENDIAN.
    */
    chunk->ChunkID    = __builtin_bswap32(0x52494646); // "RIFF" - BIG ENDIAN.
    chunk->FormatTag  = __builtin_bswap32(0x57415645); // "WAVE" - BIG ENDIAN.

    /*
    ** PCM (non-float) formats are simplest and do not use the extended format chunk.
    ** Floating-point formats must include the extra chunk that describes the float format.
    */
    if (!is_float_format) {
        chunk->ChunkSize = (uint32_t) (
                           sizeof(chunk->FormatTag)       // ChunkSize doesn't include ChunkID/ChunkSize fields.
                         + sizeof(struct RIFF_FMT_CHUNK)  // Include the size of the format chunk.
                         + sizeof(struct RIFF_DATA_CHUNK) // Include the size of the data chunk.
                         + num_data_bytes);                // Include the total bytes used for sample data.
    }
    else {
        chunk->ChunkSize = (uint32_t) (
                           sizeof(chunk->FormatTag)
                         + sizeof(struct RIFF_FMT_CHUNK)
                         + sizeof(struct RIFF_EXT_FMT_CHUNK) // Floating-point files must include this too.
                         + sizeof(struct RIFF_DATA_CHUNK)
                         + num_data_bytes);
    }
}

/*
** Write a RIFF header out to file (possibly stdout).
** Returns true if the data was written successfully.
*/
bool riff_write_header(struct RIFF_HEADER *chunk, FILE *file)
{
    if (fwrite(chunk, 1, sizeof(struct RIFF_HEADER), file) != sizeof(struct RIFF_HEADER)) {
        return false;
    }

    return true;
}

/*
** Initialise the RIFF format chunk (SubChunk#1).
**
** param  chunk       : A pointer to the struct to initialise.
** param  sample_rate : The sample rate in samples/second.
** param  user_params : The user-defined options parsed from the command-line.
*/
void riff_init_format(struct RIFF_FMT_CHUNK *chunk, uint32_t sample_rate, bool is_float,
                      uint16_t num_channels, uint16_t bytes_per_sample, uint16_t bits_per_sample)
{
    chunk->ChunkID       = __builtin_bswap32(0x666d7420);   // "fmt " - BIG ENDIAN
    chunk->ChunkSize     = 16U;                             // Always 16 for PCM format.
    chunk->AudioFormat   = is_float ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
    chunk->NumChannels   = num_channels;
    chunk->SampleRate    = sample_rate;
    chunk->ByteRate      = sample_rate * num_channels * bytes_per_sample;
    chunk->BlockAlign    = num_channels * bytes_per_sample;
    chunk->BitsPerSample = bits_per_sample;
}

/*
** Write a RIFF format chunk out to file (possibly stdout).
** Returns true if the data was written successfully.
*/
bool riff_write_format(struct RIFF_FMT_CHUNK *chunk, FILE *file)
{
    if (fwrite(chunk, 1, sizeof(struct RIFF_FMT_CHUNK), file) != sizeof(struct RIFF_FMT_CHUNK)) {
        return false;
    }

    return true;
}

/*
** Initialise the RIFF extended-format ("FACT") chunk (SubChunk#2).
**
** param  chunk : A pointer to the struct to initialise.
*/
void riff_init_fact(struct RIFF_EXT_FMT_CHUNK *chunk, uint32_t num_samples)
{
    chunk->ChunkID    = __builtin_bswap32(0x66616374); // "fact" - BIG ENDIAN.
    chunk->ChunkSize  = 4U;                            // Just the NumSamples field.
    chunk->NumSamples = num_samples;                   // The number of samples (per channel).
}

/*
** Write a RIFF extended format chunk out to file (possibly stdout).
** Returns true if the data was written successfully.
*/
bool riff_write_fact(struct RIFF_EXT_FMT_CHUNK *chunk, FILE *file)
{
    if (fwrite(chunk, 1, sizeof(struct RIFF_EXT_FMT_CHUNK), file) != sizeof(struct RIFF_EXT_FMT_CHUNK)) {
        return false;
    }

    return true;
}

/*
** Initialise the RIFF format data chunk (SubChunk#2 or SubChunk#3).
**
** param  chunk          : A pointer to the struct to initialise.
** param  num_data_bytes : The number of audio data BYTES to be appended.
*/
void riff_init_data_hdr(struct RIFF_DATA_CHUNK *chunk, uint32_t num_data_bytes)
{
    chunk->ChunkID   = __builtin_bswap32(0x64617461); // "data" - BIG ENDIAN.
    chunk->ChunkSize = num_data_bytes;
}

/*
** Write a RIFF data chunk header (NOT including the data itself) out to file (possibly stdout).
** Returns true if the data was written successfully.
*/
bool riff_write_data_hdr(struct RIFF_DATA_CHUNK *chunk, FILE *file)
{
    if (fwrite(chunk, 1, sizeof(struct RIFF_DATA_CHUNK), file) != sizeof(struct RIFF_DATA_CHUNK)) {
        return false;
    }

    return true;
}
