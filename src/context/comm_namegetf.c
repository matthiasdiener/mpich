/*
 *  $Id: comm_namegetf.c,v 1.3 1997/02/20 21:09:09 gropp Exp $
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */
/* Update log
 *
 * Nov 29 1996 jcownie@dolphinics.com: Implement MPI-2 Fortran binding for 
 *        communicator naming function.
 */

#include "mpiimpl.h"
#ifdef _CRAY
#include "fortran.h"
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_get_name_ PMPI_COMM_GET_NAME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_get_name_ pmpi_comm_get_name__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_get_name_ pmpi_comm_get_name
#else
#define mpi_comm_get_name_ pmpi_comm_get_name_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_get_name_ MPI_COMM_GET_NAME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_get_name_ mpi_comm_get_name__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_get_name_ mpi_comm_get_name
#endif
#endif

#ifdef _CRAY

void mpi_comm_get_name_( comm, string_fcd, __ierr )
MPI_Comm *comm;
_fcd string_fcd;
int *__ierr;
{
  char *cres;

  *__ierr = MPI_Comm_get_name( *comm, &cres );

  /* Assign the result to the Fortran string doing blank padding as required */
  MPIR_cstr2fstr(_fcdtocp(string_fcd), _fcdlen(string_fcd), cres);
}
#else

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_get_name_ ANSI_ARGS(( MPI_Comm *, char *, int *, long ));
void mpi_comm_get_name_( comm, string, __ierr, d )
MPI_Comm * comm;
char *string;
int *__ierr;
long d;
{
  char *cres;

  *__ierr = MPI_Comm_get_name(*comm, &cres);

  /* Assign the result to the Fortran string doing blank padding as required */
  MPIR_cstr2fstr(string,d,cres);
}
#endif
