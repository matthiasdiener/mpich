/* type_struct.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
#include "mpisys.h"
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
#define mpi_type_struct_ PMPI_TYPE_STRUCT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_struct_ pmpi_type_struct__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_struct_ pmpi_type_struct
#else
#define mpi_type_struct_ pmpi_type_struct_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_struct_ MPI_TYPE_STRUCT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_struct_ mpi_type_struct__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_struct_ mpi_type_struct
#endif
#endif

 void mpi_type_struct_( count, blocklens, indices, old_types, newtype, __ierr )
int*count;
int        blocklens[];
MPI_Aint      indices[];      
MPI_Datatype  old_types[];
MPI_Datatype *newtype;
int *__ierr;
{
MPI_Datatype lnewtype;
#ifdef POINTER_64_BITS
MPI_Datatype  *old;
int           i;

old = (MPI_Datatype *)MALLOC( *count * sizeof(MPI_Datatype) );
if (!old) {
    /* Error message for malloc */
    *__ierr =MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
			 "Out of space in MPI_TYPE_STRUCT" );
    return;  
    }
for (i=0; i<*count; i++) 
    old[i] = (MPI_Datatype)MPIR_ToPointer(old_types[i]);
*__ierr = MPI_Type_struct(*count,blocklens,indices,old,&lnewtype);
FREE( old );
#else

*__ierr = MPI_Type_struct(*count,blocklens,indices,old_types,&lnewtype);
#endif

*(int*)newtype = MPIR_FromPointer(lnewtype);
}
