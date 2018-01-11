/*
 *  $Id: init.c,v 1.64 1994/12/11 16:52:02 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: init.c,v 1.64 1994/12/11 16:52:02 gropp Exp $";
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
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpir_init_fdtes_ mpir_init_fdtes__
#define mpir_init_fcm_   mpir_init_fcm__
#define mpir_init_fop_   mpir_init_fop__
#define mpir_init_flog_  mpir_init_flog__
#define mpir_init_bottom_ mpir_init_bottom__
#define mpir_init_fattr_  mpir_init_fattr__
#elif !defined(FORTRANUNDERSCORE)
#define mpir_init_fdtes_ mpir_init_fdtes
#define mpir_init_fcm_   mpir_init_fcm
#define mpir_init_fop_   mpir_init_fop
#define mpir_init_flog_  mpir_init_flog
#define mpir_init_bottom_ mpir_init_bottom
#define mpir_init_fattr_  mpir_init_fattr
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

/* Global pre-assigned datatypes */
MPI_Datatype MPI_INT;
MPI_Datatype MPI_FLOAT;
MPI_Datatype MPI_DOUBLE;
MPI_Datatype MPI_LONG;
MPI_Datatype MPIR_complex_dte; 
MPI_Datatype MPIR_dcomplex_dte;
MPI_Datatype MPI_SHORT;
MPI_Datatype MPI_CHAR;
MPI_Datatype MPI_BYTE;
MPI_Datatype MPI_UNSIGNED_CHAR;
MPI_Datatype MPI_UNSIGNED_SHORT;
MPI_Datatype MPI_UNSIGNED_LONG;
MPI_Datatype MPI_UNSIGNED;
MPI_Datatype MPI_LONG_DOUBLE;
MPI_Datatype MPI_LONG_LONG_INT;
MPI_Datatype MPI_PACKED;
MPI_Datatype MPI_UB;
MPI_Datatype MPI_LB;
MPI_Datatype MPIR_Init_basic_datatype( );

/* C Datatypes for MINLOC and MAXLOC functions */
MPI_Datatype MPI_FLOAT_INT;
typedef struct {
  float  var;
  int    loc;
} MPI_FLOAT_INT_struct;
MPI_FLOAT_INT_struct MPI_FLOAT_INT_var;

MPI_Datatype MPI_DOUBLE_INT;
typedef struct {
  double var;
  int    loc;
} MPI_DOUBLE_INT_struct;
MPI_DOUBLE_INT_struct MPI_DOUBLE_INT_var;

MPI_Datatype MPI_LONG_INT;
typedef struct {
  long   var;
  int    loc;
} MPI_LONG_INT_struct;
MPI_LONG_INT_struct MPI_LONG_INT_var;

MPI_Datatype MPI_SHORT_INT;
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
MPI_Datatype MPIR_logical_dte;
MPI_Datatype MPIR_int1_dte;
MPI_Datatype MPIR_int2_dte;
MPI_Datatype MPIR_int4_dte;
MPI_Datatype MPIR_real4_dte;
MPI_Datatype MPIR_real8_dte;
/* FORTRAN Datatypes for MINLOC and MAXLOC functions */
MPI_Datatype MPI_2INTEGER; /* May be the same as MPI_2INT */
MPI_Datatype MPI_2INT; /* For C also */
MPI_Datatype MPIR_2real_dte;
MPI_Datatype MPIR_2double_dte;
MPI_Datatype MPIR_2complex_dte;
MPI_Datatype MPIR_2dcomplex_dte;

/* Global communicators */
MPI_Comm MPI_COMM_SELF, MPI_COMM_WORLD;
MPI_Group MPI_GROUP_EMPTY;

/* Global MPIR process id (from device) */
int MPIR_tid;

/* Predefined combination functions */
MPI_Op MPI_MAX, MPI_MIN, MPI_SUM, MPI_PROD, MPI_LAND, MPI_BAND, 
               MPI_LOR, MPI_BOR, MPI_LXOR, MPI_BXOR, MPI_MAXLOC, MPI_MINLOC;

/* Permanent attributes */
int MPI_TAG_UB, MPI_HOST, MPI_IO;
/* Places to hold the values of the attributes */
static int MPI_TAG_UB_VAL, MPI_HOST_VAL, MPI_IO_VAL;
/* Fortran versions of the names */
int MPIR_TAG_UB, MPIR_HOST, MPIR_IO;

/* Command-line flags */
int MPIR_Print_queues = 0;

/* Fortran logical values */
/* NOTE: we really have to get these for each system (possibly by calling a
    routine) and make sure that the MPI routines whose Fortran bindings
    use LOGICAL convert to/from these values 
 */
int MPIR_F_TRUE = 1, MPIR_F_FALSE = 0;
/* 
 Location of the Fortran marker for MPI_BOTTOM.  The Fortran wrappers
 must detect the use of this address and replace it with MPI_BOTTOM.

 The detection of MPIR_F_MPI_BOTTOM in the Fortran wrappers is not yet
 implemented.  
 */
void *MPIR_F_MPI_BOTTOM = 0;
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

extern void MPIR_Errors_are_fatal();
extern void MPIR_Errors_return();
extern void MPIR_Errors_warn();

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

/*@
   MPI_Init - Initialize the MPI execution environment

   Input Parameters:
.  argc - Pointer to the number of arguments 
.  argv - Pointer to the argument vector

   Command line arguments:
   MPI specifies no command-line arguments but does allow an MPI 
   implementation to make use of them.

.   -mpiqueue - print out the state of the message queues when MPI_FINALIZE
   is called.  All processors print; the output may be hard to decipher.  This
   is intended as a debugging aid.
.   -mpiversion - print out the version of the implementation (NOT of MPI),
    including the arguments that were used with configure.

.  -mpedbg - Start a debugger in an xterm window if there is an error (either
   detected by MPI or a normally fatal signal).  This works only if MPICH
   was configured with -mpedbg.

.  -mpipktsize nn - Set the message length where the ADI changed to 
   the long message protocol to nn.  This only works if MPICH was 
   configured with -var_pkt.

   The following options are available only on the Chameleon device and
   devices built with debugging code.

.  -mpichdebug - Print out the Chameleon device operations
.  -mpichmemdebug - (Chameleon device only) Print out a list of unreclaimed
   memory.  This requires that MPI be built with the -DMPIR_DEBUG_MEM
   switch.  This is intended for debugging the MPI implementation itself.
.  -mpichmsg - Print out the number of messages 
            received, by category, when the program exits.


   Notes:
   Note that the Fortran binding for this routine has only the error return
   argument (MPI_INIT(ierror))

   Because the Fortran and C versions of MPI_Init are different, there is 
   a restriction on who can call MPI_Init.  The version (Fortran or C) must
   match the main program.  That is, if the main program is in C, then 
   the C version of MPI_Init must be called.  If the main program is in 
   Fortran, the Fortran version must be called.

   On exit from this routine, all processes will have a copy of the argument
   list.  This is NOT REQUIRED by the MPI standard, and truely portable codes
   should not rely on it.  This is provided as a service by this 
   implementation (an MPI implementation is allowed to distribute the
   command line arguments but is not required to).

   Command line arguments are not provided to Fortran programs.  More 
   precisely, non-standard Fortran routines such as getarg and iargc 
   have undefined behavior in MPI and in this implementation.

   Signals used:
   The MPI standard requires that all signals used be documented.  The MPICH
   implementation itself uses no signals, but some of the softare that MPICH
   relies on may use some signals.  The list below is partial and should
   be independantly checked if you (and any package that you use) depend
   on particular signals.

   IBM POE/MPL for SP2:
   SIGHUP, SIGINT, SIGQUIT, SIGFPE, SIGSEGV, SIGPIPE, SIGALRM, SIGTERM,
   SIGIO

   -mpedbg:
   SIGQUIT, SIGILL, SIGFPE, SIGBUS, SIGSEGV, SIGSYS

@*/
int MPI_Init(argc,argv)
int  *argc;
char ***argv;
{
    int            size;
    MPI_Datatype   type[3];
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
    MPI_INT		     = MPIR_Init_basic_datatype( MPIR_INT, 
							 sizeof(int) );
    /* 
       Fortran requires that integers be the same size as 
       REALs, which are half the size of doubles.  Note that
       logicals must be the same size as integers.  Note that
       much of the code does not know about MPIR_LOGICAL or MPIR_FORT_INT
       yet. 

       We still need a FORT_REAL and FORT_DOUBLE type for some systems
     */
    if (sizeof(int) != sizeof(double)/2) {
	MPI_INTEGER          = MPIR_Init_basic_datatype( MPIR_FORT_INT, 4 );
	MPIR_logical_dte     = MPIR_Init_basic_datatype( MPIR_LOGICAL, 4 );
	}
    else {
	MPI_INTEGER          = MPI_INT;
	MPIR_logical_dte     = MPIR_Init_basic_datatype( MPIR_LOGICAL, 
							 sizeof(int) );
	}
    MPI_FLOAT		     = MPIR_Init_basic_datatype( MPIR_FLOAT, 
							 sizeof(float) );
    MPI_DOUBLE		     = MPIR_Init_basic_datatype( MPIR_DOUBLE, 
					        sizeof( double ) );
    MPIR_complex_dte	     = MPIR_Init_basic_datatype( MPIR_COMPLEX, 
						   2 * sizeof( float ) );
    MPIR_complex_dte->align  = sizeof( float );
    MPIR_dcomplex_dte	     = MPIR_Init_basic_datatype( MPIR_DOUBLE_COMPLEX, 
						  2 * sizeof( double ) );
    MPIR_dcomplex_dte->align = sizeof( double );
    MPI_LONG		     = MPIR_Init_basic_datatype( MPIR_LONG, 
							 sizeof( long ) );
    MPI_SHORT		     = MPIR_Init_basic_datatype( MPIR_SHORT, 
						  sizeof( short ) );
    MPI_CHAR		     = MPIR_Init_basic_datatype( MPIR_CHAR, 
							 sizeof( char ) );
    MPI_BYTE		     = MPIR_Init_basic_datatype( MPIR_BYTE, 
							 sizeof( char ) );
    MPI_UNSIGNED_CHAR	     = MPIR_Init_basic_datatype( MPIR_UCHAR, 
					       sizeof( unsigned char ) );
    MPI_UNSIGNED_SHORT	     = MPIR_Init_basic_datatype( MPIR_USHORT, 
					        sizeof( unsigned short ) );
    MPI_UNSIGNED_LONG	     = MPIR_Init_basic_datatype( MPIR_ULONG, 
					       sizeof( unsigned long ) );
    MPI_UNSIGNED	     = MPIR_Init_basic_datatype( MPIR_UINT, 
					       sizeof( unsigned int ) );
    MPI_PACKED		     = MPIR_Init_basic_datatype( MPIR_PACKED, 1 );
    MPI_UB		     = MPIR_Init_basic_datatype( MPIR_UB, 0 );
    MPI_UB->align	     = 1;
    MPI_UB->elements	     = 0;
    MPI_UB->count	     = 0;

    MPI_LB		     = MPIR_Init_basic_datatype( MPIR_LB, 0 );
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

    /* Initialize FORTRAN types for MINLOC and MAXLOC */
    MPI_Type_contiguous ( 2, MPI_FLOAT, &MPIR_2real_dte );
    MPI_Type_commit ( &MPIR_2real_dte );
    MPIR_Type_permanent ( MPIR_2real_dte );

    MPI_Type_contiguous ( 2, MPI_DOUBLE, &MPIR_2double_dte );
    MPI_Type_commit ( &MPIR_2double_dte );
    MPIR_Type_permanent ( MPIR_2double_dte );

    MPI_Type_contiguous ( 2, MPIR_complex_dte, &MPIR_2complex_dte );
    MPI_Type_commit ( &MPIR_2complex_dte );
    MPIR_Type_permanent ( MPIR_2complex_dte );

    MPI_Type_contiguous ( 2, MPIR_dcomplex_dte, &MPIR_2dcomplex_dte );
    MPI_Type_commit ( &MPIR_2dcomplex_dte );
    MPIR_Type_permanent ( MPIR_2dcomplex_dte );

    /* Initialize C & FORTRAN 2int type for MINLOC and MAXLOC */
    MPI_Type_contiguous ( 2, MPI_INT, &MPI_2INT );
    MPI_Type_commit ( &MPI_2INT );
    MPIR_Type_permanent ( MPI_2INT );
    /* This assumes that sizeof(double) == sizeof(double precision) */
    if (sizeof(int) != sizeof(double)/2) {
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
    MPI_Type_struct ( 3, blln, disp, type, &MPI_FLOAT_INT );
    MPIR_Type_permanent ( MPI_FLOAT_INT );
    MPI_Type_commit ( &MPI_FLOAT_INT );

    type[0] = MPI_DOUBLE;
    disp[1] = (char *)&MPI_DOUBLE_INT_var.loc - 
      (char *)&MPI_DOUBLE_INT_var;
    disp[2] = sizeof(MPI_DOUBLE_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &MPI_DOUBLE_INT );
    MPIR_Type_permanent ( MPI_DOUBLE_INT );
    MPI_Type_commit ( &MPI_DOUBLE_INT );

    type[0] = MPI_LONG;
    disp[1] = (char *)&MPI_LONG_INT_var.loc - 
      (char *)&MPI_LONG_INT_var;
    disp[2] = sizeof(MPI_LONG_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &MPI_LONG_INT );
    MPIR_Type_permanent ( MPI_LONG_INT );
    MPI_Type_commit ( &MPI_LONG_INT );

    type[0] = MPI_SHORT;
    disp[1] = (char *)&MPI_SHORT_INT_var.loc - 
      (char *)&MPI_SHORT_INT_var;
    disp[2] = sizeof(MPI_SHORT_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &MPI_SHORT_INT );
    MPIR_Type_permanent ( MPI_SHORT_INT );
    MPI_Type_commit ( &MPI_SHORT_INT );

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
	    i_float   = MPIR_FromPointer(MPI_FLOAT),
            i_double  = MPIR_FromPointer(MPI_DOUBLE),
	    i_complex = MPIR_FromPointer(MPIR_complex_dte),
            i_dcomplex = MPIR_FromPointer(MPIR_dcomplex_dte),
	    i_logical = MPIR_FromPointer(MPIR_logical_dte),
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
    mpir_init_fdtes_( &MPI_INTEGER, &MPI_FLOAT, &MPI_DOUBLE,
                     &MPIR_complex_dte, &MPIR_dcomplex_dte,
                     &MPIR_logical_dte, &MPI_CHAR, 
                     &MPI_BYTE, &MPI_2INTEGER, &MPIR_2real_dte, 
		     &MPIR_2double_dte, &MPIR_2complex_dte,
                     &MPIR_2dcomplex_dte, &MPIR_int1_dte, 
                     &MPIR_int2_dte, &MPIR_int4_dte, &MPIR_real4_dte, 
                     &MPIR_real8_dte, &MPI_PACKED, &MPI_UB, &MPI_LB );
#endif
#endif
    /* initialize queues */
    DEBUG(printf("[%d] About to setup message queues\n", MPIR_tid);)
    MPIR_posted_recvs.first        = MPIR_posted_recvs.last        = NULL;
    MPIR_posted_recvs.maxlen       = MPIR_posted_recvs.currlen     = 0; 

    MPIR_unexpected_recvs.first    = MPIR_unexpected_recvs.last    = NULL;
    MPIR_unexpected_recvs.maxlen   = MPIR_unexpected_recvs.currlen = 0;

    /* Create Error handlers */
    MPI_Errhandler_create( MPIR_Errors_are_fatal, &MPI_ERRORS_ARE_FATAL );
    MPI_Errhandler_create( MPIR_Errors_return, &MPI_ERRORS_RETURN );
    MPI_Errhandler_create( MPIR_Errors_warn, &MPIR_ERRORS_WARN );
    
    /* GROUP_EMPTY is a valid empty group */
    DEBUG(printf("[%d] About to create groups and communicators\n", MPIR_tid);)
    MPI_GROUP_EMPTY     = MPIR_CreateGroup(0);
    MPI_GROUP_EMPTY->permanent = 1;

    MPI_COMM_WORLD              = NEW(struct MPIR_COMMUNICATOR);    
    MPIR_SET_COOKIE(MPI_COMM_WORLD,MPIR_COMM_COOKIE)
    MPI_COMM_WORLD->comm_type   = MPIR_INTRA;
    MPI_COMM_WORLD->ADIctx      = ADIctx;
    MPID_Mysize( ADIctx, &size );
    MPID_Myrank( ADIctx, &MPIR_tid );
    MPI_COMM_WORLD->group         = MPIR_CreateGroup( size );
    MPIR_SetToIdentity( MPI_COMM_WORLD->group );
    MPIR_Group_dup ( MPI_COMM_WORLD->group, &(MPI_COMM_WORLD->local_group) );
    MPI_COMM_WORLD->send_context  = MPIR_WORLD_PT2PT_CONTEXT;
    MPI_COMM_WORLD->recv_context  = MPIR_WORLD_PT2PT_CONTEXT;
    MPI_COMM_WORLD->error_handler = MPI_ERRORS_ARE_FATAL;
    MPI_ERRORS_ARE_FATAL->ref_count ++;
    MPI_COMM_WORLD->ref_count     = 1;
    MPI_COMM_WORLD->permanent     = 1;
    MPIR_Attr_create_tree ( MPI_COMM_WORLD );
    MPI_COMM_WORLD->comm_cache    = 0;
    MPIR_Comm_make_coll ( MPI_COMM_WORLD, MPIR_INTRA );

    /* Predefined attributes for MPI_COMM_WORLD */
    DEBUG(printf("[%d] About to create keyvals\n", MPIR_tid);)
    MPI_Keyval_create( (int (*)())0, (int (*)())0, &MPI_TAG_UB, (void *)0 );
    MPI_Keyval_create( (int (*)())0, (int (*)())0, &MPI_HOST,   (void *)0 );
    MPI_Keyval_create( (int (*)())0, (int (*)())0, &MPI_IO,     (void *)0 );
    MPI_TAG_UB_VAL = (1<<30)-1;
    MPI_HOST_VAL   = MPI_PROC_NULL;
#if defined(MPI_rs6000)
    /* The following isn't strictly correct, but I'm going to leave it
       in for now.  I've tried to make this correct for a few systems
       for which I know the answer.  
     */
    MPI_IO_VAL = MPI_PROC_NULL;
#else
    MPI_IO_VAL = MPI_ANY_SOURCE;
#endif
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
    MPI_Attr_put( MPI_COMM_WORLD, MPIR_TAG_UB, (void*)MPI_TAG_UB_VAL );
    MPI_Attr_put( MPI_COMM_WORLD, MPIR_HOST,   (void*)MPI_HOST_VAL );
    MPI_Attr_put( MPI_COMM_WORLD, MPIR_IO,     (void*)MPI_IO_VAL );

/* Make these permanent.  Must do this AFTER the values are set (because
   changing a value of a permanent attribute is an error) */
#ifdef INT_LT_POINTER
    ((MPIR_Attr_key *)MPIR_ToPointer(MPI_TAG_UB))->permanent  = 1;
    ((MPIR_Attr_key *)MPIR_ToPointer(MPI_HOST))->permanent    = 1;
    ((MPIR_Attr_key *)MPIR_ToPointer(MPI_IO))->permanent      = 1;
    ((MPIR_Attr_key *)MPIR_ToPointer(MPIR_TAG_UB))->permanent = 1;
    ((MPIR_Attr_key *)MPIR_ToPointer(MPIR_HOST))->permanent   = 1;
    ((MPIR_Attr_key *)MPIR_ToPointer(MPIR_IO))->permanent     = 1;
#else
    ((MPIR_Attr_key *)(MPI_TAG_UB))->permanent	= 1;
    ((MPIR_Attr_key *)(MPI_HOST))->permanent	= 1;
    ((MPIR_Attr_key *)(MPI_IO))->permanent	= 1;
    ((MPIR_Attr_key *)(MPIR_TAG_UB))->permanent	= 1;
    ((MPIR_Attr_key *)(MPIR_HOST))->permanent	= 1;
    ((MPIR_Attr_key *)(MPIR_IO))->permanent	= 1;
#endif

    /* COMM_SELF is the communicator consisting only of myself */
    MPI_COMM_SELF                = NEW(struct MPIR_COMMUNICATOR);    
    MPIR_SET_COOKIE(MPI_COMM_SELF,MPIR_COMM_COOKIE)
    MPI_COMM_SELF->comm_type     = MPIR_INTRA;
    MPI_COMM_SELF->group         = MPIR_CreateGroup( 1 );
    MPI_COMM_SELF->group->local_rank = 0;
    MPI_COMM_SELF->group->lrank_to_grank[0] = MPIR_tid;
    MPIR_Group_dup ( MPI_COMM_SELF->group, &(MPI_COMM_SELF->local_group) );
    MPI_COMM_SELF->send_context  = MPIR_SELF_PT2PT_CONTEXT;
    MPI_COMM_SELF->recv_context  = MPIR_SELF_PT2PT_CONTEXT;
    MPI_COMM_SELF->error_handler = MPI_ERRORS_ARE_FATAL;
    MPI_ERRORS_ARE_FATAL->ref_count ++;
    MPI_COMM_SELF->ref_count     = 1;
    MPI_COMM_SELF->permanent     = 1;
    MPIR_Attr_create_tree ( MPI_COMM_SELF );
    MPI_COMM_SELF->comm_cache    = 0;
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
		    /* Should remove argument ... */
		    }
		else if (strcmp((*argv)[i],"-mpiversion" ) == 0) {
		    char ADIname[128];
		    MPID_Version_name( ADIctx, ADIname );
		    printf( "MPI model implementation %4.2f.%d., %s\n", 
			   PATCHLEVEL, PATCHLEVEL_SUBMINOR, ADIname );
		    printf( "Configured with %s\n", CONFIGURE_ARGS_CLEAN );
		    /* Should remove argument ... */
		    }
		else if (strcmp((*argv)[i],"-mpipktsize" ) == 0) {
		    int len;
		    i++;
		    if (i <*argc) {
			len = atoi( (*argv)[i] );
			MPID_SetPktSize( len );
			}
		    else {
			printf( "Missing argument for mpipktsize\n" );
			}
		    }
#ifdef MPE_USE_EXTENSIONS
		else if (strcmp((*argv)[i],"-mpedbg" ) == 0) {
		    MPE_Errors_call_dbx_in_xterm( (*argv)[0], (char *)0 ); 
		    MPE_Signals_call_debugger();
		    }
#endif
#ifdef MPID_HAS_DEBUG
		else if (strcmp((*argv)[i],"-mpichdebug") == 0) {
		    MPID_SetSendDebugFlag( ADIctx, 1 );
		    MPID_SetRecvDebugFlag( ADIctx, 1 );
		    }
		else if (strcmp((*argv)[i],"-chmemdebug" ) == 0) {
		    MPID_SetSpaceDebugFlag( ADIctx, 1 );
		    }
		else if (strcmp((*argv)[i],"-mpichmsg" ) == 0) {
		    MPID_SetMsgDebugFlag( ADIctx, 1 );
		    }
#endif
		}
	    }
	}
    /* barrier */
    MPIR_Has_been_initialized = 1;

    DEBUG(printf("[%d] About to exit from MPI_Init\n", MPIR_tid);)
    return MPI_SUCCESS;
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
#endif
