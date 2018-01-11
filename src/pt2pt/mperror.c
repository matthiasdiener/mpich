/*
 *  $Id: mperror.c,v 1.20 1995/01/15 06:54:43 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

#if (defined(__STDC__) || defined(__cpluscplus))
#define MPIR_USE_STDARG
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
void MPIR_Errors_are_fatal(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
  char buf[MPI_MAX_ERROR_STRING];
  int  myid, result_len; 
#endif

  MPID_Myrank( MPI_COMM_WORLD->ADIctx, &myid );
  MPI_Error_string( *code, (char *)buf, &result_len );
  fprintf( stderr, "%d - %s : %s\n", myid, 
          string ? string : "<NO ERROR MESSAGE>", buf );
  MPI_Abort( *comm, *code );
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
  
  MPID_Myrank( MPI_COMM_WORLD->ADIctx, &myid );
  MPI_Error_string( *code, buf, &result_len );
  fprintf( stderr, "%d -  File: %s   Line: %d\n", myid, 
		   file, *line );
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

  /* Call handler routine */
  (*handler->routine)( &comm, &code, string, file, &line );
  return (code);
}

