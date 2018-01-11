/* type_lb.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_lb_ PMPI_TYPE_LB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_lb_ pmpi_type_lb__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_lb_ pmpi_type_lb
#else
#define mpi_type_lb_ pmpi_type_lb_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_lb_ MPI_TYPE_LB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_lb_ mpi_type_lb__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_lb_ mpi_type_lb
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_lb_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));
void mpi_type_lb_ ( datatype, displacement, __ierr )
MPI_Fint *datatype;
MPI_Fint *displacement;
MPI_Fint *__ierr;
{
    MPI_Aint   c_displacement;
  
    *__ierr = MPI_Type_lb(MPI_Type_f2c(*datatype), &c_displacement);
    /* Should check for truncation */
    *displacement = (MPI_Fint)c_displacement;
}
