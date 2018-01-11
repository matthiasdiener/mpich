#include <stdio.h>
#ifdef HAVE_SLOGCONF_H
#include "slog_config.h"
#endif
#include "slog_fileio.h"

int slog_fseek( FILE *stream, SLOG_fptr offset, int whence )
{
    return( fseek( stream, (long) offset, whence ) );
}

SLOG_fptr slog_ftell( FILE *stream )
{
    return( (SLOG_fptr) ftell( stream ) );
}
