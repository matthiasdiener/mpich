/* type_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_free_ PMPI_TYPE_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_free_ pmpi_type_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_free_ pmpi_type_free
#else
#define mpi_type_free_ pmpi_type_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_free_ MPI_TYPE_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_free_ mpi_type_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_free_ mpi_type_free
#endif
#endif

 void mpi_type_free_ ( datatype, __ierr )
MPI_Datatype *datatype;
int *__ierr;
{
MPI_Datatype ldatatype = (MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) );
*__ierr = MPI_Type_free(&ldatatype);
/* We need to recover this pointer */
if (!ldatatype) {
    MPIR_RmPointer( *(int*)(datatype) );
    *(int*)datatype = 0;
    }
}
