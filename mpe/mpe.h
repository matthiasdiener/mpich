/* MPE_Graphics should not be included here in case the system does not
   support the graphics features. */

#ifndef _MPE_INCLUDE
#define _MPE_INCLUDE

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef MPE_GRAPHICS
#include "mpe_graphics.h"
#endif
#include "mpe_log.h"

#define MPE_SUCCESS        0		/* successful operation */
#define MPE_ERR_NOXCONNECT 1		/* could not connect to X server */
#define MPE_ERR_BAD_ARGS   2            /* graphics handle invalid */
#define MPE_ERR_LOW_MEM    3	        /* out of memory (malloc() failed) */

#if defined(MPI_rs6000)

#define mpe_ptime_            mpe_ptime
#define mpe_initlog_          mpe_initlog
#define mpe_startlog_         mpe_startlog
#define mpe_stoplog_          mpe_stoplog
#define mpe_describe_state_   mpe_describe_state
#define mpe_describe_event_   mpe_describe_event
#define mpe_log_event_        mpe_log_event
#define mpe_finishlog_        mpe_finishlog

#endif

#ifdef __STDC__
void MPE_Seq_begin( MPI_Comm, int );
void MPE_Seq_end( MPI_Comm, int );
#else
void MPE_Seq_begin( );
void MPE_Seq_end( );
#endif

#if defined(__cplusplus)
};
#endif

#endif
