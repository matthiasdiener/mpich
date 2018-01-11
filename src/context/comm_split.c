/*
 *  $Id: comm_split.c,v 1.36 1995/06/21 03:05:47 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#include "mpisys.h"

#define MPIR_Table_color(table,i) table[(i)]
#define MPIR_Table_key(table,i)   table[((i)+size)]
#define MPIR_Table_next(table,i)  table[((i)+(2*size))]

/*@

MPI_Comm_split - Creates new communicators based on colors and keys

Input Parameters:
. comm - communicator (handle) 
. color - control of subset assignment (nonnegative integer) 
. key - control of rank assigment (integer) 

Output Parameter:
. newcomm - new communicator (handle) 

Notes:
  The 'color' must be non-negative or 'MPI_UNDEFINED'.
Algorithm:

The current algorithm used has quite a few (read: a lot of) inefficiencies 
that can be removed.  Here's what we do for now

.vb
 1) A table is built of colors, and keys (has a next field also).
 2) The tables of all processes are merged using MPI_Allreduce
 3) Two contexts are allocated for all the comms to be created.  These
     same two contexts can be used for all created communicators since
     the communicators will not overlap.
 4) If the local process has a color of MPI_UNDEFINED, it can return
     a NULL comm. 
 5) The table entries that match the local process color are sorted 
     by key/rank. 
 6) A group is created from the sorted list and a communicator is created
     with this group and the previously allocated contexts.
.ve
@*/
int MPI_Comm_split ( comm, color, key, comm_out )
MPI_Comm  comm;
int       color, key;
MPI_Comm *comm_out;
{
  int           size, rank, head, new_size, i;
  int          *table, *table_in;
  MPIR_CONTEXT  context;
  int          *group_list;
  MPI_Group     comm_group, group;
  MPI_Comm      new_comm;
  int           mpi_errno = MPI_SUCCESS;

  /* If we don't have a communicator we don't have anything to do */
  if ( MPIR_TEST_COMM(comm,comm) ||
       ( (comm->comm_type == MPIR_INTER) && (mpi_errno = MPI_ERR_COMM) ) ) {
    (*comm_out) = MPI_COMM_NULL;
    return MPIR_ERROR( comm, mpi_errno, "Error in MPI_COMM_SPLIT" );
  }

  /* Create and initialize split table. */
  (void) MPIR_Comm_size ( comm, &size );
  (void) MPIR_Comm_rank ( comm, &rank );
  table = (int *) CALLOC ( 2 * 3 * size, sizeof(int) );
  table_in = table + (3 * size);
  if (!table) 
	return MPIR_ERROR( comm, MPI_ERR_EXHAUSTED,
					  "Out of space in MPI_COMM_SPLIT" );
  MPIR_Table_color(table_in,rank) = color;
  MPIR_Table_key(table_in,rank)   = key;

  /* Combine the split table. I only have to combine the colors and keys */
  (void) MPI_Allreduce(table_in, table, size * 2, MPI_INT, MPI_SUM, comm);

  /* Allocate 2 contexts */
  (void) MPIR_Context_alloc( comm, 2, &context );

  /* If my color is MPI_UNDEFINED, then I'm not in a comm and can */
  /* stop here since there are no more communications with others */
  /* I'll even go ahead and free the 2 contexts I allocated above */
  if ( MPIR_Table_color(table,rank) == MPI_UNDEFINED ) {
    FREE(table);
    (void) MPIR_Context_dealloc( comm, 2, context );
    (*comm_out) = MPI_COMM_NULL;
    return (mpi_errno);
  }

  /* Sort the table */
  (void) MPIR_Sort_split_table ( size, rank, table, &head, &new_size );
     
  /* Create group of processes that share my color */
  group_list = (int *) MALLOC ( new_size * sizeof(int) );
  for ( i=0; i<new_size; i++, head=MPIR_Table_next(table,head) )
    group_list[i] = head;
  (void) MPI_Comm_group ( comm, &comm_group );
  (void) MPI_Group_incl ( comm_group, new_size, group_list, &group );
  (void) MPI_Group_free ( &comm_group );
  FREE(table);
  FREE(group_list);

  /* Make communicator using contexts allocated */
  new_comm                = (*comm_out) = NEW(struct MPIR_COMMUNICATOR);
  if (!new_comm)
	return MPIR_ERROR( comm, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_COMM_SPLIT" );
  (void) MPIR_Comm_init( new_comm, comm, MPIR_INTRA );

  new_comm->group         = group;
  MPIR_Group_dup ( group, &(new_comm->local_group) );
  if (mpi_errno = MPID_Comm_init( new_comm->ADIctx, comm, new_comm )) 
      return mpi_errno;
  new_comm->local_rank	   = new_comm->local_group->local_rank;
  new_comm->lrank_to_grank = new_comm->group->lrank_to_grank;
  new_comm->np             = new_comm->group->np;
  new_comm->send_context   = new_comm->recv_context = context;
  (void) MPIR_Attr_create_tree ( new_comm );
  (void) MPIR_Comm_make_coll( new_comm, MPIR_INTRA );

  return (mpi_errno);
}


/*+

MPIR_Sort_split_table - sort split table using a yuckie sort (YUCK!).  I'll
                        switch to a more efficient sort one of these days.

+*/
#define MPIR_EOTABLE -1
int MPIR_Sort_split_table ( size, rank, table, head, list_size )
int  size, rank;
int *table;
int *head, *list_size;
{
  int i, j, prev;
  int color = MPIR_Table_color(table,rank);
  
  /* Initialize head and list size */
  (*head)      = MPIR_EOTABLE;
  (*list_size) = 0;

  /* Sort out only colors == to my rank's color */
  for ( i=0; i<size; i++ ) {
    for (prev = MPIR_EOTABLE, j=(*head);
		 j != MPIR_EOTABLE; prev = j, j = MPIR_Table_next(table,j)) {
      if ( MPIR_Table_color(table,i) != color )
        continue;
      if ( (j==MPIR_EOTABLE) || (MPIR_Table_key(table,i)<MPIR_Table_key(table,j)) )
	break;
    }
    if ( MPIR_Table_color(table,i) == color) {
      (*list_size)++;
      MPIR_Table_next(table,i) = j;
      if (prev == MPIR_EOTABLE)
        (*head) = i;
      else
        MPIR_Table_next(table,prev) = i;
    }
  }
  return MPI_SUCCESS;
}
