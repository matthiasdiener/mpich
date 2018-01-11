/*
 *  $Id: comm_util.c,v 1.40 1996/07/17 18:04:19 gropp Exp $
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
/* For MPIR_COLLOPS */
#include "mpicoll.h"

/*

MPIR_Comm_make_coll - make a hidden collective communicator
                      from an inter- or intra-communicator assuming
		      that an appropriate number of contexts
		      have been allocated.  An inter-communicator
		      collective can only be made from another
		      inter-communicator.

See comm_create.c for code that creates a visible communicator.
*/
int MPIR_Comm_make_coll ( comm, comm_type )
MPI_Comm       comm;
MPIR_COMM_TYPE comm_type;
{
  MPI_Comm new_comm;
  int      mpi_errno;

  MPIR_ALLOC(new_comm,NEW(struct MPIR_COMMUNICATOR),comm,MPI_ERR_EXHAUSTED,
			"Error creating new communicator" );

  (void) MPIR_Comm_init( new_comm, comm, comm_type );
  MPIR_Attr_dup_tree ( comm, new_comm );

  if (comm_type == MPIR_INTRA) {
    new_comm->recv_context    = comm->recv_context + 1;
    new_comm->send_context    = new_comm->recv_context;
    (void) MPIR_Group_dup ( comm->local_group, &(new_comm->group) );
    (void) MPIR_Group_dup ( comm->local_group, &(new_comm->local_group) );
  }
  else {
    new_comm->recv_context    = comm->recv_context + 1;
    new_comm->send_context    = comm->send_context + 1;
    (void) MPIR_Group_dup ( comm->group, &(new_comm->group) );
    (void) MPIR_Group_dup ( comm->local_group, &(new_comm->local_group) );
  }
  new_comm->local_rank     = new_comm->local_group->local_rank;
  new_comm->lrank_to_grank = new_comm->group->lrank_to_grank;
  new_comm->np             = new_comm->group->np;
  new_comm->collops        = NULL;

  new_comm->comm_coll       = new_comm;  /* a circular reference to myself */
  comm->comm_coll           = new_comm;

  /* Place the same operations on both the input (comm) communicator and
     the private copy (new_comm) */
  MPIR_Comm_collops_init( new_comm, comm_type);
  MPIR_Comm_collops_init( comm, comm_type);

  /* The MPID_Comm_init routine needs the size of the local group, and
     reads it from the new_comm structure */
#ifdef MPI_ADI2
  if ((mpi_errno = MPID_CommInit( comm, new_comm ))) 
      return mpi_errno;
  /* Is that a storage leak ??? Shouldn't we free new_comm if the init
   * fails ? (Or will it get freed higher up ???)
   */
#else
  if ((mpi_errno = MPID_Comm_init( new_comm->ADIctx, comm, new_comm ))) 
      return mpi_errno;
#endif
  
  new_comm->comm_name = 0;

  /* Remember it for the debugger */
  MPIR_Comm_remember(new_comm);

  MPID_THREAD_LOCK_INIT(new_comm->ADIctx,new_comm);
  return(MPI_SUCCESS);
}


/*+

MPIR_Comm_N2_prev - retrieve greatest power of two < size of Comm.

+*/
int MPIR_Comm_N2_prev ( comm, N2_prev )
MPI_Comm comm;
int              *N2_prev;
{
  (*N2_prev) = comm->group->N2_prev;
  return (MPI_SUCCESS);
}


/*+
  MPIR_Dump_comm - utility function to dump a communicator 
+*/
int MPIR_Dump_comm ( comm )
MPI_Comm comm;
{
  int  rank;

  MPIR_Comm_rank ( MPI_COMM_WORLD, &rank );

  printf("[%d] ----- Dumping communicator -----\n", rank );
  if (comm->comm_type == MPIR_INTRA) {
    printf("[%d] Intra-communicator\n",rank);
    printf("[%d] Group\n",rank);
    MPIR_Dump_group ( comm->group );
  }
  else {
    printf("[%d]\tInter-communicator\n",rank);
    printf("[%d] Local group\n",rank);
    MPIR_Dump_group ( comm->local_group );
    printf("[%d] Remote group\n",rank);
    MPIR_Dump_group ( comm->group );
  }
  printf ("[%d] Ref count = %d\n",rank,comm->ref_count);
  /* Assumes context stored as a unsigned long (?) */
  printf ("[%d] Send = %u   Recv =%u\n",
          rank,comm->send_context,comm->recv_context);
  printf ("[%d] permanent = %d\n",rank,comm->permanent);
  return (MPI_SUCCESS);
}

/*+
  MPIR_Intercomm_high - determine a high value for an
                        inter-communicator
+*/
int MPIR_Intercomm_high ( comm, high )
MPI_Comm  comm;
int      *high;
{
  MPI_Status status;
  MPI_Comm   inter = comm->comm_coll;
  MPI_Comm   intra = inter->comm_coll;
  int        rank, rhigh;

  MPIR_Comm_rank ( comm, &rank );

  /* Node 0 determines high value */
  if (rank == 0) {

    /* "Normalize" value for high */
    if (*high)
      (*high) = 1;
    else
      (*high) = 0;

    /* Get the remote high value from remote node 0 and determine */
    /* appropriate high */
    MPI_Sendrecv(  high, 1, MPI_INT, 0, 0, 
                 &rhigh, 1, MPI_INT, 0, 0, inter, &status);
    if ( (*high) == rhigh ) {
      if ( comm->group->lrank_to_grank[0] < 
           comm->local_group->lrank_to_grank[0] )
        (*high) = 1;
      else
        (*high) = 0;
    }
  }

  /* Broadcast high value to all */
  MPI_Bcast ( high, 1, MPI_INT, 0, intra );
  return (MPI_SUCCESS);
}


/*

MPIR_Comm_init  - Initialize some of the elements of a communicator from 
                  an existing one.
*/
int MPIR_Comm_init ( new_comm, comm, comm_type )
MPI_Comm       new_comm, comm;
MPIR_COMM_TYPE comm_type;
{
  MPIR_SET_COOKIE(new_comm,MPIR_COMM_COOKIE);
  new_comm->ADIctx	       = comm->ADIctx;
  new_comm->comm_type	       = comm_type;
  new_comm->comm_cache	       = 0;
  new_comm->error_handler      = 0;
  new_comm->use_return_handler = 0;
  MPI_Errhandler_set( new_comm, comm->error_handler );
  new_comm->ref_count	       = 1;
  new_comm->permanent	       = 0;
  new_comm->collops	       = 0;
  new_comm->attr_cache	       = 0;
  return(MPI_SUCCESS);
}

/*+

MPIR_Comm_get_name - return the print name from the communicator

+*/
int MPIR_Comm_get_name( comm, namep )
MPI_Comm comm;
char **namep;
{
  if (comm->comm_name)
    {
      *namep =  comm->comm_name;
      return  MPI_SUCCESS;
    }
  else
    {
      *namep = "+++unnamed+++";
      return MPI_SUCCESS;	/* We really want to return something else
				 * MPI_ERR_UNNAMED ?
				 */
    }
}

/*+

MPIR_Comm_set_name - give a print name to the communicator

+*/
int MPIR_Comm_set_name( comm, name )
MPI_Comm comm;
char *    name;
{
  int      mpi_errno = MPI_SUCCESS;

  /* Release any previous name */
  if (comm->comm_name)
    {
      FREE(comm->comm_name);
      comm->comm_name = 0;
    }

  /* Assign a new name */
  if (name)
    {
      char * new_string;

      MPIR_ALLOC(new_string,(char *)MALLOC(strlen(name)+1),comm,MPI_ERR_EXHAUSTED,
		 "Error setting communicator name" );
      strcpy(new_string, name);
      comm->comm_name = new_string;
    }

  return mpi_errno;
}

/*+

MPIR_Comm_remember - remember the communicator on the list of 
                     all communicators, and bump the sequence number.
		     Do this only once the communicator is well enough
		     constructed that it makes sense for the debugger	
		     to see it.

+*/
void MPIR_Comm_remember( new_comm )
MPI_Comm new_comm;
{
  /* What about thread locking ? */
  new_comm->comm_next = MPIR_All_communicators.comm_first;
  MPIR_All_communicators.comm_first = new_comm;

  ++MPIR_All_communicators.sequence_number;
}

/*+

MPIR_Comm_forget - forget a communicator which is going away
                   and bump the sequence number.
		   Do this as soon as the destruction begins, so
		   that the debugger doesn't see a partially destroyed
		   communicator.

+*/
void MPIR_Comm_forget( old_comm )
MPI_Comm old_comm;
{
  MPI_Comm *p;

  for (p = &MPIR_All_communicators.comm_first; 
       *p;
       p = &((*p)->comm_next))
    {
      if (*p == old_comm)
	{
	  *p = old_comm->comm_next;
	  break;
	}
    }
  ++MPIR_All_communicators.sequence_number;
}
/* Init the collective ops functions.
 * Default to the ones MPIR provides.
 */

void MPIR_Comm_collops_init( comm, comm_type )
MPI_Comm comm;
MPIR_COMM_TYPE comm_type;
{  
    comm->collops = (comm_type == MPIR_INTRA) ? MPIR_intra_collops :
                                                MPIR_inter_collops ;
    /* Here, we know that these collops are static, but it is still
     * useful to keep the ref count, because it avoids explicit checks
     * when we free them 
     */
    comm->collops->ref_count++;
}

/* Also used in comm_split.c */
#define MPIR_Table_color(table,i) table[(i)]
#define MPIR_Table_key(table,i)   table[((i)+size)]
#define MPIR_Table_next(table,i)  table[((i)+(2*size))]

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
