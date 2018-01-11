/*
 *  $Id: type_commit.c,v 1.12 1994/09/29 21:51:04 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_commit.c,v 1.12 1994/09/29 21:51:04 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Type_commit - Commits the datatype

Input Parameter:
. datatype - datatype (handle) 

@*/
int MPI_Type_commit ( datatype )
MPI_Datatype *datatype;
{
    int errno;
    if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,*datatype))
	return MPIR_ERROR(MPI_COMM_WORLD,errno,"Error in MPI_TYPE_COMMIT" );
    /* We could also complain about committing twice, but we chose not to, 
       based on the view that it isn't obviously an error.
       */
    
    /* Just do the simplest conversion to contiguous where possible */
    if (!(*datatype)->is_contig) {
	if ((*datatype)->size == (*datatype)->extent) {
	    /* This isn't enough of a test, since in principle 
	       you could have multiple overlapping entries and then
	       a gap, giving size == extent but not being contiguous 
	     */
	    int j, offset, is_contig;
	    MPI_Datatype type = *datatype;
	switch (type->dte_type) {
	    case MPIR_STRUCT:
	    offset = 0;
	    is_contig = 1;
	    for (j=0;j<type->count-1; j++) {
		if (!type->old_types[j]->is_contig) { is_contig = 0; break; }
		if (offset + type->old_types[j]->extent * type->blocklens[j] !=
		    type->indices[j+1]) { is_contig = 0; break; }
		offset += type->old_types[j]->extent * type->blocklens[j];
		}
	    if (!type->old_types[type->count-1]->is_contig) is_contig = 0;
	    if (is_contig) {
		type->is_contig = 1;
		}
	    break;
	    }
	    }
	}
    /* Nothing else to do yet */

    (*datatype)->committed = MPIR_YES;

    return MPI_SUCCESS;
}
