#if !defined ( _SLOG )
#include "slog.h"
#endif  /*  if !defined ( _SLOG )  */


SLOG_hdr_t *SLOG_HDR_Create( FILE  *outfd );

void SLOG_HDR_Free( SLOG_hdr_t *hdr );

int SLOG_SetFrameByteSize( SLOG_STREAM  *slog, SLOG_uint32 frame_bytesize );

int SLOG_SetFrameReservedSpace( SLOG_STREAM  *slog,
                                SLOG_uint32   reserved_bytesize );

int SLOG_SetMaxNumOfFramesPerDir( SLOG_STREAM  *slog, SLOG_uint32 Ndirframe );

int SLOG_SetIncreasingStarttimeOrder( SLOG_STREAM  *slog );

int SLOG_SetIncreasingEndtimeOrder( SLOG_STREAM  *slog );

int SLOG_HDR_Deposit( SLOG_hdr_t *hdr );

int SLOG_HDR_Withdraw( SLOG_hdr_t *hdr );

int SLOG_WriteHeader( SLOG_STREAM  *slog );

int SLOG_WriteUpdatedHeader( SLOG_STREAM  *slog );

int SLOG_HDR_CheckVersionID( const SLOG_hdr_t  *hdr );

int SLOG_ReadHeader( SLOG_STREAM *slog );

void SLOG_HDR_Print( FILE *outfd, const SLOG_hdr_t  *hdr );
