/* probe.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_probe_ PMPI_PROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_probe_ pmpi_probe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_probe_ pmpi_probe
#else
#define mpi_probe_ pmpi_probe_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_probe_ MPI_PROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_probe_ mpi_probe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_probe_ mpi_probe
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_probe_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                            MPI_Fint *, MPI_Fint * ));

void mpi_probe_( source, tag, comm, status, __ierr )
MPI_Fint *source;
MPI_Fint *tag;
MPI_Fint *comm;
MPI_Fint *status;
MPI_Fint *__ierr;
{
    MPI_Status c_status;
   
    *__ierr = MPI_Probe((int)*source, (int)*tag, MPI_Comm_f2c(*comm),
                        &c_status);
    MPI_Status_c2f(&c_status, status);
    
}
