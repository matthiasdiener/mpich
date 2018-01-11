/*
 *  $Id: reduceutil.c,v 1.1 1994/11/01 18:48:57 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: reduceutil.c,v 1.1 1994/11/01 18:48:57 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"
#include "coll.h"

int MPIR_ADIReduce( ctx, comm, sendbuf, recvbuf, count, datatype, op, root )
void             *ctx;
void             *sendbuf;
void             *recvbuf;
int               count;
MPI_Datatype      datatype;
MPI_Op            op;
int               root;
MPI_Comm          comm;
{
int err = 1; /* Stands for not supported */

if (op == MPI_SUM) {
    switch (datatype->dte_type) {
#ifdef MPID_Reduce_sum_int
	case MPIR_INT:
	MPID_Reduce_sum_int( ctx, (int *)sendbuf, (int *)recvbuf, comm );
	return MPI_SUCCESS;
	break;
#endif
#ifdef MPID_Reduce_sum_double
	case MPIR_DOUBLE:
	MPID_Reduce_sum_double( ctx, (double *)sendbuf, (double *)recvbuf, 
			        comm );
	return MPI_SUCCESS;
	break;
#endif
	default:
	;
	}
    }
return err;
}
