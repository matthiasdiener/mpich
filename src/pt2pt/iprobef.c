/* iprobe.c */
/* Custom Fortran interface file  */
#include "mpiimpl.h"
#include "mpifort.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_iprobe_ PMPI_IPROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_iprobe_ pmpi_iprobe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_iprobe_ pmpi_iprobe
#else
#define mpi_iprobe_ pmpi_iprobe_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_iprobe_ MPI_IPROBE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_iprobe_ mpi_iprobe__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_iprobe_ mpi_iprobe
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_iprobe_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                             MPI_Fint *, MPI_Fint * ));

void mpi_iprobe_( source, tag, comm, flag, status, __ierr )
MPI_Fint *source;
MPI_Fint *tag;
MPI_Fint *comm;
MPI_Fint *flag;
MPI_Fint *status;
int *__ierr;
{
    int lflag;
    MPI_Status c_status;

    *__ierr = MPI_Iprobe((int)*source,(int)*tag,MPI_Comm_f2c(*comm),
                         &lflag,&c_status);
    *flag = MPIR_TO_FLOG(lflag);
    MPI_Status_c2f(&c_status, status);
}
