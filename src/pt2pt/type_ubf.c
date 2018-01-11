/* type_ub.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_ub_ PMPI_TYPE_UB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_ub_ pmpi_type_ub__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_ub_ pmpi_type_ub
#else
#define mpi_type_ub_ pmpi_type_ub_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_ub_ MPI_TYPE_UB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_ub_ mpi_type_ub__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_ub_ mpi_type_ub
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_ub_ ANSI_ARGS(( MPI_Datatype*, int *, int * ));

void mpi_type_ub_ ( datatype, displacement, __ierr )
MPI_Datatype  *datatype;
int           *displacement;
int           *__ierr;
{
    MPI_Aint c_displacement;
    *__ierr = MPI_Type_ub(*datatype,&c_displacement);
    /* Should check for truncation */
    *displacement = (int)c_displacement;
}
