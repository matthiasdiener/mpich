/*
 *  $Id: attr_util.c,v 1.25 1996/06/07 15:08:25 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "attr.h"
#include "mpifort.h"
#include "mpimem.h"
#else
#include "mpisys.h"
#endif

/* #define DEBUG_ATTR */

/*
 * Comments on the attribute mechanism
 *
 * When a communicator is dup'ed, we need to copy the attributes (as long
 * as they want to be copied).  In addition, the communicator implementation
 * uses a "private" communicator which is a shallow dup of the communicator;
 * this is implemented by using a reference count on the attribute TREE.
 *
 * Also note that the keyvals are shared, and are actually freed only when 
 * there are no references to them (this is the attr_key->ref_count).
 * 
 */

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

  if (!attr_key MPIR_TEST_COOKIE(attr_key,MPIR_ATTR_COOKIE)) {
      return MPIR_ERROR( comm, MPI_ERR_INTERN, "Corrupted attribute key" );
  }
#ifdef FOO
  attr_key->ref_count ++;
#ifdef DEBUG_ATTR
  PRINTF( "incr attr_key ref to %d for %ld in %ld, copy to comm %ld\n", 
	  attr_key->ref_count, (long)attr_key, (long)comm, (long)comm_new );
#endif
#endif
  if (attr_key->copy_fn.c_copy_fn) {
      if (attr_key->FortranCalling) {
	  /* The following code attempts to suppress warnings about 
	     converting an int, stored in a void *, back to an int. */
	  /* We may also need to do something about the "comm" argument */
	  MPI_Aint  invall = (MPI_Aint)node->value;
          int inval = (int)invall;
          mpi_errno = (*(attr_key->copy_fn.f77_copy_fn))(comm, &node->keyval, 
                                             attr_key->extra_state,
                                             &inval, 
                                             &attr_ival, &flag );
          attr_val = (void *)(MPI_Aint)attr_ival;
          flag = MPIR_FROM_FLOG(flag);
	  }
      else {
          mpi_errno = (*(attr_key->copy_fn.c_copy_fn))(comm, node->keyval, 
                                             attr_key->extra_state,
                                             node->value, &attr_val, &flag );
      }
      if (flag) {
#ifdef DEBUG_ATTR
	  PRINTF( ".. inserting attr into comm %ld\n", comm_new );
#endif	  
  attr_key->ref_count ++;
#ifdef DEBUG_ATTR
  PRINTF( "incr attr_key ref to %d for %ld in %ld, copy to comm %ld\n", 
	  attr_key->ref_count, (long)attr_key, (long)comm, (long)comm_new );
#endif
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
MPIR_HBT tree;
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

#ifdef DEBUG_ATTR
  PRINTF( "Copy: copying attr tree to comm %ld from %ld\n", 
	  (long)comm_new, (long)comm );
#endif
  (void) MPIR_HBT_new_tree ( &(comm_new->attr_cache) );
  if ( comm_new->attr_cache != (MPIR_HBT)0 ) {
#ifdef DEBUG_ATTR
      PRINTF( "setting attr_cache %ld ref_count to 1 in comm %ld\n", 
	      (long)comm_new->attr_cache, (long)comm_new );
#endif
    comm_new->attr_cache->ref_count = 1;
    mpi_errno = MPIR_Attr_copy_subtree (comm, comm_new, comm_new->attr_cache,
                                        comm->attr_cache->root);
  }
#ifdef DEBUG_ATTR
  PRINTF( "Copy: done copying attr tree\n" );
#endif
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

  if (!attr_key MPIR_TEST_COOKIE(attr_key,MPIR_ATTR_COOKIE)) {
      return MPIR_ERROR( comm, MPI_ERR_INTERN, "Corrupted attribute key" );
  }

  if ( (node != (MPIR_HBT_node *)0) && (attr_key != 0) ) {
      attr_key->ref_count --;
#ifdef DEBUG_ATTR
  PRINTF( "decr attr_key ref to %d for attr %ld in comm %ld\n", 
	  attr_key->ref_count, (long)attr_key, (long)comm );
#endif
    if ( attr_key->delete_fn.c_delete_fn ) {
	if (attr_key->FortranCalling) {
	    MPI_Aint  invall = (MPI_Aint)node->value;
	    int inval = (int)invall;
	    /* We may also need to do something about the "comm" argument */
	    (void ) (*(attr_key->delete_fn.f77_delete_fn))(comm, 
					     &node->keyval, 
					     &inval, 
					     attr_key->extra_state);
	    node->value = (void *)(MPI_Aint)inval;
	    }
	else
	    (void ) (*(attr_key->delete_fn.c_delete_fn))(comm, node->keyval, 
					     node->value, 
					     attr_key->extra_state);
	}
    if (attr_key->ref_count <= 0) {
	MPIR_SET_COOKIE(attr_key,0);
	FREE( attr_key );
#ifdef INT_LT_POINTER
	MPIR_RmPointer( node->keyval );
#endif
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
#ifdef DEBUG_ATTR
   PRINTF( "FreeTree:Freeing attr tree for %ld, attr cache %ld\n", (long) comm,
	   (long)comm->attr_cache );
#endif
  if ( ( comm != MPI_COMM_NULL ) && ( comm->attr_cache != (MPIR_HBT)0 ) ) {
    if (comm->attr_cache->ref_count <= 1) {
      if ( comm->attr_cache->root != (MPIR_HBT_node *)0 )
        (void) MPIR_Attr_free_subtree ( comm, comm->attr_cache->root );
      (void) MPIR_HBT_free_tree ( comm->attr_cache );
    }
    else {
#ifdef DEBUG_ATTR
	PRINTF( "Decrementing attr_cache %ld ref count for comm %ld to %d\n", 
		(long)comm->attr_cache, 
		(long)comm, comm->attr_cache->ref_count-1  );
#endif	
      comm->attr_cache->ref_count--;
    }
  }
#ifdef DEBUG_ATTR
  if (comm->attr_cache)
      PRINTF( "attr_cache count is %d\n", comm->attr_cache->ref_count );
  else
      PRINTF( "No attr cache\n" );
  PRINTF( "FreeTree: done\n" );
#endif
  return (MPI_SUCCESS);
}

/*+

MPIR_Attr_dup_tree -

Dups a tree.  Used ONLY in the creation of a communicator for the 
implementation of the collective routines by point-to-point routines
(see comm_util.c/MPIR_Comm_make_coll)

+*/
int MPIR_Attr_dup_tree ( comm, new_comm )
MPI_Comm comm, new_comm;
{
  if ( comm->attr_cache != (MPIR_HBT)0 )
    comm->attr_cache->ref_count++;
  new_comm->attr_cache = comm->attr_cache;
#ifdef DEBUG_ATTR
    PRINTF( "Incr attr_cache (%ld) ref count to %d in comm %ld for dup\n", 
	    (long)comm->attr_cache, comm->attr_cache->ref_count, (long)comm );
#endif
  return (MPI_SUCCESS);
}

/*+

MPIR_Attr_create_tree -

+*/
int MPIR_Attr_create_tree ( comm )
MPI_Comm comm;
{
  (void) MPIR_HBT_new_tree ( &(comm->attr_cache) );
#ifdef DEBUG_ATTR
  PRINTF( "Setting attr cache (%ld) ref_count to 1 for comm %ld\n", 
	  (long)comm->attr_cache, (long) comm );
#endif
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
  MPIR_Attr_key *new_key;

  MPIR_ALLOC(new_key,NEW(MPIR_Attr_key),MPI_COMM_WORLD,MPI_ERR_EXHAUSTED, 
				  "Out of space in MPI_KEYVAL_CREATE" );

  /* This still requires work in the Fortran interface, in case
     sizeof(int) == sizeof(double) = sizeof(void*) */
#ifdef INT_LT_POINTER
  (*keyval)		  = MPIR_FromPointer( (void *)new_key );
#else  
  (*keyval)		  = (int)new_key;
#endif
  /* SEE ALSO THE CODE IN ENV/INIT.C; IT RELIES ON USING KEY AS THE
     POINTER TO SET THE PERMANENT FIELD */
  if (is_fortran) {
      new_key->copy_fn.f77_copy_fn      = 
	  (int (*)ANSI_ARGS(( MPI_Comm, int *, int *, int *, int *, 
				   int * )))copy_fn;
      new_key->delete_fn.f77_delete_fn  = 
	  (int (*)ANSI_ARGS(( MPI_Comm, int *, int *, void *)))delete_fn;
  }
  else {
      new_key->copy_fn.c_copy_fn      = copy_fn;
      new_key->delete_fn.c_delete_fn  = delete_fn;
  }
  new_key->ref_count	  = 1;
  new_key->extra_state	  = extra_state;
  new_key->permanent	  = 0;
  new_key->FortranCalling = is_fortran;
  MPIR_SET_COOKIE(new_key,MPIR_ATTR_COOKIE)
  return (MPI_SUCCESS);
}

/*
 * This routine is called to make a keyval permanent (used in the init routine)
 */
void MPIR_Attr_make_perm( keyval )
int keyval;
{
    MPIR_Attr_key *attr_key;

#ifdef INT_LT_POINTER
    attr_key = (MPIR_Attr_key *)MPIR_ToPointer( keyval );
#else
    attr_key = (MPIR_Attr_key *)keyval;
#endif
    attr_key->permanent = 1;
}
