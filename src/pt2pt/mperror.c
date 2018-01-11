/*
 *  $Id: mperror.c,v 1.24 1996/06/07 15:07:30 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
/* Include the prototypes for these service functions */
#include "mpipt2pt.h"
#endif

#if (defined(__STDC__) || defined(__cpluscplus))
#if !defined(MPIR_USE_STDARG)
#define MPIR_USE_STDARG
#endif
#include <stdarg.h>

/* Place to put varargs code, which should look something like

   void mpir_errors_are_fatal( MPI_Comm *comm, int *code, ... )
   {
   va_list Argp;

   va_start( Argp, code );
   string = va_arg(Argp,char *);
   file   = va_arg(Argp,char *);
   line   = va_arg(Argp,int *);
   va_end( Argp );
   ... 
   }
 */
#endif

/*
   Fatal error handler.  Prints a message and aborts.
 */
#ifdef MPIR_USE_STDARG
void MPIR_Errors_are_fatal(  MPI_Comm *comm, int * code, ... )
{
  char buf[MPI_MAX_ERROR_STRING];
  int  result_len; 
  char *string, *file;
  int  *line;
  va_list Argp;

  va_start( Argp, code );
  string = va_arg(Argp,char *);
  file   = va_arg(Argp,char *);
  line   = va_arg(Argp,int *);
  va_end( Argp );
#else
void MPIR_Errors_are_fatal(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
  char buf[MPI_MAX_ERROR_STRING];
  int  result_len; 
#endif

#ifdef MPI_ADI2
  MPI_Error_string( *code, (char *)buf, &result_len );
  fprintf( stderr, "%d - %s : %s\n", MPID_MyWorldRank,
          string ? string : "<NO ERROR MESSAGE>", buf );
  /* Comm might be null... */
  MPID_Abort( (comm) ? *comm : (MPI_Comm)0, *code, (char *)0, (char *)0 );
#else
  {
  int myid;
  MPID_Myrank( MPI_COMM_WORLD->ADIctx, &myid );
  MPI_Error_string( *code, (char *)buf, &result_len );
  fprintf( stderr, "%d - %s : %s\n", myid, 
          string ? string : "<NO ERROR MESSAGE>", buf );
  }
  /* Comm might be null... */
  MPI_Abort( (comm) ? *comm : (MPI_Comm)0, *code );
#endif
}


/*
   Handler ignores errors.
 */   
#ifdef MPIR_USE_STDARG
void MPIR_Errors_return( MPI_Comm *comm, int *code, ... )
{
}
#else
void MPIR_Errors_return(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
}
#endif

/*
   Handler prints warning messsage and returns.  Internal.  Not
   a part of the standard.
 */
#ifdef MPIR_USE_STDARG
void MPIR_Errors_warn(  MPI_Comm *comm, int *code, ... )
{  
  char buf[MPI_MAX_ERROR_STRING];
  int  myid, result_len; 
  char *string, *file;
  int  *line;
  va_list Argp;

  va_start( Argp, code );
  string = va_arg(Argp,char *);
  file   = va_arg(Argp,char *);
  line   = va_arg(Argp,int *);
  va_end( Argp );
#else
void MPIR_Errors_warn(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
  char buf[MPI_MAX_ERROR_STRING];
  int  myid, result_len; 
#endif

#ifdef MPI_ADI2
  myid = MPID_MyWorldRank;
#else  
  MPID_Myrank( MPI_COMM_WORLD->ADIctx, &myid );
#endif
  MPI_Error_string( *code, buf, &result_len );
#ifdef MPIR_DEBUG
  /* Generate this information ONLY when debugging MPIR */
  fprintf( stderr, "%d -  File: %s   Line: %d\n", myid, 
		   file, *line );
#endif
  fprintf( stderr, "%d - %s : %s\n", myid, 
          string ? string : "<NO ERROR MESSAGE>", buf );
}


/* 
   This calls the user-specified error handler.  If that handler returns,
   we return the error code 
 */
int MPIR_Error( comm, code, string, file, line )
MPI_Comm  comm;
int       code, line;
char     *string, *file;
{
  MPI_Errhandler handler;

  /* Check for bad conditions */
  if (comm == MPI_COMM_NULL) 
    comm = MPI_COMM_WORLD;
  /* This can happen if MPI_COMM_WORLD is not initialized */
  if (!comm || (handler = comm->error_handler) == MPI_ERRHANDLER_NULL) 
    handler = MPI_ERRORS_ARE_FATAL;
  if (!handler) {
      /* Fatal error, probably a call before MPI_Init */
      fprintf( stderr, "Fatal error; unknown error handler\n\
May be MPI call before MPI_INIT.  Error message is %s and code is %d\n", 
	      string, code );
      return code;
      }

  /* If we're calling MPI routines from within an MPI routine, we 
   (probably) just want to return.  If so, we set "use_return_handler" */
  if (comm && comm->use_return_handler) return code;

  /* Call handler routine */
  (*handler->routine)( &comm, &code, string, file, &line );
  return (code);
}

