/*
 *  $Id: pack.c,v 1.7 1994/09/01 22:08:34 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: pack.c,v 1.7 1994/09/01 22:08:34 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

#ifndef MPIR_TRUE
#define MPIR_TRUE  1
#define MPIR_FALSE 0
#endif


/*@
    MPI_Pack - Packs a datatype into contiguous memory

Input Parameters:
. inbuf - input buffer start (choice) 
. incount - number of input data items (integer) 
. datatype - datatype of each input data item (handle) 
. outcount - output buffer size, in bytes (integer) 
. position - current position in buffer, in bytes (integer) 
. comm - communicator for packed message (handle) 

Output Parameter:
. outbuf - output buffer start (choice) 

@*/
int MPI_Pack ( inbuf, incount, type, outbuf, outcount, position, comm )
void         *inbuf;
int           incount;
MPI_Datatype  type;
void         *outbuf;
int           outcount;
int          *position;
MPI_Comm      comm;
{
  int size, pad, errno = MPI_SUCCESS;

  /* NOT ENOUGH ERROR CHECKING AT PRESENT */
  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,type) ||
      MPIR_TEST_COUNT(comm,incount) || MPIR_TEST_ARG(position) ||
      ( (*position < 0 ) && (errno = MPI_ERR_ARG) ) ) 
      return MPIR_ERROR(comm,errno,"Error in MPI_PACK" );
  
  /* What kind of padding is necessary? */
  pad = (type->align - ((*position) % type->align)) % type->align;

  /* Is there enough room to finish packing the type? */
  MPIR_Pack_size ( incount, type, comm, &size );
  if (((*position) + pad + size) > outcount)
	return MPIR_ERROR(comm, MPI_ERR_LIMIT, "Buffer too small in MPI_PACK");

  /* Figure the pad and adjust position */
  (*position) += pad;
  errno = MPIR_Pack(comm, inbuf, incount, type, (char *)outbuf + (*position));
  if (errno) MPIR_ERROR(comm,errno,"Error in MPI_PACK" );
  (*position) += size;
  return (errno);
}


/*
   This code assumes that we can use char * pointers (previous code 
   incremented pointers by considering them integers, which is even 
   less portable).  Systems that, for example, use word-oriented pointers
   may want to use different code.

   This code is used in dmpi/dmpipk.c to pack data for a device that
   only supports contiguous messages.
 */
int MPIR_Pack ( comm, buf, count, type, dest )
MPI_Comm comm;
void *buf;
int count;
MPI_Datatype type;
void *dest;
{
  int i,j,k;
  int pad = 0;
  int errno = MPI_SUCCESS;
  char *tmp_buf;
  char *lbuf = (char *)buf, *ldest = (char *)dest;

  /* Pack contiguous data */
  /* At this point, if the type is contiguous, it should be
	 a basic type, so we could pack it with something other
	 than memcpy */
	    
  if (type->is_contig) {
#ifdef MPID_HAS_HETERO
    if (MPID_IS_HETERO == 1 &&
	MPIR_Comm_needs_conversion(comm)) {
#if HAS_XDR
      MPIR_Type_XDR_encode(dest, buf, type, count);
#else
    MPIR_ERROR( comm, MPI_ERR_TYPE, 
"Conversion requires XDR which is not available" );
#endif
      return errno;
    }
    else
#endif
    {
      if (type->size * count > 0 && buf == 0)
	  return MPI_ERR_BUFFER;
      memcpy ( dest, buf, type->size * count );
      return errno;
    }
  }


  /* For each of the count arguments, pack data */
  switch (type->dte_type) {

  /* Contiguous types */
  case MPIR_CONTIG:
	errno = 
	    MPIR_Pack ( comm, buf, count * type->count, type->old_type, dest );
	break;

  /* Vector types */
  case MPIR_VECTOR:
  case MPIR_HVECTOR:
	if (count > 1)
	  pad = (type->align - (type->size % type->align)) % type->align;
	tmp_buf = lbuf;
	for (i=0; i<count; i++) {
	  lbuf = tmp_buf;
	  for (j=0; j<type->count; j++) {
		if (errno = MPIR_Pack (comm, lbuf, type->blocklen, 
			       type->old_type, ldest)) break;
		lbuf  += (type->stride);
		if ((j+1) != type->count)
		  ldest += 
		      ((type->blocklen * type->old_type->size) + type->pad);
	  }
	  ldest += ((type->blocklen * type->old_type->size) + pad);
	  tmp_buf += type->extent;
	}
	break;

  /* Indexed types */
  case MPIR_INDEXED:
  case MPIR_HINDEXED:
	if (count > 1)
	  pad = (type->align - (type->size % type->align)) % type->align;
	for (i=0; i<count; i++) {
	  for (j=0;j<type->count; j++) {
		tmp_buf  = lbuf + type->indices[j];
		if (errno = MPIR_Pack (comm, tmp_buf, type->blocklens[j], 
				       type->old_type, ldest)) break;
		if ((j+1) != type->count)
		  ldest += 
		      ((type->blocklens[j]*type->old_type->size)+type->pad);
	  }
	  ldest += ((type->blocklens[j]*type->old_type->size) + pad);
	  lbuf += type->extent;
	}
	break;

  /* Struct type */
  case MPIR_STRUCT:
	if (count > 1)
	  pad = (type->align - (type->size % type->align)) % type->align;
	for (i=0; i<count; i++) {
	  for (j=0;j<type->count; j++) {
		tmp_buf  = lbuf + type->indices[j];
		if (errno = MPIR_Pack(comm, tmp_buf,type->blocklens[j],
				      type->old_types[j], ldest)) break;
		if ((j+1) != type->count)
		  ldest += ((type->blocklens[j] * type->old_types[j]->size) +
						 type->pads[j]);
	  }
	  ldest+=((type->blocklens[type->count-1]*
				   type->old_types[type->count-1]->size)+pad);
	  lbuf +=type->extent;
	}
	break;

  default:
	errno = MPI_ERR_TYPE;
	break;
  }

  /* Everything fell through, must have been successful */
  return errno;
}
