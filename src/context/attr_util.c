/*
 *  $Id: attr_util.c,v 1.22 1996/01/08 19:48:34 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

/* 

MPIR_Attr_copy_node -

 */
int MPIR_Attr_copy_node ( comm, comm_new, node )
MPI_Comm comm, comm_new;
MPIR_HBT_node *node;
{
  void          *attr_val;
  MPIR_Attr_key *attr_key;
  MPIR_HBT_node *attr;
  int            flag;
  int            attr_ival;
  int            mpi_errno = MPI_SUCCESS;

#ifdef INT_LT_POINTER
  attr_key = (MPIR_Attr_key *)MPIR_ToPointer( node->keyval );
#else
  attr_key = (MPIR_Attr_key *)(node->keyval);
#endif

  attr_key->ref_count ++;
  if (attr_key->copy_fn != (int (*)())0) {
      if (attr_key->FortranCalling) {
	  /* The following code attempts to suppress warnings about 
	     converting an int, stored in a void *, back to an int. */
	  /* We may also need to do something about the "comm" argument */
	  MPI_Aint  invall = (MPI_Aint)node->value;
          int inval = (int)invall;
          mpi_errno = (*(attr_key->copy_fn))(comm, &node->keyval, 
                                             attr_key->extra_state,
                                             &inval, 
                                             &attr_ival, &flag );
          attr_val = (void *)attr_ival;
          flag = MPIR_FROM_FLOG(flag);
	  }
      else {
          mpi_errno = (*(attr_key->copy_fn))(comm, node->keyval, 
                                             attr_key->extra_state,
                                             node->value, &attr_val, &flag );
      }
      if (flag) {
          (void) MPIR_HBT_new_node ( node->keyval, attr_val, &attr );
          (void) MPIR_HBT_insert ( comm_new->attr_cache, attr );
      }
  }

  if (mpi_errno) 
      return MPIR_ERROR( comm, mpi_errno,
			 "Error copying communicator attribute" );
   return MPI_SUCCESS;
}

/*+

MPIR_Attr_copy_subtree -

+*/
int MPIR_Attr_copy_subtree ( comm, comm_new, tree, subtree )
MPI_Comm comm, comm_new;
MPIR_HBT *tree;
MPIR_HBT_node *subtree;
{
  int tmp_mpi_errno, mpi_errno = MPI_SUCCESS;

  if(subtree != (MPIR_HBT_node *)0) {
      tmp_mpi_errno=MPIR_Attr_copy_node ( comm, comm_new, subtree );
      if (tmp_mpi_errno != MPI_SUCCESS) mpi_errno = tmp_mpi_errno;
      
      tmp_mpi_errno=MPIR_Attr_copy_subtree(comm,comm_new,tree,subtree->left);
      if (tmp_mpi_errno != MPI_SUCCESS) mpi_errno = tmp_mpi_errno;

      tmp_mpi_errno=MPIR_Attr_copy_subtree(comm,comm_new,tree,subtree->right);
      if (tmp_mpi_errno != MPI_SUCCESS) mpi_errno = tmp_mpi_errno;
  }
  return (mpi_errno);
}

/*+

MPIR_Attr_copy - copy a tree of attributes 

+*/
int MPIR_Attr_copy ( comm, comm_new )
MPI_Comm comm, comm_new;
{
  int mpi_errno = MPI_SUCCESS;

  (void) MPIR_HBT_new_tree ( &(comm_new->attr_cache) );
  if ( comm_new->attr_cache != (MPIR_HBT *)0 ) {
    comm_new->attr_cache->ref_count = 1;
    mpi_errno = MPIR_Attr_copy_subtree (comm, comm_new, comm_new->attr_cache,
                                        comm->attr_cache->root);
  }
  return (mpi_errno);
}


/*+

MPIR_Attr_free_node -

+*/
int MPIR_Attr_free_node ( comm, node )
MPI_Comm comm;
MPIR_HBT_node *node;
{
  MPIR_Attr_key *attr_key;

#ifdef INT_LT_POINTER
  attr_key = (MPIR_Attr_key *)MPIR_ToPointer( node->keyval );
#else
  attr_key = (MPIR_Attr_key *)(node->keyval);
#endif

  if ( (node != (MPIR_HBT_node *)0) && (attr_key != 0) ) {
      attr_key->ref_count --;
    if ( attr_key->delete_fn != (int (*)())0 ) {
	if (attr_key->FortranCalling) {
	    MPI_Aint  invall = (MPI_Aint)node->value;
	    int inval = (int)invall;
	    /* We may also need to do something about the "comm" argument */
	    (void ) (*(attr_key->delete_fn))(comm, &node->keyval, 
					     &inval, 
					     attr_key->extra_state);
	    node->value = (void *)inval;
	    }
	else
	    (void ) (*(attr_key->delete_fn))(comm, node->keyval, 
					     node->value, 
					     attr_key->extra_state);
	}
    }
  return (MPI_SUCCESS);
}

/*+

MPIR_Attr_free_subtree -

+*/
int MPIR_Attr_free_subtree ( comm, subtree )
MPI_Comm comm;
MPIR_HBT_node *subtree;
{
  if(subtree != (MPIR_HBT_node *)0) {
    (void) MPIR_Attr_free_subtree ( comm, subtree -> left );
    (void) MPIR_Attr_free_subtree ( comm, subtree -> right );
    (void) MPIR_Attr_free_node ( comm, subtree );
  }
  return (MPI_SUCCESS);
}

/*+

MPIR_Attr_free_tree -

+*/
int MPIR_Attr_free_tree ( comm )
MPI_Comm comm;
{
  if ( ( comm != MPI_COMM_NULL ) && ( comm->attr_cache != (MPIR_HBT *)0 ) ) {
    if (comm->attr_cache->ref_count <= 1) {
      if ( comm->attr_cache->root != (MPIR_HBT_node *)0 )
        (void) MPIR_Attr_free_subtree ( comm, comm->attr_cache->root );
      (void) MPIR_HBT_free_tree ( comm->attr_cache );
    }
    else
      comm->attr_cache->ref_count--;
  }
  return (MPI_SUCCESS);
}

/*+

MPIR_Attr_dup_tree -

+*/
int MPIR_Attr_dup_tree ( comm, new_comm )
MPI_Comm comm, new_comm;
{
  if ( comm->attr_cache != (MPIR_HBT *)0 )
    comm->attr_cache->ref_count++;
  new_comm->attr_cache = comm->attr_cache;
  return (MPI_SUCCESS);
}

/*+

MPIR_Attr_create_tree -

+*/
int MPIR_Attr_create_tree ( comm )
MPI_Comm comm;
{
  (void) MPIR_HBT_new_tree ( &(comm->attr_cache) );
  comm->attr_cache->ref_count = 1;
  return (MPI_SUCCESS);
}

int MPIR_Keyval_create ( copy_fn, delete_fn, keyval, extra_state, is_fortran )
MPI_Copy_function   *copy_fn;
MPI_Delete_function *delete_fn;
int                 *keyval;
void                *extra_state;
int                 is_fortran;
{
  MPIR_Attr_key *new_key = NEW(MPIR_Attr_key);

  if (!new_key) {
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
				  "Out of space in MPI_Keyval_create" );
  }

  /* This still requires work in the Fortran interface, in case
     sizeof(int) == sizeof(double) = sizeof(void*) */
#ifdef INT_LT_POINTER
  (*keyval)		  = MPIR_FromPointer( (void *)new_key );
#else  
  (*keyval)		  = (int)new_key;
#endif
  /* SEE ALSO THE CODE IN ENV/INIT.C; IT RELIES ON USING KEY AS THE
     POINTER TO SET THE PERMANENT FIELD */
  new_key->copy_fn	  = copy_fn;
  new_key->delete_fn	  = delete_fn;
  new_key->ref_count	  = 1;
  new_key->extra_state	  = extra_state;
  new_key->permanent	  = 0;
  new_key->FortranCalling = is_fortran;
  return (MPI_SUCCESS);
}
