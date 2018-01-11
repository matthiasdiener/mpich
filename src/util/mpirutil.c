/*
 *  $Id: mpirutil.c,v 1.23 1997/01/07 01:47:07 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


/* mpir helper routines
*/

#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif


int MPIR_Tab ANSI_ARGS(( int ));

#ifndef MPI_ADI2
void MPIR_dump_rhandle(handle)
MPIR_RHANDLE handle;
{
    PRINTF("  handle type = %d\n", handle.handle_type);
    PRINTF("  source      = %d\n", handle.source);
    PRINTF("  tag         = %d\n", handle.tag);
    PRINTF("  completed   = %d\n", handle.completer);
    PRINTF("  datatype    = "); MPIR_dump_dte(handle.datatype,0);
}

void MPIR_dump_shandle(handle)
MPIR_SHANDLE handle;
{
    PRINTF("  handle type = %d\n", handle.handle_type);
    PRINTF("  dest =        %d\n", handle.dest);
    PRINTF("  tag =         %d\n", handle.tag);
    PRINTF("  mode =        %d\n", handle.mode);
    PRINTF("  completed   = %d\n", handle.completer);
    PRINTF("  datatype    = "); MPIR_dump_dte(handle.datatype,0);
}
#endif

#ifndef MPI_ADI2
/*
   Queueing unexpected messages and searching for them requires particular
   care because of the three cases:
   tag              source            Ordering
   xxx              xxx               Earliest message in delivery order
   MPI_ANY_TAG      xxx               Earliest with given source, not 
                                      lowest in tag value
   xxx              MPI_ANY_SOURCE    Earliest with given tag, not lowest
                                      in source.

   Note that only the first case is explicitly required by the MPI spec.
   However, there is a requirement on progress that requires SOME mechanism;
   the one here is easy to articulate and matches what many users expect.
   
   There are many comprimises that must be considered when deciding how to
   provide for these different criteria.  The code here optimizes for the
   case when both tag and source are specified.  An additional set of links,
   providing raw delivery order, allows a sequential search of the list 
   for the other two cases.  This isn't optimal, particularly for the
   case where the tag is specified and the source is MPI_ANY_SOURCE, but
   will work.

   An enhancment of this approach is to include a delivery-order sequence 
   number in the queue elements; 

   The current system does a linear search through the entire list, and 
   thus will always give the earliest in delivery order AS RECEIVED BY THE
   ADI.  We've had trouble with message-passing systems that the ADI is
   using not providing a fair delivers (starving some sources); this 
   should be fixed by the ADI or underlying message-passing system rather
   than by this level of MPI routine.

   There are no longer any send queues, and hence only receive queues
   are of interest.  We'll leave this in in case we need it for cancel (!)
 */
void MPIR_dump_queue(header)
MPIR_QHDR *header;
{
    MPIR_QEL *p;

    if (!header) return;
    PRINTF("first = 0x%x, last = 0x%x, maxlen = %d, currlen = %d\n",
	   (long)header->first, (long)header->last, 
	   header->maxlen, header->currlen );
    p = header->first;
    while ( p )
    {
	switch ( p->qel_type )
	{
	  case MPIR_QSHANDLE:
	    PRINTF("queued send handle:\n");
	    MPIR_dump_shandle( *((MPIR_SHANDLE *) p->ptr) );
	    break;
	  case MPIR_QRHANDLE:
	    PRINTF("queued recv handle:\n");
	    MPIR_dump_rhandle( *((MPIR_RHANDLE *) p->ptr) );
	    break;
	}
	p = p->next;
    }
}

/* Made specific to MPI */

int MPIR_enqueue(header, object, object_type)
MPIR_QHDR     *header;
MPIR_COMMON   *object;
MPIR_QEL_TYPE object_type;
{
    MPIR_QEL *p;

    header->currlen++;
    if (header->currlen > header->maxlen)
	header->maxlen++;

    p           = (MPIR_QEL *) MPIR_SBalloc( MPIR_qels );
    if (!p) {
	MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_EXHAUSTED,
		   "Allocating message queue");
	MPI_Abort( MPI_COMM_WORLD, 1 );
    }
    p->ptr      = (void *)object;
    /* Store the selection criteria is an easy-to-get place */
    p->context_id = object->contextid;
    if (object->tag == MPI_ANY_TAG) {
	p->tag     = 0; 
	p->tagmask = 0;
	}
    else {
	p->tag     = object->tag; 
	p->tagmask = ~0;
	}
    if (object->partner == MPI_ANY_SOURCE) {
	p->lsrc    = 0; 
	p->srcmask = 0;
	}
    else {
	p->lsrc    = object->partner; 
	p->srcmask = ~0;
	}
    p->qel_type = object_type;

    if (header->first == NULL)
    {
	header->first	    = header->last = p;
/* 	header->deliv_first = header->deliv_last = p; */
	p->prev		    = p->next      = NULL;
    }
    else
    {
	p->prev            = header->last;
	p->next            = NULL;
	header->last->next = p;
	header->last       = p;
    }
return MPI_SUCCESS;
}

int MPIR_dequeue(header, object)
MPIR_QHDR *header;
void      *object;
{
    MPIR_QEL *p;

    p = header->first;
    while ( p != NULL )
    {
	if ( p->ptr == object )
	    break;
	else
	    p = p->next;
    }
    if ( p == NULL )
	return MPIR_ERROR((MPI_Comm)0, 
			  MPI_ERR_INTERN,"MPIR_dequeue: not found");
    else
    {
	if ( p->next != NULL )
	    p->next->prev = p->prev;
	else
	    header->last  = p->prev;

	if ( p->prev != NULL )
	    p->prev->next = p->next;
	else
	    header->first = p->next;
	header->currlen--;
	MPIR_SBfree( MPIR_qels, p );	/* free queue element */
    }	    
return MPI_SUCCESS;
}

/* search posted_receive queue for message matching criteria */
/* CHANGE TO ALLOW SEARCH AND DELETE.  NEED TO INSURE THAT ONCE MATCHED,
   A POSTED MESSAGE DOESN'T MATCH AGAIN.  Note that we need to match without
   deleting so that we can implement probe 

   A flag of 1 causes a successful search to delete the element found 
 */
int MPIR_search_posted_queue( src, tag, context_id, found, flag, handleptr )
register int          src, tag;
register MPIR_CONTEXT context_id;
int                   *found;
MPIR_RHANDLE          **handleptr;
int                   flag;
{
    MPIR_QHDR    *queue = &MPIR_posted_recvs;
    MPIR_QEL     *p;

    p      = queue->first;
    *found = 0;
    /* Eventually, we'll want to separate this into the three cases
       described above.  We may also want different queues for each
       context_id 
       The tests may become

       if (tag == MPI_ANY_TAG) 
         if (tag == MPI_ANY_SOURCE) 
	    take first message with matching context
         else
            search for context_id, source
       else if (SOURCE == MPI_ANY_SOURCE)
         search for context_id, tag
       else
         search for context_id, tag, source
     */
    while (p)
    {
	if ( context_id         == p->context_id &&
	     (tag & p->tagmask) == p->tag       &&
	     (src & p->srcmask) == p->lsrc )
	{
	    *found = 1;
	    *handleptr = ( MPIR_RHANDLE * ) p->ptr;
	    if (flag)
	    {
		if ( p->next != NULL )
		    p->next->prev = p->prev;
		else
		    queue->last  = p->prev;

		if ( p->prev != NULL )
		    p->prev->next = p->next;
		else
		    queue->first = p->next;
		queue->currlen--;
		MPIR_SBfree( MPIR_qels, p);	/* free queue element */
	    }
	    break;
	}
	p   = p->next;
    }
return MPI_SUCCESS;
}


/* search unexpected_recv queue for message matching criteria */
/* CHANGE TO ALLOW SEARCH AND DELETE.  NEED TO INSURE THAT ONCE MATCHED,
   A POSTED MESSAGE DOESN'T MATCH AGAIN.  Note that we need to match without
   deleting so that we can implement probe 

   A flag of 1 causes a successful search to delete the element found 
 */
int MPIR_search_unexpected_queue( src, tag, context_id, found, flag,
				  handleptr )
register int   	      src, tag;
register MPIR_CONTEXT context_id;
int                   *found;
MPIR_RHANDLE          **handleptr;
int                   flag;
{
    MPIR_QHDR    *queue = &MPIR_unexpected_recvs;
    MPIR_QEL     *p;
    int          tagmask, srcmask;

    if (tag == MPI_ANY_TAG) {
	tag     = 0;
	tagmask = 0;
	}
    else
	tagmask = ~0;
    if (src == MPI_ANY_SOURCE) {
	src     = 0;
	srcmask = 0;
	}
    else
	srcmask = ~0;

    p      = queue->first;
    *found = 0;
    while (p)
    {
	if ( context_id == p->context_id      &&
	     tag        == (p->tag  & tagmask) &&
	     src        == (p->lsrc & srcmask) )
	{
	    *found = 1;
	    *handleptr = ( MPIR_RHANDLE * ) p->ptr;
	    if (flag)
	    {
		if ( p->next != NULL )
		    p->next->prev = p->prev;
		else
		    queue->last  = p->prev;

		if ( p->prev != NULL )
		    p->prev->next = p->next;
		else
		    queue->first = p->next;
		queue->currlen--;
		MPIR_SBfree( MPIR_qels, p);	/* free queue element */
	    }
	    break;
	}
	p   = p->next;
    }
return MPI_SUCCESS;
}
#endif

int MPIR_dump_dte( dte, indent )
MPI_Datatype  dte;
int           indent;
{
    int i;
    struct MPIR_DATATYPE *dtype_ptr;

    dtype_ptr   = MPIR_GET_DTYPE_PTR(dte);

    switch (dtype_ptr->dte_type)
    {
    case MPIR_INT:
	MPIR_Tab( indent );
	PRINTF( "int\n" );
	break;
    case MPIR_UINT:
	MPIR_Tab( indent );
	PRINTF( "unsigned\n" );
	break;
    case MPIR_FLOAT:
	MPIR_Tab( indent );
	PRINTF( "float\n" );
	break;
    case MPIR_DOUBLE:
	MPIR_Tab( indent );
	PRINTF( "double\n" );
	break;
    case MPIR_BYTE:
	MPIR_Tab( indent );
	PRINTF( "byte\n" );
	break;
    case MPIR_PACKED:
	MPIR_Tab( indent );
	PRINTF( "packed\n" );
	break;
    case MPIR_CHAR:
	MPIR_Tab( indent );
	PRINTF( "char\n" );
	break;
    case MPIR_UCHAR:
	MPIR_Tab( indent );
	PRINTF( "unsigned char\n" );
	break;
    case MPIR_ULONG:
	MPIR_Tab( indent );
	PRINTF( "unsigned long\n" );
	break;
    case MPIR_LONG:
	MPIR_Tab( indent );
	PRINTF( "long\n" );
	break;
    case MPIR_SHORT:
	MPIR_Tab( indent );
	PRINTF( "short\n" );
	break;
    case MPIR_USHORT:
	MPIR_Tab( indent );
	PRINTF( "unsigned short\n" );
	break;
    case MPIR_CONTIG:
	MPIR_Tab( indent );
	PRINTF( "contig, count = %d\n", dtype_ptr->count );
	MPIR_dump_dte( dtype_ptr->old_type->self, indent + 2 );
	break;
    case MPIR_VECTOR:
	MPIR_Tab( indent );
	PRINTF( "vector, count = %d, stride = %ld, blocklen = %d\n",
	       dtype_ptr->count, (long)dtype_ptr->stride, dtype_ptr->blocklen );
	MPIR_dump_dte( dtype_ptr->old_type->self, indent + 2 );
	break;
    case MPIR_HVECTOR:
	MPIR_Tab( indent );
	PRINTF( "hvector, count = %d, stride = %ld, blocklen = %d\n",
	       dtype_ptr->count, (long)dtype_ptr->stride, dtype_ptr->blocklen );
	MPIR_dump_dte( dtype_ptr->old_type->self, indent + 2 );
	break;
    case MPIR_INDEXED:
	MPIR_Tab( indent );
	PRINTF( "indexed, count = %d\n", dtype_ptr->count );
	MPIR_dump_dte( dtype_ptr->old_type->self, indent + 2 );
	for ( i = 0; i < dtype_ptr->count; i++)
	{
	    MPIR_Tab( indent + 4 );
	    PRINTF("index = %ld, blocklen = %d\n",
		   dtype_ptr->indices[i], dtype_ptr->blocklens[i] );
	}
	break;
    case MPIR_HINDEXED:
	MPIR_Tab( indent );
	PRINTF( "hindexed, count = %d\n", dtype_ptr->count );
	MPIR_dump_dte( dtype_ptr->old_type->self, indent + 2 );
	for ( i = 0; i < dtype_ptr->count; i++)
	{
	    MPIR_Tab( indent + 4 );
	    PRINTF("index = %ld, blocklen = %d\n",
		   dtype_ptr->indices[i], dtype_ptr->blocklens[i] );
	}
	break;
    case MPIR_STRUCT:
	MPIR_Tab( indent );
	PRINTF( "struct, count = %d\n", dtype_ptr->count );
	for ( i = 0; i < dtype_ptr->count; i++)
	{
	    MPIR_Tab( indent + 2 );
	    PRINTF("index = %ld, blocklen = %d\n",
		   dtype_ptr->indices[i], dtype_ptr->blocklens[i] );
	    MPIR_dump_dte( dtype_ptr->old_types[i]->self, indent + 2 );
	}
	break;
    case MPIR_COMPLEX:
	MPIR_Tab( indent );
	PRINTF( "complex\n" );
	break;
    case MPIR_DOUBLE_COMPLEX:
	MPIR_Tab( indent );
	PRINTF( "double complex\n" );
	break;
    case MPIR_LONGDOUBLE:
	MPIR_Tab( indent );
	PRINTF( "long double\n" );
	break;
    case MPIR_LONGLONGINT:
	MPIR_Tab( indent );
	PRINTF( "long long int\n" );
	break;
    case MPIR_LOGICAL:
	MPIR_Tab( indent );
	PRINTF( "logical (Fortran)\n" );
	break;
    case MPIR_FORT_INT:
	MPIR_Tab( indent );
	PRINTF( "INTEGER (Fortran)\n" );
	break;
    case MPIR_UB:
	MPIR_Tab( indent );
	PRINTF( "UB\n" );
	break;
    case MPIR_LB:
	MPIR_Tab( indent );
	PRINTF( "LB\n" );
	break;
    }
return MPI_SUCCESS;
}

#ifdef FOO
/* adds to singly-linked list of flat datatype elements, returns pointer to
   head and to "next" pointer in last element, for appending.  Updates 
   current displacement.
 */
int MPIR_flatten_dte( dte, fdte, tailptr, disp )
MPI_Datatype  dte;
MPIR_FDTEL    **fdte, ***tailptr;
int           *disp;
{
    int i;
    MPIR_FDTEL *p, **q, **r;

/*
    PRINTF("entering flatten dte, dte type = %d, count = %d\n",
	   dte->dte_type, dte->count);
*/
    switch (dte->dte_type)
    {
      case MPIR_INT:
	p         = (MPIR_FDTEL *) MPIR_SBalloc( MPIR_fdtels );
	p->disp   = *disp;
	p->type   =  MPIR_INT;
	*disp     += sizeof( MPIR_INT );
	*tailptr  = &(p->next);
	*fdte     = p;
	break;
      case MPIR_FLOAT:
	p         = (MPIR_FDTEL *) MPIR_SBalloc( MPIR_fdtels );
	p->disp   = *disp;
	p->type   =  MPIR_FLOAT;
	*disp     += sizeof( MPIR_FLOAT );
	*tailptr  = &(p->next);
	*fdte     = p;
	break;
      case MPIR_DOUBLE:
	p         = (MPIR_FDTEL *) MPIR_SBalloc( MPIR_fdtels );
	p->disp   = *disp;
	p->type   = MPIR_DOUBLE;
	*disp    += sizeof( MPIR_DOUBLE );
	*tailptr  = &(p->next);
	*fdte     = p;
	break;
      case MPIR_CONTIG:
	r = &p;
	for (i = 0; i < dte->count; i++)
	{
	    MPIR_flatten_dte(dte->old_type, r, &q, disp );
	    if ( i == 0 )
		*fdte = p;	/* remember the first one */
	    r = q;
	}
	*tailptr = r;
	break;
      default:
	PRINTF("mpir_flatten not implemented yet for type %d\n",dte->dte_type);
    }
    return MPI_SUCCESS;
}

int MPIR_dump_flat_dte( fdte )
MPIR_FDTEL *fdte;
{
    MPIR_FDTEL *p;

    p = fdte;
    while ( p )
    {
	PRINTF("(%d,%d),", p->type, p->disp );
	p = p->next;
    }
    PRINTF("\n");
    return MPI_SUCCESS;
}

#endif

int MPIR_Tab( n  )
int n;
{
    int i;

    for (i = 0; i < n; i++)
	putchar(' ');
    return MPI_SUCCESS;
}

#ifndef MPI_ADI2
/*
   MPIR_ArgSqueeze - Remove all null arguments from an arg vector; 
   update the number of arguments.
 */
void MPIR_ArgSqueeze( Argc, argv )
int  *Argc;
char **argv;
{
int argc, i, j;
    
/* Compress out the eliminated args */
argc = *Argc;
j    = 0;
i    = 0;
while (j < argc) {
    while (argv[j] == 0 && j < argc) j++;
    if (j < argc) argv[i++] = argv[j++];
    }
/* Back off the last value if it is null */
if (!argv[i-1]) i--;
*Argc = i;
}
#endif
