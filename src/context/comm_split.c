/*
 *  $Id: comm_split.c,v 1.40 1996/06/26 19:27:26 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
#else
#include "mpisys.h"
#endif

/* Also used in comm_util.c */
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

.N fortran

Algorithm:

The current algorithm used has quite a few (read: a lot of) inefficiencies 
that can be removed.  Here is what we do for now

.vb
 1) A table is built of colors, and keys (has a next field also).
 2) The tables of all processes are merged using 'MPI_Allreduce'.
 3) Two contexts are allocated for all the comms to be created.  These
     same two contexts can be used for all created communicators since
     the communicators will not overlap.
 4) If the local process has a color of 'MPI_UNDEFINED', it can return
     a 'NULL' comm. 
 5) The table entries that match the local process color are sorted 
     by key/rank. 
 6) A group is created from the sorted list and a communicator is created
     with this group and the previously allocated contexts.
.ve

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_EXHAUSTED

.seealso: MPI_Comm_free
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
  MPIR_ERROR_DECL;
  static char myname[] = "Error in MPI_COMM_SPLIT";

  /* If we don't have a communicator we don't have anything to do */
  if ( MPIR_TEST_COMM(comm,comm) ||
       ( (comm->comm_type == MPIR_INTER) && (mpi_errno = MPI_ERR_COMM) ) ) {
    (*comm_out) = MPI_COMM_NULL;
    return MPIR_ERROR( comm, mpi_errno, myname );
  }

  /* Create and initialize split table. */
  (void) MPIR_Comm_size ( comm, &size );
  (void) MPIR_Comm_rank ( comm, &rank );
  MPIR_ALLOC(table,(int *) CALLOC ( 2 * 3 * size, sizeof(int) ),
	     comm,MPI_ERR_EXHAUSTED,"Out of space in MPI_COMM_SPLIT" );
  
  table_in = table + (3 * size);
  MPIR_Table_color(table_in,rank) = color;
  MPIR_Table_key(table_in,rank)   = key;

  MPIR_ERROR_PUSH(comm);

  /* Combine the split table. I only have to combine the colors and keys */
  mpi_errno = MPI_Allreduce(table_in, table, size * 2, MPI_INT, MPI_SUM, comm);

  /* Allocate 2 contexts */
  mpi_errno = MPIR_Context_alloc( comm, 2, &context );

  /* If my color is MPI_UNDEFINED, then I'm not in a comm and can */
  /* stop here since there are no more communications with others */
  /* I'll even go ahead and free the 2 contexts I allocated above */
  if ( MPIR_Table_color(table,rank) == MPI_UNDEFINED ) {
      MPIR_ERROR_POP(comm);
      FREE(table);
      (void) MPIR_Context_dealloc( comm, 2, context );
      (*comm_out) = MPI_COMM_NULL;
      return (mpi_errno);
  }

  /* Sort the table */
  (void) MPIR_Sort_split_table ( size, rank, table, &head, &new_size );
     
  /* Create group of processes that share my color */
  MPIR_ALLOC(group_list,(int *) MALLOC ( new_size * sizeof(int) ),
	     comm,MPI_ERR_EXHAUSTED,myname);
  for ( i=0; i<new_size; i++, head=MPIR_Table_next(table,head) )
    group_list[i] = head;
  MPIR_CALL_POP(MPI_Comm_group ( comm, &comm_group ),comm, myname );
  MPIR_CALL_POP(MPI_Group_incl ( comm_group, new_size, group_list, &group ),
		comm, myname );
  MPIR_CALL_POP(MPI_Group_free ( &comm_group ),comm,myname);
  FREE(table);
  FREE(group_list);

  MPIR_ERROR_POP(comm);

  /* Make communicator using contexts allocated */
  MPIR_ALLOC(new_comm,NEW(struct MPIR_COMMUNICATOR),comm, MPI_ERR_EXHAUSTED, 
	     "Out of space in MPI_COMM_SPLIT" );
  *comm_out = new_comm;
  (void) MPIR_Comm_init( new_comm, comm, MPIR_INTRA );

  new_comm->group         = group;
  (void) MPIR_Group_dup ( group, &(new_comm->local_group) );
#ifndef MPI_ADI2
  if ((mpi_errno = MPID_Comm_init( new_comm->ADIctx, comm, new_comm )) )
      return mpi_errno;
#endif  
  new_comm->local_rank	   = new_comm->local_group->local_rank;
  new_comm->lrank_to_grank = new_comm->group->lrank_to_grank;
  new_comm->np             = new_comm->group->np;
  new_comm->send_context   = new_comm->recv_context = context;
  new_comm->comm_name	   = 0;
#ifdef MPI_ADI2
  /* CommInit may need lrank_to_grank, etc */
  if ((mpi_errno = MPID_CommInit( comm, new_comm )) )
      return mpi_errno;
#endif  
  (void) MPIR_Attr_create_tree ( new_comm );
  (void) MPIR_Comm_make_coll( new_comm, MPIR_INTRA );

  /* Remember it for the debugger */
  MPIR_Comm_remember ( new_comm );

  return (mpi_errno);
}


