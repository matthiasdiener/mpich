/*
 *  $Id: initdte.c,v 1.2 1996/06/07 15:12:21 gropp Exp $
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#ifdef MPI_ADI2
/* MPIR_Type_xxx routines are prototyped in mpipt2pt.h */
#include "mpipt2pt.h"
#include "sbcnst2.h"
#define MPIR_SBinit MPID_SBinit
#define MPIR_SBalloc MPID_SBalloc
#define MPIR_SBfree MPID_SBfree
#define MPIR_trmalloc MPID_trmalloc
#else
#include "mpisys.h"
#endif

#if defined(POINTER_64_BITS)
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) (int)a
#define MPIR_RmPointer(a) 
#endif

/* #define DEBUG(a) {a}  */
#define DEBUG(a)


#ifdef FORTRANCAPS
#define mpir_init_fdtes_ MPIR_INIT_FDTES
#define mpir_init_fsize_  MPIR_INIT_FSIZE
#define mpir_get_fsize_   MPIR_GET_FSIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpir_init_fdtes_ mpir_init_fdtes__
#define mpir_init_fsize_  mpir_init_fsize__
#define mpir_get_fsize_   mpir_get_fsize__
#elif !defined(FORTRANUNDERSCORE)
#define mpir_init_fdtes_ mpir_init_fdtes
#define mpir_init_fsize_  mpir_init_fsize
#define mpir_get_fsize_   mpir_get_fsize
#endif

/* Global memory management variables for fixed-size blocks */
void *MPIR_dtes;      /* sbcnst datatype elements */

/* Predefined datatypes are mapped into an array; this allows the datatypes
   themselves to be fixed integers (possibly encoding their length) */
MPI_Datatype MPIR_datatypes[MPIR_MAX_DATATYPE_ARRAY];

/* Static space for predefined datatypes */
struct MPIR_DATATYPE MPIR_I_CHAR, MPIR_I_SHORT, MPIR_I_INT, MPIR_I_LONG,
                            MPIR_I_UCHAR, MPIR_I_USHORT, MPIR_I_UINT, 
                            MPIR_I_ULONG, MPIR_I_FLOAT, MPIR_I_DOUBLE, 
                            MPIR_I_LONG_DOUBLE, MPIR_I_LONG_LONG_INT, 
                            MPIR_I_BYTE;
/* Derived and special types */
struct MPIR_DATATYPE MPIR_I_PACKED, MPIR_I_LONG_DOUBLE_INT,
                            MPIR_I_UB, MPIR_I_LB,
                            MPIR_I_2INTEGER, 
                            MPIR_I_FLOAT_INT, MPIR_I_DOUBLE_INT, 
                            MPIR_I_LONG_INT, MPIR_I_SHORT_INT, MPIR_I_2INT,
                            MPIR_I_REAL, MPIR_I_DOUBLE_PRECISION, 
                            MPIR_I_COMPLEX, MPIR_I_DCOMPLEX, 
                            MPIR_I_LONG_DOUBLE_INT, 
                            MPIR_I_LOGICAL;

/* Global pre-assigned datatypes */
#ifdef MPIR_MEMDEBUG
MPI_Datatype MPIR_Init_basic_datatype ANSI_ARGS(( MPIR_NODETYPE, int,
						  char *, int ));
#else
MPI_Datatype MPIR_Init_basic_datatype ANSI_ARGS(( MPIR_NODETYPE, int ));
#endif
void MPIR_Setup_datatype ANSI_ARGS((MPI_Datatype,MPIR_NODETYPE,int));
void MPIR_Setup_base_datatype ANSI_ARGS((MPI_Datatype, MPI_Datatype, 
					 MPIR_NODETYPE, int));
void MPIR_Setup_complex_datatype ANSI_ARGS((MPI_Datatype,MPI_Datatype));

/* C Datatypes for MINLOC and MAXLOC functions. 
   These allow us to construct the structures to match the compiler that
   is used to build MPICH, and, if the user uses the recommended compile
   scripts or makefiles, they will match the compiler that they use.
 */
typedef struct {
  float  var;
  int    loc;
} MPI_FLOAT_INT_struct;
MPI_FLOAT_INT_struct MPI_FLOAT_INT_var;

typedef struct {
  double var;
  int    loc;
} MPI_DOUBLE_INT_struct;
MPI_DOUBLE_INT_struct MPI_DOUBLE_INT_var;

typedef struct {
  long   var;
  int    loc;
} MPI_LONG_INT_struct;
MPI_LONG_INT_struct MPI_LONG_INT_var;

typedef struct {
  short  var;
  int    loc;
} MPI_SHORT_INT_struct;
MPI_SHORT_INT_struct MPI_SHORT_INT_var;

#if defined(HAVE_LONG_DOUBLE)
typedef struct {
  long double   var;
  int           loc;
} MPI_LONG_DOUBLE_INT_struct;
MPI_LONG_DOUBLE_INT_struct MPI_LONG_DOUBLE_INT_var;
#endif

/* Fortran datatypes */
MPI_Datatype MPI_INTEGER; /* May be the same as MPI_INT */
MPI_Datatype MPI_REAL;
MPI_Datatype MPI_DOUBLE_PRECISION;
/* MPI_Datatype MPIR_logical_dte; */
MPI_Datatype MPIR_int1_dte;
MPI_Datatype MPIR_int2_dte;
MPI_Datatype MPIR_int4_dte;
MPI_Datatype MPIR_real4_dte;
MPI_Datatype MPIR_real8_dte;
/* FORTRAN Datatypes for MINLOC and MAXLOC functions */
MPI_Datatype MPI_2INTEGER; /* May be the same as MPI_2INT */
MPI_Datatype MPIR_2real_dte;
MPI_Datatype MPIR_2double_dte;
MPI_Datatype MPIR_2complex_dte;
MPI_Datatype MPIR_2dcomplex_dte;

/* Sizes of Fortran types; computed when initialized. */
static int MPIR_FSIZE_C = 0;
static int MPIR_FSIZE_R = 0;
static int MPIR_FSIZE_D = 0;

#ifdef MPIR_MEMDEBUG
/* Convert the datatype init routine to pass through the file and line
   so that we can more easily find allocation problems */
#define MPIR_Init_basic_datatype( type, size ) \
        MPIR_Init_basic_datatype_real( type, size, __FILE__, __LINE__ )
MPI_Datatype MPIR_Init_basic_datatype_real ANSI_ARGS(( MPIR_NODETYPE, int, 
						       char *, int ));
#else
MPI_Datatype MPIR_Init_basic_datatype ANSI_ARGS(( MPIR_NODETYPE, int ));
#endif

void MPIR_Init_dtes()
{
    MPI_Datatype   type[3], temptype;
    MPI_Aint       disp[3];
    int            blln[3];
    int            i;

    /* set up pre-defined data types */
    DEBUG(PRINTF("[%d] About to create datatypes\n", MPIR_tid);)

    MPIR_dtes       = MPIR_SBinit( sizeof( struct MPIR_DATATYPE ), 100, 100 );

    /* Setup the array of predefined datatypes.
       Currently, this will include only the basic language types
     */
    for (i=0; i<MPIR_MAX_DATATYPE_ARRAY; i++) {
	MPIR_datatypes[i] = 0;
	}
    MPIR_Setup_base_datatype( MPI_INT, &MPIR_I_INT, MPIR_INT, sizeof(int) );

    /* 
       Fortran requires that integers be the same size as 
       REALs, which are half the size of doubles.  Note that
       logicals must be the same size as integers.  Note that
       much of the code does not know about MPIR_LOGICAL or MPIR_FORT_INT
       yet. 

       We still need a FORT_REAL and FORT_DOUBLE type for some systems
     */
#ifdef MPID_NO_FORTRAN
    MPIR_FSIZE_R = sizeof(float);
    MPIR_FSIZE_D = sizeof(double);
#else
    mpir_get_fsize_();
#endif
    if (sizeof(int) == MPIR_FSIZE_R) {
	MPI_INTEGER          = MPI_INT;
	}
    else if (sizeof(long) == MPIR_FSIZE_R) {
        MPI_INTEGER          = MPI_LONG;
        }
    else {
	MPI_INTEGER          = MPIR_Init_basic_datatype( MPIR_FORT_INT, 
							 MPIR_FSIZE_R );
	}
    MPIR_Setup_base_datatype( MPI_DOUBLE, &MPIR_I_DOUBLE, 
			      MPIR_DOUBLE, sizeof(double) );
#if !defined(MPID_NO_FORTRAN)
    /* 
       Check for consistent definition of ints in the interfaces.
       If we get this message, we need to make changes in all of the 
       Fortran wrappers (replacing int types and adding casts)
     */
    if (sizeof(int) != MPIR_FSIZE_R) {
	fprintf( stderr, 
	    "WARNING: Fortran integers are different in size from C ints\n" );
	}
#endif
    MPIR_Setup_datatype( &MPIR_I_LOGICAL, MPIR_LOGICAL, MPIR_FSIZE_R );
    MPIR_Setup_base_datatype( MPI_FLOAT, &MPIR_I_FLOAT, 
			      MPIR_FLOAT, sizeof(float) );
    /* Hunt for Fortran real size */
    if (sizeof(float) == MPIR_FSIZE_R) {
	MPI_REAL		     = MPIR_Init_basic_datatype( MPIR_FLOAT, 
							       sizeof(float) );
	MPIR_Setup_datatype( &MPIR_I_COMPLEX, MPIR_COMPLEX, 2 * MPIR_FSIZE_R );
	MPIR_I_COMPLEX.align  = MPIR_FSIZE_R;

	MPI_Type_contiguous ( 2, MPI_FLOAT, &MPIR_2real_dte );
	MPI_Type_commit ( &MPIR_2real_dte );
	MPIR_Type_permanent ( MPIR_2real_dte );
	}
    else if (sizeof(double) == MPIR_FSIZE_R) {
	MPI_REAL		     = MPIR_Init_basic_datatype( MPIR_DOUBLE, 
							      sizeof(double) );
	MPIR_Setup_datatype( &MPIR_I_COMPLEX, MPIR_DOUBLE_COMPLEX, 
			     2 * MPIR_FSIZE_R );
	MPIR_I_COMPLEX.align  = MPIR_FSIZE_R;

	MPI_Type_contiguous ( 2, MPI_DOUBLE, &MPIR_2real_dte );
	MPI_Type_commit ( &MPIR_2real_dte );
	MPIR_Type_permanent ( MPIR_2real_dte );
	}
    else {
	/* we'll have a problem with the reduce/scan ops */
	MPI_REAL		     = MPIR_Init_basic_datatype( MPIR_FLOAT, 
							      MPIR_FSIZE_R );
	MPIR_Setup_datatype( &MPIR_I_COMPLEX, MPIR_COMPLEX, 2 * MPIR_FSIZE_R );
	MPIR_I_COMPLEX.align  = MPIR_FSIZE_R;

	MPI_Type_contiguous ( 2, MPI_FLOAT, &MPIR_2real_dte );
	MPI_Type_commit ( &MPIR_2real_dte );
	MPIR_Type_permanent ( MPIR_2real_dte );
	}

    /* Note that dcomplex is needed for src/pt2pt/pack_size.c */
    if (sizeof(double) == MPIR_FSIZE_D) {
	MPI_DOUBLE_PRECISION = MPIR_Init_basic_datatype( MPIR_DOUBLE, 
							   sizeof( double ) );
	MPIR_Setup_datatype( &MPIR_I_DCOMPLEX, MPIR_DOUBLE_COMPLEX, 
			    2 * MPIR_FSIZE_D );
	MPIR_I_DCOMPLEX.align = MPIR_FSIZE_D;

	MPI_Type_contiguous ( 2, MPI_DOUBLE, &MPIR_2double_dte );
	MPI_Type_commit ( &MPIR_2double_dte );
	MPIR_Type_permanent ( MPIR_2double_dte );
	}
#if defined(HAVE_LONG_DOUBLE)
    else if (sizeof(long double) == MPIR_FSIZE_D) {
	MPI_DOUBLE_PRECISION = MPIR_Init_basic_datatype( 
					    MPIR_LONGDOUBLE, MPIR_FSIZE_D );
	/* These aren't correct (we need a ldcomplex datatype in 
	   global_ops.c */
	MPIR_Setup_datatype( &MPIR_I_DCOMPLEX, MPIR_DOUBLE_COMPLEX, 
			    2 * MPIR_FSIZE_D );
	MPIR_I_DCOMPLEX.align = MPIR_FSIZE_D;

	MPI_Type_contiguous ( 2, MPI_DOUBLE_PRECISION, &MPIR_2double_dte );
	MPI_Type_commit ( &MPIR_2double_dte );
	MPIR_Type_permanent ( MPIR_2double_dte );
	}
#endif
    else {
	/* we'll have a problem with the reduce/scan ops */
	MPI_DOUBLE_PRECISION = MPIR_Init_basic_datatype( MPIR_DOUBLE, 
							   MPIR_FSIZE_D );
	MPIR_Setup_datatype( &MPIR_I_DCOMPLEX, MPIR_DOUBLE_COMPLEX, 
			    2 * MPIR_FSIZE_D );
	MPIR_I_DCOMPLEX.align = MPIR_FSIZE_D;

	MPI_Type_contiguous ( 2, MPI_DOUBLE, &MPIR_2double_dte );
	MPI_Type_commit ( &MPIR_2double_dte );
	MPIR_Type_permanent ( MPIR_2double_dte );
	}

    MPIR_Setup_base_datatype( MPI_LONG, &MPIR_I_LONG, 
			      MPIR_LONG, sizeof(long) );
    MPIR_Setup_base_datatype( MPI_SHORT, &MPIR_I_SHORT, 
			      MPIR_SHORT, sizeof(short) );
    MPIR_Setup_base_datatype( MPI_CHAR, &MPIR_I_CHAR, 
			      MPIR_CHAR, sizeof(char) );
    MPIR_Setup_base_datatype( MPI_BYTE, &MPIR_I_BYTE, 
			      MPIR_BYTE, sizeof(char) );
    MPIR_Setup_base_datatype( MPI_UNSIGNED_CHAR, &MPIR_I_UCHAR, 
			      MPIR_UCHAR, sizeof(char) );
    MPIR_Setup_base_datatype( MPI_UNSIGNED_SHORT, &MPIR_I_USHORT, 
			      MPIR_USHORT, sizeof(short) );
    MPIR_Setup_base_datatype( MPI_UNSIGNED_LONG, &MPIR_I_ULONG, 
			      MPIR_ULONG, sizeof(unsigned long) );
    MPIR_Setup_base_datatype( MPI_UNSIGNED, &MPIR_I_UINT, 
			      MPIR_UINT, sizeof(unsigned int) );
    MPIR_Setup_datatype( MPI_PACKED, MPIR_PACKED, 1 );
    MPIR_Setup_datatype( MPI_UB, MPIR_UB, 0 );
    MPI_UB->align	     = 1;
    MPI_UB->elements	     = 0;
    MPI_UB->count	     = 0;

    MPIR_Setup_datatype( MPI_LB, MPIR_LB, 0 );
    MPI_LB->align	     = 1;
    MPI_LB->elements	     = 0;
    MPI_LB->count	     = 0;


#if defined(HAVE_LONG_DOUBLE)
    MPIR_Setup_base_datatype( MPI_LONG_DOUBLE, &MPIR_I_LONG_DOUBLE, 
			      MPIR_LONGDOUBLE, sizeof(long double) );
#else
    MPIR_Setup_base_datatype( MPI_LONG_DOUBLE, &MPIR_I_LONG_DOUBLE, 
			      MPIR_LONGDOUBLE, 2*sizeof(double) );
#endif

    /* Initialize C and FORTRAN types for MINLOC and MAXLOC */
    MPI_Type_contiguous ( 2, &MPIR_I_COMPLEX, &MPIR_2complex_dte );
    MPI_Type_commit ( &MPIR_2complex_dte );
    MPIR_Type_permanent ( MPIR_2complex_dte );
    
    MPI_Type_contiguous ( 2, &MPIR_I_DCOMPLEX, &MPIR_2dcomplex_dte );
    MPI_Type_commit ( &MPIR_2dcomplex_dte );
    MPIR_Type_permanent ( MPIR_2dcomplex_dte );

    /* Initialize C & FORTRAN 2int type for MINLOC and MAXLOC */
    MPI_Type_contiguous ( 2, MPI_INT, &temptype );
    MPIR_Setup_complex_datatype( temptype, MPI_2INT );

    /* Note Fortran requires sizeof(INTEGER) == sizeof(REAL) */
    if (sizeof(int) != MPIR_FSIZE_R) {
	MPI_Type_contiguous ( 2, MPI_INTEGER, &MPI_2INTEGER );
	MPI_Type_commit ( &MPI_2INTEGER );
	MPIR_Type_permanent ( MPI_2INTEGER );
	}
    else
	MPI_2INTEGER          = MPI_2INT;

    /* Initialize C types for MINLOC and MAXLOC */
    /* I'm not sure that this is 100% portable */
    blln[0] = blln[1] = blln[2] = 1;
    type[1] = MPI_INT;   
    type[2] = MPI_UB;
    disp[0] = 0;

    type[0] = MPI_FLOAT;     
    disp[1] = (char *)&MPI_FLOAT_INT_var.loc - 
      (char *)&MPI_FLOAT_INT_var;
    disp[2] = sizeof(MPI_FLOAT_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &temptype );
    MPIR_Setup_complex_datatype( temptype, MPI_FLOAT_INT );

    type[0] = MPI_DOUBLE;
    disp[1] = (char *)&MPI_DOUBLE_INT_var.loc - 
      (char *)&MPI_DOUBLE_INT_var;
    disp[2] = sizeof(MPI_DOUBLE_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &temptype );
    MPIR_Setup_complex_datatype( temptype, MPI_DOUBLE_INT );

    type[0] = MPI_LONG;
    disp[1] = (char *)&MPI_LONG_INT_var.loc - 
      (char *)&MPI_LONG_INT_var;
    disp[2] = sizeof(MPI_LONG_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &temptype );
    MPIR_Setup_complex_datatype( temptype, MPI_LONG_INT );

    type[0] = MPI_SHORT;
    disp[1] = (char *)&MPI_SHORT_INT_var.loc - 
      (char *)&MPI_SHORT_INT_var;
    disp[2] = sizeof(MPI_SHORT_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &temptype );
    MPIR_Setup_complex_datatype( temptype, MPI_SHORT_INT );

#if defined(HAVE_LONG_DOUBLE)
    type[0] = MPI_LONG_DOUBLE;
    disp[1] = (char *)&MPI_LONG_DOUBLE_INT_var.loc - 
      (char *)&MPI_LONG_DOUBLE_INT_var;
    disp[2] = sizeof(MPI_LONG_DOUBLE_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &temptype );
    MPIR_Setup_complex_datatype( temptype, MPI_LONG_DOUBLE_INT );
#else
    /* use just double if long double not available */
    memcpy( MPI_LONG_DOUBLE_INT, MPI_DOUBLE_INT, sizeof(struct MPIR_DATATYPE));
#endif

#if defined(HAVE_LONG_LONG_INT)
    MPIR_Setup_base_datatype( MPI_LONG_LONG_INT, &MPIR_I_LONG_LONG_INT,
			      MPIR_LONGLONGINT, sizeof(long long int) );
#else
    MPIR_Setup_base_datatype( MPI_LONG_LONG_INT, &MPIR_I_LONG_LONG_INT,
			      MPIR_LONGLONGINT, 2*sizeof(long) );
#endif

    /* Set the values of the Fortran versions */
    /* Logical and character aren't portable in the code below */
#ifndef MPID_NO_FORTRAN
    DEBUG(PRINTF("[%d] About to setup Fortran datatypes\n", MPIR_tid);)
    /* Try to generate int1, int2, int4, real4, and real8 datatypes */
    MPIR_int1_dte  = 0;
    MPIR_int2_dte  = 0;
    MPIR_int4_dte  = 0;
    MPIR_real4_dte = 0;
    MPIR_real8_dte = 0;
    /* If these are changed to create new types, change the code in
       finalize.c to free the created types */
    if (sizeof(char) == 1)   MPIR_int1_dte = MPI_CHAR;
    if (sizeof(short) == 2)  MPIR_int2_dte = MPI_SHORT;
    if (sizeof(int) == 4)    MPIR_int4_dte = MPI_INT;
    if (sizeof(float) == 4)  MPIR_real4_dte = MPI_FLOAT;
    if (sizeof(double) == 8) MPIR_real8_dte = MPI_DOUBLE;

    /* These need to be converted into legal integer values for systems
       with 64 bit pointers */
#ifdef POINTER_64_BITS
	{ extern int MPIR_FromPointer();
	int i_integer = MPIR_FromPointer(MPI_INTEGER),
	    i_float   = MPIR_FromPointer(MPI_REAL),
            i_double  = MPIR_FromPointer(MPI_DOUBLE_PRECISION),
	    i_complex = MPIR_FromPointer(&MPIR_I_COMPLEX),
            i_dcomplex = MPIR_FromPointer(&MPIR_I_DCOMPLEX),
	    i_logical = MPIR_FromPointer(&MPIR_I_LOGICAL),
            i_char    = MPIR_FromPointer(MPI_CHAR),
	    i_byte    = MPIR_FromPointer(MPI_BYTE),
	    i_2int    = MPIR_FromPointer(MPI_2INTEGER),
	    i_2real   = MPIR_FromPointer(MPIR_2real_dte),
	    i_2double = MPIR_FromPointer(MPIR_2double_dte),
	    i_2complex= MPIR_FromPointer(MPIR_2complex_dte),
	    i_2dcomplex= MPIR_FromPointer(MPIR_2dcomplex_dte),
            i_int1    = MPIR_FromPointer(MPIR_int1_dte),
            i_int2    = MPIR_FromPointer(MPIR_int2_dte),
            i_int4    = MPIR_FromPointer(MPIR_int4_dte),
            i_real4   = MPIR_FromPointer(MPIR_real4_dte),
            i_real8   = MPIR_FromPointer(MPIR_real8_dte),
	    i_packed  = MPIR_FromPointer(MPI_PACKED),
	    i_ub      = MPIR_FromPointer(MPI_UB),
	    i_lb      = MPIR_FromPointer(MPI_LB);
	    
    mpir_init_fdtes_( &i_integer, &i_float, &i_double,
                     &i_complex, &i_dcomplex,
                     &i_logical, &i_char, 
                     &i_byte, &i_2int, &i_2real, 
		     &i_2double, &i_2complex,
                     &i_2dcomplex, &i_int1, &i_int2, &i_int4, 
                     &i_real4, &i_real8, &i_packed, &i_ub, &i_lb );
	}
#else    	
    {
    static MPI_Datatype MPIR_CHAR = MPI_CHAR,
                        MPIR_BYTE = MPI_BYTE,
                        MPIR_PACKED = MPI_PACKED,
                        MPIR_UB = MPI_UB,
                        MPIR_LB = MPI_LB,
                        MPIR_COMPLEX = &MPIR_I_COMPLEX,
                        MPIR_DCOMPLEX = &MPIR_I_DCOMPLEX,
                        MPIR_LOGICAL = &MPIR_I_LOGICAL;
    mpir_init_fdtes_( &MPI_INTEGER, &MPI_REAL, &MPI_DOUBLE_PRECISION,
                     &MPIR_COMPLEX, &MPIR_DCOMPLEX,
                     &MPIR_LOGICAL, &MPIR_CHAR, 
                     &MPIR_BYTE, &MPI_2INTEGER, &MPIR_2real_dte, 
		     &MPIR_2double_dte, &MPIR_2complex_dte,
                     &MPIR_2dcomplex_dte, &MPIR_int1_dte, 
                     &MPIR_int2_dte, &MPIR_int4_dte, &MPIR_real4_dte, 
                     &MPIR_real8_dte, &MPIR_PACKED, &MPIR_UB, &MPIR_LB );
    }
#endif
#endif

}

/*
 * Having initialized the datatype, we also need to free them
 */
void MPIR_Free_dtes()
{
    MPIR_Free_perm_type( &MPI_REAL );
    MPIR_Free_perm_type( &MPI_DOUBLE_PRECISION );
/*     MPI_Type_free( &MPIR_complex_dte );
    MPI_Type_free( &MPIR_dcomplex_dte ); 
    MPI_Type_free( &MPIR_logical_dte ); */
#ifndef MPID_NO_FORTRAN
#ifdef FOO
    /* Note that currently (see init.c), these are all copies of the
       existing C types, and so must not be freed (that will
       cause them to be freed twice) */
    if (MPIR_int1_dte)  MPI_Type_free( &MPIR_int1_dte );
    if (MPIR_int2_dte)  MPI_Type_free( &MPIR_int2_dte );
    if (MPIR_int4_dte)  MPI_Type_free( &MPIR_int4_dte );
    if (MPIR_real4_dte) MPI_Type_free( &MPIR_real4_dte );
    if (MPIR_real8_dte) MPI_Type_free( &MPIR_real8_dte );
#endif
#endif
    /* Free the parts of the structure types */
    MPIR_Type_free_struct( MPI_FLOAT_INT );
    MPIR_Type_free_struct( MPI_DOUBLE_INT );
    MPIR_Type_free_struct( MPI_LONG_INT );
    MPIR_Type_free_struct( MPI_SHORT_INT );
    MPIR_Type_free_struct( MPI_2INT );
    
    if (MPI_2INT != MPI_2INTEGER)
	MPIR_Free_perm_type( &MPI_2INTEGER );
    MPIR_Free_perm_type( &MPIR_2real_dte );
    MPIR_Free_perm_type( &MPIR_2double_dte );
    MPIR_Free_perm_type( &MPIR_2complex_dte );
    MPIR_Free_perm_type( &MPIR_2dcomplex_dte );

#if defined(HAVE_LONG_DOUBLE)
    {MPI_Datatype t = MPI_LONG_DOUBLE_INT;
    MPI_Type_free( &t );}
/*     MPI_Type_free( &MPI_LONG_DOUBLE ); */
#endif
#if defined(HAVE_LONG_LONG_INT)
  /*  MPI_Type_free( &MPI_LONG_LONG_INT ); */
#endif
    
}

void MPIR_Setup_datatype( new, type, size )
MPI_Datatype  new;
MPIR_NODETYPE type;
int           size;
{
  MPIR_SET_COOKIE(new,MPIR_DATATYPE_COOKIE)
  new->dte_type       = type;
  new->committed      = 1;
  new->is_contig      = 1;
  new->lb             = 0;
  new->ub             = size;
  new->extent         = size;
  new->size           = size;
  new->align          = size;
  new->stride         = size;
  new->elements       = 1;
  new->count          = 1;
  new->blocklen       = 1;
  new->basic          = 1;
  new->permanent      = 1;
  new->old_type       = new;
  new->ref_count      = 1;
}

/* This routine sets up a datatype that is specified as a small integer (val)
   and has a local reference
 */
static int base_size_miss = 0;
void MPIR_Setup_base_datatype( val, lval, type, size )
MPI_Datatype  val, lval;
MPIR_NODETYPE type;
int           size;
{
MPIR_Setup_datatype( lval, type, size );
/* Debugging test for sizes correct if encoded size implemented */
if (size != MPIR_DATATYPE_SIZE(val)) base_size_miss++;
MPIR_datatypes[(MPI_Aint)val] = lval;
}

/* This takes a datatype created with the datatype routines, copies the
   data into the "newtype" structure, and frees the old datatype storage 
 */
void MPIR_Setup_complex_datatype( oldtype, newtype )
MPI_Datatype oldtype, newtype;
{
    memcpy( newtype, oldtype, sizeof(struct MPIR_DATATYPE) );
    MPIR_SBfree ( MPIR_dtes, oldtype );
    MPIR_Type_permanent ( newtype );
    MPI_Type_commit( &newtype );
}

#ifdef MPIR_MEMDEBUG
MPI_Datatype MPIR_Init_basic_datatype_real( type, size, file, line )
MPIR_NODETYPE type;
int           size;
char          *file;
int           line;
#else
MPI_Datatype MPIR_Init_basic_datatype( type, size )
MPIR_NODETYPE type;
int           size;
#endif
{
    MPI_Datatype new;

#ifdef MPIR_MEMDEBUG
    new                 = (MPI_Datatype) MPIR_trmalloc( (unsigned)MPIR_dtes, 
							line, file );
#else
    new                 = (MPI_Datatype) MPIR_SBalloc( MPIR_dtes );
#endif
    if (!new) {
	MPIR_ERROR((MPI_Comm)0,MPI_ERR_EXHAUSTED,
		   "Could not allocated predefined datatypes");
    }
    MPIR_Setup_datatype( new, type, size );
    return new;
}

#ifndef MPID_NO_FORTRAN
/* 
   This routine computes the sizes of the Fortran data types.  It is 
   called from a Fortran routine that passes consequtive elements of 
   an array of the three Fortran types (character, real, double).
   Note that Fortran REQUIRES that integers have the same size as reals.
 */
void mpir_init_fsize_( r1, r2, d1, d2 )
/* char   *c1, *c2; */
float  *r1, *r2;
double *d1, *d2;
{
/* MPIR_FSIZE_C = (int)(c2 - c1); */
/* Because of problems in passing characters, we pick the most likely size
   for now */
MPIR_FSIZE_C = sizeof(char);
MPIR_FSIZE_R = (int)( (char*)r2 - (char*)r1 );
MPIR_FSIZE_D = (int)( (char*)d2 - (char*)d1 );
}
#endif /* MPID_NO_FORTRAN */
