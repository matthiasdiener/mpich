/*
 *  $Id: comm_namegetf.c,v 1.3 1998/01/16 16:24:36 swider Exp $
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

void mpi_comm_get_name_( comm, string_fcd, nml, __ierr )
MPI_Comm *comm;
_fcd string_fcd;
int *nml;
int *__ierr;
{
  char cres [MPI_MAX_NAME_STRING];

  *__ierr = MPI_Comm_get_name( *comm, cres, nml);
  if (*nml > _fcdlen(string_fcd))
    *nml = _fcdlen(string_fcd);

  /* Assign the result to the Fortran string doing blank padding as required */
  MPIR_cstr2fstr(_fcdtocp(string_fcd), _fcdlen(string_fcd), cres);
}
#else

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_get_name_ ANSI_ARGS(( MPI_Fint *, char *, MPI_Fint *, 
                                    MPI_Fint *, MPI_Fint ));
void mpi_comm_get_name_( comm, string, reslen, __ierr, d )
MPI_Fint * comm;
char     *string;
MPI_Fint *reslen;
MPI_Fint *__ierr;
MPI_Fint d;
{
  char cres [MPI_MAX_NAME_STRING];
  int l_reslen;

  *__ierr = MPI_Comm_get_name(MPI_Comm_f2c(*comm), cres, &l_reslen);
  *reslen = l_reslen;

  if (*reslen > (long)d)
    *reslen = (long)d;

  /* Assign the result to the Fortran string doing blank padding as required */
  MPIR_cstr2fstr(string,(long)d,cres);

}
#endif
