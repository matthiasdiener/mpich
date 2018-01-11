#include "mpi_fortconf.h"
/* mpi_fortconf must come before mpi.h because a few defines (e.g., USE_STDARG)
   are defined in mpi_fortconf for mpi.h */
#include "mpi.h"
#include "mpi_fortdefs.h"

#ifdef BUILDING_IN_MPICH
extern struct MPIR_COMMUNICATOR *MPIR_COMM_WORLD;
/* For MPIR_GET_COMM_PTR */
/* #include "comm.h" */
extern void *MPIR_ToPointer ( int );
#define MPIR_GET_COMM_PTR(idx) \
    (struct MPIR_COMMUNICATOR *)MPIR_ToPointer( idx )
/* For error codes */
#include "mpi_error.h"

/* Error handling */
#if defined(USE_STDARG) && !defined(USE_OLDSTYLE_STDARG)
int MPIR_Err_setmsg( int, int, const char *, const char *, const char *, ... );
#else
int MPIR_Err_setmsg();
#endif

/* Utility functions */
/* NOTE: These will cause problems for non-MPICH MPI implementations */
int MPIR_Keyval_create( MPI_Copy_function *, MPI_Delete_function *, 
				    int *, void *, int );
void MPIR_Attr_make_perm ( int );
void MPIR_Free_perm_type ( MPI_Datatype );

/* Device utility functions */
void MPID_Node_name ( char *, int );
void MPID_ArgSqueeze ( int *, char ** );
void MPID_Dump_queues (void);

#endif

/* #include "mpi_fortnames.h" */
#include "mpi_fort.h"

/* Fortran <-> C string conversions in env/fstrutils.c */
int MPIR_fstr2cstr  (char *, long, char *, long) ;
int MPIR_cstr2fstr  (char *, long, char *) ;

#define EXPORT_MPI_API
