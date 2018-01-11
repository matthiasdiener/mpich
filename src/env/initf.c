/*
 *  $Id: initf.c,v 1.17 1997/02/18 23:06:23 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
#else
#include "mpisys.h"
#endif

#ifdef MPI_CRAY
/* Cray requires special code for sending strings to/from Fortran */
#include <fortran.h>
#endif

#if defined(MPI_BUILD_PROFILING)
#if defined(FORTRANCAPS)
#define mpi_init_ PMPI_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_init_ pmpi_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_init_ pmpi_init
#else
#define mpi_init_ pmpi_init_
#endif

#else
#if defined(FORTRANCAPS)
#define mpi_init_ MPI_INIT
#define mpir_iargc_ MPIR_IARGC
#define mpir_getarg_ MPIR_GETARG
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_init_ mpi_init__
#define mpir_iargc_ mpir_iargc__
#define mpir_getarg_ mpir_getarg__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_init_ mpi_init
#define mpir_iargc_ mpir_iargc
#define mpir_getarg_ mpir_getarg
#endif
#endif

/* #define DEBUG(a) {a}  */
#define DEBUG(a)
/* Prototype to suppress warnings about missing prototypes */
void mpi_init_ ANSI_ARGS(( int * ));
int mpir_iargc_ ANSI_ARGS((void));
#ifdef MPI_CRAY
void mpir_getarg_ ANSI_ARGS(( int *, _fcd ));
#else
void mpir_getarg_ ANSI_ARGS(( int *, char *, int ));
#endif

void mpi_init_( ierr )
int *ierr;
{
    int  Argc, i, argsize = 1024;
    char **Argv, *p;
    int  ArgcSave;           /* Save the argument count */
    char **ArgvSave,         /* Save the pointer to the argument vector */
 	 **ArgvValSave;      /* Save the ENTRIES in the argument vector */
#ifdef MPI_CRAY
    _fcd tmparg;
#endif

/* Recover the args with the Fortran routines iargc_ and getarg_ */
    ArgcSave	= Argc = mpir_iargc_() + 1;
    ArgvSave	= Argv = (char **)MALLOC( Argc * sizeof(char *) );    
    ArgvValSave	= (char **)MALLOC( Argc * sizeof(char *) );
    if (!Argv || !ArgvValSave) {
	*ierr = MPIR_ERROR( (struct MPIR_COMMUNICATOR *)0, MPI_ERR_EXHAUSTED, 
			    "Out of space in MPI_INIT" );
	return;
    }
    for (i=0; i<Argc; i++) {
	ArgvValSave[i] = Argv[i] = (char *)MALLOC( argsize + 1 );
	if (!Argv[i]) {
	    *ierr = MPIR_ERROR( (struct MPIR_COMMUNICATOR *)0, 
				MPI_ERR_EXHAUSTED, 
				"Out of space in MPI_INIT" );
	    return;
        }
#ifdef MPI_CRAY
	tmparg = _cptofcd( Argv[i], argsize );
	mpir_getarg_( &i, tmparg );
#else
	mpir_getarg_( &i, Argv[i], argsize );
#endif
	DEBUG(MPID_trvalid( "after getarg" ); fflush(stderr););
	/* Trim trailing blanks */
	p = Argv[i] + argsize - 1;
	while (p >= Argv[i]) {
	    if (*p != ' ' && *p) {
		p[1] = '\0';
		break;
	    }
	    p--;
	}
    }

    DEBUG(for (i=0; i<ArgcSave; i++) {
	PRINTF( "[%d] argv[%d] = |%s|\n", MPIR_tid, i, ArgvSave[i] );
    });
    *ierr = MPI_Init( &Argc, &Argv );
    
    /* Recover space */
    DEBUG(PRINTF("[%d] About to recover space\n", MPIR_tid ););
    DEBUG(MPID_trdump(stdout););
    for (i=0; i<ArgcSave; i++) {
	DEBUG(PRINTF("[%d] About to recover ArgvSave[%d]=|%s|\n",\
		     MPIR_tid,i,ArgvSave[i] ););
	FREE( ArgvValSave[i] );
    }
    DEBUG(PRINTF("[%d] About to recover ArgvSave\n", MPIR_tid ););
    FREE( ArgvSave );
    FREE( ArgvValSave );
}
