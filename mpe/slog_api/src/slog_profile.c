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
#if defined( HAVE_UNISTD_H )
#include <unistd.h>
#endif
#if defined( STDC_HEADERS ) || defined( HAVE_STDARG_H )
#include <stdarg.h>
#endif
#if defined( STDC_HEADERS ) || defined( HAVE_STRING_H )
#include <string.h>
#endif

#ifndef __GNUC__
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
 #pragma alloca
#  else
#   ifdef HAVE_ALLOCA
#     ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#     endif
#   endif
#  endif
# endif
# else
/* alloca.h is not loaded by default by gcc if the -ansi (or any of a
   number of other C dialects are selected) */
# ifdef HAVE_ALLOCA_H
#  include <alloca.h>
# endif
#endif

#include "str_util.h"
#include "bswp_fileio.h"
#include "slog_fileio.h"
#include "slog_strlist.h"
#include "slog_header.h"
#include "slog_bebits.h"
#include "slog_profile.h"              /*I "slog_profile.h" I*/



int SLOG_PROF_IsDuplicated( const SLOG_prof_t       *profile,
                            const SLOG_intvlinfo_t  *in_info )
{
    SLOG_intvlinfo_t  *cur_info;
    int                ii;
    int                NotMatched = 1;

    for ( ii = 0; NotMatched && (ii < (int)profile->Nentries); ii++ ) {
        cur_info = &( profile->entries[ ii ] );
        NotMatched = ! SLOG_IntvlInfo_IsKeyEqualTo( cur_info, in_info );
    }

    return ! NotMatched;
}


/*@C
    SLOG_PROF_Open - Creates the SLOG Display Profile, 
                     SLOG_prof_t, needed to write the to
                     the disk.

  Modified Input Variables :
. slog - pointer to the SLOG_STREAM where SLOG Display Profile is
         located.

  Modified Output Variables :
. returned value - integer return status.

  Include file needed : 
  slog_profile.h

.N SLOG_RETURN_STATUS
@*/
int SLOG_PROF_Open( SLOG_STREAM  *slog )
{
    slog->prof = ( SLOG_prof_t * ) malloc( sizeof( SLOG_prof_t ) );
    if ( slog->prof == NULL ) {
        fprintf( errfile, __FILE__":SLOG_PROF_Open() - "
                          "malloc() fails, Cannot allocate memory for "
                          "SLOG_prof_t\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }

    slog->prof->increment  = 10;
    slog->prof->capacity   = slog->prof->increment;
    slog->prof->Nentries   = 0;
    slog->prof->entries    = ( SLOG_intvlinfo_t * )
                               malloc(   slog->prof->capacity
                                       * sizeof( SLOG_intvlinfo_t ) );
    slog->prof->file_loc   = SLOG_fptr_NULL;

    return SLOG_SUCCESS;
}



/*@C
    SLOG_PROF_AddIntvlInfo - Add one display description of an 
                             interval to the SLOG Display Profile table.

  Modified Input Variables :
. slog - pointer to the SLOG_STREAM where SLOG Display Profile Table is
         located.

  Unmodified Input Variables :
. intvltype - index to the interval type.
. bebit_0   - the 1st bebit.
. bebit_1   - the 2nd bebit.
. classtype - character string for the classtype of the interval
. label     - character string for the label of the interval
. color     - character string for the color of the interval
. Nargs     - Number of MPI call argument for this interval record type.
. ...       - optional set of string argument description labels whose total 
              number should match Nargs supplied, otherwise, unpredicted 
              behaviour will occur.

  Modified Output Variables :
. returned value - integer return status.

  Usage Notes on this subroutine :
     all the input characters strings, classtype, label, color and the
     optional string argument description label are NOT allowed to be
     longer than SLOG_STRING_LEN - 1, including trailing null character.
     ( SLOG_STRING_LEN is defined in slog.h ).  They are also allowed 
     to contain alphanumeric, underscore, blank and tab characters.
     But tab characters, if exist, in the string are all converted to
     blanks.

.N SLOG_EXTRA_INTERVAL_INFOS

  Include file needed : 
  slog_profile.h

.N SLOG_RETURN_STATUS
@*/
int SLOG_PROF_AddIntvlInfo(       SLOG_STREAM       *slog,
                            const SLOG_intvltype_t   intvltype,
                            const SLOG_bebit_t       bebit_0,
                            const SLOG_bebit_t       bebit_1,
                            const char *             classtype,
                            const char *             label,
                            const char *             color,
                            const SLOG_N_args_t      Nargs,
                                  ... )
{
    SLOG_prof_t     *profile;
    SLOG_uint32      idx;
    va_list          unnamed_arg;
    char           **arg_strs;
    char            *in_str;
    int              in_strlen;
    int              ii;

    profile = slog->prof;

#if defined( DEBUG )
    if ( profile == NULL ) {
        fprintf( errfile, __FILE__":SLOG_PROF_AddIntvlInfo() - "
                          "the SLOG_prof_t pointer in slog is NULL\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
#endif

    /*  Check if there is any space left for a new record definition */
    if ( profile->Nentries + 1 > profile->capacity ) {
        profile->capacity += profile->increment;
        profile->entries   = ( SLOG_intvlinfo_t * )
                             realloc( profile->entries,
                                      profile->capacity
                                    * sizeof( SLOG_intvlinfo_t )
                                    );
        if ( profile->entries == NULL ) {
            fprintf( errfile, __FILE__":SLOG_PROF_AddIntvlInfo() -\n"
                              "\t""realloc() fails, cannot increase the size "
                              "of profile->entries[]\n" );
            fprintf( errfile, "\t""The input Interval Info = [ "
                              fmt_itype_t", ("fmt_bebit_t", "fmt_bebit_t"), "
                              "%s, %s, %s, "fmt_Nargs_t" ] \n",
                              intvltype, bebit_0, bebit_1,
                              classtype, label, color, Nargs );
            fflush( errfile );
            return SLOG_FAIL;
        }
    }

    
    idx = profile->Nentries;
    SLOG_IntvlInfo_Assign( &( profile->entries[ idx ] ),
                           intvltype, bebit_0, bebit_1, 
                           classtype, label, color );
    SLOG_IntvlInfo_SetUsed( &( profile->entries[ idx ] ), SLOG_FALSE );

    arg_strs = NULL;
    if ( Nargs > 0 ) {
#if ! defined( HAVE_ALLOCA )
        arg_strs  = ( char ** ) malloc( Nargs * sizeof( char * ) );
#else
        arg_strs  = ( char ** ) alloca( Nargs * sizeof( char * ) );
#endif
        if ( Nargs > 0 && arg_strs == NULL ) {
            fprintf( errfile, __FILE__":SLOG_PROF_AddIntvlInfo() - "
                              "malloc() for arg_strs[] fails\n" );
            fflush( errfile );
            return SLOG_FAIL;
        }

        va_start( unnamed_arg, Nargs );
        for ( ii = 0; ii < Nargs; ii++ ) {
            in_str = va_arg( unnamed_arg, char * );
            in_strlen = strlen( in_str ) + 1;
#if ! defined( HAVE_ALLOCA )
            arg_strs[ ii ] = ( char * ) malloc( in_strlen * sizeof( char ) );
#else
            arg_strs[ ii ] = ( char * ) alloca( in_strlen * sizeof( char ) );
#endif
            if ( in_strlen > 0 && arg_strs[ ii ] == NULL ) {
                 fprintf( errfile, __FILE__
                                   ":SLOG_PROF_AddIntvlInfo() -\n"
                                   "\t""malloc() for arg_strs[ %d ] fails "
                                   "for string \"%s\"\n", ii, in_str );
                 fflush( errfile );
                 return SLOG_FAIL;
            }
            strcpy( arg_strs[ ii ], in_str );
        }
        va_end( unnamed_arg );
    }

    ( profile->entries[ idx ] ).arg_labels
    = SLOG_StrList_CreateFromArray( Nargs, arg_strs );

#if ! defined( HAVE_ALLOCA )
    if ( arg_strs != NULL ) free( arg_strs );
#endif

    if ( SLOG_PROF_IsDuplicated( profile, &(profile->entries[ idx ]) ) ) {
        fprintf( errfile, __FILE__":SLOG_PROF_AddIntvlInfo() - \n"
                          "\t""there is already a copy of input interval\n"
                          "\t""information with identical keys "
                          "in the SLOG Display Profile table\n" );
        fprintf( errfile, "\t""The input Interval Info = [ "
                          fmt_itype_t", ("fmt_bebit_t", "fmt_bebit_t"), "
                          "%s, %s, %s, "fmt_Nargs_t" ] \n",
                          intvltype, bebit_0, bebit_1,
                          classtype, label, color, Nargs );
        fflush( errfile );
        return SLOG_FAIL;
    }
    profile->Nentries += 1;

    return SLOG_SUCCESS;
}


/*@C
    SLOG_PROF_AddIntvlInfoWithArray - Add one display description of an 
                                      interval to the SLOG Display 
                                      Profile table.

  Modified Input Variables :
. slog - pointer to the SLOG_STREAM where SLOG Display Profile Table is
         located.

  Unmodified Input Variables :
. intvltype - index to the interval type.
. bebit_0   - the 1st bebit.
. bebit_1   - the 2nd bebit.
. classtype - character string for the classtype of the interval
. label     - character string for the label of the interval
. color     - character string for the color of the interval
. Nargs     - Number of MPI call argument for this interval record type.
. arg_strs  - pointer to an array of strings whose total number should 
              match Nargs supplied, otherwise, unpredicted behaviour 
              will occur.   If Nargs is 0, arg_strs will be ignored, so
              NULL pointer can be used for this argument.

  Modified Output Variables :
. returned value - integer return status.

  Usage Notes on this subroutine :
     all the input characters strings, classtype, label, color and the
     optional string argument description label are NOT allowed to be
     longer than SLOG_STRING_LEN - 1, including trailing null character.
     ( SLOG_STRING_LEN is defined in slog.h ).  They are also allowed 
     to contain alphanumeric, underscore, blank and tab characters.
     But tab characters, if exist, in the string are all converted to
     blanks.

.N SLOG_EXTRA_INTERVAL_INFOS

  Include file needed : 
  slog_profile.h

.N SLOG_RETURN_STATUS
@*/
int SLOG_PROF_AddIntvlInfoWithArray(       SLOG_STREAM       *slog,
                                     const SLOG_intvltype_t   intvltype,
                                     const SLOG_bebit_t       bebit_0,
                                     const SLOG_bebit_t       bebit_1,
                                     const char *             classtype,
                                     const char *             label,
                                     const char *             color,
                                     const SLOG_N_args_t      Nargs,
                                           char             **arg_strs )
{
    SLOG_prof_t     *profile;
    SLOG_uint32      idx;

    profile = slog->prof;

#if defined( DEBUG )
    if ( profile == NULL ) {
        fprintf( errfile, __FILE__":SLOG_PROF_AddIntvlInfoWithArray() - "
                          "the SLOG_prof_t pointer in slog is NULL\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
#endif

    /*  Check if there is any space left for a new record definition */
    if ( profile->Nentries + 1 > profile->capacity ) {
        profile->capacity += profile->increment;
        profile->entries   = ( SLOG_intvlinfo_t * )
                             realloc( profile->entries,
                                      profile->capacity
                                    * sizeof( SLOG_intvlinfo_t )
                                    );
        if ( profile->entries == NULL ) {
            fprintf( errfile, __FILE__":SLOG_PROF_AddIntvlInfoWithArray() - "
                              "\t""realloc() fails, cannot increase the size "
                              "of profile->entries[]\n" );
            fprintf( errfile, "\t""The input Interval Info = [ "
                              fmt_itype_t", ("fmt_bebit_t", "fmt_bebit_t"), "
                              "%s, %s, %s, "fmt_Nargs_t" ] \n",
                              intvltype, bebit_0, bebit_1,
                              classtype, label, color, Nargs );
            fflush( errfile );
            return SLOG_FAIL;
        }
    }

    
    idx = profile->Nentries;
    SLOG_IntvlInfo_Assign( &( profile->entries[ idx ] ),
                           intvltype, bebit_0, bebit_1, 
                           classtype, label, color );
    SLOG_IntvlInfo_SetUsed( &( profile->entries[ idx ] ), SLOG_FALSE );

    if ( Nargs > 0 ) {
        ( profile->entries[ idx ] ).arg_labels
        = SLOG_StrList_CreateFromArray( Nargs, arg_strs );
    }

    if ( SLOG_PROF_IsDuplicated( profile, &(profile->entries[ idx ]) ) ) {
        fprintf( errfile, __FILE__":SLOG_PROF_AddIntvlInfoWithArray() - \n"
                          "\t""there is already a copy of input interval\n"
                          "\t""information with identical keys "
                          "in the SLOG Display Profile table\n" );
        fprintf( errfile, "\t""The input Interval Info = [ "
                          fmt_itype_t", ("fmt_bebit_t", "fmt_bebit_t"), "
                          "%s, %s, %s, "fmt_Nargs_t" ] \n",
                          intvltype, bebit_0, bebit_1,
                          classtype, label, color, Nargs );
        fflush( errfile );
        return SLOG_FAIL;
    }
    profile->Nentries += 1;

    return SLOG_SUCCESS;
}



/*@C
    SLOG_PROF_Close - Write the SLOG_prof_t onto the disk

  Modified Input Variables :
. slog - pointer to the SLOG_STREAM where SLOG Display Profile Table is
         located.

  Modified Output Variables :
. returned value - integer status code.

  Usage Notes on this subroutine :

.N SLOG_EXTRA_INTERVAL_INFOS

  Include file needed : 
  slog_profile.h

.N SLOG_RETURN_STATUS
@*/
int SLOG_PROF_Close( SLOG_STREAM  *slog )
{
    SLOG_prof_t            *profile;
    int                     ierr;
    int                     ii;

    /*  Update the content of SLOG_hdr_t to both memory and disk  */
    slog->hdr->fptr2profile = slog_ftell( slog->fd );
    ierr = SLOG_WriteUpdatedHeader( slog );
    if ( ierr != SLOG_SUCCESS ) {
        fprintf( errfile,  __FILE__":SLOG_PROF_Close() - "
                                   "SLOG_WriteUpdatedHeader() fails\n" );
        fflush( errfile );
        return ierr;
    }

    /*  Write the content of SLOG Record Definition Table to the disk  */
    profile = slog->prof;
    
    ierr = bswp_fwrite( &( profile->Nentries ),
                        sizeof( SLOG_uint32 ), 1, slog->fd );
    if ( ierr < 1 ) {
        fprintf( errfile,  __FILE__":SLOG_PROF_Close() - \n"
                           "\t""fails at "
                           "writing the number of interval infos to "
                           "table section of logfile\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
    for ( ii = 0; ii < (int)profile->Nentries; ii++ ) {
        ierr = SLOG_IntvlInfo_WriteToFile( &( profile->entries[ ii ] ),
                                        slog->fd );
        if ( ierr != SLOG_TRUE ) {
            fprintf( errfile,  __FILE__":SLOG_PROF_Close() - \n"
                               "\t""SLOG_IntvlInfo_WriteToFile() fails at "
                               "%d-th addition of the interval infos to "
                               "table section of logfile\n", ii );
            fflush( errfile );
            return SLOG_FAIL;
        }
    }
  
    return SLOG_SUCCESS;
}



/*@C
    SLOG_PROF_SetExtraNumOfIntvlInfos - Write the starting file pointer 
                                        SLOG_prof_t to the slog's
                                        header and then set the maximum
                                        extra number of Interval 
                                        Description entries
                                        in Display Profile Table.

  Modified Input Variables :
. slog - pointer to the SLOG_STREAM where SLOG Display Profile Table is
         located.

  Unmodifued Input Variables :
. Nentries_extra - Number of extra interval description to be reserved
                   on the disk

  Modified Output Variables :
. returned value - integer status code.

  Usage Notes on this subroutine :

.N SLOG_EXTRA_RECDEFS

  Include file needed : 
  slog_profile.h

.N SLOG_RETURN_STATUS
@*/
int SLOG_PROF_SetExtraNumOfIntvlInfos(       SLOG_STREAM  *slog,
                                       const SLOG_uint32   Nentries_extra )
{
    /*
         fill in an unrealistic number for SLOG_uint16, 2^(16)-1, in
         intvltype of blank_info which is type SLOG_intvlinfo_t
    */
    SLOG_prof_t            *profile;
    SLOG_intvlinfo_t        blank_info = { 65535, " ", " ", " " };
    SLOG_uint32             old_profile_capacity;
    int                     ierr;
    int                     ii;

    /*  Update the content of SLOG_hdr_t to both memory and disk  */
    slog->hdr->fptr2profile = slog_ftell( slog->fd );
    ierr = SLOG_WriteUpdatedHeader( slog );
    if ( ierr != SLOG_SUCCESS ) {
        fprintf( errfile,  __FILE__":SLOG_PROF_SetExtraNumOfIntvlInfos() - "
                                   "SLOG_WriteUpdatedHeader() fails\n" );
        fflush( errfile );
        return ierr;
    }

    profile = slog->prof;
    
    /*  Determine the new storage  */
    old_profile_capacity = profile->capacity;
    profile->capacity    = profile->Nentries + Nentries_extra;
    profile->entries = ( SLOG_intvlinfo_t * )
                       realloc( profile->entries,
                                profile->capacity * sizeof( SLOG_intvlinfo_t )
                              );
    if ( profile->entries == NULL ) {
        fprintf( errfile, __FILE__":SLOG_PROF_SetExtraNumOfIntvlInfos() - \n"
                          "\t""realloc() fails, cannot increase the size "
                          "of profile->entries["fmt_ui32"]\n",
                          profile->capacity );
        fflush( errfile );
        profile->capacity = old_profile_capacity;
        return SLOG_FAIL;
    }

    /*  Write the content of SLOG Record Definition Table to the disk  */

    ierr = bswp_fwrite( &( profile->Nentries ),
                        sizeof( SLOG_uint32 ), 1, slog->fd );
    if ( ierr < 1 ) {
        fprintf( errfile,  __FILE__":SLOG_PROF_SetExtraNumOfIntvlInfos() - \n"
                           "\t""fails at "
                           "writing the number of interval descriptors to "
                           "table section of logfile\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
    for ( ii = 0; ii < (int)profile->Nentries; ii++ ) {
        ierr = SLOG_IntvlInfo_WriteToFile( &( profile->entries[ ii ] ), 
                                        slog->fd );
        if ( ierr != SLOG_TRUE ) {
            fprintf( errfile,  __FILE__
                               ":SLOG_PROF_SetExtraNumOfIntvlInfos() - \n"
                               "\t""SLOG_IntvlInfo_WriteToFile() fails at the "
                               "%d-th addition of the interval descriptors to "
                               "table section of logfile\n", ii );
            fflush( errfile ); 
            return SLOG_FAIL;
        }
    }

    /*
        Save the location where the extra interval descriptors
        can be inserted
    */

    profile->file_loc = slog_ftell( slog->fd ); 

    /*  Write some blank record definitions to fill up the reserved space.  */

    for ( ii = (int)profile->Nentries; ii < (int)profile->capacity; ii++ ) {
        ierr = SLOG_IntvlInfo_WriteToFile( &blank_info, slog->fd );
        if ( ierr != SLOG_TRUE ) {
            fprintf( errfile,  __FILE__
                               ":SLOG_PROF_SetExtraNumOfIntvlInfos() - \n"
                               "\t""SLOG_IntvlInfo_WriteToFile() fails at "
                               "%d-th addition of the blank descriptor to "
                               "table section of logfile\n", ii );
            fflush( errfile ); 
            return SLOG_FAIL;
        }
    }
  
    return SLOG_SUCCESS;
}



/*@C
    SLOG_PROF_AddExtraIntvlInfo - Add one display description of an
                                  interval to the reserved space of
                                  the SLOG Display Profile table.

  Modified Input Variables :
. slog - pointer to the SLOG_STREAM where SLOG Display Profile Table is
         located.

  Unmodified Input Variables :
. intvltype - index to the interval type.
. bebit_0   - the 1st bebit.
. bebit_1   - the 2nd bebit.
. classtype - character string for the classtype of the interval
. label     - character string for the label of the interval
. color     - character string for the color of the interval

  Modified Output Variables :
. returned value - integer return status.

  Usage Notes on this subroutine :
     all the input characters strings, classtype, label and color are
     allowed to contain alphanumeric, underscore, blank and tab characters.
     But tab characters, if exist, in the string are all converted to
     blanks for ease of management.

.N SLOG_EXTRA_INTERVAL_INFOS

  Include file needed : 
  slog_profile.h

.N SLOG_RETURN_STATUS
@*/
int SLOG_PROF_AddExtraIntvlInfo(       SLOG_STREAM      *slog,
                                 const SLOG_intvltype_t  intvltype,
                                 const SLOG_bebit_t      bebit_0,
                                 const SLOG_bebit_t      bebit_1,
                                 const char *            classtype,
                                 const char *            label,
                                 const char *            color )
{
    SLOG_prof_t            *profile;
    SLOG_fptr               file_loc_saved;
    SLOG_uint32             idx;
    int                     ierr;

    profile = slog->prof;

#if defined( DEBUG )
    if ( profile == NULL ) {
        fprintf( errfile, __FILE__":SLOG_PROF_AddExtraIntvlInfo() - "
                          "the SLOG_prof_t pointer in slog "
                          "is NULL\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
#endif

    if ( profile->Nentries + 1 > profile->capacity ) {
        fprintf( errfile, __FILE__":SLOG_PROF_AddExtraIntvlInfo() - \n"
                          "\t""All reserved space in the "
                          "Record Definition Table of the logfile "
                          "has been used up!\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }

    idx = profile->Nentries;
    SLOG_IntvlInfo_Assign( &( profile->entries[ idx ] ),
                           intvltype, bebit_0, bebit_1,
                           classtype, label, color );
    SLOG_IntvlInfo_SetUsed( &( profile->entries[ idx ] ), SLOG_FALSE );
    ( profile->entries[ idx ] ).arg_labels = NULL;
    if ( SLOG_PROF_IsDuplicated( profile, &(profile->entries[ idx ]) ) ) {
        fprintf( errfile, __FILE__":SLOG_PROF_AddExtraIntvlInfo() - "
                          "there is already a copy of input interval\n"
                          "\t""information with identical keys "
                          "in the SLOG record definition table\n" );
        fprintf( errfile, "\t""The input Interval Info = [ "
                          fmt_itype_t", ("fmt_bebit_t", "fmt_bebit_t"), "
                          "%s, %s, %s ] \n",
                          intvltype, bebit_0, bebit_1,
                          classtype, label, color );
        fflush( errfile );
        return SLOG_FAIL;
    }
    profile->Nentries += 1;

    /*  Save the current position in the logfile for later restoration  */
    file_loc_saved = slog_ftell( slog->fd );

    /*  Update the number of interval descriptors on the file/disk  */
    ierr = slog_fseek( slog->fd, slog->hdr->fptr2profile, SEEK_SET );
    if ( ierr != 0 ) {
        fprintf( errfile, __FILE__":SLOG_PROF_AddExtraIntvlInfo() - "
                          "slog_fseek( fptr2profile ) fails\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
    ierr = bswp_fwrite( &( profile->Nentries ),
                        sizeof( SLOG_uint32 ), 1, slog->fd );
    if ( ierr < 1 ) {
        fprintf( errfile,  __FILE__":SLOG_PROF_AddExtraIntvlInfo() - \n"
                           "\t""Update the number of entries in the record "
                           "definition table on the file fails\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }

    /*  Insert the new record definition to the reserved space  */
    ierr = slog_fseek( slog->fd, profile->file_loc, SEEK_SET );
    if ( ierr != 0 ) {
        fprintf( errfile, __FILE__":SLOG_PROF_AddExtraIntvlInfo() - "
                          "slog_fseek( profile->file_loc ) fails\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
    ierr = SLOG_IntvlInfo_WriteToFile( &( profile->entries[idx] ), slog->fd );
    if ( ierr != SLOG_TRUE ) {
        fprintf( errfile,  __FILE__":SLOG_PROF_AddExtraIntvlInfo() - \n"
                           "\t""SLOG_IntvlInfo_WriteToFile() fails at "
                           "%d-th addition of the interval decriptor to "
                           "table section of logfile\n", idx );
        fflush( errfile );
        return SLOG_FAIL;
    }
    fflush( slog->fd );

    /*  Update the file pointer to the reserved space of IntvlInfos in file */
    profile->file_loc = slog_ftell( slog->fd );

    /*  Restore the original file position in the logfile  */
    ierr = slog_fseek( slog->fd, file_loc_saved, SEEK_SET );

    return SLOG_SUCCESS;
}



SLOG_intvlinfo_t *SLOG_PROF_GetIntvlInfo( const SLOG_prof_t       *profile,
                                          const SLOG_intvltype_t   intvltype,
                                          const SLOG_bebit_t       bebit_0,
                                          const SLOG_bebit_t       bebit_1 )
{
    SLOG_intvlinfo_t  *cur_def;
    int                ii;

    for ( ii = 0; ii < (int)profile->Nentries; ii++ ) {
        cur_def = &( profile->entries[ ii ] );
        if (    ( cur_def->intvltype == intvltype )
             && ( cur_def->bebits[0] == bebit_0 )
             && ( cur_def->bebits[1] == bebit_1 ) ) {
             return cur_def;
        }
    }

    fprintf( errfile, __FILE__":SLOG_PROF_GetIntvlInfo() - \n"
                      "\t""Cannot find [ intvltype = "fmt_itype_t", ("
                      fmt_bebit_t", "fmt_bebit_t") ] "
                      "in the Display Profile Table\n",
                      intvltype, bebit_0, bebit_1 );
    fflush( errfile );
    return NULL;
}



static int SLOG_IntvlInfo_CompareKey( const SLOG_intvlinfo_t *info1,
                                      const SLOG_intvlinfo_t *info2 )
{
    int order1, order2;

    if ( info1->intvltype < info2->intvltype )
        return -1;
    else if ( info1->intvltype == info2->intvltype ) {
        order1 = SLOG_bebits_GetOrder( info1->bebits );
        order2 = SLOG_bebits_GetOrder( info2->bebits );
        if ( order1 < order2 )
            return -1;
        else if ( order1 == order2 )
            return 0;
        else
            return 1;
    }
    else
        return 1;
}



static int SLOG_IntvlInfo_CompareKeyFn( const void *info1, const void *info2 )
{
    return( SLOG_IntvlInfo_CompareKey( (SLOG_intvlinfo_t *) info1,
                                       (SLOG_intvlinfo_t *) info2 ) );
}



void SLOG_PROF_SortTable( SLOG_prof_t  *profile )
{
    qsort( profile->entries, profile->Nentries, sizeof( SLOG_intvlinfo_t ),
           &SLOG_IntvlInfo_CompareKeyFn );
}



int SLOG_PROF_IsBebitIntvlsConsistent( SLOG_prof_t *profile )
{
    SLOG_intvlinfo_t       *info;
    SLOG_intvlinfo_t        joined_info;
    SLOG_uint32             idx_begin, idx_end;
    SLOG_uint32             ii, jj;
    int                     DoPartIntvlsExist4itype;
    int                     Nerrors;

#if defined( DEBUG )
    if ( profile == NULL ) {
        fprintf( errfile, __FILE__":SLOG_PROF_IsBebitIntvlsConsistent() - \n"
                          "\t""the SLOG_prof_t pointer in slog "
                          "is NULL\n" );
        fflush( errfile );
        return SLOG_FALSE;
    }
#endif

    /*  Sort the table in increasing intvltype and bebits order  */
    SLOG_PROF_SortTable( profile );

    /*
        The following statements assume intvltype will be changed when
        bebits are recycled
    */
    DoPartIntvlsExist4itype = SLOG_FALSE;
    idx_begin               = 0;
    Nerrors                 = 0;
    for ( ii = 0; ii < profile->Nentries; ii++ ) {

        /*
        fprintf( outfile, "%d >  ", ii );
        SLOG_IntvlInfo_Print( &( profile->entries[ ii ] ), outfile );
        fprintf( outfile, "\n" );
        fflush( outfile );
        */
        
        info = &( profile->entries[ ii ] );
        if ( ! SLOG_bebits_IsWholeIntvl( info->bebits ) ) {
            if ( ! DoPartIntvlsExist4itype ) {
                /*  The 1st bebit interval  */
                idx_begin = ii;
                SLOG_IntvlInfo_Assign( &joined_info,
                                       info->intvltype, 1, 1,
                                       info->classtype, info->label,
                                       info->color );
                joined_info.arg_labels = SLOG_StrList_Copy( info->arg_labels );
            }
            else {
                /*  The non-1st bebit interval, but NOT whole interval  */
                idx_end = ii;
                if (    info->intvltype != joined_info.intvltype 
                     || strcmp( info->classtype, joined_info.classtype ) != 0  
                     || strcmp( info->label    , joined_info.label     ) != 0  
                     || strcmp( info->color    , joined_info.color     ) != 0 )
                {
                    fprintf( errfile, __FILE__
                                      ":SLOG_PROF_IsBebitIntvlsConsistent() -\n"
                                      "\t""Inconsistency detected for "
                                      "intvltype = "fmt_itype_t"\n",
                                      info->intvltype );
                    for ( jj = idx_begin; jj <= idx_end; jj++ ) {
                        fprintf( errfile, "%d, ", jj );
                        SLOG_IntvlInfo_Print( &( profile->entries[ jj ] ),
                                              errfile );
                        fprintf( errfile, "\n" );
                    }
                    fflush( errfile );
                    Nerrors ++;
                    /* return SLOG_FALSE; */
                }

                joined_info.arg_labels
                = SLOG_StrList_Append( joined_info.arg_labels,
                                       info->arg_labels );
            }
            DoPartIntvlsExist4itype  = SLOG_TRUE;
        }
        else {  /*  if ( SLOG_bebits_IsWholeIntvl( info->bebits ) )  */
            /*  The whole bebit interval  */
            idx_end = ii;
            if ( DoPartIntvlsExist4itype ) {
                if ( ! SLOG_IntvlInfo_IsEqualTo( &joined_info, info ) ) {
                    fprintf( errfile, __FILE__
                                      ":SLOG_PROF_IsBebitIntvlsConsistent() -\n"
                                      "\t""Inconsistency detected for "
                                      "intvltype = "fmt_itype_t"\n",
                                      info->intvltype );
                    for ( jj = idx_begin; jj <= idx_end; jj++ ) {
                        fprintf( errfile, "%d, ", jj );
                        SLOG_IntvlInfo_Print( &( profile->entries[ jj ] ),
                                              errfile );
                        fprintf( errfile, "\n" );
                    }
                    fflush( errfile );
                    Nerrors ++;
                    /* return SLOG_FALSE; */
                }
                SLOG_StrList_Free( joined_info.arg_labels );
                joined_info.arg_labels = NULL;
            }
            DoPartIntvlsExist4itype = SLOG_FALSE;
        }

        /*
        fprintf( outfile, "j >  ");
        SLOG_IntvlInfo_Print( &joined_info, outfile );
        fprintf( outfile, "\n" );
        fflush( outfile );
        */
        
    }

    if ( Nerrors > 0 )
        return SLOG_FALSE;
    else
        return SLOG_TRUE;
}

int SLOG_PROF_CompressTable( SLOG_prof_t *profile )
{
    SLOG_intvlinfo_t       *entries_used;
    SLOG_intvlinfo_t       *info;
    SLOG_intvltype_t        prev_used_itype;
    SLOG_uint32             Nentries_used;
    SLOG_uint32             idx, ii;
    int                     itmp;

#if defined( DEBUG )
    if ( profile == NULL ) {
        fprintf( errfile, __FILE__":SLOG_PROF_CompressTable() - "
                          "the SLOG_prof_t pointer in slog "
                          "is NULL\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
#endif

    /*  Sort the display profile in increasing intvltype and bebits order  */
    SLOG_PROF_SortTable( profile );

    /*  Set idx to first non scanned but used entry  */
    itmp = -1;
    for ( idx = 0; idx < profile->Nentries && itmp != -1; idx++ ) {
        info = &( profile->entries[ idx ] );
        if ( info->used )
            itmp = info->intvltype;
    }

    /* 
        If any of partial bebit intervals is set,
        set the whole interval as used
    */
    for ( prev_used_itype = itmp; idx < profile->Nentries; idx++ ) {
        info = &( profile->entries[ idx ] );
        if ( info->used )
            prev_used_itype = info->intvltype;
        else {
            if (    SLOG_bebits_IsWholeIntvl( info->bebits ) 
                 && info->intvltype == prev_used_itype ) {
                 SLOG_IntvlInfo_SetUsed( info, SLOG_TRUE );
            }
        }
    }

    /*  Compute the Number of __used__ entries in the display profile table  */
    Nentries_used = 0;
    for ( ii = 0; ii < profile->Nentries; ii++ )
        if ( ( profile->entries[ ii ] ).used )
            Nentries_used++;

    if ( Nentries_used == profile->Nentries )
        return SLOG_SUCCESS;

    /*  Allocate the temporary storage for the used definition  */
    entries_used = ( SLOG_intvlinfo_t * )
                   malloc( Nentries_used * sizeof( SLOG_intvlinfo_t ) );
    if ( Nentries_used > 0 && entries_used == NULL ) {
        fprintf( errfile, __FILE__":SLOG_PROF_CompressTable() - \n"
                          "\t""malloc() for temporary storage, "
                          "entries_used, fails for Nentries_used = "
                          fmt_ui32" ! \n", Nentries_used );
        fflush( errfile );
        return SLOG_FAIL;
    }

    /*  Copy the used display profile definitions to the temporary storage  */
    idx = 0;
    for ( ii = 0; ii < profile->Nentries; ii++ ) {
        info = &( profile->entries[ ii ] );
        if ( info->used ) {
            SLOG_IntvlInfo_Assign( &( entries_used[ idx ] ),
                                   info->intvltype, 
                                   info->bebits[0], info->bebits[1],
                                   info->classtype, info->label, info->color );
            /*  Pointer Copy  */
            entries_used[ idx ].arg_labels = info->arg_labels;
            idx++;
        }
        else {
            /*
                Since a pointer copy has been done, so SLOG_IntvlInfo's
                arg_labels needs to be freed when SLOG_IntvlInfo is NOT 
                used( to avoid inconsistent memory leak )
            */
            SLOG_StrList_Free( info->arg_labels );
            info->arg_labels = NULL;
        }
    }

    /*  Throw away the original display profile table  */
    if ( profile->Nentries > 0 && profile->entries != NULL ) {
        free( profile->entries );
        profile->entries    = NULL;
        profile->capacity   = 0;
        profile->Nentries   = 0;
    }

    /*  
        Shrink the original table in memory to store the used definitions 
        by pointing the display profile table to the temporary storage.
    */
    profile->capacity   = Nentries_used;
    profile->Nentries   = Nentries_used;
    profile->entries    = entries_used;

    return SLOG_SUCCESS;
}



int SLOG_PROF_CompressedTableToFile( SLOG_STREAM  *slog )
{
    SLOG_prof_t            *profile;
    SLOG_fptr               file_loc_saved;
    SLOG_uint32             ii;
    int                     ierr;

    profile = slog->prof;

#if defined( DEBUG )
    if ( profile == NULL ) {
        fprintf( errfile, __FILE__":SLOG_PROF_CompressedTableToFile() - "
                          "the SLOG_prof_t pointer in slog "
                          "is NULL\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
#endif

    /*  Save the current position in the logfile for later restoration  */
    file_loc_saved = slog_ftell( slog->fd );

    /*  Update the number of record definitions on the file/disk  */
    ierr = slog_fseek( slog->fd, slog->hdr->fptr2profile, SEEK_SET );
    if ( ierr != 0 ) {
        fprintf( errfile, __FILE__":SLOG_PROF_CompressedTableToFile() - \n"
                          "\t""slog_fseek( fptr2profile ) fails\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
    ierr = bswp_fwrite( &( profile->Nentries ),
                        sizeof( SLOG_uint32 ), 1, slog->fd );
    if ( ierr < 1 ) {
        fprintf( errfile, __FILE__":SLOG_PROF_CompressedTableToFile() - \n"
                          "\t""Update the number of entries in the profile "
                          "definition table on the file fails\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
    for ( ii = 0; ii < profile->Nentries; ii++ ) {
        ierr = SLOG_IntvlInfo_WriteToFile( &( profile->entries[ ii ] ),
                                           slog->fd );
        if ( ierr != SLOG_TRUE ) {
            fprintf( errfile,  __FILE__":SLOG_PROF_CompressedTableToFile() - \n"
                               "\t""SLOG_IntvlInfo_WriteToFile() fails at "
                               "%d-th addition of the record definition to "
                               "table section of logfile\n", ii );
            fflush( errfile );
            return SLOG_FAIL;
        }
    }

    /*  Probably unnecessary  */
    /*  Update the file pointer to the reserved space of RecDefs in file */
    profile->file_loc = slog_ftell( slog->fd );

    /*  Restore the original file position in the logfile  */
    ierr = slog_fseek( slog->fd, file_loc_saved, SEEK_SET );

    return SLOG_SUCCESS;
}



void SLOG_PROF_Free( SLOG_prof_t *profile )
{
    int ii;

    if ( profile != NULL ) {
        if ( profile->Nentries > 0 && profile->entries != NULL ) {
            for ( ii = 0; ii < (int)profile->Nentries; ii ++ )
                SLOG_StrList_Free( profile->entries[ ii ].arg_labels );
            free( profile->entries );
            profile->entries = NULL;
        }
        free( profile );
        profile = NULL;
    }
}



int SLOG_PROF_ReadIntvlInfos( SLOG_STREAM  *slog )
{
    SLOG_prof_t            *profile;
    int                     ii;
    int                     ierr;

    slog->prof = ( SLOG_prof_t * ) malloc( sizeof( SLOG_prof_t ) );
    if ( slog->prof == NULL ) {
        fprintf( errfile, __FILE__":SLOG_PROF_ReadIntvlInfos() - "
                          "malloc() fails, Cannot allocate memory for "
                          "SLOG_prof_t\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }

    /*  Initialize the Record Definition table data structure  */
    slog->prof->increment  = 10;
    slog->prof->capacity   = slog->prof->increment;
    slog->prof->Nentries   = 0;
    slog->prof->entries    = NULL;
    
    /*  Seek to the beginning of the Record Def'n Table in the SLOG file  */
    if ( slog->hdr->fptr2profile == SLOG_fptr_NULL ) {
        fprintf( errfile, __FILE__":SLOG_PROF_ReadIntvlInfos() - "
                          "slog->hdr->fptr2profile == SLOG_fptr_NULL\n" );
        fprintf( errfile, "\t""SLOG_PROF_Close() may NOT be called\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
    ierr = slog_fseek( slog->fd, slog->hdr->fptr2profile, SEEK_SET );
    if ( ierr != 0 ) {
        fprintf( errfile, __FILE__":SLOG_PROF_ReadIntvlInfos() - "
                          "slog_fseek( fptr2profile ) fails\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }

    /*  Read in the data structure  */
    profile = slog->prof;
    
    ierr = bswp_fread( &( profile->Nentries ),
                       sizeof( SLOG_uint32 ), 1, slog->fd );
    if ( ierr < 1 ) {
        fprintf( errfile, __FILE__":SLOG_PROF_ReadIntvlInfos() - \n"
                          "\t""Fails at reading the number of entries "
                          "of the display profile table\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
    profile->capacity = profile->Nentries;
    profile->entries  = ( SLOG_intvlinfo_t * )
                        malloc( profile->capacity
                              * sizeof( SLOG_intvlinfo_t ) );
    if ( profile->Nentries > 0 && profile->entries == NULL ) {
        fprintf( errfile, __FILE__":SLOG_PROF_ReadIntvlInfos() - "
                          "malloc() fails for profile->entries[]\n" );
        fflush( errfile );
        return SLOG_FAIL;
    }
    
    for ( ii = 0; ii < (int)profile->Nentries; ii++ ) {
        ierr = SLOG_IntvlInfo_ReadFromFile( &( profile->entries[ ii ] ), 
                                            slog->fd );
        if ( ierr != SLOG_TRUE ) {
            fprintf( errfile, __FILE__":SLOG_PROF_ReadIntvlInfos() - \n"
                              "\t""SLOG_IntvlInfo_ReadFromFile() fails at "
                              "%d-th reading of interval decriptor from "
                              "the table section of logfile\n", ii );
            fflush( errfile ); 
            return SLOG_FAIL;
        }
    }

#if defined( DEBUG )
    fprintf( outfile, __FILE__":SLOG_PROF_ReadIntvlInfos() - Read\n" );
    SLOG_PROF_Print( outfile, profile );
    fflush( outfile );
#endif
  
    return SLOG_SUCCESS;
}



void SLOG_PROF_Print( FILE* fd, const SLOG_prof_t *profile )
{
    int ii;

    for ( ii = 0; ii < (int)profile->Nentries; ii++ ) {
        fprintf( fd, " def[%i] = ", ii );
        SLOG_IntvlInfo_Print( &( profile->entries[ ii ] ), fd );
        fprintf( fd, "\n" );
    }
    fflush( fd );
}



/*  -- Component level supporting subroutines  -- */

        /* SLOG_IntvlInfo_xxxx methods */

void SLOG_IntvlInfo_Assign(       SLOG_intvlinfo_t        *intvlinfo,
                            const SLOG_intvltype_t         in_intvltype,
                            const SLOG_bebit_t             in_bebit_0,
                            const SLOG_bebit_t             in_bebit_1,
                            const char *                   in_classtype,
                            const char *                   in_label,
                            const char *                   in_color )
{
    intvlinfo->intvltype = in_intvltype;
    intvlinfo->bebits[0] = in_bebit_0;
    intvlinfo->bebits[1] = in_bebit_1;
    SLOG_str_ncopy_set( intvlinfo->classtype, in_classtype, SLOG_STRING_LEN );
    SLOG_str_ncopy_set( intvlinfo->label,     in_label,     SLOG_STRING_LEN );
    SLOG_str_ncopy_set( intvlinfo->color,     in_color,     SLOG_STRING_LEN );
}



void SLOG_IntvlInfo_Print( const SLOG_intvlinfo_t *info, FILE *fd )
{
    fprintf( fd, "[ "fmt_itype_t", ("fmt_bebit_t", "fmt_bebit_t"), %s, %s, %s",
                 info->intvltype, info->bebits[0], info->bebits[1],
                 info->classtype, info->label, info->color );
  
    if ( info->arg_labels != NULL ) {
        fprintf( fd, " : " );
        SLOG_StrList_Print( info->arg_labels, fd );
    }

    fprintf( fd, " ]" );
}



int SLOG_IntvlInfo_WriteToFile( SLOG_intvlinfo_t *info, FILE *fd )
{
    const char          delimiter = '\t';
    const char          newline   = '\n';
          char          intvltype_str[ SLOG_STRING_LEN ]    = SLOG_STRING_INIT;
          char          bebits_str[ SLOG_STRING_LEN ]       = SLOG_STRING_INIT;
          char          N_arg_labels_str[ SLOG_STRING_LEN ] = SLOG_STRING_INIT;
          SLOG_bebits_t bebits;
          SLOG_uint32   N_arg_labels;
          int           lgth;
          int           ierr;

    /*  snprintf() counts '\0' in the copied char length parameter  */
    /*
    ierr = bswp_fwrite( &( info->intvltype ),
                        sizeof( SLOG_intvltype_t ), 1, fd );
    if ( ierr < 1 ) return SLOG_FALSE;
    ierr = snprintf( intvltype_str,
                     lgth, "%u", info->intvltype );
    */

    lgth = SLOG_STRING_LEN - 2;

    sprintf( intvltype_str, fmt_itype_t, info->intvltype );
    if ( intvltype_str[ lgth+1 ] != ' ' ) {
        fprintf( errfile, __FILE__":SLOG_IntvlInfo_WriteToFile() - \n"
                          "\t""intvltype of input SLOG_intvlinfo_t is "
                          "too large to fit into %d characters, intvltype "
                          "= "fmt_itype_t"\n", lgth, info->intvltype );
        fflush( errfile ); 
        return SLOG_FALSE;
    }
    intvltype_str[ lgth+1 ] = delimiter;
    ierr = bswp_fwrite( intvltype_str,
                        sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FALSE;

    bebits = SLOG_bebits_encode( info->bebits[0], info->bebits[1] );
    sprintf( bebits_str, fmt_bebits_t, bebits );
    if ( bebits_str[ lgth+1 ] != ' ' ) {
        fprintf( errfile, __FILE__":SLOG_IntvlInfo_WriteToFile() - \n"
                          "\t""encoded bebits of input SLOG_intvlinfo_t is "
                          "too large to fit into %d characters, encoded_bebits "
                          "= "fmt_bebits_t"\n", lgth, bebits );
        fflush( errfile ); 
        return SLOG_FALSE;
    }
    bebits_str[ lgth+1 ] = delimiter;
    ierr = bswp_fwrite( bebits_str,
                        sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FALSE;

    info->classtype[ lgth+1 ] = delimiter;
    ierr = bswp_fwrite( &( info->classtype ),
                        sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FALSE;

    info->label[ lgth+1 ] = delimiter;
    ierr = bswp_fwrite( &( info->label ),
                        sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FALSE;

    info->color[ lgth+1 ] = delimiter;
    ierr = bswp_fwrite( &( info->color ),
                        sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FALSE;

    if ( info->arg_labels != NULL )
        N_arg_labels = 1;
    else 
        N_arg_labels = 0;

    sprintf( N_arg_labels_str, fmt_ui32, N_arg_labels );
    if ( N_arg_labels_str[ lgth+1 ] != ' ' ) {
        fprintf( errfile, __FILE__":SLOG_IntvlInfo_WriteToFile() - \n"
                          "\t""N_arg_labels of input SLOG_intvlinfo_t is "
                          "too large to fit into %d characters, N_arg_labels "
                          "= "fmt_ui32"\n", lgth, N_arg_labels );
        fflush( errfile ); 
        return SLOG_FALSE;
    }
    N_arg_labels_str[ lgth+1 ] = delimiter;
    ierr = bswp_fwrite( N_arg_labels_str,
                        sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FALSE;

    switch ( N_arg_labels ) {
    case 0 :
        break;
    case 1 :
        ierr = SLOG_StrList_WriteToFileAsStrs( info->arg_labels, fd );
        if ( ierr != SLOG_SUCCESS ) {
            fprintf( errfile, __FILE__":SLOG_IntvlInfo_WriteToFile() - \n"
                              "\t""SLOG_StrList_WriteToFileAsStrs() fails "
                              "for the arg_labels :\n\t" );
            SLOG_StrList_Print( info->arg_labels, errfile );
            fprintf( errfile, "\n" ); 
            fflush( errfile ); 
            return SLOG_FALSE;
        }
        break;
    default :
        fprintf( errfile, __FILE__":SLOG_IntvlInfo_WriteToFile() - \n"
                          "\t""The invalid N_arg_labels is "fmt_ui32"\n",
                          N_arg_labels );
        fflush( errfile ); 
        return SLOG_FALSE;
    }

    ierr = bswp_fwrite( &newline, sizeof( char ), 1, fd );
    if ( ierr < 1 ) return SLOG_FALSE;

    return SLOG_TRUE;
}



int SLOG_IntvlInfo_ReadFromFile( SLOG_intvlinfo_t *info, FILE *fd )
{
          char          newline = '\n';
          char          intvltype_str[ SLOG_STRING_LEN ];
          char          bebits_str[ SLOG_STRING_LEN ];
          char          N_arg_labels_str[ SLOG_STRING_LEN ];
          SLOG_bebits_t bebits;
          SLOG_uint32   N_arg_labels;
          int           ierr;

    /*
    ierr = bswp_fread( &( info->intvltype ),
                       sizeof( SLOG_intvltype_t ), 1, fd );
    if ( ierr < 1 ) return SLOG_FALSE;
    */
    ierr = bswp_fread( intvltype_str,
                       sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FALSE;
    info->intvltype = atoi( intvltype_str );

    ierr = bswp_fread( bebits_str,
                       sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FALSE;
    bebits = atoi( bebits_str );
    SLOG_bebits_decode( bebits, info->bebits );
    
    ierr = bswp_fread( &( info->classtype ),
                       sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FALSE;

    ierr = bswp_fread( &( info->label ),
                       sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FALSE;

    ierr = bswp_fread( &( info->color ),
                       sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FALSE;

    ierr = bswp_fread( N_arg_labels_str,
                       sizeof( char ), SLOG_STRING_LEN, fd );
    if ( ierr < SLOG_STRING_LEN ) return SLOG_FALSE;
    N_arg_labels = atoi( N_arg_labels_str );

    switch ( N_arg_labels ) {
    case 0 :
        info->arg_labels = NULL;
        break;
    case 1 :
        info->arg_labels = SLOG_StrList_ReadFromFileAsStrs( fd );
        if ( info->arg_labels == NULL ) {
            fprintf( errfile, __FILE__":SLOG_IntvlInfo_ReadFromFile() - \n"
                              "\t""SLOG_StrList_ReadFromFileAsStrs() fails "
                              "for the SLOG_strlist_t.\n" );
            fflush( errfile ); 
            return SLOG_FALSE;
        }
        break;
    default :
        fprintf( errfile, __FILE__":SLOG_IntvlInfo_ReadFromFile() - \n"
                          "\t""The invalid N_arg_labels read is "fmt_ui32"\n",
                          N_arg_labels );
        fflush( errfile ); 
        return SLOG_FALSE;
    }

    ierr = bswp_fread( &newline, sizeof( char ), 1, fd );
    if ( ierr < 1 ) return SLOG_FALSE;

    return SLOG_TRUE;
}



int SLOG_IntvlInfo_IsKeyEqualTo( const SLOG_intvlinfo_t *info1,
                                 const SLOG_intvlinfo_t *info2 )
{
    return (    ( info1->intvltype == info2->intvltype )
             && ( info1->bebits[0] == info2->bebits[0] )
             && ( info1->bebits[1] == info2->bebits[1] ) );
}

int SLOG_IntvlInfo_IsValueEqualTo( const SLOG_intvlinfo_t *info1,
                                   const SLOG_intvlinfo_t *info2 )
{
    if (    strcmp( info1->classtype, info2->classtype ) != 0
         || strcmp( info1->label,     info2->label )     != 0
         || strcmp( info1->color,     info2->color )     != 0 )
        return SLOG_FALSE;

    if ( ! SLOG_StrList_IsEqualTo( info1->arg_labels, info2->arg_labels ) )
        return SLOG_FALSE;

    return SLOG_TRUE;
}

int SLOG_IntvlInfo_IsEqualTo( const SLOG_intvlinfo_t *info1,
                              const SLOG_intvlinfo_t *info2 )
{
    return (    SLOG_IntvlInfo_IsKeyEqualTo( info1, info2 )
             && SLOG_IntvlInfo_IsValueEqualTo( info1, info2 ) );
}


int SLOG_IntvlInfo_IsKeyWholeIntvl( const SLOG_intvlinfo_t *info )
{
    return ( SLOG_bebits_IsWholeIntvl( info->bebits ) );
}


void SLOG_IntvlInfo_SetUsed(       SLOG_intvlinfo_t *info,
                             const int               boolean_flag )
{
    if ( info != NULL )
        info->used = boolean_flag;
}
