#include "mpi.h"
#ifdef PETSC_AVAILABLE
#include "system/system.h"
#endif
#include "mpe.h"

/*@
    MPE_Ptime - Returns process time stamp

    Notes:
    This routine returns the CPU time charged to the process only if the
    PETSC package is available (it uses the PETSc routine SYGetCPUTime).
    Otherwise, it returns MPI_Wtime (wall-clock time).
@*/
double MPE_Ptime()
{
#ifdef PETSC_AVAILABLE
    return SYGetCPUTime();
#else
    return MPI_Wtime();
#endif
}

/*@
    MPE_Wtime - Returns wall clock time stamp
@*/
double MPE_Wtime()
{
    return MPI_Wtime();
}
