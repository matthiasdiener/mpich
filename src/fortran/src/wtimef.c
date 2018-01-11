/* wtime.c */
/* Custom Fortran interface file */
#include "mpi_fortimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(F77_NAME_UPPER)
#pragma weak MPI_WTIME = PMPI_WTIME
EXPORT_MPI_API double MPI_WTIME ( void );
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma weak mpi_wtime__ = pmpi_wtime__
EXPORT_MPI_API double mpi_wtime__ ( void );
#elif !defined(F77_NAME_LOWER_USCORE)
#pragma weak mpi_wtime = pmpi_wtime
EXPORT_MPI_API double mpi_wtime ( void );
#else
#pragma weak mpi_wtime_ = pmpi_wtime_
EXPORT_MPI_API double mpi_time_ ( void );

#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(F77_NAME_UPPER)
#pragma _HP_SECONDARY_DEF PMPI_WTIME  MPI_WTIME
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma _HP_SECONDARY_DEF pmpi_wtime__  mpi_wtime__
#elif !defined(F77_NAME_LOWER_USCORE)
#pragma _HP_SECONDARY_DEF pmpi_wtime  mpi_wtime
#else
#pragma _HP_SECONDARY_DEF pmpi_wtime_  mpi_wtime_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(F77_NAME_UPPER)
#pragma _CRI duplicate MPI_WTIME as PMPI_WTIME
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma _CRI duplicate mpi_wtime__ as pmpi_wtime__
#elif !defined(F77_NAME_LOWER_USCORE)
#pragma _CRI duplicate mpi_wtime as pmpi_wtime
#else
#pragma _CRI duplicate mpi_wtime_ as pmpi_wtime_
#endif

/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif

#ifdef F77_NAME_UPPER
#define mpi_wtime_ PMPI_WTIME
#elif defined(F77_NAME_LOWER_2USCORE)
#define mpi_wtime_ pmpi_wtime__
#elif !defined(F77_NAME_LOWER_USCORE)
#define mpi_wtime_ pmpi_wtime
#else
#define mpi_wtime_ pmpi_wtime_
#endif

#else

#ifdef F77_NAME_UPPER
#define mpi_wtime_ MPI_WTIME
#elif defined(F77_NAME_LOWER_2USCORE)
#define mpi_wtime_ mpi_wtime__
#elif !defined(F77_NAME_LOWER_USCORE)
#define mpi_wtime_ mpi_wtime
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API double mpi_wtime_ ( void );

EXPORT_MPI_API double  mpi_wtime_()
{
    return MPI_Wtime();
}
