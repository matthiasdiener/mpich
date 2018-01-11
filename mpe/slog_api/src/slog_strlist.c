#include <stdio.h>

#ifdef HAVE_SLOGCONF_H
#include "slog_config.h"
#endif
#ifdef HAVE_SLOG_WINCONFIG_H
#include "slog_winconfig.h"
#endif
#if defined( STDC_HEADERS ) || defined( HAVE_STDLIB_H )
#include <stdlib.h>
#endif
#if defined( STDC_HEADERS ) || defined( HAVE_STDARG_H )
#include <stdarg.h>
#endif
#if defined( STDC_HEADERS ) || defined( HAVE_STRING_H )
#include <string.h>
#endif

#include "str_util.h"
#include "bswp_fileio.h"
#include "slog_fileio.h"
#include "slog_strlist.h"

SLOG_strlist_t *SLOG_StrList_CreateFromVarArgs( const SLOG_uint32 Nstrs, ... )
{
    SLOG_strlist_t  *strlist;
    va_list          unnamed_arg;
    char            *in_str;
    int              in_strlen;
    int              ii;

    if ( Nstrs <= 0 )
        return NULL;

    strlist = ( SLOG_strlist_t * ) malloc( sizeof( SLOG_strlist_t ) );
    if ( strlist == NULL ) {
        fprintf( errfile, __FILE__":SLOG_StrList_CreateFromVarArgs() - "
                          "malloc() for SLOG_strlist_t fails\n" );
        fflush( errfile );
        return NULL;
    }

    strlist->Nstrs = Nstrs;

    strlist->strs  = ( char ** ) malloc( strlist->Nstrs * sizeof( char * ) );
    if ( strlist->Nstrs > 0 && strlist->strs == NULL ) {
        fprintf( errfile, __FILE__":SLOG_StrList_CreateFromVarArgs() - "
                          "malloc() for strlist->strs[] fails\n" );
        fflush( errfile );
        return NULL;
    }

    va_start( unnamed_arg, Nstrs );
    for ( ii = 0; ii < (int)strlist->Nstrs; ii++ ) {
        in_str = va_arg( unnamed_arg, char * );
        in_strlen = strlen( in_str ) + 1;
        strlist->strs[ ii ] = ( char * ) malloc( in_strlen * sizeof( char ) );
        if ( in_strlen > 0 && strlist->strs[ ii ] == NULL ) {
            fprintf( errfile, __FILE__":SLOG_StrList_CreateFromVarArgs() - "
                              "malloc() for strlist->strs[ %d ] fails for "
                              "string \"%s\"\n", ii, in_str );
            fflush( errfile );
            return NULL;
        }
        strcpy( strlist->strs[ ii ], in_str );
    }
    va_end( unnamed_arg );

    return strlist;
}



SLOG_strlist_t *SLOG_StrList_CreateFromArray( const SLOG_uint32   Nstrs, 
                                                    char        **strs )
{
    SLOG_strlist_t  *strlist;
    int              in_strlen;
    int              ii;

    if ( Nstrs <= 0 )
        return NULL;

    strlist = ( SLOG_strlist_t * ) malloc( sizeof( SLOG_strlist_t ) );
    if ( strlist == NULL ) {
        fprintf( errfile, __FILE__":SLOG_StrList_CreateFromArray() - "
                          "malloc() for SLOG_strlist_t fails\n" );
        fflush( errfile );
        return NULL;
    }

    strlist->Nstrs = Nstrs;

    strlist->strs  = ( char ** ) malloc( strlist->Nstrs * sizeof( char * ) );
    if ( strlist->Nstrs > 0 && strlist->strs == NULL ) {
        fprintf( errfile, __FILE__":SLOG_StrList_CreateFromArray() - "
                          "malloc() for strlist->strs[] fails\n" );
        fflush( errfile );
        return NULL;
    }

    for ( ii = 0; ii < (int)strlist->Nstrs; ii++ ) {
        in_strlen = strlen( strs[ ii ] ) + 1;
        strlist->strs[ ii ] = ( char * ) malloc( in_strlen * sizeof( char ) );
        if ( in_strlen > 0 && strlist->strs[ ii ] == NULL ) {
            fprintf( errfile, __FILE__":SLOG_StrList_CreateFromArray() - "
                              "malloc() for strlist->strs[ %d ] fails for "
                              "string \"%s\"\n", ii, strs[ ii ] );
            fflush( errfile );
            return NULL;
        }
        strcpy( strlist->strs[ ii ], strs[ ii ] );
    }

    return strlist;
}



SLOG_strlist_t *SLOG_StrList_Copy( const SLOG_strlist_t *src )
{
    SLOG_strlist_t  *dest;
    int              src_strlen;
    int              ii;

    if ( src == NULL )
        return NULL;

    if ( src->Nstrs <= 0 )
        return NULL;

    dest = ( SLOG_strlist_t * ) malloc( sizeof( SLOG_strlist_t ) );
    if ( dest == NULL ) {
        fprintf( errfile, __FILE__":SLOG_StrList_Copy() - "
                          "malloc() for SLOG_strlist_t fails\n" );
        fflush( errfile );
        return NULL;
    }

    dest->Nstrs = src->Nstrs;
    dest->strs  = ( char ** ) malloc( dest->Nstrs * sizeof( char * ) );
    if ( dest->Nstrs > 0 && dest->strs == NULL ) {
        fprintf( errfile, __FILE__":SLOG_StrList_Copy() - "
                          "malloc() for dest->strs[] fails\n" );
        fflush( errfile );
        return NULL;
    }

    for ( ii = 0; ii < (int)src->Nstrs; ii++ ) {
        src_strlen = strlen( src->strs[ ii ] ) + 1;
        dest->strs[ ii ] = ( char * ) malloc( src_strlen * sizeof( char ) );
        strcpy( dest->strs[ ii ], src->strs[ ii ] );
    }

    return dest;
}



SLOG_strlist_t *SLOG_StrList_Append(       SLOG_strlist_t *dest,
                                     const SLOG_strlist_t *src )
{
    int              src_strlen;
    int              ii;

    if ( src == NULL )
        return dest;
    
    if ( src->Nstrs <= 0 )
        return dest;

    if ( dest == NULL )
        return SLOG_StrList_Copy( src );

    if ( dest->Nstrs <= 0 )
        return SLOG_StrList_Copy( src );

    dest->strs = ( char ** ) realloc( dest->strs,
                                      ( dest->Nstrs + src->Nstrs )
                                    * sizeof( char * ) );
    if ( dest->strs == NULL ) {
        fprintf( errfile, __FILE__":SLOG_StrList_Append() - "
                          "realloc() for dest->strs[] fails\n" );
        fflush( errfile );
        return NULL;
    }

    for ( ii = 0; ii < (int)src->Nstrs; ii++ ) {
        src_strlen = strlen( src->strs[ ii ] ) + 1;
        dest->strs[ dest->Nstrs + ii ] = ( char * )
                                         malloc( src_strlen * sizeof( char ) );
        strcpy( dest->strs[ dest->Nstrs + ii ], src->strs[ ii ] );
    }
    dest->Nstrs += src->Nstrs;

    return dest;
}



void SLOG_StrList_Free( SLOG_strlist_t  *strlist )
{
    int  ii;

    if ( strlist != NULL ) {
        if ( strlist->Nstrs > 0 && strlist->strs == NULL ) {
            for ( ii = 0; ii < (int)strlist->Nstrs; ii++ )
                free( strlist->strs[ ii ] );
            free( strlist->strs );
        }
        free( strlist );
    }
}



int SLOG_StrList_Print( const SLOG_strlist_t *strlist, FILE *outfd )
{
    int ii;

    if ( strlist == NULL ) {
        fprintf( outfd, __FILE__":SLOG_StrList_Print() - "
                        "the input StrList pointer is NULL\n" );
        fflush( outfd );
        return SLOG_FAIL;
    }

    fprintf( outfd, "( " );
    for ( ii = 0; ii < (int)strlist->Nstrs; ii++ )
        fprintf( outfd, "\"%s\" ", strlist->strs[ ii ] );
    fprintf( outfd, ") " );

    fflush( outfd );

    return SLOG_SUCCESS;
}



int SLOG_StrList_WriteToFile( const SLOG_strlist_t *strlist, FILE *fd )
{
    int lgth;
    int ierr;
    int ii;

    ierr = bswp_fwrite( &( strlist->Nstrs ), SLOG_typesz[ ui32 ], 1, fd );
    if ( ierr < 1 ) return SLOG_FAIL;

    for ( ii = 0; ii < (int)strlist->Nstrs; ii++ ) {
        /*  The last "+ 1" should include the NULL terminating character  */
        lgth = strlen( strlist->strs[ ii ] ) + 1;

        ierr = bswp_fwrite( &lgth, SLOG_typesz[ ui32 ], 1, fd );
        if ( ierr < 1 ) return SLOG_FAIL;
        
        ierr = bswp_fwrite( strlist->strs[ ii ], sizeof( char ), lgth, fd );
        if ( ierr < lgth ) return SLOG_FAIL;
    }

    return SLOG_SUCCESS;
}



SLOG_strlist_t *SLOG_StrList_ReadFromFile( FILE *fd )
{
    SLOG_strlist_t  *strlist;
    int              ierr;
    int              lgth;
    int              ii;

    strlist = ( SLOG_strlist_t * ) malloc( sizeof( SLOG_strlist_t ) );
    if ( strlist == NULL ) {
        fprintf( errfile, __FILE__":SLOG_StrList_ReadFromFile() - "
                          "malloc() for SLOG_strlist_t fails\n" );
        fflush( errfile );
        return NULL;
    }

    ierr = bswp_fread( &( strlist->Nstrs ), SLOG_typesz[ ui32 ], 1, fd );
    if ( ierr < 1 ) return NULL;

    strlist->strs  = ( char ** ) malloc( strlist->Nstrs * sizeof( char * ) );
    if ( strlist->Nstrs > 0 && strlist->strs == NULL ) {
        fprintf( errfile, __FILE__":SLOG_StrList_ReadFromFile() - "
                          "malloc() for strlist->strs[] fails\n" );
        fflush( errfile );
        return NULL;
    }

    for ( ii = 0; ii < (int)strlist->Nstrs; ii++ ) {
        ierr = bswp_fread( &lgth, SLOG_typesz[ ui32 ], 1, fd );
        if ( ierr < 1 ) return NULL;

        strlist->strs[ ii ] = ( char * ) malloc( lgth * sizeof( char ) );
        if ( lgth > 0 && strlist->strs[ ii ] == NULL ) {
            fprintf( errfile, __FILE__":SLOG_StrList_ReadFromFile() - "
                              "malloc() for strlist->strs[ %d ] fails\n", ii );
            fflush( errfile );
            return NULL;
        }
        
        ierr = bswp_fread( strlist->strs[ ii ], sizeof( char ), lgth, fd );
        if ( ierr < lgth ) return NULL;
    }

    return strlist;
}



int SLOG_StrList_WriteToFileAsStrs( const SLOG_strlist_t *strlist, FILE *fd )
{
    const char          delimiter = '\t';
          char          Nstrs_str[ SLOG_STRING_LEN ]  = SLOG_STRING_INIT;
          char          strs_str[ SLOG_STRING_LEN ]   = SLOG_STRING_INIT;
          int           lgth;
          int           ierr;
          int           ii;

    lgth = SLOG_STRING_LEN - 2;

    sprintf( Nstrs_str, fmt_ui32, strlist->Nstrs );
    if ( Nstrs_str[ lgth+1 ] != ' ' ) {
        fprintf( errfile, __FILE__":SLOG_StrList_WriteToFileAsStrs() - \n"
                          "\t""Nstrs of the input SLOG_strlist_t is "
                          "too large to fit into %d characters, Nstrs "
                          "= "fmt_ui32"\n", lgth, strlist->Nstrs );
        fflush( errfile );
        return SLOG_FAIL;
    }
    Nstrs_str[ lgth+1 ] = delimiter;
    ierr = bswp_fwrite( Nstrs_str,
                        sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FAIL;

    for ( ii = 0; ii < (int)strlist->Nstrs; ii++ ) {
        if ( (int)strlen( strlist->strs[ ii ] ) <= lgth ) {
            sprintf( strs_str, "%s", strlist->strs[ ii ] );
            strs_str[ lgth+1 ] = delimiter;
            ierr = bswp_fwrite( strs_str,
                                sizeof( char ), SLOG_STRING_LEN, fd );
            if ( ierr < SLOG_STRING_LEN ) return SLOG_FAIL;
        }
        else {
            fprintf( errfile, __FILE__":SLOG_StrList_WriteToFileAsStrs() - \n"
                              "\t""strs[%d] of the input SLOG_strlist_t is "
                              "too large to fit into %d characters.\n"
                              "\t""strs[%d] = %s\n",
                              ii, lgth, ii, strlist->strs[ii] );
            fflush( errfile );
            return SLOG_FAIL;
        }
    }

    return SLOG_SUCCESS;
}



SLOG_strlist_t *SLOG_StrList_ReadFromFileAsStrs( FILE *fd )
{
          SLOG_strlist_t  *strlist;
          char             Nstrs_str[ SLOG_STRING_LEN ]  = SLOG_STRING_INIT;
          char             strs_str[ SLOG_STRING_LEN ]   = SLOG_STRING_INIT;
          int              ierr;
          int              lgth;
          int              ii;

    strlist = ( SLOG_strlist_t * ) malloc( sizeof( SLOG_strlist_t ) );
    if ( strlist == NULL ) {
        fprintf( errfile, __FILE__":SLOG_StrList_ReadFromFileAsStrs() - "
                          "malloc() for SLOG_strlist_t fails\n" );
        fflush( errfile );
        return NULL;
    }

    ierr = bswp_fread( Nstrs_str,
                       sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return NULL;
    strlist->Nstrs = atoi( Nstrs_str );

    strlist->strs  = ( char ** ) malloc( strlist->Nstrs * sizeof( char * ) );
    if ( strlist->Nstrs > 0 && strlist->strs == NULL ) {
        fprintf( errfile, __FILE__":SLOG_StrList_ReadFromFileAsStrs() - "
                          "malloc() for strlist->strs[] fails\n" );
        fflush( errfile );
        return NULL;
    }

    for ( ii = 0; ii < (int)strlist->Nstrs; ii++ ) {
        ierr = bswp_fread( strs_str,
                           sizeof( char ), SLOG_STRING_LEN, fd );
        if ( ierr < SLOG_STRING_LEN ) return NULL;
        lgth = strlen( strs_str ) + 1;

        strlist->strs[ ii ] = ( char * ) malloc( lgth * sizeof( char ) );
        if ( lgth > 0 && strlist->strs[ ii ] == NULL ) {
            fprintf( errfile, __FILE__":SLOG_StrList_ReadFromFileAsStrs() - "
                              "malloc() for strlist->strs[ %d ] fails\n", ii );
            fflush( errfile );
            return NULL;
        }

        strcpy( strlist->strs[ ii ], strs_str );
    }

    return strlist;
}



int SLOG_StrList_IsEqualTo( const SLOG_strlist_t *strlist1,
                            const SLOG_strlist_t *strlist2 )
{
    int ii;

    if ( strlist1 != strlist2 ) {
       if ( strlist1->Nstrs != strlist2->Nstrs )
           return SLOG_FALSE;

       if ( strlist1->strs != strlist2->strs ) {
           for ( ii = 0; ii < (int)strlist1->Nstrs; ii++ )
               if ( strcmp( strlist1->strs[ ii ], strlist2->strs[ ii ] ) != 0 )
                   return SLOG_FALSE;
       }
    }

    return SLOG_TRUE;
}
