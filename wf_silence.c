/*
** wf_silence.c
**
** Generate silence. Occasionally it's useful to have a totally silent source when
** tracking-down unexpected clicks and pops in an audio playback application.
*/
#include "wavgen.h"

/*
** Simply fill the file with silence.
*/
void generate_silence(struct FIXED_PARAMS *fixed)
{
    fixed->sample_value.i = 0;
}
