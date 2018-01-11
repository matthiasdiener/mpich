/*
 *  $Id: initutil.c,v 1.1 1995/09/27 14:42:03 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: initutil.c,v 1.1 1995/09/27 14:42:03 gropp Exp $";
#endif /* lint */

/* 
   define MPID_NO_FORTRAN if the Fortran interface is not to be supported
   (perhaps because there is no Fortran compiler)
 */
#include "mpiimpl.h"
#include "mpisys.h"

#ifndef PATCHLEVEL_SUBMINOR
#define PATCHLEVEL_SUBMINOR 0
#endif

/* #define DEBUG(a) {a}  */
#define DEBUG(a)

#ifdef FORTRANCAPS
#define mpir_init_fdtes_ MPIR_INIT_FDTES
#define mpir_init_fcm_   MPIR_INIT_FCM
#define mpir_init_fop_   MPIR_INIT_FOP
#define mpir_init_flog_  MPIR_INIT_FLOG
#define mpir_init_bottom_ MPIR_INIT_BOTTOM
#define mpir_init_fattr_  MPIR_INIT_FATTR
#define mpir_init_fsize_  MPIR_INIT_FSIZE
#define mpir_get_fsize_   MPIR_GET_FSIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpir_init_fdtes_ mpir_init_fdtes__
#define mpir_init_fcm_   mpir_init_fcm__
#define mpir_init_fop_   mpir_init_fop__
#define mpir_init_flog_  mpir_init_flog__
#define mpir_init_bottom_ mpir_init_bottom__
#define mpir_init_fattr_  mpir_init_fattr__
#define mpir_init_fsize_  mpir_init_fsize__
#define mpir_get_fsize_   mpir_get_fsize__
#elif !defined(FORTRANUNDERSCORE)
#define mpir_init_fdtes_ mpir_init_fdtes
#define mpir_init_fcm_   mpir_init_fcm
#define mpir_init_fop_   mpir_init_fop
#define mpir_init_flog_  mpir_init_flog
#define mpir_init_bottom_ mpir_init_bottom
#define mpir_init_fattr_  mpir_init_fattr
#define mpir_init_fsize_  mpir_init_fsize
#define mpir_get_fsize_   mpir_get_fsize
#endif

/* Global memory management variables for fixed-size blocks */
void *MPIR_shandles;        /* sbcnst MPIR_SHANDLES */
void *MPIR_rhandles;        /* sbcnst MPIR_RHANDLES */
void *MPIR_errhandlers;  /* sbcnst Error handlers */
void *MPIR_dtes;      /* sbcnst datatype elements */
void *MPIR_qels;      /* sbcnst queue elements */
void *MPIR_fdtels; /* sbcnst flat datatype elements */
void *MPIR_hbts;   /* sbcnst height balanced tree roots for cacheing */
void *MPIR_hbt_els;/* sbcnst height balanced tree nodes for cacheing */
void *MPIR_topo_els;/* sbcnst topology elements */

/* Global queues */
MPIR_QHDR MPIR_posted_recvs;
MPIR_QHDR MPIR_unexpected_recvs;

/* Static space for datatypes */
struct MPIR_DATATYPE MPIR_I_CHAR, MPIR_I_SHORT, MPIR_I_INT, MPIR_I_LONG,
                            MPIR_I_UCHAR, MPIR_I_USHORT, MPIR_I_UINT, 
                            MPIR_I_ULONG, MPIR_I_FLOAT, MPIR_I_DOUBLE, 
                            MPIR_I_LONG_DOUBLE, MPIR_I_LONG_DOUBLE_INT,
                            MPIR_I_BYTE, MPIR_I_PACKED, MPIR_I_UB, MPIR_I_LB,
                            MPIR_I_LONG_LONG_INT, MPIR_I_2INTEGER, 
                            MPIR_I_FLOAT_INT, MPIR_I_DOUBLE_INT, 
                            MPIR_I_LONG_INT, MPIR_I_SHORT_INT, MPIR_I_2INT,
                            MPIR_I_REAL, MPIR_I_DOUBLE_PRECISION,
                            MPIR_I_COMPLEX, MPIR_I_DCOMPLEX, 
                            MPIR_I_LONG_DOUBLE, MPIR_I_LONG_LONG_INT, 
                            MPIR_I_LOGICAL;

/* Global pre-assigned datatypes */
MPI_Datatype MPI_LONG_DOUBLE;
MPI_Datatype MPI_LONG_LONG_INT;

MPI_Datatype MPIR_Init_basic_datatype( );
void MPIR_Setup_datatype( );
void MPIR_Setup_complex_datatype( );

/* C Datatypes for MINLOC and MAXLOC functions */
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

MPI_Datatype MPI_LONG_DOUBLE_INT;
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

/* Global communicators.  Initialize as null in case we fail during startup */
MPI_Comm MPI_COMM_SELF = 0, MPI_COMM_WORLD = 0;
MPI_Group MPI_GROUP_EMPTY = 0;

/* Global MPIR process id (from device) */
int MPIR_tid;

/* Predefined combination functions */
MPI_Op MPI_MAX, MPI_MIN, MPI_SUM, MPI_PROD, MPI_LAND, MPI_BAND, 
               MPI_LOR, MPI_BOR, MPI_LXOR, MPI_BXOR, MPI_MAXLOC, MPI_MINLOC;

/* Permanent attributes */
int MPI_TAG_UB, MPI_HOST, MPI_IO, MPI_WTIME_IS_GLOBAL;
/* Places to hold the values of the attributes */
static int MPI_TAG_UB_VAL, MPI_HOST_VAL, MPI_IO_VAL, MPI_WTIME_IS_GLOBAL_VAL;
/* Fortran versions of the names */
int MPIR_TAG_UB, MPIR_HOST, MPIR_IO, MPIR_WTIME_IS_GLOBAL;

/* Command-line flags */
int MPIR_Print_queues = 0;

/* Fortran logical values */
int MPIR_F_TRUE, MPIR_F_FALSE;

/* 
 Location of the Fortran marker for MPI_BOTTOM.  The Fortran wrappers
 must detect the use of this address and replace it with MPI_BOTTOM.
 This is done by the macro MPIR_F_PTR.
 */
void *MPIR_F_MPI_BOTTOM = 0;

/* Sizes of Fortran types; computed when initialized. */
static int MPIR_FSIZE_C = 0;
static int MPIR_FSIZE_R = 0;
static int MPIR_FSIZE_D = 0;

/* 
   If I want to use __STDC__, I need to include the full prototypes of
   these functions ... 
 */
extern void MPIR_MAXF();
extern void MPIR_MINF();
extern void MPIR_SUM();
extern void MPIR_PROD();
extern void MPIR_LAND();
extern void MPIR_BAND();
extern void MPIR_LOR();
extern void MPIR_BOR();
extern void MPIR_LXOR();
extern void MPIR_BXOR();
extern void MPIR_MAXLOC();
extern void MPIR_MINLOC();

#if (defined(__STDC__) || defined(__cpluscplus))
extern MPI_Handler_function MPIR_Errors_are_fatal;
extern MPI_Handler_function MPIR_Errors_return;
extern MPI_Handler_function MPIR_Errors_warn;
/* ( MPI_Comm *, int *, ... ); */
#else
extern void MPIR_Errors_are_fatal();
extern void MPIR_Errors_return();
extern void MPIR_Errors_warn();
#endif

MPI_Errhandler MPI_ERRORS_ARE_FATAL, MPI_ERRORS_RETURN, 
       MPIR_ERRORS_WARN;

#ifdef MPIR_MEMDEBUG
/* Convert the datatype init routine to pass through the file and line
   so that we can more easily find allocation problems */
#define MPIR_Init_basic_datatype( type, size ) \
        MPIR_Init_basic_datatype_real( type, size, __FILE__, __LINE__ )
MPI_Datatype MPIR_Init_basic_datatype_real();
#endif

/* 
   Initialization function for topology code.
 */
extern void MPIR_Topology_init();

/*
   MPIR_Init - Initialize the MPI execution environment

   Input Parameters:
.  argc - Pointer to the number of arguments 
.  argv - Pointer to the argument vector

   See MPI_Init for the description of the input to this routine.

   This routine is in a separate file form MPI_Init to allow profiling 
   libraries to not replace MPI_Init; without this, you can get errors
   from the linker about multiply defined libraries.

@*/
int MPIR_Init(argc,argv)
int  *argc;
char ***argv;
{
    int            size;
    MPI_Datatype   type[3], temptype;
    MPI_Aint       disp[3];
    int            blln[3];
    void           *ADIctx;

    if (MPIR_Has_been_initialized) 
    return 
        MPIR_ERROR( (MPI_Comm)0, MPI_ERR_INIT, "Can not MPI_INIT again" );

    ADIctx = MPID_INIT( argc, argv );

    DEBUG(MPID_Myrank( ADIctx, &MPIR_tid);)
    DEBUG(printf("[%d] About to do allocations\n", MPIR_tid);)

    /* initialize topology code */
    MPIR_Topology_init();

    /* initialize memory allocation data structures */
    MPIR_shandles   = MPIR_SBinit( sizeof( MPIR_SHANDLE ), 100, 100 );
    MPIR_rhandles   = MPIR_SBinit( sizeof( MPIR_RHANDLE ), 100, 100 );
    MPIR_errhandlers= MPIR_SBinit( sizeof( struct MPIR_Errhandler ), 10, 10 );
    MPIR_dtes       = MPIR_SBinit( sizeof( struct MPIR_DATATYPE ), 100, 100 );
    MPIR_qels       = MPIR_SBinit( sizeof( MPIR_QEL ), 100, 100 );
    MPIR_fdtels     = MPIR_SBinit( sizeof( MPIR_FDTEL ), 100, 100 );
    MPIR_hbts       = MPIR_SBinit( sizeof( MPIR_HBT ), 5, 5 );
    MPIR_hbt_els    = MPIR_SBinit( sizeof( MPIR_HBT_node ), 20, 20);
    MPIR_topo_els   = MPIR_SBinit( sizeof( MPIR_TOPOLOGY ),  4,  4);

    /* set up pre-defined data types */
    DEBUG(printf("[%d] About to create datatypes\n", MPIR_tid);)

    MPIR_Setup_datatype( MPI_INT, MPIR_INT, sizeof(int) );

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
    MPIR_Setup_datatype( MPI_DOUBLE, MPIR_DOUBLE, sizeof(double) );
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
    MPIR_Setup_datatype( MPI_FLOAT, MPIR_FLOAT, sizeof(float) );
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

    MPIR_Setup_datatype( MPI_LONG, MPIR_LONG, sizeof(long) );
    MPIR_Setup_datatype( MPI_SHORT, MPIR_SHORT, sizeof(short) );
    MPIR_Setup_datatype( MPI_CHAR, MPIR_CHAR, sizeof(char) );
    MPIR_Setup_datatype( MPI_BYTE, MPIR_BYTE, sizeof(char) );
    MPIR_Setup_datatype( MPI_UNSIGNED_CHAR, MPIR_UCHAR, sizeof(char) );
    MPIR_Setup_datatype( MPI_UNSIGNED_SHORT, MPIR_USHORT, sizeof(short) );
    MPIR_Setup_datatype( MPI_UNSIGNED_LONG, MPIR_ULONG, 
			 sizeof(unsigned long) );
    MPIR_Setup_datatype( MPI_UNSIGNED, MPIR_UINT, sizeof(unsigned int) );
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
    MPI_LONG_DOUBLE	= MPIR_Init_basic_datatype( MPIR_LONGDOUBLE, 
						    sizeof ( long double ) );
#else
    /* Unsupported are null */
    MPI_LONG_DOUBLE = 0;
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
    MPI_Type_struct ( 3, blln, disp, type, &MPI_LONG_DOUBLE_INT );
    MPIR_Type_permanent ( MPI_LONG_DOUBLE_INT );
    MPI_Type_commit ( &MPI_LONG_DOUBLE_INT );
#else
    MPI_LONG_DOUBLE_INT = 0;
#endif

#if defined(HAVE_LONG_LONG_INT)
    MPI_LONG_LONG_INT	= MPIR_Init_basic_datatype( MPIR_LONGLONGINT, 
						    sizeof ( long long int ) );
#else
    /* Unsupported are null */
    MPI_LONG_LONG_INT = 0;
#endif

    /* Set the values of the Fortran versions */
    /* Logical and character aren't portable in the code below */
#ifndef MPID_NO_FORTRAN
    DEBUG(printf("[%d] About to setup Fortran datatypes\n", MPIR_tid);)
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
    /* initialize queues */
    DEBUG(printf("[%d] About to setup message queues\n", MPIR_tid);)
    MPIR_posted_recvs.first        = MPIR_posted_recvs.last        = NULL;
    MPIR_posted_recvs.maxlen       = MPIR_posted_recvs.currlen     = 0; 
    MPID_THREAD_DS_LOCK_INIT(&MPIR_posted_recvs)

    MPIR_unexpected_recvs.first    = MPIR_unexpected_recvs.last    = NULL;
    MPIR_unexpected_recvs.maxlen   = MPIR_unexpected_recvs.currlen = 0;
    MPID_THREAD_DS_LOCK_INIT(&MPIR_unexpected_recvs)

    /* Create Error handlers */
    MPI_Errhandler_create( MPIR_Errors_are_fatal, &MPI_ERRORS_ARE_FATAL );
    MPI_Errhandler_create( MPIR_Errors_return, &MPI_ERRORS_RETURN );
    MPI_Errhandler_create( MPIR_Errors_warn, &MPIR_ERRORS_WARN );
    
    /* GROUP_EMPTY is a valid empty group */
    DEBUG(printf("[%d] About to create groups and communicators\n", MPIR_tid);)
    MPI_GROUP_EMPTY     = MPIR_CreateGroup(0);
    MPI_GROUP_EMPTY->permanent = 1;

    MPI_COMM_WORLD		   = NEW(struct MPIR_COMMUNICATOR);    
    MPIR_SET_COOKIE(MPI_COMM_WORLD,MPIR_COMM_COOKIE)
    MPI_COMM_WORLD->comm_type	   = MPIR_INTRA;
    MPI_COMM_WORLD->ADIctx	   = ADIctx;
    MPID_Mysize( ADIctx, &size );
    MPID_Myrank( ADIctx, &MPIR_tid );
    MPI_COMM_WORLD->group	   = MPIR_CreateGroup( size );
    MPIR_SetToIdentity( MPI_COMM_WORLD->group );
    MPIR_Group_dup ( MPI_COMM_WORLD->group, &(MPI_COMM_WORLD->local_group) );
    MPI_COMM_WORLD->local_rank	   = MPI_COMM_WORLD->local_group->local_rank;
    MPI_COMM_WORLD->lrank_to_grank = MPI_COMM_WORLD->group->lrank_to_grank;
    MPI_COMM_WORLD->np		   = MPI_COMM_WORLD->group->np;
    MPI_COMM_WORLD->send_context   = MPIR_WORLD_PT2PT_CONTEXT;
    MPI_COMM_WORLD->recv_context   = MPIR_WORLD_PT2PT_CONTEXT;
    MPI_COMM_WORLD->error_handler  = MPI_ERRORS_ARE_FATAL;
    MPI_ERRORS_ARE_FATAL->ref_count ++;
    MPI_COMM_WORLD->ref_count	   = 1;
    MPI_COMM_WORLD->permanent	   = 1;
    MPIR_Attr_create_tree ( MPI_COMM_WORLD );
    MPI_COMM_WORLD->comm_cache	   = 0;
    MPIR_Comm_make_coll ( MPI_COMM_WORLD, MPIR_INTRA );

    /* Predefined attributes for MPI_COMM_WORLD */
    DEBUG(printf("[%d] About to create keyvals\n", MPIR_tid);)
    MPI_Keyval_create( (int (*)())0, (int (*)())0, &MPI_TAG_UB, (void *)0 );
    MPI_Keyval_create( (int (*)())0, (int (*)())0, &MPI_HOST,   (void *)0 );
    MPI_Keyval_create( (int (*)())0, (int (*)())0, &MPI_IO,     (void *)0 );
    MPI_Keyval_create( (int (*)())0, (int (*)())0, &MPI_WTIME_IS_GLOBAL, 
		       (void *)0 );
    MPI_TAG_UB_VAL = MPID_TAG_UB;
    MPI_HOST_VAL   = MPI_PROC_NULL;
    /* The following isn't strictly correct, but I'm going to leave it
       in for now.  I've tried to make this correct for a few systems
       for which I know the answer.  
     */
    /* MPI_PROC_NULL is the correct answer for IBM MPL version 1 and
       perhaps for some other systems */
    /*     MPI_IO_VAL = MPI_PROC_NULL; */
    MPI_IO_VAL = MPI_ANY_SOURCE;
    /* The C versions - pass the address of the variable containing the 
       value */
    MPI_Attr_put( MPI_COMM_WORLD, MPI_TAG_UB, (void*)&MPI_TAG_UB_VAL );
    MPI_Attr_put( MPI_COMM_WORLD, MPI_HOST,   (void*)&MPI_HOST_VAL );
    MPI_Attr_put( MPI_COMM_WORLD, MPI_IO,     (void*)&MPI_IO_VAL );

    /* Do the Fortran versions - Pass the actual value.  Note that these
       use MPIR_Keyval_create with the "is_fortran" flag set. 
       If you change these; change the removal in finalize.c. */
    MPIR_Keyval_create( (int (*)())0, (int (*)())0, &MPIR_TAG_UB, 
		        (void *)0, 1 );
    MPIR_Keyval_create( (int (*)())0, (int (*)())0, &MPIR_HOST, 
		        (void *)0, 1 );
    MPIR_Keyval_create( (int (*)())0, (int (*)())0, &MPIR_IO, 
		        (void *)0, 1 );
    MPIR_Keyval_create( (int (*)())0, (int (*)())0, &MPIR_WTIME_IS_GLOBAL, 
		        (void *)0, 1 );
    MPI_Attr_put( MPI_COMM_WORLD, MPIR_TAG_UB, (void*)MPI_TAG_UB_VAL );
    MPI_Attr_put( MPI_COMM_WORLD, MPIR_HOST,   (void*)MPI_HOST_VAL );
    MPI_Attr_put( MPI_COMM_WORLD, MPIR_IO,     (void*)MPI_IO_VAL );

/* Add the flag on whether the timer is global */
#ifdef MPID_Wtime_is_global
    MPI_WTIME_IS_GLOBAL_VAL = MPID_Wtime_is_global();
#else
    MPI_WTIME_IS_GLOBAL_VAL = 0;
#endif    
    MPI_Attr_put( MPI_COMM_WORLD, MPI_WTIME_IS_GLOBAL, 
		  (void *)&MPI_WTIME_IS_GLOBAL_VAL );
    MPI_Attr_put( MPI_COMM_WORLD, MPIR_WTIME_IS_GLOBAL, 
		  (void *)MPI_WTIME_IS_GLOBAL_VAL );
/* Make these permanent.  Must do this AFTER the values are set (because
   changing a value of a permanent attribute is an error) */
#ifdef INT_LT_POINTER
    ((MPIR_Attr_key *)MPIR_ToPointer(MPI_TAG_UB))->permanent	       = 1;
    ((MPIR_Attr_key *)MPIR_ToPointer(MPI_HOST))->permanent	       = 1;
    ((MPIR_Attr_key *)MPIR_ToPointer(MPI_IO))->permanent	       = 1;
    ((MPIR_Attr_key *)MPIR_ToPointer(MPI_WTIME_IS_GLOBAL))->permanent  = 1;
    ((MPIR_Attr_key *)MPIR_ToPointer(MPIR_TAG_UB))->permanent	       = 1;
    ((MPIR_Attr_key *)MPIR_ToPointer(MPIR_HOST))->permanent	       = 1;
    ((MPIR_Attr_key *)MPIR_ToPointer(MPIR_IO))->permanent	       = 1;
    ((MPIR_Attr_key *)MPIR_ToPointer(MPIR_WTIME_IS_GLOBAL))->permanent = 1;
#else
    ((MPIR_Attr_key *)(MPI_TAG_UB))->permanent		 = 1;
    ((MPIR_Attr_key *)(MPI_HOST))->permanent		 = 1;
    ((MPIR_Attr_key *)(MPI_IO))->permanent		 = 1;
    ((MPIR_Attr_key *)(MPI_WTIME_IS_GLOBAL))->permanent	 = 1;
    ((MPIR_Attr_key *)(MPIR_TAG_UB))->permanent		 = 1;
    ((MPIR_Attr_key *)(MPIR_HOST))->permanent		 = 1;
    ((MPIR_Attr_key *)(MPIR_IO))->permanent		 = 1;
    ((MPIR_Attr_key *)(MPIR_WTIME_IS_GLOBAL))->permanent = 1;
#endif

    /* COMM_SELF is the communicator consisting only of myself */
    MPI_COMM_SELF			    = NEW(struct MPIR_COMMUNICATOR);
    MPIR_SET_COOKIE(MPI_COMM_SELF,MPIR_COMM_COOKIE)
    MPI_COMM_SELF->comm_type		    = MPIR_INTRA;
    MPI_COMM_SELF->group		    = MPIR_CreateGroup( 1 );
    MPI_COMM_SELF->group->local_rank	    = 0;
    MPI_COMM_SELF->group->lrank_to_grank[0] = MPIR_tid;
    MPIR_Group_dup ( MPI_COMM_SELF->group, &(MPI_COMM_SELF->local_group) );
    MPI_COMM_SELF->local_rank		    = 
	MPI_COMM_SELF->local_group->local_rank;
    MPI_COMM_SELF->lrank_to_grank	    = 
	MPI_COMM_SELF->group->lrank_to_grank;
    MPI_COMM_SELF->np			    = MPI_COMM_SELF->group->np;
    MPI_COMM_SELF->send_context		    = MPIR_SELF_PT2PT_CONTEXT;
    MPI_COMM_SELF->recv_context		    = MPIR_SELF_PT2PT_CONTEXT;
    MPI_COMM_SELF->error_handler	    = MPI_ERRORS_ARE_FATAL;
    MPI_ERRORS_ARE_FATAL->ref_count ++;
    MPI_COMM_SELF->ref_count		    = 1;
    MPI_COMM_SELF->permanent		    = 1;
    MPIR_Attr_create_tree ( MPI_COMM_SELF );
    MPI_COMM_SELF->comm_cache		    = 0;
    MPIR_Comm_make_coll ( MPI_COMM_SELF, MPIR_INTRA );

    /* Predefined combination functions */
    DEBUG(printf("[%d] About to create combination functions\n", MPIR_tid);)
    MPI_Op_create( MPIR_MAXF,   1, &MPI_MAX );
	MPI_MAX->permanent = 1;
    MPI_Op_create( MPIR_MINF,   1, &MPI_MIN );
	MPI_MIN->permanent = 1;
    MPI_Op_create( MPIR_SUM,    1, &MPI_SUM );
	MPI_SUM->permanent = 1;
    MPI_Op_create( MPIR_PROD,   1, &MPI_PROD );
	MPI_PROD->permanent = 1;
    MPI_Op_create( MPIR_LAND,   1, &MPI_LAND );
	MPI_LAND->permanent = 1;
    MPI_Op_create( MPIR_BAND,   1, &MPI_BAND );
	MPI_BAND->permanent = 1;
    MPI_Op_create( MPIR_LOR,    1, &MPI_LOR );
	MPI_LOR->permanent = 1;
    MPI_Op_create( MPIR_BOR,    1, &MPI_BOR );
	MPI_BOR->permanent = 1;
    MPI_Op_create( MPIR_LXOR,   1, &MPI_LXOR );
	MPI_LXOR->permanent = 1;
    MPI_Op_create( MPIR_BXOR,   1, &MPI_BXOR );
	MPI_BXOR->permanent = 1;
    MPI_Op_create( MPIR_MAXLOC, 1, &MPI_MAXLOC );
	MPI_MAXLOC->permanent = 1;
    MPI_Op_create( MPIR_MINLOC, 1, &MPI_MINLOC );
	MPI_MINLOC->permanent = 1;
#ifndef MPID_NO_FORTRAN
    DEBUG(printf("[%d] About to setup Fortran functions\n", MPIR_tid);)
#ifdef POINTER_64_BITS
	{ extern int MPIR_FromPointer();

	int i_max = MPIR_FromPointer(MPI_MAX),
	    i_min = MPIR_FromPointer(MPI_MIN),
	    i_sum = MPIR_FromPointer(MPI_SUM),
	    i_prod = MPIR_FromPointer(MPI_PROD),
	    i_land = MPIR_FromPointer(MPI_LAND),
	    i_band = MPIR_FromPointer(MPI_BAND),
	    i_lor  = MPIR_FromPointer(MPI_LOR),
	    i_bor  = MPIR_FromPointer(MPI_BOR),
	    i_lxor = MPIR_FromPointer(MPI_LXOR),
	    i_bxor = MPIR_FromPointer(MPI_BXOR),
	    i_maxloc = MPIR_FromPointer(MPI_MAXLOC),
	    i_minloc = MPIR_FromPointer(MPI_MINLOC),
	    i_err_fatal = MPIR_FromPointer( MPI_ERRORS_ARE_FATAL ),
	    i_err_ret   = MPIR_FromPointer( MPI_ERRORS_RETURN );
	  mpir_init_fop_( &i_max, &i_min, &i_sum, &i_prod, 
                    &i_land, &i_band,
                    &i_lor, &i_bor, &i_lxor, &i_bxor, 
                    &i_maxloc, &i_minloc, &i_err_fatal, &i_err_ret );
	}
#else
    mpir_init_fop_( &MPI_MAX, &MPI_MIN, &MPI_SUM, &MPI_PROD, 
                    &MPI_LAND, &MPI_BAND,
                    &MPI_LOR, &MPI_BOR, &MPI_LXOR, &MPI_BXOR, 
                    &MPI_MAXLOC, &MPI_MINLOC, &MPI_ERRORS_ARE_FATAL,
		    &MPI_ERRORS_RETURN );
#endif
#endif

#ifndef MPID_NO_FORTRAN
    mpir_init_flog_( &MPIR_F_TRUE, &MPIR_F_FALSE );
#endif

#ifndef MPID_NO_FORTRAN
    DEBUG(printf("[%d] About to setup Fortran communicators\n", MPIR_tid);)
#ifdef POINTER_64_BITS
	{ extern int MPIR_FromPointer();
	  int i_world = MPIR_FromPointer(MPI_COMM_WORLD),
	      i_self  = MPIR_FromPointer(MPI_COMM_SELF),
	      i_empty = MPIR_FromPointer(MPI_GROUP_EMPTY);
	  mpir_init_fcm_( &i_world, &i_self, &i_empty );
	  }
#else
    mpir_init_fcm_( &MPI_COMM_WORLD, &MPI_COMM_SELF, &MPI_GROUP_EMPTY );
#endif
    DEBUG(printf("[%d] About to setup Fortran attributes\n", MPIR_tid););
    mpir_init_fattr_( &MPIR_TAG_UB, &MPIR_HOST, &MPIR_IO );
#endif
    DEBUG(printf("[%d] About to search for argument list options\n",MPIR_tid);)
    /* Search for "-mpi debug" options etc.  We need a better interface.... */
    if (argv && *argv) {
	int i;
	for (i=1; i<*argc; i++) {
	    if ((*argv)[i]) {
		if (strcmp( (*argv)[i], "-mpiqueue" ) == 0) {
		    MPIR_Print_queues = 1;
		    (*argv)[i] = 0;
		    }
		else if (strcmp((*argv)[i],"-mpiversion" ) == 0) {
		    char ADIname[128];
		    MPID_Version_name( ADIctx, ADIname );
		    printf( "MPI model implementation %4.2f.%d., %s\n", 
			   PATCHLEVEL, PATCHLEVEL_SUBMINOR, ADIname );
		    printf( "Configured with %s\n", CONFIGURE_ARGS_CLEAN );
		    (*argv)[i] = 0;
		    }
		else if (strcmp((*argv)[i],"-mpipktsize" ) == 0) {
		    int len;
		    (*argv)[i] = 0;
		    i++;
		    if (i <*argc) {
			len = atoi( (*argv)[i] );
			MPID_SetPktSize( len );
			(*argv)[i] = 0;
			}
		    else {
			printf( "Missing argument for -mpipktsize\n" );
			}
		    }
#ifdef HAVE_NICE
		else if (strcmp((*argv)[i],"-mpinice" ) == 0) {
		    int niceincr;
		    (*argv)[i] = 0;
		    i++;
		    if (i <*argc) {
			niceincr = atoi( (*argv)[i] );
			nice(niceincr);
			(*argv)[i] = 0;
			}
		    else {
			printf( "Missing argument for -mpinice\n" );
			}
		    }
#endif
#ifdef MPE_USE_EXTENSIONS
		else if (strcmp((*argv)[i],"-mpedbg" ) == 0) {
		    MPE_Errors_call_dbx_in_xterm( (*argv)[0], (char *)0 ); 
		    MPE_Signals_call_debugger();
		    (*argv)[i] = 0;
		    }
#endif
#ifdef MPID_HAS_DEBUG
		else if (strcmp((*argv)[i],"-mpichdebug") == 0) {
		    MPID_SetDebugFlag( ADIctx, 1 );
		    (*argv)[i] = 0;
		    }
		else if (strcmp((*argv)[i],"-mpidbfile" ) == 0) {
		    MPID_SetDebugFlag( ADIctx, 1 );
		    (*argv)[i] = 0;
		    i++;
		    if (i <*argc) {
			MPID_SetDebugFile( (*argv)[i] );
			(*argv)[i] = 0;
			}
		    else {
			printf( "Missing filename for -mpdbfile\n" );
			}
		    }
		else if (strcmp((*argv)[i],"-chmemdebug" ) == 0) {
		    MPID_SetSpaceDebugFlag( 1 );
		    (*argv)[i] = 0;
		    }
		else if (strcmp((*argv)[i],"-mpichmsg" ) == 0) {
		    MPID_SetMsgDebugFlag( ADIctx, 1 );
		    (*argv)[i] = 0;
		    }
		else if (strcmp((*argv)[i],"-mpitrace" ) == 0) {
		    (*argv)[i] = 0;
		    i++;
		    if (i <*argc) {
			MPID_Set_tracefile( (*argv)[i] );
			(*argv)[i] = 0;
			}
		    else {
			printf( "Missing filename for -mpitrace\n" );
			}
		    }
#endif
#ifdef MPIR_MEMDEBUG
		else if (strcmp((*argv)[i],"-mpimem" ) == 0) {
		    MPIR_trDebugLevel( 1 );
		    }
#endif
		}
	    }
	/* Remove the null arguments */
	MPIR_ArgSqueeze( argc, *argv );
	}

    /* barrier */
    MPIR_Has_been_initialized = 1;

    DEBUG(printf("[%d] About to exit from MPI_Init\n", MPIR_tid);)
    return MPI_SUCCESS;
}

void MPIR_Setup_datatype( new, type, size )
MPI_Datatype  new;
MPIR_NODETYPE type;
int           size;
{
  MPIR_SET_COOKIE(new,MPIR_DATATYPE_COOKIE)
  new->dte_type       = type;
  new->committed      = MPIR_YES;
  new->is_contig      = MPIR_YES;
  new->lb             = 0;
  new->ub             = size;
  new->extent         = size;
  new->size           = size;
  new->align          = size;
  new->stride         = size;
  new->elements       = 1;
  new->count          = 1;
  new->blocklen       = 1;
  new->basic          = MPIR_YES;
  new->permanent      = MPIR_YES;
  new->old_type       = new;
  new->ref_count      = 1;
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
  MPIR_Setup_datatype( new, type, size );
  return new;
}

#ifndef MPID_NO_FORTRAN
/* 
   This routine is CALLED by MPIR_init_fcm to provide the address of 
   the Fortran MPI_BOTTOM to C 
 */ 
void mpir_init_bottom_( p )
void *p;
{
MPIR_F_MPI_BOTTOM = p;
}

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
