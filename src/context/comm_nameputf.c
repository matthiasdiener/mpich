/*
 *  $Id: comm_nameputf.c,v 1.2 1998/01/16 16:24:37 swider Exp $
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
#define mpi_comm_set_name_ PMPI_COMM_SET_NAME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_set_name_ pmpi_comm_set_name__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_set_name_ pmpi_comm_set_name
#else
#define mpi_comm_set_name_ pmpi_comm_set_name_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_set_name_ MPI_COMM_SET_NAME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_set_name_ mpi_comm_set_name__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_set_name_ mpi_comm_set_name
#endif
#endif

#ifdef _CRAY

void mpi_comm_set_name_( comm, string_fcd, __ierr )
MPI_Comm *comm;
_fcd string_fcd;
int *__ierr;
{
  char cres[MPI_MAX_NAME_STRING];

  /* Convert to a C string */
  MPIR_fstr2cstr( cres, MPI_MAX_NAME_STRING, _fcdtocp(string_fcd), _fcdlen(string_fcd));

  *__ierr = MPI_Comm_set_name( *comm, cres );
}
#else

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_set_name_ ANSI_ARGS(( MPI_Fint *, char *, MPI_Fint *, 
                                    MPI_Fint ));
void mpi_comm_set_name_( comm, string, __ierr, d )
MPI_Fint *comm;
char     *string;
MPI_Fint *__ierr;
MPI_Fint d;
{
  char cres[MPI_MAX_ERROR_STRING];

  /* Assign the result to the Fortran string doing blank padding as required */
  MPIR_fstr2cstr(cres, MPI_MAX_ERROR_STRING, string, (long)d);
  *__ierr = MPI_Comm_set_name(MPI_Comm_f2c(*comm), cres);
}
#endif
