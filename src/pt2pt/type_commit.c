/*
 *  $Id: type_commit.c,v 1.22 1996/07/17 18:04:00 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif

/*@
    MPI_Type_commit - Commits the datatype

Input Parameter:
. datatype - datatype (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TYPE
@*/
int MPI_Type_commit ( datatype )
MPI_Datatype *datatype;
{
    int mpi_errno;
    if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,*datatype))
	return MPIR_ERROR(MPI_COMM_WORLD,mpi_errno,
			  "Error in MPI_TYPE_COMMIT" );
    /* We could also complain about committing twice, but we chose not to, 
       based on the view that it isn't obviously an error.
       */
    
    /* Just do the simplest conversion to contiguous where possible */
#if defined(MPID_HAS_HETERO)
    if (!MPID_IS_HETERO)
#endif
    {	
    if (!(*datatype)->is_contig) {
	/* I want to add a test for the struct { contig, UB } form of
	   variable count strided vectors; this will not have
	   size == extent.  Because of this, using the simple test of
	   size == extent as a filter is not useful.
	   */
	int          j, is_contig;
	MPI_Aint     offset;
	MPI_Datatype type = *datatype;
	if ((MPI_Aint)type->size == type->extent) {
	switch (type->dte_type) {
	case MPIR_STRUCT:
	    offset    = type->indices[0];
	    /* If the initial offset is not 0, then mark as non-contiguous.
	       This is because many of the quick tests for valid buffers
	       depend on the initial address being valid if is_contig is
	       set */
	    is_contig = (offset == 0);
	    for (j=0;is_contig && j<type->count-1; j++) {
		if (!type->old_types[j]->is_contig) { is_contig = 0; break; }
		if (offset + 
		   type->old_types[j]->extent * (MPI_Aint)type->blocklens[j] !=
		    type->indices[j+1]) { is_contig = 0; break; }
		offset += type->old_types[j]->extent * 
		    (MPI_Aint)type->blocklens[j];
		}
	    if (!type->old_types[type->count-1]->is_contig) is_contig = 0;
	    if (is_contig) {
		type->is_contig = 1;
		type->old_type  = 0;
		/* If we don't set to null, then the code in type_contig.c
		   will use the extent of type->old_types[0] */
		/* type->old_type  = type->old_types[0]; */
		/* PRINTF( "Making structure type contiguous..." ); */
		/* Should free all old structure members ... */
		}
	    break;
	default:
	    /* Just to indicate that we want all the other types to be 
	       ignored */
	    break;
	    }
	}
	}
    }
    /* Nothing else to do yet */

    (*datatype)->committed = 1;

    return MPI_SUCCESS;
}
