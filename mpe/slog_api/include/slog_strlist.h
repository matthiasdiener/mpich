#if !defined( _SLOG )
#include "slog.h"
#endif  /*  if !defined( _SLOG )  */

SLOG_strlist_t *SLOG_StrList_CreateFromVarArgs( const SLOG_uint32 Nstrs, ... );

SLOG_strlist_t *SLOG_StrList_CreateFromArray( const SLOG_uint32   Nstrs,
                                                    char        **strs );

SLOG_strlist_t *SLOG_StrList_Copy( const SLOG_strlist_t *src );

SLOG_strlist_t *SLOG_StrList_Append(       SLOG_strlist_t *dest,
                                     const SLOG_strlist_t *src );

void SLOG_StrList_Free( SLOG_strlist_t  *strlist );

int SLOG_StrList_Print( const SLOG_strlist_t *strlist, FILE *outfd );

int SLOG_StrList_WriteToFile( const SLOG_strlist_t *strlist, FILE *fd );

SLOG_strlist_t *SLOG_StrList_ReadFromFile( FILE *fd );

int SLOG_StrList_WriteToFileAsStrs( const SLOG_strlist_t *strlist, FILE *fd );

SLOG_strlist_t *SLOG_StrList_ReadFromFileAsStrs( FILE *fd );

int SLOG_StrList_IsEqualTo( const SLOG_strlist_t *strlist1,
                            const SLOG_strlist_t *strlist2 );
