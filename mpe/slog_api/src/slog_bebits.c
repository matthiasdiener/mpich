#include <stdio.h>

#ifdef HAVE_SLOGCONF_H
#include "slog_config.h"
#endif
#if defined( STDC_HEADERS ) || defined( HAVE_STDLIB_H )
#include <stdlib.h>
#endif

#include "slog_bebits.h"                  /*I  "slog_bebits.h"  I*/


        /* SLOG_bebit(s)_xxxx methods */

int SLOG_bebit_valid( const SLOG_bebit_t bebit )
{
    return ( bebit == 0 || bebit == 1 );
}



int SLOG_bebits_valid( const SLOG_bebits_t bebits )
{
    /*  return( bebits >= 0 && bebits <= 3 );   */
    return( bebits <= 3 );
}



SLOG_bebit_t SLOG_bebit_conv( const SLOG_bebit_t bebit )
{
    return ( bebit > 0 ? 1 : 0 );
}



SLOG_bebits_t SLOG_bebits_encode( const SLOG_bebit_t bebit_0,
                                  const SLOG_bebit_t bebit_1 )
{
    /*
    return (   SLOG_bebit_conv( bebit_1 ) * 2
             + SLOG_bebit_conv( bebit_0 ) );
    */
    return (   ( SLOG_bebit_conv( bebit_1 ) << 1 )
             +   SLOG_bebit_conv( bebit_0 ) );
}



int SLOG_bebits_decode( const SLOG_bebits_t bebits, SLOG_bebit_t *bebit_ary )
{
    /*
    bebit_ary[ 0 ] = bebits % 2;
    bebit_ary[ 1 ] = bebits / 2;
    */

    bebit_ary[ 0 ] = bebits & 0x1;
    bebit_ary[ 1 ] = bebits >> 1;

    if ( SLOG_bebits_valid( bebits ) )
        return SLOG_SUCCESS;
    else {
        fprintf( errfile, __FILE__":SLOG_bebit_decode() - "
                          "Input bebitS, "fmt_bebits_t", is INVALID\n",
                          bebits );
        fflush( errfile );
        return SLOG_FAIL;
    }
}



int SLOG_bebits_IsBeginIntvl( const SLOG_bebit_t *bebit_ary )
{
    return ( bebit_ary[ 0 ] == 1 && bebit_ary[ 1 ] == 0 );
}



int SLOG_bebits_IsMiddleIntvl( const SLOG_bebit_t *bebit_ary )
{
    return ( bebit_ary[ 0 ] == 0 && bebit_ary[ 1 ] == 0 );
}



int SLOG_bebits_IsFinalIntvl( const SLOG_bebit_t *bebit_ary )
{
    return ( bebit_ary[ 0 ] == 0 && bebit_ary[ 1 ] == 1 );
}



int SLOG_bebits_IsWholeIntvl( const SLOG_bebit_t *bebit_ary )
{
    return ( bebit_ary[ 0 ] == 1 && bebit_ary[ 1 ] == 1 );
}



int SLOG_bebits_GetOrder( const SLOG_bebit_t *bebit_ary )
{
    if ( SLOG_bebits_IsBeginIntvl( bebit_ary ) )
        return 0;
    else if ( SLOG_bebits_IsMiddleIntvl( bebit_ary ) )
        return 1;
    else if ( SLOG_bebits_IsFinalIntvl( bebit_ary ) )
        return 2;
    else if ( SLOG_bebits_IsWholeIntvl( bebit_ary ) )
        return 3;
    else {
        fprintf( errfile, __FILE__":SLOG_bebits_GetOrder() - "
                          "Input bebitS, ("fmt_bebit_t","fmt_bebit_t"), "
                          "is INVALID\n", bebit_ary[0], bebit_ary[1] );
        fflush( errfile );
        return SLOG_FAIL;
    }
}
