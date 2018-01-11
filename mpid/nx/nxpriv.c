/*
 * This implements the NX device for MPICH.
 * It sits directly on top of NX, the context is encoded in the ptype field
 * and sender is encoded into the node field by setting mcmsg_mynode to the 
 * sender lrank before each send call. The sign bit of mcmsg_mynode is also 
 * set to indicate the sender is a MPI process.
 *
 * Updated to have ANSI style code, since I know that it is only compiled
 * for the Paragon, and there is definitely and ANSI compiler available !
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/errno.h>
#include <sys/utsname.h>
#include <nx.h>
#include "mpid.h"
#ifdef ICCLIB
#include <iCC_group.h>
#endif

#define NX_VERSION (MPIDTRANSPORT MPIDPATCHLEVEL)

static struct {
	char mpiss_name[8];
	unsigned long len;
} mpi_sync_send;
static char mpi_sync_send_recv[16];
#define SYNC_SEND_PTYPE 32768
#define COMM_FREE_PTYPE 32769

int MPIR_Error(MPI_Comm comm, int code, char *string, char *file, int line);

typedef struct
{
   int             node;           /* Who am I ?                      */
   int             numnodes;       /* How many TOTAL processes ?      */
   int		   ptype;	   /* Original ptype		      */
   double	   mclock0;        /* The epoch                       */
   int		   initialized;	   /* boolean */
} NX_STATE;

static NX_STATE nx_state; 
long MPID_NX_initialized = 0;
static nx2mpi_errno;

int NX2MPI_Error(int code)
{
    switch (code) {
	case EQPBUF:
		return(MPI_ERR_BUFFER);
	case EQBLEN:
		return(MPI_ERR_BUFFER | MPIR_ERR_USER_BUFFER_EXHAUSTED);
	case EQLEN:
		return(MPI_ERR_COUNT);
/*	case EQTIME:   */  /* MPI cannot generate this error */
	case EQMSGLONG:
		return(MPI_ERR_BUFFER | MPIR_ERR_USER_BUFFER_EXHAUSTED);
	case EQPID:
		return(MPI_ERR_COMM);
	case EQNODE:
		return(MPI_ERR_RANK);
	case EQTYPE:
		return(MPI_ERR_TAG);
	case EQMID:
		return(MPI_ERR_REQUEST);
/*	case EQHND:   */  /* MPI cannot generate this error */	
/*	case EQNOPROC: */  /* MPI cannot generate this error */ 
	case EQUSEPID: 
		return (MPI_ERR_INTERN);
/*	case EQNOACT:
	case EQBADFIL:
	case EQPARAM:
	case EQPFIL:
	case EQPCNODE:
	case EQPCPID:
	case EQPCCODE:
	case EQPRIV:	*/ /* not likely, or no MPI equivalent */
	case EQMEM:
		return (MPI_ERR_EXHAUSTED);
	case EQNOMID:
/*		return(MPI_ERR_LIMIT); */ /* NX error is more informative */
	case EQSET:
	case EQNOSET:
		return (MPI_ERR_INTERN);

	/* The rest have no useful MPI equivalent */

	default:
		return(MPI_ERR_OTHER);
    }
}


#ifdef ICCLIB
	/* MPI_2_ICC defines */
#define MPI2ICC_OP(type,op) \
	((type == iCC_g_LONG) ? \
		((op == MPI_MAX) ? (iCC_g_IMAX) : \
		((op == MPI_MIN) ? (iCC_g_IMIN) : \
		((op == MPI_SUM) ? (iCC_g_ISUM) : \
		((op == MPI_PROD) ? (iCC_g_IPROD) : \
		((op == MPI_BAND) ? (iCC_g_BAND) : \
		((op == MPI_BOR) ? (iCC_g_BOR) : (0)))))))\
	: ((type == iCC_g_FLOAT) ? \
		((op == MPI_MAX) ? (iCC_g_SMAX) : \
		((op == MPI_MIN) ? (iCC_g_SMIN) : \
		((op == MPI_SUM) ? (iCC_g_SSUM) : \
		((op == MPI_PROD) ? (iCC_g_SPROD) :(0))))) \
	: ((type == iCC_g_DOUBLE) ? \
		((op == MPI_MAX) ? (iCC_g_DMAX) : \
		((op == MPI_MIN) ? (iCC_g_DMIN) : \
		((op == MPI_SUM) ? (iCC_g_DSUM) : \
		((op == MPI_PROD) ? (iCC_g_DPROD) : (0)))))\
	: ((type == iCC_g_LOGICAL) ? \
		((op == MPI_LAND) ? (iCC_g_LAND) : \
		((op == MPI_LOR) ? (iCC_g_LOR) : (0)))\
	: (0))))) 

	/* MPI_2_ICC global data */
extern MPIR_COLLOPS *MPIR_intra_collops;
static MPIR_COLLOPS Save_MPIR_collops;
static iCC_g_Datatype MPI2ICC_Datatype[32];

	/* ICC function prototypes */
void iCC_initialize(void);
long iCC_g_Group_init(void);
long iCC_g_Comm_create(iCC_g_Comm comm, iCC_g_Group group, iCC_g_Comm *newcomm);
long iCC_g_Comm_free(iCC_g_Comm *comm);
long iCC_g_Comm_group(iCC_g_Comm comm, iCC_g_Group *group);
long iCC_g_Group_incl(iCC_g_Group group,long n,long *ranks,iCC_g_Group *ngroup);
long iCC_g_Group_free(iCC_g_Group *group);
long iCC_g_Barrier(iCC_g_Comm comm);
long iCC_g_Bcast(void *buffer, long count, iCC_g_Datatype datatype,
		long root, iCC_g_Comm comm);
long iCC_g_Reduce(void *sbuf, void *rbuf, long count, iCC_g_Datatype datatype,
		iCC_g_Op op, long root, iCC_g_Comm icc_comm);
long iCC_g_Gatherv(void *sbuf, long scnt, iCC_g_Datatype stype, 
		void *rbuf, long *rcnt, long *displs, iCC_g_Datatype rtype,
		long root, iCC_g_Comm comm);
long iCC_g_Scatterv(void *sbuf, long *scnt, long *displs, iCC_g_Datatype stype, 
		void *rbuf, long rcnt, iCC_g_Datatype rtype,
		long root, iCC_g_Comm comm);
long iCC_g_Allgatherv(void *sbuf, long scnt, iCC_g_Datatype stype,
		void *rbuf, long *rcnt, long *displs, iCC_g_Datatype rtype,
		iCC_g_Comm comm);
long iCC_g_Allreduce(void *sbuf, void *rbuf, long cnt, iCC_g_Datatype type,
		iCC_g_Op op, iCC_g_Comm comm);
long iCC_g_Reduce_scatter(void *sbuf, void *rbuf, long *rcnts, 
		iCC_g_Datatype type, iCC_g_Op op, iCC_g_Comm comm);


	/* MPI_2_ICC function prototypes */
int MPI_ICC_Barrier(MPI_Comm mpi_comm);
int MPI_ICC_Bcast(void *buffer, int count, MPI_Datatype datatype, 
			int root, MPI_Comm mpi_comm);
int MPI_ICC_Gather(void *sbuf, int scnt,MPI_Datatype stype,
		  void *rbuf, int rcnt, MPI_Datatype rtype,
		  int root, MPI_Comm comm);
int MPI_ICC_Scatter(void *sbuf, int scnt, MPI_Datatype stype,
		  void *rbuf, int rcnt, MPI_Datatype rtype,
		  int root, MPI_Comm comm);
int MPI_ICC_Allgather(void *sbuf, int scnt, MPI_Datatype stype,
		  void *rbuf, int rcnt, MPI_Datatype rtype,
		  MPI_Comm comm);
int MPI_ICC_Allreduce(void *sbuf, void *rbuf, int scnt, MPI_Datatype type,
		  MPI_Op op, MPI_Comm comm);
int MPI_ICC_Reduce(void *sbuf, void *rbuf, int scnt, MPI_Datatype type,
		  MPI_Op op, int root, MPI_Comm comm);
int MPI_ICC_Reduce_scatter(void *sbuf, void *rbuf, int *rcnts, 
		  MPI_Datatype type, MPI_Op op, MPI_Comm comm);

int ICC2MPI_Error(int code)
{
    switch (code) {
	case iCC_g_ERROR_NULL:
	case iCC_g_ERROR_COMM_NULL:
	case iCC_g_ERROR_MEMBERSHIP:
		return(MPI_ERR_COMM);
	case iCC_g_ERROR_GROUP_NULL:
	case iCC_g_ERROR_GROUP_EMPTY:
		return(MPI_ERR_GROUP);
	case iCC_g_ERROR_OP_NULL:
		return(MPI_ERR_OP);
	case iCC_g_ERROR_DATATYPE_NULL:
	case iCC_g_ERROR_DATATYPE_MISMATCH:
		return(MPI_ERR_TYPE);
	case iCC_g_ERROR_INTERCOMMUNICATOR:
		return(MPI_ERR_COMM_INTRA);
	case iCC_g_ERROR_ROOT:
		return(MPI_ERR_ROOT);
	case iCC_g_ERROR_MYPOS:
		return(MPI_ERR_RANK);
	case iCC_g_ERROR_NO_INITIALIZATION:
		/* Can't happen, always init'd in MPID_NX_Init below */
printf("ICC_g_ERROR_NO_INITIALIZATION\n");
		return(MPI_ERR_COMM);
	case iCC_g_ERROR_MALLOC:
		return(MPI_ERR_EXHAUSTED);
	case iCC_g_ERROR_UNEQUAL_COUNTS:
		return(MPI_ERR_COUNT);
	case iCC_g_ERROR_BLOCK_SIZE:
		return(MPI_ERR_DIMS);
	case iCC_g_OK:
		return(MPI_SUCCESS);

	default:
		return(MPI_ERR_ERRORCODE);
    }
  
}

int
MPI_ICC_Barrier(MPI_Comm mpi_comm)
{
	iCC_g_Comm icc_comm;
	int stat;

	if ((icc_comm = (iCC_g_Comm)mpi_comm->adiCollCtx) == (iCC_g_Comm) 0) {
		return(MPIR_ERROR(mpi_comm,MPI_ERR_INTERN,"Error in Barrier"));
	}
	if ((stat = iCC_g_Barrier(icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Barrier"));
	}
	return(MPI_SUCCESS);

}

int
MPI_ICC_Bcast(void *buffer, int count, MPI_Datatype datatype, 
			int root, MPI_Comm mpi_comm)
{
	iCC_g_Comm icc_comm;
	int stat;

	if ((icc_comm = (iCC_g_Comm)mpi_comm->adiCollCtx) == (iCC_g_Comm) 0) {
		return(MPIR_ERROR(mpi_comm,MPI_ERR_INTERN,"Error in Bcast"));
	}
	MPIR_GET_REAL_DATATYPE(datatype)
	if ((datatype->dte_type > MPIR_FORT_INT) ||
	    (MPI2ICC_Datatype[datatype->dte_type] == (iCC_g_Datatype) -1)) {
		return(Save_MPIR_collops.Bcast(buffer, count, datatype,
						root, mpi_comm));
	}
	if ((stat = iCC_g_Bcast(buffer, count, 
				MPI2ICC_Datatype[datatype->dte_type],
				root, icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Bcast"));
	}
	return(MPI_SUCCESS);
}

int 
MPI_ICC_Gather(void* sendbuf, int sendcount,MPI_Datatype sendtype,
		  void* recvbuf, int recvcount, MPI_Datatype recvtype,
		  int root, MPI_Comm mpi_comm)
{
	iCC_g_Comm icc_comm;
	int stat, i; 
	int sendextent; 
	int extents[2]; /* low and high */
	long *rcounts;

	if ((icc_comm = (iCC_g_Comm)mpi_comm->adiCollCtx) == (iCC_g_Comm) 0) {
		return(MPIR_ERROR(mpi_comm,MPI_ERR_INTERN,"Error in Gather"));
	}
	/* Collect extent of send buffers from all processes, error check it*/
	MPIR_GET_REAL_DATATYPE(sendtype)
	MPIR_GET_REAL_DATATYPE(recvtype)
	if ((sendtype->dte_type > MPIR_FORT_INT) ||
	    (MPI2ICC_Datatype[sendtype->dte_type] == (iCC_g_Datatype) -1) ||
	    (recvtype->dte_type > MPIR_FORT_INT) ||
	    (MPI2ICC_Datatype[recvtype->dte_type] == (iCC_g_Datatype) -1)) {
		sendextent = -1;
	} else {
		sendextent = (MPI2ICC_Datatype[sendtype->dte_type])->size * sendcount;
	}
	
	if ((stat = iCC_g_Reduce(&sendextent,&extents[0],1,iCC_g_int_dtype,
					iCC_g_IMIN,0,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Gather/reduce"));
	}
	if ((stat = iCC_g_Reduce(&sendextent,&extents[1],1,iCC_g_int_dtype,
					iCC_g_IMAX,0,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Gather/reduce"));
	}
	if (icc_comm->source == 0) {
	    if (extents[0] != -1)
		/* check against extent of receive buffer */
		if ((MPI2ICC_Datatype[recvtype->dte_type])->size * recvcount !=
				extents[0]) { 
			extents[0] = -1;
		}
	}
	/* broadcast results of check */
	if ((stat = iCC_g_Bcast(extents,2,iCC_g_int_dtype, 
					0,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
				"Error in Gather/Bcast"));
	}

	if ((extents[0] == -1) || 
	    (extents[0] != extents[1])) {
		/* let MPICH try to handle it */
		return(Save_MPIR_collops.Gather(sendbuf, sendcount, sendtype,
			recvbuf, recvcount, recvtype, root, mpi_comm));
	}
	if ((rcounts = (long *)malloc(mpi_comm->local_group->np * sizeof(int))) == (long *) 0 ) {
		return(MPIR_ERROR(mpi_comm,MPI_ERR_EXHAUSTED,
					"Error in Gather"));
	}
	for (i = 0; i < mpi_comm->local_group->np; i++) {	
		rcounts[i] = recvcount;
	}
	if ((stat = iCC_g_Gatherv(sendbuf,sendcount,
			      MPI2ICC_Datatype[sendtype->dte_type],
			      recvbuf,rcounts,(long *) 0,
			      MPI2ICC_Datatype[recvtype->dte_type],
			      root,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
				"Error in Gather"));
	}
	return(MPI_SUCCESS);

}


int 
MPI_ICC_Scatter(void* sendbuf, int sendcount, MPI_Datatype sendtype,
		  void* recvbuf, int recvcount, MPI_Datatype recvtype,
		  int root, MPI_Comm mpi_comm)
{
	iCC_g_Comm icc_comm;
	int stat, i; 
	int recvextent; 
	int extents[2]; /* low and high */
	long *scounts;

	if ((icc_comm = (iCC_g_Comm)mpi_comm->adiCollCtx) == (iCC_g_Comm) 0) {
		return(MPIR_ERROR(mpi_comm,MPI_ERR_INTERN,"Error in Scatter"));
	}
	/* Collect extent of send buffers from all processes, error check it*/
	MPIR_GET_REAL_DATATYPE(sendtype)
	MPIR_GET_REAL_DATATYPE(recvtype)
	if ((sendtype->dte_type > MPIR_FORT_INT) ||
	    (MPI2ICC_Datatype[sendtype->dte_type] == (iCC_g_Datatype) -1) ||
	    (recvtype->dte_type > MPIR_FORT_INT) ||
	    (MPI2ICC_Datatype[recvtype->dte_type] == (iCC_g_Datatype) -1)) {
		recvextent = -1;
	} else {
		recvextent = (MPI2ICC_Datatype[recvtype->dte_type])->size * recvcount;
	}
	
	if ((stat = iCC_g_Reduce(&recvextent,&extents[0],1,iCC_g_int_dtype,
					iCC_g_IMIN,0,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Scatter/reduce"));
	}
	if ((stat = iCC_g_Reduce(&recvextent,&extents[1],1,iCC_g_int_dtype,
					iCC_g_IMAX,0,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Scatter/reduce"));
	}
	if (icc_comm->source == 0) {
	    if (extents[0] != -1)
		/* check against extent of receive buffer */
		if ((MPI2ICC_Datatype[sendtype->dte_type])->size * sendcount !=
				extents[0]) { 
			extents[0] = -1;
		}
	}
	/* broadcast results of check */
	if ((stat = iCC_g_Bcast(extents,2,iCC_g_int_dtype, 
				0,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
				"Error in Scatter/Bcast"));
	}

	if ((extents[0] == -1) || 
	    (extents[0] != extents[1])) {
		/* let MPICH try to handle it */
		return(Save_MPIR_collops.Scatter(sendbuf, sendcount, sendtype,
			recvbuf, recvcount, recvtype, root, mpi_comm));
	}
	if ((scounts = (long *)malloc(mpi_comm->local_group->np * sizeof(int))) == (long *) 0 ) {
		return(MPIR_ERROR(mpi_comm,MPI_ERR_EXHAUSTED,
					"Error in Scatter"));
	}
	for (i = 0; i < mpi_comm->local_group->np; i++) {	
		scounts[i] = sendcount;
	}
	if((stat = iCC_g_Scatterv(sendbuf,scounts,(long *) 0,
			      MPI2ICC_Datatype[sendtype->dte_type],
			      recvbuf,recvcount,
			      MPI2ICC_Datatype[recvtype->dte_type],
			      root,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Scatter"));
	}
	return(MPI_SUCCESS);

}

int 
MPI_ICC_Allgather(void *sendbuf, int sendcount,MPI_Datatype sendtype,
		  void *recvbuf, int recvcount, MPI_Datatype recvtype,
		  MPI_Comm mpi_comm)
{
	iCC_g_Comm icc_comm;
	int stat, i; 
	int sendextent,recvextent; 
	int extents[4]; /* low and high for both send and recv*/
	long *rcounts;

	if ((icc_comm = (iCC_g_Comm)mpi_comm->adiCollCtx) == (iCC_g_Comm) 0) {
	       return(MPIR_ERROR(mpi_comm,MPI_ERR_INTERN,"Error in Allgather"));
	}
	/* Collect extent of send buffers from all processes, error check it*/
	MPIR_GET_REAL_DATATYPE(sendtype)
	MPIR_GET_REAL_DATATYPE(recvtype)
	if ((sendtype->dte_type > MPIR_FORT_INT) ||
	    (MPI2ICC_Datatype[sendtype->dte_type] == (iCC_g_Datatype) -1) ||
	    (recvtype->dte_type > MPIR_FORT_INT) ||
	    (MPI2ICC_Datatype[recvtype->dte_type] == (iCC_g_Datatype) -1)) {
		sendextent = -1;
		recvextent = -1;
	} else {
		sendextent = (MPI2ICC_Datatype[sendtype->dte_type])->size * sendcount;
		recvextent = (MPI2ICC_Datatype[recvtype->dte_type])->size * recvcount;
	}
	
	if ((stat = iCC_g_Reduce(&sendextent,&extents[0],1,iCC_g_int_dtype,
					iCC_g_IMIN,0,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Allgather/reduce"));
	}
	if ((stat = iCC_g_Reduce(&sendextent,&extents[1],1,iCC_g_int_dtype,
					iCC_g_IMAX,0,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Allgather/reduce"));
	}
	if ((stat = iCC_g_Reduce(&recvextent,&extents[2],1,iCC_g_int_dtype,
					iCC_g_IMIN,0,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Allgather/reduce"));
	}
	if ((stat = iCC_g_Reduce(&recvextent,&extents[3],1,iCC_g_int_dtype,
					iCC_g_IMAX,0,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Allgather/reduce"));
	}
	if (icc_comm->source == 0) {
	    if (extents[0] != -1)
		/* check against extent of receive buffer */
		if (extents[0] != extents[1] ||
		    extents[0] != extents[2] ||
		    extents[0] != extents[3]) { 
			extents[0] = -1;
		}
	}
	/* broadcast results of check */
	if ((stat = iCC_g_Bcast(extents,2,iCC_g_int_dtype, 
					0,icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
				"Error in Allgather/Bcast"));
	}

	if (extents[0] == -1) {
		/* let MPICH try to handle it */
		return(Save_MPIR_collops.Allgather(sendbuf, sendcount, sendtype,
			recvbuf, recvcount, recvtype, mpi_comm));
	}
	if ((rcounts = (long *)malloc(mpi_comm->local_group->np * sizeof(int))) == (long *) 0 ) {
		return(MPIR_ERROR(mpi_comm,MPI_ERR_EXHAUSTED,
					"Error in Allgather"));
	}
	for (i = 0; i < mpi_comm->local_group->np; i++) {	
		rcounts[i] = recvcount;
	}
	if ((stat = iCC_g_Allgatherv(sendbuf,sendcount,
			      MPI2ICC_Datatype[sendtype->dte_type],
			      recvbuf,rcounts,(long *) 0,
			      MPI2ICC_Datatype[recvtype->dte_type],
			      icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
				"Error in Allgather"));
	}
	return(MPI_SUCCESS);

}



int
MPI_ICC_Allreduce(void *sbuf, void *rbuf,int count, MPI_Datatype datatype, 
			MPI_Op op, MPI_Comm mpi_comm)
{
	iCC_g_Comm icc_comm;
	iCC_g_Op icc_op;
	int stat;

	if ((icc_comm = (iCC_g_Comm)mpi_comm->adiCollCtx) == (iCC_g_Comm) 0) {
	       return(MPIR_ERROR(mpi_comm,MPI_ERR_INTERN,"Error in Allreduce"));
	}
	MPIR_GET_REAL_DATATYPE(datatype)
	if ((datatype->dte_type > MPIR_FORT_INT) ||
	    (MPI2ICC_Datatype[datatype->dte_type] == (iCC_g_Datatype) -1) ||
	    ((icc_op = MPI2ICC_OP(MPI2ICC_Datatype[datatype->dte_type],op)) ==
								(iCC_g_Op)0)){
		return(Save_MPIR_collops.Allreduce(sbuf, rbuf, count, datatype,
						op, mpi_comm));
	}
	if ((stat = iCC_g_Allreduce(sbuf, rbuf, count, 
				MPI2ICC_Datatype[datatype->dte_type],
				icc_op, icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Allreduce"));
	}
	return(MPI_SUCCESS);
}

int
MPI_ICC_Reduce(void *sbuf, void *rbuf,int count, MPI_Datatype datatype, 
			MPI_Op op, int root, MPI_Comm mpi_comm)
{
	iCC_g_Comm icc_comm;
	iCC_g_Op icc_op;
	int stat;

	if ((icc_comm = (iCC_g_Comm)mpi_comm->adiCollCtx) == (iCC_g_Comm) 0) {
	       return(MPIR_ERROR(mpi_comm,MPI_ERR_INTERN,"Error in Reduce"));
	}
	MPIR_GET_REAL_DATATYPE(datatype)
	if ((datatype->dte_type > MPIR_FORT_INT) ||
	    (MPI2ICC_Datatype[datatype->dte_type] == (iCC_g_Datatype) -1) ||
	    ((icc_op = MPI2ICC_OP(MPI2ICC_Datatype[datatype->dte_type],op)) ==
								(iCC_g_Op)0)){
		return(Save_MPIR_collops.Reduce(sbuf, rbuf, count, datatype,
						op, root, mpi_comm));
	}
	if ((stat = iCC_g_Reduce(sbuf, rbuf, count, 
				MPI2ICC_Datatype[datatype->dte_type],
				icc_op, root, icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Reduce"));
	}
	return(MPI_SUCCESS);
}

int
MPI_ICC_Reduce_scatter(void *sbuf, void *rbuf,int *cnts, MPI_Datatype datatype, 
			MPI_Op op, MPI_Comm mpi_comm)
{
	iCC_g_Comm icc_comm;
	iCC_g_Op icc_op;
	int stat;

	if ((icc_comm = (iCC_g_Comm)mpi_comm->adiCollCtx) == (iCC_g_Comm) 0) {
	       return(MPIR_ERROR(mpi_comm,MPI_ERR_INTERN,"Error in Reduce_scatter"));
	}
	MPIR_GET_REAL_DATATYPE(datatype)
	if ((datatype->dte_type > MPIR_FORT_INT) ||
	    (MPI2ICC_Datatype[datatype->dte_type] == (iCC_g_Datatype) -1) ||
	    ((icc_op = MPI2ICC_OP(MPI2ICC_Datatype[datatype->dte_type],op)) ==
								(iCC_g_Op)0)){
		return(Save_MPIR_collops.Reduce_scatter(sbuf, rbuf, cnts, 
						datatype, op, mpi_comm));
	}
	if ((stat = iCC_g_Reduce_scatter(sbuf, rbuf, (long *)cnts, 
				MPI2ICC_Datatype[datatype->dte_type],
				icc_op, icc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Reduce_scatter"));
	}
	return(MPI_SUCCESS);
}

#endif  /* ICCLIB */

char 
*nx_version (void)
{
    return (NX_VERSION);
}

int 
nx_checkVersion (char *version)
{
    return (!strcmp (version, NX_VERSION));
}

void 
MPID_NX_End (void)
{
    /* Only run once even if called by the user, and then by atexit */
    static int exited = FALSE;

    if (!exited)
    {
	exited = TRUE;
	gsync();		/* Don't leave until we all get here */
   	nx_state.initialized = 0;
	MPID_NX_initialized = 2;
    }
    return;
}

void 
MPID_NX_Mysize( int *size )
{
    *size = nx_state.numnodes;
}

void 
MPID_NX_Myrank( int *rank )
{
    *rank = nx_state.node;
}

double 
MPID_NX_Wtime(void)
{
    return (dclock() - nx_state.mclock0);
}

void 
MPID_NX_Abort( int errcode  )
{
    _killproc(-1,-1);
    exit(errcode);
}


/****************************************************************/
/*                                                              */
/*                 Message Passing                              */
/*                                                              */
/****************************************************************/

/* from /afs/ssd/i860/R1_4/Cmds/obj/ipsc860/usr/include/dbglib.h */

typedef struct ipd_msg_info { 
        char 	**buffer_p;		/* xmsg_t  **buffer_p; */
        unsigned long buffer_size;
        char 	*nxreq;			/* nxreq_t *nxreq; */
        long    nxreq_size;
        char 	*ipd_plist;		/* ipd_ptype_list_t *ipd_plist; */
        long    num_ptypes;
        long    *current_ptype;
        long    *app_id;
        long    nodenum;
        char 	*ipd_msg_aux;		/* ipd_msg_aux_t *ipd_msg_aux; */
} ipd_msg_info_t;

#define PTYPE_ENTRIES   31  /* maximum number of ptype ranges supported by OS */

typedef struct ipd_ptype_list {
        long            method;
        unsigned long   last_entry;
        long            app;
        unsigned long   numnodes;
        unsigned long   *phys_node_list;
        char 	      *node_ptype_list; /* struct ptype_list *node_ptype_list;*/
        unsigned long   ref_count;
        unsigned long   fill7;
        struct ptype_entry {
                long            ptype;
                long            item;
                long            offset;
        } ptype_entry[PTYPE_ENTRIES];
} ipd_ptype_list_t;

typedef struct ipd_msg_aux {
        long    call_type;
        long    ptype;
} ipd_msg_aux_t;

typedef
struct xmsg {
        unsigned char   chain_number;
        unsigned char   state;
        unsigned short  source_node;
        unsigned long   originating_node;
        struct xmsg     *link;
        struct xmsg     *backlink;
        unsigned long   size;
        unsigned long   totalsize;
        unsigned long   length;
        void 		*si;
        long            dest_ptype;
        long            source_ptype;
        long            msg_type;
        struct xmsg     *prev_adjacent;
        struct xmsg     *next_adjacent;
        unsigned long   fill1;
        unsigned long   fill2;
        unsigned long   fill3;
} xmsg_t;
/* xmsg size must be a multiple of 32 bytes */

/* nxreq size must be a multiple of 32 bytes */
typedef
struct nxreq {
        long            state;
        long            err;
        unsigned long   *info;
        unsigned long   link;
        xmsg_t          *xmsg;
        long            req;
        long            type;
        unsigned long   node;
        long            ptype;
        unsigned long   bcount;
        unsigned long   buf;
        unsigned long   bsize;
        unsigned long   boffset;
        unsigned long   handler;        /* for hsend/hrecv */
        unsigned long   hparam;         /* for hsendx/hrecvx */
        long            localinfo[8];
        long            monitored;
/* Fill structure to cache line size - if necessary. */
} nxreq_t;


extern long mcmsg_ptype_range_low;
extern long mcmsg_ptype_range_high;
extern long mcmsg_ptype;
extern unsigned long mcmsg_mynode;
extern ipd_msg_info_t ipd_msg_info;
extern ipd_ptype_list_t ipd_plist;
extern void mcmsg_nx_reserve_ptype();
int ipd_ptype_add(ipd_ptype_list_t *ipd_plist,long ptype,long item,long offset);

#define MPID_ptype(ptype)  \
{ \
	int st; \
 \
	if (ptype > mcmsg_ptype_range_high || \
	   ptype < mcmsg_ptype_range_low) { \
		st = 0; \
		mcmsg_nx_reserve_ptype(ptype, 1, &st); \
		while (st == 0) { \
			flick(); \
		} \
		if (st != -1) {\
 /* This area is now write protected in nxlib.c{}_setptype() \
		    if (ipd_ptype_add(&ipd_plist, ptype, 1, 0) == 1) { \
			ipd_msg_info.num_ptypes++; \
		    } \
 */ \
		    if (mcmsg_ptype_range_high == ptype-1) { \
			mcmsg_ptype_range_high = ptype; \
		    } else if (mcmsg_ptype_range_low == ptype+1) { \
			mcmsg_ptype_range_low = ptype; \
		    } \
		} \
	} \
	mcmsg_ptype = ptype; \
}

extern long ipd_call_type;
extern nxreq_t *nxreq;
extern ipd_msg_aux_t ipd_msg_aux[];
int alloc_mid();

void 
*MPID_NX_Init ( int * Argc, char *** Argv )
{
	int i,j;
	int msgid[950];
	char buf[4];
   
	if (nx_state.initialized) {	/* already fully initialised */
		fprintf( stderr, "Already initialized!\n" );
		return 0;
	}

	/* Make 100% sure the state is clean before we start */
	memset(&nx_state, 0, sizeof(nx_state));

	nx_state.node   = mynode();
	nx_state.numnodes  = numnodes();
	nx_state.ptype  = myptype();
	MPID_ptype(SYNC_SEND_PTYPE);
	mcmsg_ptype = nx_state.ptype;
        sprintf(mpi_sync_send.mpiss_name,"MPISSND");


	/* Do NX initialization handshake for all nodes, using global 
	 * rank and context, (node and ptype).
	 */
	for (j = 0; j < (nx_state.numnodes / 950); j++) {
		for (i = 0; i < 950; i++) {
			if (i + j * 950 == nx_state.node) ++i;
			if (i < 950) {
				msgid[i] = _irecv(5878,buf,4);
			}
		}	
		for (i = 0; i < 950; i++) { 
			if (i + j * 950 == nx_state.node) ++i;
			if (i < 950) {
				_csend(5878,buf,0,i + j * 950,0);
			}
		}
		for (i = 0; i < 950; i++) {
			if (i + j * 950 == nx_state.node) ++i;
			if (i < 950) {
				_msgwait(msgid[i]);
			}
		}
	}
	for (i = 0; i < nx_state.numnodes % 950; i++) {
		if (i + (nx_state.numnodes / 950) * 950 == nx_state.node) ++i;
		if (i < nx_state.numnodes % 950) {
			msgid[i] = _irecv(5878,buf,4);
		}
	}
	for (i = (nx_state.numnodes / 950) * 950; i < nx_state.numnodes; i++) {
                if (i == nx_state.node) i++;
		if (i < nx_state.numnodes) {
			_csend(5878,buf,0,i,0);
		}
	}
	for (i = 0; i < (nx_state.numnodes % 950); i++) {
                if (i + (nx_state.numnodes / 950) * 950 == nx_state.node) ++i;
		if (i < (nx_state.numnodes % 950)) {
                	_msgwait(msgid[i]);
		}
	}
	gsync();		/* sync all the nodes */
	nx_state.initialized = 1;
	MPID_NX_initialized = 1;

    /* Store start time AFTER gsync, so mclock will look similar on all nodes */
	nx_state.mclock0 = dclock();

#ifdef ICCLIB
	iCC_initialize();
	iCC_g_Group_init();
		/* Save the collops table */
		/* We assume for now MPIR_intra_collops are always used for
		   all intra-communicators */
	Save_MPIR_collops.Barrier = MPIR_intra_collops->Barrier;
	Save_MPIR_collops.Bcast = MPIR_intra_collops->Bcast;
	Save_MPIR_collops.Gather = MPIR_intra_collops->Gather;
	Save_MPIR_collops.Gatherv = MPIR_intra_collops->Gatherv;
	Save_MPIR_collops.Scatter = MPIR_intra_collops->Scatter;
	Save_MPIR_collops.Scatterv = MPIR_intra_collops->Scatterv;
	Save_MPIR_collops.Allgather = MPIR_intra_collops->Allgather;
	Save_MPIR_collops.Allgatherv = MPIR_intra_collops->Allgatherv;
	Save_MPIR_collops.Alltoall = MPIR_intra_collops->Alltoall;
	Save_MPIR_collops.Alltoallv = MPIR_intra_collops->Alltoallv;
	Save_MPIR_collops.Reduce = MPIR_intra_collops->Reduce;
	Save_MPIR_collops.Allreduce = MPIR_intra_collops->Allreduce;
	Save_MPIR_collops.Reduce_scatter = MPIR_intra_collops->Reduce_scatter;
	Save_MPIR_collops.Scan = MPIR_intra_collops->Scan;

		/* Map ICC ops to MPI ops, MPI_COMM_WORLD is not yet 
		   initialized so we have to map onto MPIR_intra_collops */
	MPIR_intra_collops->Barrier = MPI_ICC_Barrier;
	MPIR_intra_collops->Bcast = MPI_ICC_Bcast;
	MPIR_intra_collops->Gather = MPI_ICC_Gather;
	MPIR_intra_collops->Scatter = MPI_ICC_Scatter;
	MPIR_intra_collops->Allgather = MPI_ICC_Allgather;
	MPIR_intra_collops->Allreduce = MPI_ICC_Allreduce;
	MPIR_intra_collops->Reduce = MPI_ICC_Reduce;

	/* Initialize MPI to ICC type mapping, -1 means no defined iCC type */
	MPI2ICC_Datatype[MPIR_INT] = iCC_g_int_dtype;
	MPI2ICC_Datatype[MPIR_FLOAT] = iCC_g_float_dtype;
	MPI2ICC_Datatype[MPIR_DOUBLE] = iCC_g_double_dtype;
	MPI2ICC_Datatype[MPIR_COMPLEX] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_LONG] = iCC_g_long_dtype;
	MPI2ICC_Datatype[MPIR_SHORT] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_CHAR] = iCC_g_char_dtype;
	MPI2ICC_Datatype[MPIR_BYTE] = iCC_g_byte_dtype;
	MPI2ICC_Datatype[MPIR_UCHAR] = iCC_g_byte_dtype;
	MPI2ICC_Datatype[MPIR_USHORT] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_ULONG] = iCC_g_long_dtype;
	MPI2ICC_Datatype[MPIR_UINT] = iCC_g_int_dtype;
	MPI2ICC_Datatype[MPIR_CONTIG] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_VECTOR] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_HVECTOR] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_INDEXED] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_HINDEXED] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_STRUCT] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_DOUBLE_COMPLEX] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_PACKED] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_UB] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_LB] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_LONGDOUBLE] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_LONGLONGINT] = (iCC_g_Datatype) -1;
	MPI2ICC_Datatype[MPIR_LOGICAL] = iCC_g_logical_dtype;
	MPI2ICC_Datatype[MPIR_FORT_INT] = (iCC_g_Datatype) -1;

#endif /* ICCLIB */
	return (void *)&nx_state;
}


long
MPID_NX_iprobex(type,node, ptype, info)
        long    type;
        long    node;
        long    ptype;
        long    *info;
{
        int     st, i;
        int     mid;
        nxreq_t *nrq;
        nxreq_t *nrq2;
        xmsg_t  *xmsg;
        long    save_ipd_call_type;
	char 	*p;

        if ((save_ipd_call_type = ipd_call_type) == -1)
                ipd_call_type = 26;
 
        mid = _alloc_mid();
        if (mid == -1) {
                ipd_call_type = save_ipd_call_type;
                return -1;
        }
        nrq = &nxreq[mid];
	nrq->xmsg = 0;
        ipd_msg_aux[mid].call_type = ipd_call_type;
        ipd_msg_aux[mid].ptype = mcmsg_ptype;
 
        _setup_recv(type, 0, -1, node, ptype, 0, 0, 0, nrq);
 
/* 	PERFMON_IDLE_START(_probex); */
        for (;;) {
                if (nrq->state != 1) {
                        break;
                }
 
                _flick();
        }
/*        PERFMON_IDLE_END(_probex); */
 
        ipd_call_type = save_ipd_call_type;
 
        st = 0;
        if (nrq->state == 3) {
                if (info != 0) {
                        xmsg = nrq->xmsg;
                        info[0] = xmsg->msg_type;
                        info[1] = xmsg->length;
                        info[2] = xmsg->originating_node;
                        info[3] = xmsg->source_ptype & ~(0x40000000);
                        info[4] = xmsg->source_node;
			p = (char *) (xmsg+1);
			if (*p++ == 'M' &&
			    *p++ == 'P' &&
			    *p++ == 'I' &&
			    *p++ == 'S' &&
			    *p++ == 'S' &&
			    *p++ == 'N' &&
			    *p++ == 'D') {
				p++;
				info[1] = *(long *)p;
			}

                }
                st = 1;
        }
        _free_mid(mid);
        return st;
}




int 
MPID_NX_Probe(int tag, int source, int context_id, MPI_Status *status )
{
	long info[8];
	int stat;
	
	while ((stat = MPID_NX_iprobex((tag == MPI_ANY_TAG ? -1 : tag),
	 	    (source == MPI_ANY_SOURCE ? -1 : (source | MPID_NODE_MASK)),
		    context_id,info)) == 0);
	if (stat < 0) {
		if ((nx2mpi_errno = NX2MPI_Error(errno)) == MPI_ERR_OTHER) {
			perror("Error in MPI_Probe");
			return(errno);
		} else {
			char errstring[MPI_MAX_ERROR_STRING];
			int elen;
			MPI_Error_string(nx2mpi_errno,(char *)errstring,&elen);
			fprintf( stderr, "%d - %s : %s\n", mynode(),
				"Error in MPI_Probe",errstring);
			return(nx2mpi_errno);
		}
	}

	status->MPI_TAG    = info[0];
	status->count    = info[1];
	status->MPI_SOURCE  = (info[2] & ~MPID_NODE_MASK);

	return 0;
}

long MPID_NX_Iprobe_info[8];

int 
MPID_NX_Iprobe(int tag, int source, int context_id, 
		      int *flag, MPI_Status *status )
{
	

	if ((*flag = MPID_NX_iprobex((tag == MPI_ANY_TAG ? -1 : tag),
                	(source == MPI_ANY_SOURCE ? -1 : 
				(source | MPID_NODE_MASK)),
                	context_id,MPID_NX_Iprobe_info)) < 0) {
		if ((nx2mpi_errno = NX2MPI_Error(errno)) == MPI_ERR_OTHER) {
			perror("Error in MPI_Probe");
			return(errno);
		} else {
			char errstring[MPI_MAX_ERROR_STRING];
			int elen;
			MPI_Error_string(nx2mpi_errno,(char *)errstring,&elen);
			fprintf( stderr, "%d - %s : %s\n", mynode(),
				"Error in MPI_Probe",errstring);
			return(nx2mpi_errno);
		}
	}

	if (*flag == 1) {
		status->MPI_TAG    = MPID_NX_Iprobe_info[0];
		status->count    = MPID_NX_Iprobe_info[1];
		status->MPI_SOURCE = (MPID_NX_Iprobe_info[2] & ~MPID_NODE_MASK);
	} else {
		*flag = 0;
	}
	return *flag;
}

int 
MPID_NX_Blocking_recv(MPIR_RHANDLE *dmpi_recv_handle )
{
	long info[8];
	MPID_RHANDLE *rhandle = &dmpi_recv_handle->dev_rhandle;
	int len = dmpi_recv_handle->dev_rhandle.bytes_as_contig;
	char *p;
	
	/* if the recv buffer is < 8 bytes save its address and use the
	 * sync_buf in case this message matches a MPI_SSEND()
	 * message (which always sends 8 bytes to indicate its synchronous)
	 */
	if (len < 12) {
		len = 12;
		rhandle->save_start = rhandle->start;
		rhandle->start = rhandle->sync_buf;
	} else {
		rhandle->save_start = 0;
	}
	p = (char *)rhandle->start;
	if (_crecvx((dmpi_recv_handle->tag == MPI_ANY_TAG ? -1 : 
				dmpi_recv_handle->tag),
			rhandle->start, len,
                	(dmpi_recv_handle->source == MPI_ANY_SOURCE ? -1 : 
				(dmpi_recv_handle->source | MPID_NODE_MASK)),
			dmpi_recv_handle->contextid, info) < 0) {
		if ((nx2mpi_errno = NX2MPI_Error(errno)) == MPI_ERR_OTHER) {
			perror("");
		}
		return MPIR_ERROR(dmpi_recv_handle->comm, 
			nx2mpi_errno, "Error in MPI_Recv" );
	}
	       
	dmpi_recv_handle->tag = info[0];
	dmpi_recv_handle->totallen = info[1];
	dmpi_recv_handle->source = (info[2] & ~MPID_NODE_MASK);
	/* Test for MPI_Synchronous_send */
	if (*p++ == 'M' &&
	    *p++ == 'P' &&
	    *p++ == 'I' &&
	    *p++ == 'S' &&
	    *p++ == 'S' &&
	    *p++ == 'N' &&
	    *p++ == 'D') {

		int msgid;
		if ((msgid = _irecvx(dmpi_recv_handle->tag,
			((rhandle->save_start != 0) ? rhandle->save_start :
			 rhandle->start),
			rhandle->bytes_as_contig,
			dmpi_recv_handle->source, info[3],info)) < 0) {
			if((nx2mpi_errno = NX2MPI_Error(errno))
					     == MPI_ERR_OTHER) {
				perror("");
			}
			return MPIR_ERROR(dmpi_recv_handle->comm, 
				nx2mpi_errno, "Error in MPI_Recv" );
		}
		
		mcmsg_ptype = SYNC_SEND_PTYPE;
		if (_csend(dmpi_recv_handle->tag,
		    &mpi_sync_send, sizeof(mpi_sync_send),
		    dmpi_recv_handle->source, nx_state.ptype) < 0) {
			if((nx2mpi_errno = NX2MPI_Error(errno))
					     == MPI_ERR_OTHER) {
				perror("");
			}
			return MPIR_ERROR(dmpi_recv_handle->comm, 
				nx2mpi_errno, "Error in MPI_Recv" );
		}
		mcmsg_ptype = nx_state.ptype;
		if (_msgwait(msgid) < 0) {
			if((nx2mpi_errno = NX2MPI_Error(errno))
					     == MPI_ERR_OTHER) {
				perror("");
			}
			return MPIR_ERROR(dmpi_recv_handle->comm, 
				nx2mpi_errno, "Error in MPI_Recv" );
		}
		dmpi_recv_handle->totallen = info[1];
	} else if (rhandle->save_start != 0) {
	/* This recv did not match a MPI_SSEND() and was less than
	 * 12 bytes (the size of a MPI_SSEND sync msg), so copy the data 
	 * to the real buffer.
	 */
		int jj;
		for (jj = 0; jj < rhandle->bytes_as_contig; jj++) {
			rhandle->save_start[jj] = rhandle->start[jj];
		}
	}
	return (0);
}

long MPID_NX_Post_recv_info[8];

int 
MPID_NX_Post_recv(MPIR_RHANDLE *dmpi_recv_handle )
{
	MPID_RHANDLE *rhandle = &dmpi_recv_handle->dev_rhandle;
	int len = dmpi_recv_handle->dev_rhandle.bytes_as_contig;

	if (len < 12) {
		len = 12;
		rhandle->save_start = rhandle->start;
		rhandle->start = rhandle->sync_buf;
	} else {
		rhandle->save_start = 0;
	}
    if ((rhandle->id = _irecvx((dmpi_recv_handle->tag == MPI_ANY_TAG ? -1 : 
				dmpi_recv_handle->tag),
			rhandle->start, len,
                	(dmpi_recv_handle->source == MPI_ANY_SOURCE ? -1 : 
				(dmpi_recv_handle->source | MPID_NODE_MASK)),
			dmpi_recv_handle->contextid, 
			MPID_NX_Post_recv_info)) < 0) {
		if ((nx2mpi_errno = NX2MPI_Error(errno)) == MPI_ERR_OTHER) {
			perror("");
		}
		return MPIR_ERROR(dmpi_recv_handle->comm, 
			nx2mpi_errno, "Error in MPI_Recv" );
    }
    return 0;
}

int 
MPID_NX_Blocking_send(MPIR_SHANDLE *dmpi_send_handle )
{
	if (dmpi_send_handle->contextid != mcmsg_ptype) {
		MPID_ptype(dmpi_send_handle->contextid);
	}
	mcmsg_mynode = (dmpi_send_handle->lrank | MPID_NODE_MASK);
	dmpi_send_handle->dev_shandle.recvid = -1;
	dmpi_send_handle->dev_shandle.rtq_id = -1;
	if (_csend(dmpi_send_handle->tag,
			dmpi_send_handle->dev_shandle.start,
			dmpi_send_handle->dev_shandle.bytes_as_contig,
			dmpi_send_handle->dest,
			nx_state.ptype) < 0) {
		if ((nx2mpi_errno = NX2MPI_Error(errno)) == MPI_ERR_OTHER) {
			perror("");
		}
		return MPIR_ERROR(dmpi_send_handle->comm, 
			nx2mpi_errno, "Error in MPI_Send" );
	}
	mcmsg_ptype = nx_state.ptype;
	mcmsg_mynode = nx_state.node;
   
    return (0);
}

int 
MPID_NX_Post_send(MPIR_SHANDLE *dmpi_send_handle)
{
	if (dmpi_send_handle->contextid != mcmsg_ptype) {
		MPID_ptype(dmpi_send_handle->contextid);
	}
	mcmsg_mynode = (dmpi_send_handle->lrank | MPID_NODE_MASK);
	dmpi_send_handle->dev_shandle.recvid = -1;
	dmpi_send_handle->dev_shandle.rtq_id = -1;
	if ((dmpi_send_handle->dev_shandle.id = 
	    _isend(dmpi_send_handle->tag,
			dmpi_send_handle->dev_shandle.start,
			dmpi_send_handle->dev_shandle.bytes_as_contig,
			dmpi_send_handle->dest,
			nx_state.ptype)) < 0) {
		if ((nx2mpi_errno = NX2MPI_Error(errno)) == MPI_ERR_OTHER) {
			perror("");
		}
		return MPIR_ERROR(dmpi_send_handle->comm, 
			nx2mpi_errno, "Error in MPI_Send" );
	}
	mcmsg_ptype = nx_state.ptype;
	mcmsg_mynode = nx_state.node;
    return 0;
}

long NX_Post_send_sync_info[8];
int 
MPID_NX_Post_send_sync(MPIR_SHANDLE *dmpi_send_handle)
{
	long recvid;
	if (dmpi_send_handle->contextid != mcmsg_ptype) {
		MPID_ptype(dmpi_send_handle->contextid);
	}
	mcmsg_mynode = (dmpi_send_handle->lrank | MPID_NODE_MASK);
	mpi_sync_send.len = dmpi_send_handle->dev_shandle.bytes_as_contig;
	if ((dmpi_send_handle->dev_shandle.recvid = 
		_irecvx(dmpi_send_handle->tag,
			mpi_sync_send_recv, sizeof(mpi_sync_send_recv),
			dmpi_send_handle->dest, SYNC_SEND_PTYPE, 
			NX_Post_send_sync_info)) < 0) {
		if ((nx2mpi_errno = NX2MPI_Error(errno)) == MPI_ERR_OTHER) {
			perror("");
		}
		return MPIR_ERROR(dmpi_send_handle->comm, 
			nx2mpi_errno, "Error in MPI_Ssend" );
	}

	if ((dmpi_send_handle->dev_shandle.rtq_id = 
		_isend(dmpi_send_handle->tag,
			&mpi_sync_send, sizeof(mpi_sync_send),
			dmpi_send_handle->dest,
			nx_state.ptype)) < 0) {
		if ((nx2mpi_errno = NX2MPI_Error(errno)) == MPI_ERR_OTHER) {
			perror("");
		}
		return MPIR_ERROR(dmpi_send_handle->comm, 
			nx2mpi_errno, "Error in MPI_Ssend" );
	}
	dmpi_send_handle->dev_shandle.id = dmpi_send_handle->dev_shandle.recvid;
	mcmsg_ptype = nx_state.ptype;
	mcmsg_mynode = nx_state.node;
    return 0;
}

int 
MPID_NX_Test_send(MPIR_SHANDLE *dmpi_send_handle )
{
	int stat;
	if (dmpi_send_handle->dev_shandle.id == -1) {
		return 1;
	}
	if (dmpi_send_handle->dev_shandle.rtq_id != -1) {
	    if((stat = _msgdone(dmpi_send_handle->dev_shandle.rtq_id)) < 0) {
		if ((nx2mpi_errno = NX2MPI_Error(errno)) == MPI_ERR_OTHER) {
			perror("");
		}
		return MPIR_ERROR(dmpi_send_handle->comm, 
			nx2mpi_errno, "Error in MPI_Ssend" );
	    }
	    if (stat == 0) {
		return(0);
	    } else {
		dmpi_send_handle->dev_shandle.rtq_id = -1;
	    }
	}
	if (dmpi_send_handle->dev_shandle.recvid != -1) {
	    if ((stat = _msgdone(dmpi_send_handle->dev_shandle.recvid)) < 0) {
		if ((nx2mpi_errno = NX2MPI_Error(errno)) == MPI_ERR_OTHER) {
			perror("");
		}
		return MPIR_ERROR(dmpi_send_handle->comm, 
			nx2mpi_errno, "Error in MPI_Ssend" );
	    }
	    if (stat == 0) {
		return(0);
	    } else {
		dmpi_send_handle->dev_shandle.recvid = -1;
		if ((dmpi_send_handle->dev_shandle.id = 
		    		_isend(dmpi_send_handle->tag,
				dmpi_send_handle->dev_shandle.start,
				dmpi_send_handle->dev_shandle.bytes_as_contig,
				dmpi_send_handle->dest,
				nx_state.ptype)) < 0) {
			if((nx2mpi_errno=NX2MPI_Error(errno)) == MPI_ERR_OTHER){
				perror("");
			}
			return MPIR_ERROR(dmpi_send_handle->comm, 
				nx2mpi_errno, "Error in MPI_Ssend" );
		}
	    }
	}
	if ((stat = _msgdone(dmpi_send_handle->dev_shandle.id)) == 1) {
		dmpi_send_handle->dev_shandle.id = -1;
		return 1;
	} else {
		if (stat == -1) {
			dmpi_send_handle->dev_shandle.id = -1;
			if ((nx2mpi_errno = NX2MPI_Error(errno)) ==
					MPI_ERR_OTHER) {
				perror("");
			}
			return MPIR_ERROR(dmpi_send_handle->comm, 
				nx2mpi_errno, "Error in MPI_Test" );
		}
		return 0;
	}
}


int 
MPID_NX_Test_recv(MPIR_RHANDLE *dmpi_recv_handle )
{
	int stat; 
	long info[8];
	MPID_RHANDLE *rhandle = &dmpi_recv_handle->dev_rhandle;
	char *p = (char *)rhandle->start;

	if (rhandle->id == -1) {
		return 1;
	}
	if ((stat = _msgdone(rhandle->id)) == 1) {
		rhandle->id = -1;
		dmpi_recv_handle->tag = MPID_NX_Post_recv_info[0];
		dmpi_recv_handle->totallen = MPID_NX_Post_recv_info[1];
		dmpi_recv_handle->source = (MPID_NX_Post_recv_info[2] & ~MPID_NODE_MASK);
	    /* Test for MPI_Synchronous_send */
		if (*p++ == 'M' &&
		    *p++ == 'P' &&
		    *p++ == 'I' &&
		    *p++ == 'S' &&
		    *p++ == 'S' &&
		    *p++ == 'N' &&
		    *p++ == 'D') {
			int msgid;
			if ((msgid = _irecvx(dmpi_recv_handle->tag,
			    ((rhandle->save_start != 0) ? 
			    	rhandle->save_start :
			     	rhandle->start),
			    rhandle->bytes_as_contig,
			    dmpi_recv_handle->source, 
			    MPID_NX_Post_recv_info[3],info)) < 0) {
				if((nx2mpi_errno = NX2MPI_Error(errno))
					     == MPI_ERR_OTHER) {
					perror("");
				}
				return MPIR_ERROR(dmpi_recv_handle->comm, 
					nx2mpi_errno, "Error in MPI_Test" );
			}
			
			mcmsg_ptype = SYNC_SEND_PTYPE;
			if (_csend(dmpi_recv_handle->tag,
			    &mpi_sync_send, sizeof(mpi_sync_send),
			    dmpi_recv_handle->source,
			    nx_state.ptype) < 0) {
				if((nx2mpi_errno = NX2MPI_Error(errno))
						     == MPI_ERR_OTHER) {
					perror("");
				}
				return MPIR_ERROR(dmpi_recv_handle->comm, 
					nx2mpi_errno, "Error in MPI_Test" );
			}
			mcmsg_ptype = nx_state.ptype;
			if (_msgwait(msgid) < 0) {
				if((nx2mpi_errno = NX2MPI_Error(errno))
						     == MPI_ERR_OTHER) {
					perror("");
				}
				return MPIR_ERROR(dmpi_recv_handle->comm, 
					nx2mpi_errno, "Error in MPI_Test" );
			}
			dmpi_recv_handle->totallen = info[1];
		} else if (rhandle->save_start != 0) {
		/* This recv did not match a MPI_SSEND() and was less than
		 * 8 bytes (the size of a MPI_SSEND sync msg), so copy the data
		 * to the real buffer.
		 */
			int jj;
			for (jj = 0; jj < rhandle->bytes_as_contig; jj++) {
				rhandle->save_start[jj] = rhandle->start[jj];
			}
		}
	} else {
		if (stat == -1) {
			rhandle->id = -1;
			if ((nx2mpi_errno = NX2MPI_Error(errno)) ==
					MPI_ERR_OTHER) {
				perror("");
			}
			return MPIR_ERROR(dmpi_recv_handle->comm, 
				nx2mpi_errno, "Error in MPI_Test" );
		}
	}
	return 0;
}

int 
MPID_NX_Complete_recv(MPIR_RHANDLE *dmpi_recv_handle )
{
	long info[8];
	MPID_RHANDLE *rhandle = &dmpi_recv_handle->dev_rhandle;
	char *p = (char *)rhandle->start;
 
	if (rhandle->id != -1) {
		if (_msgwait(rhandle->id) < 0) {
			rhandle->id = -1;
			if ((nx2mpi_errno = NX2MPI_Error(errno)) ==
					MPI_ERR_OTHER) {
				perror("");
			}
			return MPIR_ERROR(dmpi_recv_handle->comm, 
				nx2mpi_errno, "Error in MPI_Wait" );
		}

		rhandle->id = -1;
		dmpi_recv_handle->tag = MPID_NX_Post_recv_info[0];
		dmpi_recv_handle->totallen = MPID_NX_Post_recv_info[1];
		dmpi_recv_handle->source = (MPID_NX_Post_recv_info[2] & ~MPID_NODE_MASK);
	    /* Test for MPI_Synchronous_send */
		if (*p++ == 'M' &&
		    *p++ == 'P' &&
		    *p++ == 'I' &&
		    *p++ == 'S' &&
		    *p++ == 'S' &&
		    *p++ == 'N' &&
		    *p++ == 'D') {
			int msgid;
			if ((msgid = _irecvx(dmpi_recv_handle->tag,
			    ((rhandle->save_start != 0) ? 
			    	rhandle->save_start :
			     	rhandle->start),
			    rhandle->bytes_as_contig,
			    dmpi_recv_handle->source, 
			    MPID_NX_Post_recv_info[3],info)) < 0) {
				if((nx2mpi_errno = NX2MPI_Error(errno))
					     == MPI_ERR_OTHER) {
					perror("");
				}
				return MPIR_ERROR(dmpi_recv_handle->comm, 
					nx2mpi_errno, "Error in MPI_Wait" );
			}

			mcmsg_ptype = SYNC_SEND_PTYPE;
			if (_csend(dmpi_recv_handle->tag,
			    &mpi_sync_send, sizeof(mpi_sync_send),
			    dmpi_recv_handle->source,
			    nx_state.ptype) < 0) {
				if((nx2mpi_errno = NX2MPI_Error(errno))
						     == MPI_ERR_OTHER) {
					perror("");
				}
				return MPIR_ERROR(dmpi_recv_handle->comm, 
					nx2mpi_errno, "Error in MPI_Wait" );
			}
			mcmsg_ptype = nx_state.ptype;
			if (_msgwait(msgid) < 0) {
				if((nx2mpi_errno = NX2MPI_Error(errno))
						     == MPI_ERR_OTHER) {
					perror("");
				}
				return MPIR_ERROR(dmpi_recv_handle->comm, 
					nx2mpi_errno, "Error in MPI_Wait" );
			}
			dmpi_recv_handle->totallen = info[1];
		} else if (rhandle->save_start != 0) {
		/* This recv did not match a MPI_SSEND() and was less than
		 * 8 bytes (the size of a MPI_SSEND sync msg), so copy the data
		 * to the real buffer.
		 */
			int jj;
			for (jj = 0; jj < rhandle->bytes_as_contig; jj++) {
				rhandle->save_start[jj] = rhandle->start[jj];
			}
		}
	}
   
	return (0);
}

int 
MPID_NX_Complete_send(MPIR_SHANDLE *dmpi_send_handle )
{
	if (dmpi_send_handle->dev_shandle.id != -1) {
	    if (dmpi_send_handle->dev_shandle.rtq_id != -1) {
		if (_msgwait(dmpi_send_handle->dev_shandle.rtq_id) < 0) {
			dmpi_send_handle->dev_shandle.id = -1;
			if ((nx2mpi_errno = NX2MPI_Error(errno)) ==
					MPI_ERR_OTHER) {
				perror("");
			}
			return MPIR_ERROR(dmpi_send_handle->comm, 
				nx2mpi_errno, "Error in MPI_Wait" );
		}
		dmpi_send_handle->dev_shandle.rtq_id = -1;
	    }
	    if (dmpi_send_handle->dev_shandle.recvid != -1) {
		if (_msgwait(dmpi_send_handle->dev_shandle.recvid) < 0) {
			dmpi_send_handle->dev_shandle.id = -1;
			if ((nx2mpi_errno = NX2MPI_Error(errno)) ==
					MPI_ERR_OTHER) {
				perror("");
			}
			return MPIR_ERROR(dmpi_send_handle->comm, 
				nx2mpi_errno, "Error in MPI_Wait" );
		}
		dmpi_send_handle->dev_shandle.recvid = -1;
		if ((dmpi_send_handle->dev_shandle.id = 
		    		_isend(dmpi_send_handle->tag,
				dmpi_send_handle->dev_shandle.start,
				dmpi_send_handle->dev_shandle.bytes_as_contig,
				dmpi_send_handle->dest,
				nx_state.ptype)) < 0) {
			if((nx2mpi_errno=NX2MPI_Error(errno)) == MPI_ERR_OTHER){
				perror("");
			}
			return MPIR_ERROR(dmpi_send_handle->comm, 
				nx2mpi_errno, "Error in MPI_Ssend" );
		}
	    }
	    if (_msgwait(dmpi_send_handle->dev_shandle.id) < 0) {
		dmpi_send_handle->dev_shandle.id = -1;
		if ((nx2mpi_errno = NX2MPI_Error(errno)) == MPI_ERR_OTHER) {
			perror("");
		}
		return MPIR_ERROR(dmpi_send_handle->comm, 
				nx2mpi_errno, "Error in MPI_Wait" );
	    }
	    dmpi_send_handle->dev_shandle.id = -1;
	}
   
	return (0);
}

int 
MPID_NX_Cancel(MPIR_COMMON *dmpi_handle )
{
	if (dmpi_handle->handle_type == MPIR_SEND) {
	    if((_msgcancel(((MPIR_SHANDLE *)dmpi_handle)->dev_shandle.id)) < 0){
		((MPIR_SHANDLE *)dmpi_handle)->dev_shandle.id = -1;
		if ((nx2mpi_errno = NX2MPI_Error(errno)) ==
					MPI_ERR_OTHER) {
				perror("");
			}
		return MPIR_ERROR(dmpi_handle->comm, 
			nx2mpi_errno, "Error in MPI_Cancel" );
	    }
	    ((MPIR_SHANDLE *)dmpi_handle)->dev_shandle.id = -1;
	} else {
	    if((_msgcancel(((MPIR_RHANDLE *)dmpi_handle)->dev_rhandle.id)) < 0){
		((MPIR_RHANDLE *)dmpi_handle)->dev_rhandle.id = -1;
		if ((nx2mpi_errno = NX2MPI_Error(errno)) ==
					MPI_ERR_OTHER) {
				perror("");
			}
		return MPIR_ERROR(dmpi_handle->comm, 
			nx2mpi_errno, "Error in MPI_Cancel" );
	    }
	    ((MPIR_RHANDLE *)dmpi_handle)->dev_rhandle.id = -1;
	}
	return 0;
}

/*
 *  Miscellaneous stuff
 */
void MPID_NX_Version_name(char *name)
{
    sprintf( name, "ADI version %s - transport %s", MPIDPATCHLEVEL, 
	    MPIDTRANSPORT );
}

double MPID_NX_Wtick(void)
{
    return 1.0 / 1.0e6;
}

void MPID_NX_Node_name(char *name, int len)
{
	struct utsname nam;
	
	uname(&nam);
	if ((len - ((long)strlen(nam.machine) + 1 +
		  (long)strlen(nam.nodename) + 3 +
		  (long)strlen(nam.version))) >= 0) {
			sprintf(name,"%s '%s' %s",
				nam.machine, 
				nam.nodename, 
				nam.version);
	} else if ((len - ((long)strlen(nam.machine) + 1 +
			  (long)strlen(nam.nodename) + 2)) >= 0) {
		sprintf(name,"%s '%s'", nam.machine, nam.nodename);

	} else if ((len - ((long)strlen(nam.machine))) >= 0) {
		sprintf(name,"%s '%s'", nam.machine);
	} else {
		name[0] = NULL;
	}
}
 

int
MPID_NX_Comm_init(MPI_Comm mpi_comm,MPI_Comm newcomm)
{
#ifdef ICCLIB
	iCC_g_Comm icc_comm, newicc_comm;
	iCC_g_Group icc_group, newicc_group;
	int stat;

#endif

#ifdef ICCLIB
/* if iCC_comm is not yet cached in mpi_comm, create & cache it */
	if (newcomm == MPI_COMM_WORLD) {
		MPI_COMM_WORLD->adiCollCtx = (void *)iCC_g_COMM_WORLD;
		return (MPI_SUCCESS);
	} 
	if (newcomm == MPI_COMM_SELF) {
		MPI_COMM_SELF->adiCollCtx = (void *)iCC_g_COMM_SELF;
		return (MPI_SUCCESS);
	}
	if (mpi_comm = (MPI_Comm)0) {
                return(MPIR_ERROR(mpi_comm,MPI_ERR_COMM,"Error in Comm_init"));
        }
	icc_comm = iCC_g_COMM_WORLD;
	if((stat = iCC_g_Comm_group(icc_comm, &icc_group)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Comm_init"));
	}
	if ((stat = iCC_g_Group_incl(icc_group,newcomm->group->np,
				(long *)(newcomm->group->lrank_to_grank),
					&newicc_group)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Comm_init"));
	}
	if ((stat = iCC_g_Comm_create(icc_comm, newicc_group, 
						&newicc_comm)) != iCC_g_OK) {
		return(MPIR_ERROR(mpi_comm,ICC2MPI_Error(stat),
					"Error in Comm_init"));
	}
	newcomm->adiCollCtx = (void *) newicc_comm;

#endif
	return (MPI_SUCCESS);
}


int
MPID_NX_Comm_free(MPI_Comm mpi_comm)
{
#ifdef ICCLIB
	iCC_g_Comm icc_comm;
	iCC_g_Group icc_group;
	int stat;

/* free from IPD comm_list */

/* free iCC_Comm and iCC_Group */
	icc_comm = (iCC_g_Comm)mpi_comm->adiCollCtx;
	MPID_ptype(COMM_FREE_PTYPE);
	if((stat = iCC_g_Comm_group(icc_comm,&icc_group)) != iCC_g_OK) {
		return(MPI_SUCCESS);
	}
	if ((stat = iCC_g_Group_free(&icc_group)) != iCC_g_OK) {
		return(MPI_SUCCESS);
	}
	if ((stat = iCC_g_Comm_free(&icc_comm)) != iCC_g_OK) {
		return(MPI_SUCCESS);
	}
	mcmsg_ptype = nx_state.ptype;
#endif
	return(MPI_SUCCESS);	
}
