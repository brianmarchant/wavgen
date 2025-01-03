/*
** log.c
**
** Simple printf wrapper so that verbose output ("logging") to the console can be
** enabled by the --verbose option, but suppressed completely when piping output
** to another application.
*/
#include <stdarg.h>
#include "wavgen.h"

/*
** Standard log (printf) output, sent to the console if file output is chosen.
*/
__attribute__((__format__ (__printf__, 2, 0)))
void log_info(struct FIXED_PARAMS *fixed, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    if (!fixed->piping) {
        vprintf(format, args);
    }

    va_end(args);
}

/*
** Extra log output, only sent to the console if file output is chosen AND "verbose" is selected.
*/
__attribute__((__format__ (__printf__, 2, 0)))
void log_extra(struct FIXED_PARAMS *fixed, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    if (fixed->verbose & !fixed->piping) {
        vprintf(format, args);
    }

    va_end(args);
}
