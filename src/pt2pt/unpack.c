/*
 *  $Id: unpack.c,v 1.11 1995/01/04 22:15:25 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: unpack.c,v 1.11 1995/01/04 22:15:25 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

#ifndef MPIR_TRUE
#define MPIR_TRUE  1
#define MPIR_FALSE 0
#endif


/*@
    MPI_Unpack - Unpack a datatype into contiguous memory

Input Parameters:
. inbuf - input buffer start (choice) 
. insize - size of input buffer, in bytes (integer) 
. position - current position in bytes (integer) 
. outcount - number of items to be unpacked (integer) 
. datatype - datatype of each output data item (handle) 
. comm - communicator for packed message (handle) 

Output Parameter:
. outbuf - output buffer start (choice) 

@*/
int MPI_Unpack ( inbuf, insize, position, outbuf, outcount, type, comm )
void         *inbuf;
int           insize;
int          *position;
void         *outbuf;
int           outcount;
MPI_Datatype  type;
MPI_Comm      comm;
{
  int size, pad, mpi_errno = MPI_SUCCESS;

  /* NOT ENOUGH ERROR CHECKING AT PRESENT */
  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,type) ||
      MPIR_TEST_COUNT(comm,insize) || MPIR_TEST_ARG(position) ||
      ( (*position < 0 ) && (mpi_errno = MPI_ERR_ARG) ) ) 
      return MPIR_ERROR(comm,mpi_errno,"Error in MPI_UNPACK" );
  
  /* What kind of padding is necessary? */
  pad = (type->align - ((*position) % type->align)) % type->align;

  /* Is the inbuf big enough for the type that's being unpacked? */
  MPIR_Pack_size ( outcount, type, comm, &size );
  if (((*position) + pad + size) > insize)
	return MPIR_ERROR(comm, MPI_ERR_LIMIT, 
				       "Input buffer too small in MPI_UNPACK");

  /* Figure the pad and adjust position */
  (*position) += pad;
  mpi_errno = MPIR_Unpack( outbuf, outcount, type, 
			  (char *)inbuf + (*position));
  if (mpi_errno) MPIR_ERROR(comm,mpi_errno,"Error in MPI_UNPACK" );
  (*position) += size;
  return (mpi_errno);
}


/*
   This code assumes that we can use char * pointers (previous code 
   incremented pointers by considering them integers, which is even 
   less portable).  Systems that, for example, use word-oriented pointers
   may want to use different code.

   This code is used in dmpi/dmpipk.c to unpack data from a device that
   only supports contiguous messages.
 */
int MPIR_Unpack ( buf, count, type, in )
void         *buf;
int          count;
MPI_Datatype type;
void         *in;
{
  int i,j,k;
  int pad = 0;
  int mpi_errno = MPI_SUCCESS;
  char *tmp_buf;
  char *lbuf = (char *)buf, *lin = (char *)in;
  int  len;

  if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,type))
	return MPIR_ERROR(MPI_COMM_WORLD, mpi_errno, 
			  "Internal Error in MPIR_UNPACK");

  /* If all you did was READ the ANSI C standard, you'd think the next line
     was unnecessary.  Silly you.  Without this, on rs6000s and SPx's, 
     you will get erroneous behavior.  Configure no longer allows you to
     build on these systems
    */
  /* mpi_errno = MPI_SUCCESS; */
  /* printf( ".unpack. %x <- %x (%d types)\n", buf, in, count ); */
  /* Unpack contiguous data */
  if (type->is_contig) {
      len = type->size * count;
      if (len == 0) return mpi_errno;
      if (buf == 0) 
	  return MPI_ERR_BUFFER;
      /* printf( ".copy. %x <- %x (%d)\n", buf, in, len ); */
      memcpy ( buf, in, len );
      return MPI_SUCCESS;
      }

  /* For each of the count arguments, unpack data */
  switch (type->dte_type) {

  /* Contiguous types */
  case MPIR_CONTIG:
	mpi_errno = MPIR_Unpack ( buf, count * type->count, type->old_type, 
				  in );
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
		if (mpi_errno = MPIR_Unpack (lbuf, type->blocklen, 
					     type->old_type, lin)) 
		    return mpi_errno;
		lbuf  += (type->stride);
		if ((j+1) != type->count)
		  lin += ((type->blocklen * type->old_type->size) + type->pad);
	  }
	  lin += ((type->blocklen * type->old_type->size) + pad);
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
		if (mpi_errno = MPIR_Unpack (tmp_buf, type->blocklens[j], 
					     type->old_type, lin)) 
		    return mpi_errno;
		lin += (type->blocklens[j]*type->old_type->size);
		if ((j+1) != type->count)
		    lin += type->pad;
		}
	    lin += pad;
	    lbuf += type->extent;
	    }
	break;

  /* Struct type */
  case MPIR_STRUCT:
	/* printf( ".struct.\n" ); */
	if (count > 1)
	  pad = (type->align - (type->size % type->align)) % type->align;
	for (i=0; i<count; i++) {
	    /* printf( ".struct.[%d]\n", i ); */
	  for (j=0;j<type->count; j++) {
		tmp_buf  = lbuf + type->indices[j];
		if (mpi_errno = MPIR_Unpack(tmp_buf,type->blocklens[j],
					type->old_types[j],lin)) {
		    /* printf( ".!error return %d\n", mpi_errno ); */
		    return mpi_errno;
		    }
		if ((j+1) != type->count)
		  lin += ((type->blocklens[j] * type->old_types[j]->size) +
						 type->pads[j]);
	  }
	  lin+=((type->blocklens[type->count-1]*
				   type->old_types[type->count-1]->size)+pad);
	  lbuf +=type->extent;
	}
	break;

  default:
	mpi_errno = MPI_ERR_TYPE;
	break;
  }

  /* Everything fell through, must have been successful */
  return mpi_errno;
}
