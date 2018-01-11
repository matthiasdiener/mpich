/*
 *  $Id: mperror.c,v 1.17 1994/09/30 22:11:19 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"


/*
   Fatal error handler.  Prints a message and aborts.
 */
void MPIR_Errors_are_fatal(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
  char buf[MPI_MAX_ERROR_STRING];
  int  myid, result_len; 
  
  MPID_Myrank( MPI_COMM_WORLD->ADIctx, &myid );
  MPI_Error_string( *code, (char *)buf, &result_len );
  fprintf( stderr, "%d - %s : %s\n", myid, 
          string ? string : "<NO ERROR MESSAGE>", buf );
  MPI_Abort( *comm, *code );
}


/*
   Handler ignores errors.
 */   
void MPIR_Errors_return(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
}


/*
   Handler prints warning messsage and returns.  Internal.  Not
   a part of the standard.
 */
void MPIR_Errors_warn(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
  char buf[MPI_MAX_ERROR_STRING];
  int  myid, result_len; 
  
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
  if (!comm || (handler = comm->error_handler) == MPI_ERRHANDLER_NULL) 
    handler = MPI_ERRORS_ARE_FATAL;

  /* Call handler routine */
  (*handler->routine)( &comm, &code, string, file, &line );
  return (code);
}

#ifdef FOO
#ifdef FORTRANCAPS
#elif defined(FORTRANDOUBLEUNDERSCORE)
void mpi_errors_are_fatal__(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
MPIR_errors_are_fatal( comm, code, string, file, line );
}
void mpi_errors_return__(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
MPI_ERRORS_RETURN( comm, code, string, file, line );
}
#elif !defined(FORTRANUNDERSCORE)
void mpi_errors_are_fatal(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
MPI_ERRORS_ARE_FATAL( comm, code, string, file, line );
}
void mpi_errors_return(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
MPI_ERRORS_RETURN( comm, code, string, file, line );
}
#else
void mpi_errors_are_fatal_(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
MPI_ERRORS_ARE_FATAL( comm, code, string, file, line );
}
void mpi_errors_return_(  comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
MPI_ERRORS_RETURN( comm, code, string, file, line );
}
#endif
#endif

