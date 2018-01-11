/* get_elements.c */
/* Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_get_elements_ PMPI_GET_ELEMENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_elements_ pmpi_get_elements__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_elements_ pmpi_get_elements
#else
#define mpi_get_elements_ pmpi_get_elements_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_get_elements_ MPI_GET_ELEMENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_elements_ mpi_get_elements__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_elements_ mpi_get_elements
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_get_elements_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                   MPI_Fint * ));

void mpi_get_elements_ ( status, datatype, elements, __ierr )
MPI_Fint *status;
MPI_Fint *datatype;
MPI_Fint *elements;
MPI_Fint *__ierr;
{
    int lelements;
    MPI_Status c_status;

    MPI_Status_f2c(status, &c_status);
    *__ierr = MPI_Get_elements(&c_status,MPI_Type_f2c(*datatype),
                               &lelements);
    *elements = (MPI_Fint)lelements;
}
