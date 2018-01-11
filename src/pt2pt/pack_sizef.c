/* pack_size.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_pack_size_ PMPI_PACK_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pack_size_ pmpi_pack_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pack_size_ pmpi_pack_size
#else
#define mpi_pack_size_ pmpi_pack_size_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_pack_size_ MPI_PACK_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pack_size_ mpi_pack_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pack_size_ mpi_pack_size
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_pack_size_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                MPI_Fint *, MPI_Fint * ));

void mpi_pack_size_ ( incount, datatype, comm, size, __ierr )
MPI_Fint *incount;
MPI_Fint *datatype;
MPI_Fint *comm;
MPI_Fint *size;
MPI_Fint *__ierr;
{
    int lsize;

    *__ierr = MPI_Pack_size((int)*incount, MPI_Type_f2c(*datatype),
                            MPI_Comm_f2c(*comm), &lsize);
    *size = (MPI_Fint)lsize;
}
