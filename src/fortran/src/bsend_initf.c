/* bsend_init.c */
/* Custom Fortran interface file */
#include "mpi_fortimpl.h"
#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(F77_NAME_UPPER)
#pragma weak MPI_BSEND_INIT = PMPI_BSEND_INIT
EXPORT_MPI_API void MPI_BSEND_INIT ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma weak mpi_bsend_init__ = pmpi_bsend_init__
EXPORT_MPI_API void mpi_bsend_init__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(F77_NAME_LOWER_USCORE)
#pragma weak mpi_bsend_init = pmpi_bsend_init
EXPORT_MPI_API void mpi_bsend_init ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_bsend_init_ = pmpi_bsend_init_
EXPORT_MPI_API void mpi_bsend_init_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(F77_NAME_UPPER)
#pragma _HP_SECONDARY_DEF PMPI_BSEND_INIT  MPI_BSEND_INIT
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma _HP_SECONDARY_DEF pmpi_bsend_init__  mpi_bsend_init__
#elif !defined(F77_NAME_LOWER_USCORE)
#pragma _HP_SECONDARY_DEF pmpi_bsend_init  mpi_bsend_init
#else
#pragma _HP_SECONDARY_DEF pmpi_bsend_init_  mpi_bsend_init_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(F77_NAME_UPPER)
#pragma _CRI duplicate MPI_BSEND_INIT as PMPI_BSEND_INIT
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma _CRI duplicate mpi_bsend_init__ as pmpi_bsend_init__
#elif !defined(F77_NAME_LOWER_USCORE)
#pragma _CRI duplicate mpi_bsend_init as pmpi_bsend_init
#else
#pragma _CRI duplicate mpi_bsend_init_ as pmpi_bsend_init_
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
#define mpi_bsend_init_ PMPI_BSEND_INIT
#elif defined(F77_NAME_LOWER_2USCORE)
#define mpi_bsend_init_ pmpi_bsend_init__
#elif !defined(F77_NAME_LOWER_USCORE)
#define mpi_bsend_init_ pmpi_bsend_init
#else
#define mpi_bsend_init_ pmpi_bsend_init_
#endif

#else

#ifdef F77_NAME_UPPER
#define mpi_bsend_init_ MPI_BSEND_INIT
#elif defined(F77_NAME_LOWER_2USCORE)
#define mpi_bsend_init_ mpi_bsend_init__
#elif !defined(F77_NAME_LOWER_USCORE)
#define mpi_bsend_init_ mpi_bsend_init
#endif
#endif


#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 8

 void mpi_bsend_init_( void *unknown, ...)
{

/*  Must use variable argument list because buffer parameter may be a two
	word Fortran character descriptor.  If it is, need to extract the
	parameters by hand from the list as it is defined as a void * instead
	of _fcd * and it has already been pulled off the list. */

char *		buf;
int *		count;
MPI_Datatype	*datatype;
int*		dest;
int*		tag;
MPI_Comm      	*comm;
MPI_Request*	request;
int *		__ierr;
MPI_Request 	lrequest;
int buflen;
va_list ap;

va_start(ap, unknown);
buf = unknown;
if (_numargs() == NUMPARAMS+1) {
	buflen = (va_arg(ap, int)) / 8;		/* This is in bits. */
}
count = 	va_arg (ap, int *);
datatype = 	va_arg(ap, MPI_Datatype*);
dest = 		va_arg(ap, int *);
tag =		va_arg(ap, int *);
comm =		va_arg(ap, MPI_Comm *);
request =	va_arg(ap, MPI_Request *);
__ierr =	va_arg(ap, int *);


*__ierr = MPI_Bsend_init(MPIR_F_PTR(buf),*count,*datatype,*dest,*tag,*comm,
			 &lrequest);
*(int*)request = MPI_Request_c2f(lrequest);
}

#else
 void mpi_bsend_init_( buf, count, datatype, dest, tag, comm, request, __ierr )
void          *buf;
int*		count;
MPI_Datatype  	*datatype;
int*		dest;
int*		tag;
MPI_Comm      	*comm;
MPI_Request*	request;
int *		__ierr;
{
MPI_Request lrequest;
_fcd temp;
if (_isfcd(buf)) {
	temp = _fcdtocp(buf);
	buf = (void *)temp;
}
*__ierr = MPI_Bsend_init(MPIR_F_PTR(buf),*count,*datatype,*dest,*tag,*comm,
			 &lrequest);
*(int*)request = MPI_Request_c2f(lrequest);
}
#endif
#else
/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void mpi_bsend_init_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                 MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                 MPI_Fint * );

EXPORT_MPI_API void mpi_bsend_init_( void *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Bsend_init(MPIR_F_PTR(buf),(int)*count,
                             MPI_Type_f2c(*datatype),
                             (int)*dest,
                             (int)*tag,MPI_Comm_f2c(*comm), 
                             &lrequest);
    *request = MPI_Request_c2f(lrequest);
}
#endif