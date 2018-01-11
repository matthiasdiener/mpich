/* myprof.c */
/* Custom Fortran interface file */
/* These have been edited because they require special string processing */
#ifdef MPI_BUILD_PROFILING
#undef MPI_BUILD_PROFILING
#endif
#include "mpi.h"
#include "mpisys.h"

#ifndef DEBUG_ALL
#define DEBUG_ALL
#endif

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef FORTRANCAPS
#define mpi_init_ MPI_INIT
#define mpi_send_ MPI_SEND
#define mpi_recv_ MPI_RECV
#define mpi_sendrecv_ MPI_SENDRECV
#define mpi_bcast_ MPI_BCAST
#define mpi_reduce_ MPI_REDUCE
#define mpi_barrier_ MPI_BARRIER
#define mpi_isend_ MPI_ISEND
#define mpi_irecv_ MPI_IRECV
#define mpi_wait_ MPI_WAIT
#define mpi_test_ MPI_TEST
#define mpi_waitall_ MPI_WAITALL
#define mpi_waitany_ MPI_WAITANY
#define mpi_ssend_ MPI_SSEND
#define mpi_finalize_ MPI_FINALIZE
#define mpi_allreduce_ MPI_ALLREDUCE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_init_ mpi_init__
#define mpi_send_ mpi_send__
#define mpi_recv_ mpi_recv__
#define mpi_sendrecv_ mpi_sendrecv__
#define mpi_bcast_ mpi_bcast__
#define mpi_reduce_ mpi_reduce__
#define mpi_barrier_ mpi_barrier__
#define mpi_isend_ mpi_isend__
#define mpi_irecv_ mpi_irecv__
#define mpi_wait_ mpi_wait__
#define mpi_test_ mpi_test__
#define mpi_waitall_ mpi_waitall__
#define mpi_waitany_ mpi_waitany__
#define mpi_ssend_ mpi_ssend__
#define mpi_finalize_ mpi_finalize__
#define mpi_allreduce_ mpi_allreduce__
#elif defined(FORTRANNOUNDERSCORE)
#define mpi_init_ mpi_init
#define mpi_send_ mpi_send
#define mpi_recv_ mpi_recv
#define mpi_sendrecv_ mpi_sendrecv
#define mpi_bcast_ mpi_bcast
#define mpi_reduce_ mpi_reduce
#define mpi_barrier_ mpi_barrier
#define mpi_isend_ mpi_isend
#define mpi_irecv_ mpi_irecv
#define mpi_wait_ mpi_wait
#define mpi_test_ mpi_test
#define mpi_waitall_ mpi_waitall
#define mpi_waitany_ mpi_waitany
#define mpi_ssend_ mpi_ssend
#define mpi_finalize_ mpi_finalize
#define mpi_allreduce_ mpi_allreduce
#endif

void mpi_init_( ierr )
int *ierr;
{
int  Argc, i, argsize = 40;
char **Argv, *p;
int  ArgcSave;
char **ArgvSave;

/* Recover the args with the Fortran routines iargc_ and getarg_ */
ArgcSave = Argc = mpir_iargc_() + 1;
ArgvSave = Argv = (char **)MALLOC( Argc * sizeof(char *) );    
if (!Argv) {
    *ierr = MPIR_ERROR( (MPI_Comm)0, MPI_ERR_EXHAUSTED, 
                       "Out of space in MPI_INIT" );
    return;
    }
for (i=0; i<Argc; i++) {
    ArgvSave[i] = Argv[i] = (char *)MALLOC( argsize + 1 );
    if (!Argv[i]) {
        *ierr = MPIR_ERROR( (MPI_Comm)0, MPI_ERR_EXHAUSTED, 
                           "Out of space in MPI_INIT" );
        return;
        }
    mpir_getarg_( &i, Argv[i], argsize );
    /* Trim trailing blanks */
    p = Argv[i] + argsize - 1;
    while (p > Argv[i]) {
	if (*p != ' ') {
	    p[1] = '\0';
	    break;
	    }
	p--;
	}
    }

*ierr = MPI_Init( &Argc, &Argv );

/* Recover space */
for (i=0; i<ArgcSave; i++) {
    FREE( ArgvSave[i] );
    }
FREE( ArgvSave );

}

 void mpi_send_( buf, count, datatype, dest, tag, comm, __ierr )
void             *buf;
int              *count, *dest, *tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
int *__ierr;
{
*__ierr = MPI_Send(buf,*count,
	(MPI_Datatype)MPIR_ToPointer(*((int*)datatype)),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer(*((int*)comm)));
}

 void mpi_recv_( buf, count, datatype, source, tag, comm, status, __ierr )
void             *buf;
int              *count, *source, *tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
MPI_Status       *status;
int *__ierr;
{
*__ierr = MPI_Recv(buf,*count,
	(MPI_Datatype)MPIR_ToPointer(*((int*)datatype)),*source,*tag,
	(MPI_Comm)MPIR_ToPointer(*((int*)comm)),status);
}

 void mpi_sendrecv_( sendbuf, sendcount, sendtype, dest, sendtag, 
                  recvbuf, recvcount, recvtype, source, recvtag, 
                  comm, status, __ierr )
void         *sendbuf;
int           *sendcount;
MPI_Datatype  sendtype;
int           *dest, *sendtag;
void         *recvbuf;
int           *recvcount;
MPI_Datatype  recvtype;
int           *source, *recvtag;
MPI_Comm      comm;
MPI_Status   *status;
int *__ierr;
{
*__ierr = MPI_Sendrecv(sendbuf,*sendcount,
	(MPI_Datatype)MPIR_ToPointer(*((int*)sendtype)),
		       *dest,*sendtag,recvbuf,*recvcount,
	(MPI_Datatype)MPIR_ToPointer(*((int*)recvtype)),*source,*recvtag,
	(MPI_Comm)MPIR_ToPointer(*((int*)comm)),status);
}

 void mpi_bcast_ ( buffer, count, datatype, root, comm, __ierr )
void             *buffer;
int               *count;
MPI_Datatype      datatype;
int               *root;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Bcast(buffer,*count,
	(MPI_Datatype)MPIR_ToPointer(*((int*)datatype)),*root,
	(MPI_Comm)MPIR_ToPointer(*((int*)comm)));
}

 void mpi_reduce_ ( sendbuf, recvbuf, count, datatype, op, root, comm, __ierr )
void             *sendbuf;
void             *recvbuf;
int               *count;
MPI_Datatype      datatype;
MPI_Op            op;
int               *root;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Reduce(sendbuf,recvbuf,*count,
	(MPI_Datatype)MPIR_ToPointer(*((int*)datatype)),
	(MPI_Op)MPIR_ToPointer(*((int*)op)),*root,
	(MPI_Comm)MPIR_ToPointer(*((int*)comm)));
}

 void mpi_barrier_ ( comm, __ierr )
MPI_Comm comm;
int *__ierr;
{
*__ierr = MPI_Barrier(
	(MPI_Comm)MPIR_ToPointer(*((int*)comm)));
}

 void mpi_isend_( buf, count, datatype, dest, tag, comm, request, __ierr )
void             *buf;
int              *count;
MPI_Datatype     datatype;
int              *dest;
int              *tag;
MPI_Comm         comm;
MPI_Request      *request;
int *__ierr;
{
*__ierr = MPI_Isend(buf,*count,
	(MPI_Datatype)MPIR_ToPointer(*((int*)datatype)),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer(*((int*)comm)),request);
}

 void mpi_irecv_( buf, count, datatype, source, tag, comm, request, __ierr )
void             *buf;
int              *count;
MPI_Datatype     datatype;
int              *source;
int              *tag;
MPI_Comm         comm;
MPI_Request      *request;
int *__ierr;
{
*__ierr = MPI_Irecv(buf,*count,
	(MPI_Datatype)MPIR_ToPointer(*((int*)datatype)),*source,*tag,
	(MPI_Comm)MPIR_ToPointer(*((int*)comm)),request);
}

 void mpi_wait_ ( request, status, __ierr )
MPI_Request  *request;
MPI_Status   *status;
int *__ierr;
{
*__ierr = MPI_Wait(request,status);
}

 void mpi_test_ ( request, flag, status, __ierr )
MPI_Request  *request;
int          *flag;
MPI_Status   *status;
int *__ierr;
{
*__ierr = MPI_Test(request,flag,status);
}

 void mpi_waitall_(count, array_of_requests, array_of_statuses, __ierr )
int         *count;
MPI_Request array_of_requests[];
MPI_Status  array_of_statuses[];
int *__ierr;
{
*__ierr = MPI_Waitall(*count,array_of_requests,array_of_statuses);
}

 void mpi_waitany_(count, array_of_requests, index, status, __ierr )
int         *count;
MPI_Request array_of_requests[];
int         *index;
MPI_Status  *status;
int *__ierr;
{
*__ierr = MPI_Waitany(*count,array_of_requests,index,status);
}

 void mpi_ssend_( buf, count, datatype, dest, tag, comm, __ierr )
void             *buf;
int              *count, *dest, *tag;
MPI_Datatype     datatype;
MPI_Comm         comm;
int *__ierr;
{
*__ierr = MPI_Ssend(buf,*count,
	(MPI_Datatype)MPIR_ToPointer(*((int*)datatype)),*dest,*tag,
	(MPI_Comm)MPIR_ToPointer(*((int*)comm)));
}

 void mpi_finalize_(__ierr )
int *__ierr;
{
*__ierr = MPI_Finalize();
}
 void mpi_allreduce_ ( sendbuf, recvbuf, count, datatype, op, comm, __ierr )
void             *sendbuf;
void             *recvbuf;
int               *count;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
int *__ierr;
{
*__ierr = MPI_Allreduce(sendbuf,recvbuf,*count,
	(MPI_Datatype)MPIR_ToPointer(*((int*)datatype)),
	(MPI_Op)MPIR_ToPointer(*((int*)op)),
	(MPI_Comm)MPIR_ToPointer(*((int*)comm)));
}
