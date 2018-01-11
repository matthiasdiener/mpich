/*
 *  $Id: dup_fn.c,v 1.8 1996/01/08 19:48:40 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
  
MPI_DUP_FN - A function to simple-mindedly copy attributes  

@*/
int MPIR_dup_fn ( comm, keyval, extra_state, attr_in, attr_out, flag )
MPI_Comm  comm;
int       keyval;
void      *extra_state;
void      *attr_in;
void      *attr_out;
int       *flag;
{
  /* No error checking at present */

  /* Set attr_out, the flag and return success */
  (*(void **)attr_out) = attr_in;
  (*flag) = 1;
  return (MPI_SUCCESS);
}
