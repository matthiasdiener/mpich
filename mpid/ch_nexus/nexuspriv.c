/* This file contains the implementation of routines private to this device */
#include "mpid.h"
#include "nexuspriv.h"
#include <stdio.h>

#ifdef FORTRANM
#include "mpi_nexus_with_fm.h"
#endif

/**
 ** global variables
 **/
/* barriers */
static barrier_t MPID_NEXUS_control_barrier, MPID_NEXUS_data_barrier;
static barrier_t MPID_NEXUS_init_barrier;
static barrier_t MPID_NEXUS_get_types_barrier;

#ifdef EXPERIMENTAL
#ifndef NEXUS_LITE
static nexus_bool_t control_wait = NEXUS_FALSE;
static nexus_bool_t data_wait = NEXUS_FALSE;
#endif
#endif

/* gps */
/* this is extern for mqxu's mtio stuff */
nexus_global_pointer_t *MPID_NEXUS_nodes;
static nexus_global_pointer_t *type_return_gps;

/* queues */
static controlQ_t MPID_NEXUS_controlQ;
static dataQ_t MPID_NEXUS_dataQ;

/* freelists */
static MPI_control_t *control_list = NULL;
static int n_controls = 0;
static MPI_data_t *data_list = NULL;
static int n_datas = 0;


/* misc., but important */
static MPID_INFO *master_procinfo;
static int MPID_NEXUS_argc;
static char **MPID_NEXUS_argv;
static int MPID_NEXUS_num_gps;
static int MPID_NEXUS_my_id;

/**
 ** function prototypes
 **/

/* send/receive functions */
static nexus_bool_t find_control(int *source, MPID_PKT_T *packet);
static void rndv_size(nexus_buffer_t *buffer,
		      int *buf_size,
		      int *num_elements,
		      MPID_PKT_T *packet);
static void pack_rndv(nexus_buffer_t *buffer, MPID_PKT_T *packet);
static void unpack_rndv(nexus_stashed_buffer_t *buffer, MPID_PKT_T *packet);
static void get_control(void *address, nexus_buffer_t *buffer);
static void get_data(void *address, nexus_buffer_t *buffer);

static void master_get_types(void *address, nexus_buffer_t *buffer);
static void slave_get_types(void *address, nexus_buffer_t *buffer);

static void get_initial_nodes(void *address, nexus_buffer_t *buf);
static void add_nodes(void *address, nexus_buffer_t *buf);

/* misc */
int _MPID_NEXUS_my_id(void)
{
    return MPID_NEXUS_my_id;
}

int _MPID_NEXUS_num_gps(void)
{
    return MPID_NEXUS_num_gps;
}

int MPID_NEXUS_control(void)
{
    int rc;

#ifdef EXPERIMENTAL
#ifndef NEXUS_LITE
    while (control_wait)
    {
	return NEXUS_FALSE;
	nexus_thread_yield();
    }
#endif
#endif
    if (MPID_NEXUS_controlQ.head)
    {
	rc = NEXUS_TRUE;
    }
    else
    {
#ifdef NEXUS_LITE
        nexus_poll();
	if (MPID_NEXUS_controlQ.head)
	{
	    rc = NEXUS_TRUE;
	}
#endif
	rc = NEXUS_FALSE;
    }

    return rc;
}

/*
 * This function was added for Ming's mtio stuff, but it has
 * depreciated by changing MPID_NEXUS_nodes to an extern.  It may not
 * last long unless some other program (maybe Fortan-M?) needs it.
 */
void _MPID_NEXUS_nodes(nexus_global_pointer_t **nodes, int *num)
{
    *nodes = MPID_NEXUS_nodes;
    *num = MPID_NEXUS_num_gps;
}

/*
 * Compute the basic size of a packet.
 * Possible future changes:
 *  A.  Make this a macro
 *  B.  Remove unneeded arguments (packet) that were put in because of
 *      possible changes that could affect the function in the future.
 */
#define basic_size(buffer, size, num_elements, packet) \
{ \
    *(size) += nexus_sizeof_int(buffer, 7); \
    *(num_elements) += 1; \
}

/*
 * If the structure of MPID_PKT_T ever changes, these functions will
 * have to be modified to reflect those changes.  This packing assures
 * that all data translation is done correctly over heterogeneous
 * machines.
 */
#define pack_basic(buffer, packet) \
{ \
    nexus_put_int(buffer, &((packet)->head), 7); \
}

#define unpack_basic(buffer, packet) \
{ \
    nexus_get_int(buffer, packet, 7); \
}

/*
 * Nexus shouldn't support the rendevous protocol currently, so this is
 * for error checking purposes mostly.  If these functions get called,
 * something really got messed up!
 */
static void rndv_size(nexus_buffer_t *buffer,
		      int *buf_size,
		      int *num_elements,
		      MPID_PKT_T *packet)
{
    fprintf(stderr, "Please email geisler@mcs.anl.gov saying that you\n");
    fprintf(stderr, "have reached the rendevous code in the Nexus device\n");
    fprintf(stderr, "for MPICH.\n");
    nexus_fatal("You must put in logic for rndv_size()\n");
}

static void pack_rndv(nexus_buffer_t *buffer, MPID_PKT_T *packet)
{
    fprintf(stderr, "Please email geisler@mcs.anl.gov saying that you\n");
    fprintf(stderr, "have reached the rendevous code in the Nexus device\n");
    fprintf(stderr, "for MPICH.\n");
    nexus_fatal("add code for pack_rndv()\n");
}

static void unpack_rndv(nexus_stashed_buffer_t *buffer, MPID_PKT_T *packet)
{
    fprintf(stderr, "Please email geisler@mcs.anl.gov saying that you\n");
    fprintf(stderr, "have reached the rendevous code in the Nexus device\n");
    fprintf(stderr, "for MPICH.\n");
    nexus_fatal("put code in for unpack_rndv()\n");
}

/*
 * find_control()
 *
 *  This function looks through the control Q for a packet that matches
 *  the source and returns any data that exists in buffer.
 *
 *  RETURNS:  NEXUS_TRUE if it found a packet.
 *            NEXUS_FALSE otherwise.
 */
static nexus_bool_t find_control(int *source, MPID_PKT_T *packet)
{
    MPI_control_t *cur;
    nexus_bool_t got_control;
    int buf_size;
    int temp;

#ifdef DEBUG
    nexus_printf("looking for control packet...\n");
#endif

    got_control = NEXUS_FALSE;

    for (cur = MPID_NEXUS_controlQ.head; !got_control && cur; cur=cur->next)
    {
#ifdef DEBUG
	nexus_printf("source == %d\tcur->header.src == %d\n",
		     *source,
		     cur->header.src);
#endif

	if(   *source == ANY_NODE
	   || cur->header.src == *source)
	{
	    *source = cur->header.src;
	    got_control = NEXUS_TRUE;
	    MPI_GetQ(&MPID_NEXUS_controlQ, cur);

	    packet->head.mode = cur->header.mode;
	    packet->head.src = cur->header.src;
	    packet->head.context_id = cur->header.context_id;
	    packet->head.lrank = cur->header.lrank;
	    packet->head.tag = cur->header.tag;
	    packet->head.len = cur->header.len;
	    packet->head.has_xdr = cur->header.xdr;

	    switch(packet->head.mode)
	    {
	      case 0: /* MPID_PKT_SHORT: */
	      case 4: /* MPID_PKT_SHORT_READY: */
		if (packet->head.len > 0)
		{
		    nexus_get_stashed_byte(&cur->data,
				           packet->short_pkt.buffer,
				           packet->head.len);
		}
		break;
	      case 1: /* MPID_PKT_LONG: */
	      case 3: /* MPID_PKT_LONG_SYNC: */
	      case 5: /* MPID_PKT_LONG_READY: */
	      case 12: /* MPID_PKT_READY_ERROR: */
		break;
	      case 2: /* MPID_PKT_SHORT_SYNC: */
		nexus_get_stashed_long(&cur->data,
				       &packet->short_sync_pkt.sync_id,
				       1);
		if (packet->head.len > 0)
		{
		    nexus_get_stashed_byte(&cur->data,
				           packet->short_sync_pkt.buffer,
				           packet->head.len);
		}
		break;
	      case 6: /* MPID_PKT_REQUEST_SEND: */
	      case 7: /* MPID_PKT_REQUEST_SEND_READY: */
	      case 10: /* MPID_PKT_OK_TO_SEND: */
		nexus_get_stashed_long(&cur->data,
				       &packet->request_pkt.send_id,
				       1);
		unpack_rndv(&cur->data, packet);
		break;
	      case 11: /* MPID_PKT_SYNC_ACK: */
	      case 15: /* MPID_PKT_COMPLETE_SEND: */
	      case 16: /* MPID_PKT_COMPLETE_RECV: */
		nexus_get_stashed_long(&cur->data,
				       &packet->sync_ack_pkt.sync_id,
				       1);
		break;
	      default:
		nexus_fatal("find_control(): Unknown MPID_PKT_T (%d).\n", packet->head.mode);
	    }

	    nexus_free_stashed_buffer(&cur->data);
	    cur->next = control_list;
	    control_list = cur;
	    n_controls++;
	}
    }
#ifdef DEBUG
    nexus_printf("done receiving...\n");
#endif

    return got_control;
}

/*
 * find_data()
 *
 *  This function looks through the data Q for a packet that matches
 *  the source and returns any data that exists in buffer.
 *
 *  RETURNS:  NEXUS_TRUE if it found a packet.
 *            NEXUS_FALSE otherwise.
 */
static nexus_bool_t find_data(int *source, char *buffer)
{
    MPI_data_t *cur;
    nexus_bool_t got_data;

#ifdef DEBUG
    nexus_printf("looking for data packet...\n");
#endif

    got_data = NEXUS_FALSE;

    for (cur = MPID_NEXUS_dataQ.head; !got_data && cur; cur=cur->next)
    {
        if(   *source == ANY_NODE
	   || cur->source == *source)
	{
	    *source = cur->source;
	    got_data = NEXUS_TRUE;
	    MPI_GetQ(&MPID_NEXUS_dataQ, cur);

	    nexus_get_stashed_byte(&cur->data, buffer, cur->data_size);
	    nexus_free_stashed_buffer(&cur->data);
	    cur->next = data_list;
	    data_list = cur;
	    n_datas++;
	}
    }
    return got_data;
}

/*
 * receive()
 *
 *  This function waits for a message that matches both type && source
 *  and returns the value in buffer.  This is a blocking function and
 *  could potentially cause deadlock if not used properly.
 */
void MPID_NEXUS_receive(MPI_recv_tag_t type,
		        int *source,
			MPID_PKT_T *buffer,
		        int buf_size)
{
#ifdef DEBUG
    nexus_printf("starting receive()\n");
#endif

    if (type == MPI_CTRL_TAG)
    {
	nexus_bool_t got_control;

#ifdef DEBUG
	nexus_printf("locking control barrier\n");
#endif
#ifdef EXPERIMENTAL
#ifndef NEXUS_LITE
	while (control_wait)
	{
	    nexus_thread_yield();
	}
#endif
#endif
	nexus_mutex_lock(&MPID_NEXUS_control_barrier.mutex);

	got_control = find_control(source, buffer);
	if (!got_control)
	{
#ifdef DEBUG
	    nexus_printf("waiting for control packet from node %d\n",
		*source);
#endif

	    MPID_NEXUS_control_barrier.count = *source;
	    while (MPID_NEXUS_control_barrier.count != NO_NODE_WAITING)
	    {
#ifdef EXPERIMENTAL
#ifndef NEXUS_LITE
		control_wait = NEXUS_TRUE;
#endif
#endif
		nexus_cond_wait(&MPID_NEXUS_control_barrier.cond,
				&MPID_NEXUS_control_barrier.mutex);
	    }

	    /* control exists now, so find it */
	    if (!find_control(source, buffer))
	    {
		nexus_fatal("receive(): stored control not found.\n");
	    }

#ifdef EXPERIMENTAL
#ifndef NEXUS_LITE
	    control_wait = NEXUS_FALSE;
#endif
#endif
	}
#ifdef DEBUG
	nexus_printf("unlocking control barrier\n");
#endif
	nexus_mutex_unlock(&MPID_NEXUS_control_barrier.mutex);
    }
    else /* type == MPI_DATA_TAG */
    {
	nexus_bool_t got_data;

#ifdef DEBUG
	nexus_printf("locking data barrier\n");
#endif
#ifdef EXPERIMENTAL
#ifndef NEXUS_LITE
	while(data_wait)
	{
	    nexus_thread_yield();
	}
#endif
#endif
	nexus_mutex_lock(&MPID_NEXUS_data_barrier.mutex);
	got_data = find_data(source, (char *)buffer);
	if (!got_data)
	{
#ifdef DEBUG
	    nexus_printf("waiting for data packet from node %d\n",
		*source);
#endif

	    MPID_NEXUS_data_barrier.count = *source;
	    while (MPID_NEXUS_data_barrier.count != NO_NODE_WAITING)
	    {
#ifdef EXPERIMENTAL
#ifndef NEXUS_LITE
		data_wait = NEXUS_TRUE;
#endif
#endif
		nexus_cond_wait(&MPID_NEXUS_data_barrier.cond,
				&MPID_NEXUS_data_barrier.mutex);
	    }

	    /* data exists now, so find it */
	    if (!find_data(source, (char *)buffer))
	    {
		nexus_fatal("receive(): stored data not found\n");
	    }
#ifdef EXPERIMENTAL
#ifndef NEXUS_LITE
	    data_wait = NEXUS_FALSE;
#endif
#endif
	}
#ifdef DEBUG
	nexus_printf("unlocking data barrier\n");
#endif
	nexus_mutex_unlock(&MPID_NEXUS_data_barrier.mutex);
    }
}

/*
 * send_control()
 *
 *  This function takes optional data in buffer and sends it to the node
 *  that dest points to.  This is an asychronous send, so this should
 *  not be a point of blocking.
 */
void MPID_NEXUS_send_control(MPI_dest_t dest, MPID_PKT_T *buffer, int size)
{
    nexus_buffer_t send_buf;
    nexus_global_pointer_t send_gp;
    int buf_size;
    int num_elements;
    int data_size;
    int temp;

#ifdef DEBUG
    nexus_printf("sending...\n");
#endif

    if (!buffer)
    {
	nexus_fatal("send_control(): No control being sent.\n");
    }

    send_gp = MPID_NEXUS_nodes[dest.value];

    nexus_init_remote_service_request(&send_buf,
    				      &send_gp,
				      CONTROL_PACKET,
				      CONTROL_PACKET_HASH);

    num_elements = 0;
    buf_size = 0;
    basic_size(&send_buf, &buf_size, &num_elements, buffer);

    switch(buffer->head.mode)
    {
      case 0: /* MPID_PKT_SHORT: */
      case 4: /* MPID_PKT_SHORT_READY */
	if (buffer->head.len > 0)
	{
	    buf_size += nexus_sizeof_byte(&send_buf, buffer->head.len);
	    num_elements += 1;
	}
	break;
      case 1: /* MPID_PKT_LONG: */
      case 3: /* MPID_PKT_LONG_SYNC: */
      case 5: /* MPID_PKT_LONG_READY: */
      case 12: /* MPID_PKT_READY_ERROR: */
	break;
      case 2: /* MPID_PKT_SHORT_SYNC: */
	buf_size += nexus_sizeof_long(&send_buf, 1)
		    + nexus_sizeof_byte(&send_buf, buffer->head.len);
	num_elements += 2;
	break;
      case 6: /* MPID_PKT_REQUEST_SEND */
      case 7: /* MPID_PKT_REQUEST_SEND_READY: */
      case 10: /* MPID_PKT_OK_TO_SEND: */
	buf_size += nexus_sizeof_long(&send_buf, 1);
	num_elements += 1;
	rndv_size(&send_buf, &buf_size, &num_elements, buffer);
	break;
      case 11: /* MPID_PKT_SYNC_ACK: */
      case 15: /* MPID_PKT_COMPLETE_SEND: */
      case 16: /* MPID_PKT_COMPLETE_RECV: */
	buf_size += nexus_sizeof_long(&send_buf, 1);
	num_elements += 1;
	break;
      default:
	nexus_fatal("send_control(): Unknown MPID_PKT_T.\n");
    }

    nexus_set_buffer_size(&send_buf, buf_size, num_elements);

    buffer->head.src = MPID_NEXUS_my_id;
    pack_basic(&send_buf, buffer);

    switch(buffer->head.mode)
    {
      case 0: /* MPID_PKT_SHORT: */
      case 4: /* MPID_PKT_SHORT_READY: */
	if (buffer->head.len > 0)
	{
	    nexus_put_byte(&send_buf,
	    		   buffer->short_pkt.buffer,
			   buffer->head.len);
	}
	break;
      case 1:
      case 3:
      case 5:
      case 12:
	break;
      case 2: /* MPID_PKT_SHORT_SYNC: */
	nexus_put_long(&send_buf, &buffer->short_sync_pkt.sync_id, 1);
	if (buffer->head.len > 0)
	{
	    nexus_put_byte(&send_buf,
	    		   buffer->short_sync_pkt.buffer,
			   buffer->head.len);
	}
	break;
      case 6: /* MPID_PKT_REQUEST_SEND: */
      case 7: /* MPID_PKT_REQUEST_SEND_READY: */
      case 10: /* MPID_PKT_OK_TO_SEND: */
	nexus_put_long(&send_buf, &buffer->request_pkt.send_id, 1);
	pack_rndv(&send_buf, buffer);
	break;
      case 11: /* MPID_PKT_SYNC_ACK: */
      case 15: /* MPID_PKT_COMPLETE_SEND: */
      case 16: /* MPID_PKT_COMPLETE_RECV: */
	nexus_put_long(&send_buf, &buffer->sync_ack_pkt.sync_id, 1);
	break;
      default:
	nexus_fatal("send_control(): Unknown MPID_PKT_T.\n");
    }

    nexus_send_remote_service_request(&send_buf);
}

/*
 * send_data()
 *
 *  This function takes mandatory data in buffer and sends it to node
 *  pointed to in dest.  This send is asychronous and should not be the
 *  cause of blocking.
 */
void MPID_NEXUS_send_data(MPI_dest_t dest, void *buffer, int size)
{
    nexus_buffer_t send_buf;
    nexus_global_pointer_t send_gp;

    if (!buffer)
    {
	nexus_fatal("send_data(): no data is being sent.\n");
    }

    send_gp = MPID_NEXUS_nodes[dest.value];

    nexus_init_remote_service_request(&send_buf,
    				      &send_gp,
				      DATA_PACKET,
				      DATA_PACKET_HASH);
    nexus_set_buffer_size(&send_buf,
			  (  nexus_sizeof_int(&send_buf, 1)
			   + nexus_sizeof_int(&send_buf, 1)
			   + (size ? nexus_sizeof_byte(&send_buf,size) : 0)),
			  (size ? 3 : 2));
    nexus_put_int(&send_buf, &MPID_NEXUS_my_id, 1);
    nexus_put_int(&send_buf, &size, 1);
    nexus_put_byte(&send_buf, (char *)buffer, size);

    nexus_send_remote_service_request(&send_buf);
}

/*
 * get_control()
 *
 *  This function gets called by the receiving node when a control
 *  packet is sent between two nodes.  It puts the received packet on
 *  the control Q and looks to see if this node is waiting for a control
 *  packet from this node.  If it is, it wakes up the waiting thread.
 */
static void get_control(void *address, nexus_buffer_t *buffer)
{
    MPI_control_t *control;

#ifdef DEBUG
    nexus_printf("get_control called...\n");
#endif

    if (!control_list)
    {
        NexusMalloc(get_control(),
		    control,
		    MPI_control_t *,
		    sizeof(MPI_control_t));
    }
    else
    {
	control = control_list;
	control_list = control_list->next;
	n_controls--;
    }

    unpack_basic(buffer, &control->header);
    nexus_stash_buffer(buffer, &control->data);

#ifdef DEBUG
    nexus_printf("header info:\n");
    nexus_printf("    mode == %d\n", control->header.mode);
    nexus_printf("    src == %d\n", control->header.src);
    nexus_printf("    context_id == %d\n", control->header.context_id);
    nexus_printf("    lrank == %d\n", control->header.lrank);
    nexus_printf("    tag == %d\n", control->header.tag);
    nexus_printf("    len == %d\n", control->header.len);
    nexus_printf("    xdr == %d\n", control->header.xdr);
    nexus_printf("locking control barrier\n");
#endif

    nexus_mutex_lock(&MPID_NEXUS_control_barrier.mutex);
    MPI_PutQ(&MPID_NEXUS_controlQ, control);
    if (   MPID_NEXUS_control_barrier.count == ANY_NODE
	|| MPID_NEXUS_control_barrier.count == control->header.src)
    {
	MPID_NEXUS_control_barrier.count = NO_NODE_WAITING;
    }
    nexus_cond_signal(&MPID_NEXUS_control_barrier.cond);

#ifdef DEBUG
    nexus_printf("unlocking control barrier\n");
#endif

    nexus_mutex_unlock(&MPID_NEXUS_control_barrier.mutex);

#ifdef DEBUG
    nexus_printf("done gettting...\n");
#endif
}

/*
 * get_data()
 *
 *  This function gets called by the receiving node when a data
 *  packet is sent between two nodes.  It puts the received packet on
 *  the data Q and looks to see if this node is waiting for a data
 *  packet from this node.  If it is, it wakes up the waiting thread.
 */
static void get_data(void *address, nexus_buffer_t *buffer)
{
    MPI_data_t *data;

    if (!data_list)
    {
        NexusMalloc(get_data(),
		    data,
		    MPI_data_t *,
		    sizeof(MPI_data_t));
    }
    else
    {
	data = data_list;
	data_list = data_list->next;
	n_datas--;
    }

    nexus_get_int(buffer, &data->source, 1);
    nexus_get_int(buffer, &data->data_size, 1);
    nexus_stash_buffer(buffer, &data->data);

#ifdef DEBUG
    nexus_printf("DATA: I just got \"%s\" from node %d\n", data->data,
        data->source);
    nexus_printf("locking data barrier\n");
#endif

    nexus_mutex_lock(&MPID_NEXUS_data_barrier.mutex);
    MPI_PutQ(&MPID_NEXUS_dataQ, data);
    if (   MPID_NEXUS_data_barrier.count == ANY_NODE
	|| MPID_NEXUS_data_barrier.count == data->source)
    {
	MPID_NEXUS_data_barrier.count = NO_NODE_WAITING;
    }
    nexus_cond_signal(&MPID_NEXUS_data_barrier.cond);

#ifdef DEBUG
    nexus_printf("unlocking data barrier\n");
#endif

    nexus_mutex_unlock(&MPID_NEXUS_data_barrier.mutex);
}

/*
 * get_initial_nodes()
 *
 *  This function receives the global pointers to all the other nodes
 *  along with this node's id.  This routine must be run before any
 *  send will work, because the node will not know where to go without
 *  the nodes.
 *
 *  POTENTIAL PROBLEM:  We may need to put an internal barrier in the
 *  code to guarantee that before the user tries any sends or receives
 *  that this code has been run on all nodes.
 */
static void get_initial_nodes(void *address, nexus_buffer_t *buf)
{
    int arg_len;
    int i;

    nexus_get_int(buf, &MPID_NEXUS_my_id, 1);

#ifdef DEBUG
    nexus_printf("My id is %d\n", MPID_NEXUS_my_id);
#endif

    nexus_get_int(buf, &MPID_NEXUS_num_gps, 1);
    NexusMalloc(get_initail_nodes(),
		MPID_NEXUS_nodes,
		nexus_global_pointer_t *,
		sizeof(nexus_global_pointer_t) * MPID_NEXUS_num_gps);
    nexus_get_global_pointer(buf, MPID_NEXUS_nodes, MPID_NEXUS_num_gps);
    nexus_get_int(buf, &MPID_NEXUS_argc, 1);
    NexusMalloc(get_initial_nodes(),
		MPID_NEXUS_argv,
		char **,
		sizeof(char *) * (MPID_NEXUS_argc + 1 /* '\0' */));
    for (i = 0; i < MPID_NEXUS_argc; i++)
    {
	nexus_get_int(buf, &arg_len, 1);
	NexusMalloc(get_initial_nodes(),
		    MPID_NEXUS_argv[i],
		    char *,
		    sizeof(char) * arg_len);
	nexus_get_char(buf, MPID_NEXUS_argv[i], arg_len);
	MPID_NEXUS_argv[i][arg_len] = '\0';
    }

#ifdef DEBUG
    nexus_printf("I now have gps to %d nodes\n", MPID_NEXUS_num_gps);
    nexus_printf("locking init barrier\n");
#endif

    nexus_mutex_lock(&MPID_NEXUS_init_barrier.mutex);
    MPID_NEXUS_init_barrier.count = 0;
    nexus_cond_signal(&MPID_NEXUS_init_barrier.cond);

#ifdef DEBUG
    nexus_printf("unlocking init barrier\n");
#endif

    nexus_mutex_unlock(&MPID_NEXUS_init_barrier.mutex);
}

/*
 * add_nodes()
 *
 *  This function gets called if a new node is added to the computation.
 *  It just adds the node to the end of the nodes array, and presumes
 *  that the new node is given a new node number that corresponds to
 *  this new spot in the array.
 *
 *  POTENTIAL PROBLEM:  There is a possible race condition if two nodes
 *  are being added to computation at the same time.
 *
 *  OTHER:  This function won't be necessary until MPI allows new nodes
 *  to be added to the computation.
 *
 *  Currently untested.
 */
static void add_nodes(void *address, nexus_buffer_t *buf)
{
    int num_new_gps;

    nexus_get_int(buf, &num_new_gps, 1);
    ports0_realloc(MPID_NEXUS_nodes,
    		   sizeof(nexus_global_pointer_t)
		       * (MPID_NEXUS_num_gps + num_new_gps));

    nexus_get_global_pointer(buf,
    			     MPID_NEXUS_nodes + MPID_NEXUS_num_gps,
			     num_new_gps);

    MPID_NEXUS_num_gps += num_new_gps;
}

#ifdef FORTRANM
/*
 *  We may have to add more logic here.  Like calling NexusBoot() and
 *  any other functions that we assume should be working for any of the
 *  other functions after MPI_Init() has been called.
 */
void MPID_NEXUS_get_FM_nodes(int n_nodes, nexus_global_pointer_t *nodes)
{
    MPID_NEXUS_num_gps = n_nodes;
    NexusMalloc(get_initail_nodes(),
		MPID_NEXUS_nodes,
		nexus_global_pointer_t *,
		sizeof(nexus_global_pointer_t) * MPID_NEXUS_num_gps);
    for (i = 0; i < MPID_NEXUS_num_gps; i++)
    {
	MPID_NEXUS_nodes[i] = nodes[i];
    }
}
#endif /* FORTRANM */

/*
 * handlers specific to the MPI system
 */
static nexus_handler_t system_handlers[] =
{
    {CONTROL_PACKET,
     CONTROL_PACKET_HASH,
     NEXUS_HANDLER_TYPE_NON_THREADED,
     (nexus_handler_func_t) get_control},
    {DATA_PACKET,
     DATA_PACKET_HASH,
     NEXUS_HANDLER_TYPE_NON_THREADED,
     (nexus_handler_func_t) get_data},
    {SEND_TYPES,
     SEND_TYPES_HASH,
     NEXUS_HANDLER_TYPE_NON_THREADED,
     (nexus_handler_func_t) master_get_types},
    {RETURN_TYPES,
     RETURN_TYPES_HASH,
     NEXUS_HANDLER_TYPE_NON_THREADED,
     (nexus_handler_func_t) slave_get_types},
    {INITIAL_NODES,
     INITIAL_NODES_HASH,
     NEXUS_HANDLER_TYPE_NON_THREADED,
     (nexus_handler_func_t) get_initial_nodes},
    {ADD_NODES,
     ADD_NODES_HASH,
     NEXUS_HANDLER_TYPE_NON_THREADED,
     (nexus_handler_func_t) add_nodes},
    {(char *) NULL,
     0,
     NEXUS_HANDLER_TYPE_NON_THREADED,
     (nexus_handler_func_t) NULL},
};

/*
 * NexusBoot()
 *
 *  This is a required function for any Nexus program.  In this
 *  situation, it registers the necessary handler functions and
 *  initializes the global variables used.
 */
#ifdef FORTRANM
int MPID_NEXUS_register_MPI_handlers(void)
#else
int NexusBoot(void)
#endif
{
    MPID_NEXUS_num_gps = MPID_NEXUS_my_id = 0;

    nexus_mutex_init(&MPID_NEXUS_control_barrier.mutex,
    		     (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&MPID_NEXUS_control_barrier.cond,
    		    (nexus_condattr_t *) NULL);
    MPID_NEXUS_control_barrier.count = NO_NODE_WAITING;

    nexus_mutex_init(&MPID_NEXUS_data_barrier.mutex,
    		     (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&MPID_NEXUS_data_barrier.cond,
    		    (nexus_condattr_t *) NULL);
    MPID_NEXUS_data_barrier.count = NO_NODE_WAITING;

    nexus_mutex_init(&MPID_NEXUS_init_barrier.mutex,
		     (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&MPID_NEXUS_init_barrier.cond,
		    (nexus_condattr_t *) NULL);
    MPID_NEXUS_init_barrier.count = 1;

    nexus_mutex_init(&MPID_NEXUS_get_types_barrier.mutex,
		     (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&MPID_NEXUS_get_types_barrier.cond,
		    (nexus_condattr_t *) NULL);
    MPID_NEXUS_get_types_barrier.count = 0;

    nexus_register_handlers(system_handlers);

    return 0;
}

/*
 * setup_nodes()
 *
 *  This function is called from the master node.  It looks at the nodes
 *  returned by nexus_init() and makes a local global pointer array.
 */
void MPID_NEXUS_setup_nodes(nexus_node_t *orig_nodes, int n_orig_nodes)
{
    int i, j;

    /*
     *  I would like to have only one for loop where I allocate a node
     *  and copy it, but ports0_realloc doesn't seem to be working
     *  properly.
     */
    for (i = 0; i < n_orig_nodes; i++)
    {
	if (orig_nodes[i].return_code == NEXUS_NODE_NEW)
	{
	    MPID_NEXUS_num_gps++;
	}
    }

    NexusMalloc(setup_nodes(),
		MPID_NEXUS_nodes,
		nexus_global_pointer_t *,
		sizeof(nexus_global_pointer_t) * MPID_NEXUS_num_gps);

    for (i = 0, j = 0; i < n_orig_nodes; i++)
    {
	if (orig_nodes[i].return_code == NEXUS_NODE_NEW)
	{
	    MPID_NEXUS_nodes[j++] = orig_nodes[i].gp;
#ifdef EXPERIMENTAL2
	    nexus_copy_global_pointer(&MPID_NEXUS_nodes[j++],
	    			      &orig_nodes[i].gp);
#endif
	}
	/*
	 *  I would like to have a function/macro called
	 *  nexus_copy_global_pointer() so that if I want to do
	 *  something like A = B, the reference count on the global
	 *  pointer is incremented and I MUST do a
	 *  nexus_destroy_global_pointer() to both A and B.  Currently,
	 *  I can only destroy one of them.
	 *
	 *  I now have this functionality--it just needs to be tested.
	 */
#ifdef EXPERIMENTAL2
	nexus_destroy_global_pointer(&orig_nodes[i].gp);
#endif
    }
    nexus_free(orig_nodes);

    /*
     * Setup master_procinfo so it is ready to receive from other nodes
     * as soon as MPID_NEXUS_publicize_nodes() is called.
     */
    NexusMalloc(MPID_NEXUS_setup_nodes(),
    		master_procinfo,
		MPID_INFO *,
		sizeof(MPID_INFO) * MPID_NEXUS_num_gps);
    NexusMalloc(MPID_NEXUS_setup_nodes(),
		type_return_gps,
		nexus_global_pointer_t *,
		sizeof(nexus_global_pointer_t) * MPID_NEXUS_num_gps);

#ifdef DEBUG
    nexus_printf("locking get types barrier\n");
#endif

    nexus_mutex_lock(&MPID_NEXUS_get_types_barrier.mutex);
    /* We don't wait for the master, since it is doing the work. */
    MPID_NEXUS_get_types_barrier.count = MPID_NEXUS_num_gps - 1;

#ifdef DEBUG
    nexus_printf("unlocking get types barrier\n");
#endif

    nexus_mutex_unlock(&MPID_NEXUS_get_types_barrier.mutex);
}

/*
 * publicize_nodes()
 *
 *  This function sends the local global pointers array to each node
 *  along with an assigned node number (the node's location in the
 *  array).
 *
 *  COMMENTS:  In get_initial_nodes(), a possible race condition was
 *  mentioned.  This is not possible here, because the nodes have
 *  already been allocated and the numbers are being atomically assigned
 *  by only one node.  
 *  
 *  POSSIBLE PROBLEM:  The sending of the global pointers is asychronous
 *  so just because this function returns, it does not mean that all the
 *  nodes have received and processed the global pointers.  As mentioned
 *  earlier, it might be a good ideal to include an internal barrier so
 *  that publicize_nodes() returns only when all other nodes have their
 *  global_pointers.
 * 
 */
void MPID_NEXUS_publicize_nodes(int *argc, char ***argv)
{
    nexus_buffer_t send_buffer;
    int gp_n_elements;
    int buffer_size;
    int arg_len;
    int i, j;

    /* since the master node is known to be node 0, skip it */
    for (i = 1; i < MPID_NEXUS_num_gps; i++)
    {
	nexus_init_remote_service_request(&send_buffer,
					  &MPID_NEXUS_nodes[i],
					  INITIAL_NODES,
					  INITIAL_NODES_HASH);

	/*
	 * This buffer contains:
	 *  1.  node number
	 *  2.  # of gps
	 *  3.  gps to all nodes
	 *  4.  argc
	 *  5.  argv[0] ... argv[argc-1]
	 *
	 *  5 is actually a tuple of {size of argv[i], value of argv[i]}
	 */
	buffer_size = nexus_sizeof_global_pointer(&send_buffer,
						  MPID_NEXUS_nodes,
						  MPID_NEXUS_num_gps,
						  &gp_n_elements)
	    + nexus_sizeof_int(&send_buffer, 1) * (3 + *argc);
	for (j = 0; j < *argc; j++)
	{
	    buffer_size += nexus_sizeof_char(&send_buffer,
	    				     strlen((*argv)[j]));
	}
	nexus_set_buffer_size(&send_buffer,
			      buffer_size,
			      gp_n_elements + 3 + (2 * (*argc)));

	nexus_put_int(&send_buffer, &i, 1);
	nexus_put_int(&send_buffer, &MPID_NEXUS_num_gps, 1);
	nexus_put_global_pointer(&send_buffer,
				 MPID_NEXUS_nodes,
				 MPID_NEXUS_num_gps);
	nexus_put_int(&send_buffer, argc, 1);
	for (j = 0; j < *argc; j++)
	{
	    arg_len = strlen((*argv)[j]);
	    nexus_put_int(&send_buffer, &arg_len, 1);
	    nexus_put_char(&send_buffer, (*argv)[j], arg_len);
	}

	nexus_send_remote_service_request(&send_buffer);
    }

    MPID_NEXUS_argc = *argc;
    MPID_NEXUS_argv = *argv;

#ifdef DEBUG
    nexus_printf("locking init barrier\n");
#endif

    nexus_mutex_lock(&MPID_NEXUS_init_barrier.mutex);
    MPID_NEXUS_init_barrier.count = 0;
    nexus_cond_signal(&MPID_NEXUS_init_barrier.cond);

#ifdef DEBUG
    nexus_printf("unlocking init barrier\n");
#endif

    nexus_mutex_unlock(&MPID_NEXUS_init_barrier.mutex);
}

void MPID_NEXUS_wait_for_gps(int *argc, char ***argv)
{
#ifdef DEBUG
    nexus_printf("locking init barrier\n");
#endif

    nexus_mutex_lock(&MPID_NEXUS_init_barrier.mutex);
    while (MPID_NEXUS_init_barrier.count)
    {
	nexus_cond_wait(&MPID_NEXUS_init_barrier.cond,
			&MPID_NEXUS_init_barrier.mutex);
    }

#ifdef DEBUG
    nexus_printf("unlocking init barrier\n");
#endif

    nexus_mutex_unlock(&MPID_NEXUS_init_barrier.mutex);

    *argc = MPID_NEXUS_argc;
    *argv = MPID_NEXUS_argv;
}

void MPID_NEXUS_broadcast_types(MPID_INFO *procinfo)
{
    nexus_buffer_t send_buf;
    int temp;
    int i, j;

    master_procinfo[0].byte_order = procinfo[0].byte_order;
    master_procinfo[0].short_size = procinfo[0].short_size;
    master_procinfo[0].int_size = procinfo[0].int_size;
    master_procinfo[0].long_size = procinfo[0].long_size;
    master_procinfo[0].float_size = procinfo[0].float_size;
    master_procinfo[0].double_size = procinfo[0].double_size;
    master_procinfo[0].float_type = procinfo[0].float_type;

    /* Wait for other nodes */
#ifdef DEBUG
    nexus_printf("locking get types barrier\n");
#endif

    nexus_mutex_lock(&MPID_NEXUS_get_types_barrier.mutex);
    while(MPID_NEXUS_get_types_barrier.count)
    {
	nexus_cond_wait(&MPID_NEXUS_get_types_barrier.cond,
			&MPID_NEXUS_get_types_barrier.mutex);
    }

#ifdef DEBUG
    nexus_printf("unlocking get types barrier\n");
#endif

    nexus_mutex_unlock(&MPID_NEXUS_get_types_barrier.mutex);
    
    /* send procinfo back to everyone */
    for (i = 1; i < MPID_NEXUS_num_gps; i++)
    {
	nexus_init_remote_service_request(&send_buf,
					  &type_return_gps[i],
					  RETURN_TYPES,
					  RETURN_TYPES_HASH);
        nexus_set_buffer_size(&send_buf,
		      nexus_sizeof_int(&send_buf, 1) * 7 * MPID_NEXUS_num_gps,
		      7 * MPID_NEXUS_num_gps);
        for (j = 0; j < MPID_NEXUS_num_gps; j++)
	{
	    nexus_put_int(&send_buf, &master_procinfo[j].byte_order, 1);
	    nexus_put_int(&send_buf, &master_procinfo[j].short_size, 1);
	    nexus_put_int(&send_buf, &master_procinfo[j].int_size, 1);
	    nexus_put_int(&send_buf, &master_procinfo[j].long_size, 1);
	    nexus_put_int(&send_buf, &master_procinfo[j].float_size, 1);
	    nexus_put_int(&send_buf, &master_procinfo[j].double_size, 1);
	    nexus_put_int(&send_buf, &master_procinfo[j].float_type, 1);
	}
	nexus_send_remote_service_request(&send_buf);
    }

    /* Fill in local procinfo */
    for (i = 1; i < MPID_NEXUS_num_gps; i++)
    {
	procinfo[i].byte_order = master_procinfo[i].byte_order;
	procinfo[i].short_size = master_procinfo[i].short_size;
	procinfo[i].int_size = master_procinfo[i].int_size;
	procinfo[i].long_size = master_procinfo[i].long_size;
	procinfo[i].float_size = master_procinfo[i].float_size;
	procinfo[i].double_size = master_procinfo[i].double_size;
	procinfo[i].float_type = master_procinfo[i].float_type;
    }

    /* destroy master procinfo table since it isn't needed anymore */
    NexusFree(master_procinfo);
}

void MPID_NEXUS_get_types(MPID_INFO *procinfo)
{
    nexus_global_pointer_t return_value;
    nexus_buffer_t send_buffer;
    int buffer_size;
    int n_gp_elements;

    MPID_NEXUS_get_types_barrier.count = NEXUS_TRUE;

    nexus_global_pointer(&return_value, (void *)procinfo);

    nexus_init_remote_service_request(&send_buffer,
				      &MPID_NEXUS_nodes[0],
				      SEND_TYPES,
				      SEND_TYPES_HASH);
    buffer_size = nexus_sizeof_int(&send_buffer, 1) * 8
		  + nexus_sizeof_global_pointer(&send_buffer,
						&return_value,
						1,
						&n_gp_elements);
    nexus_set_buffer_size(&send_buffer, buffer_size, 8 + n_gp_elements);

    nexus_put_int(&send_buffer, &MPID_NEXUS_my_id, 1);
    nexus_put_global_pointer(&send_buffer, &return_value, 1);
    nexus_put_int(&send_buffer,
    		  &procinfo[MPID_NEXUS_my_id].byte_order,
		  1);
    nexus_put_int(&send_buffer,
		  &procinfo[MPID_NEXUS_my_id].short_size,
		  1);
    nexus_put_int(&send_buffer,
		  &procinfo[MPID_NEXUS_my_id].int_size,
		  1);
    nexus_put_int(&send_buffer,
		  &procinfo[MPID_NEXUS_my_id].long_size,
		  1);
    nexus_put_int(&send_buffer,
		  &procinfo[MPID_NEXUS_my_id].float_size,
		  1);
    nexus_put_int(&send_buffer,
		  &procinfo[MPID_NEXUS_my_id].double_size,
		  1);
    nexus_put_int(&send_buffer,
		  &procinfo[MPID_NEXUS_my_id].float_type,
		  1);
    nexus_send_remote_service_request(&send_buffer);

    /* Next, wait for return values */
#ifdef DEBUG
    nexus_printf("locking get types barrier\n");
#endif

    nexus_mutex_lock(&MPID_NEXUS_get_types_barrier.mutex);
    while(MPID_NEXUS_get_types_barrier.count)
    {
	nexus_cond_wait(&MPID_NEXUS_get_types_barrier.cond,
			&MPID_NEXUS_get_types_barrier.mutex);
    }

#ifdef DEBUG
    nexus_printf("unlocking get types barrier\n");
#endif

    nexus_mutex_unlock(&MPID_NEXUS_get_types_barrier.mutex);
}

void slave_get_types(void *address, nexus_buffer_t *buffer)
{
    MPID_INFO *procinfo = (MPID_INFO *)address;
    int i;

    for (i = 0; i < MPID_NEXUS_num_gps; i++)
    {
	nexus_get_int(buffer, &procinfo[i].byte_order, 1);
	nexus_get_int(buffer, &procinfo[i].short_size, 1);
	nexus_get_int(buffer, &procinfo[i].int_size, 1);
	nexus_get_int(buffer, &procinfo[i].long_size, 1);
	nexus_get_int(buffer, &procinfo[i].float_size, 1);
	nexus_get_int(buffer, &procinfo[i].double_size, 1);
	nexus_get_int(buffer, &procinfo[i].float_type, 1);
    }

#ifdef DEBUG
    nexus_printf("locking get types barrier\n");
#endif

    nexus_mutex_lock(&MPID_NEXUS_get_types_barrier.mutex);
    MPID_NEXUS_get_types_barrier.count = NEXUS_FALSE;
    nexus_cond_signal(&MPID_NEXUS_get_types_barrier.cond);

#ifdef DEBUG
    nexus_printf("unlocking get types barrier\n");
#endif

    nexus_mutex_unlock(&MPID_NEXUS_get_types_barrier.mutex);
}

void master_get_types(void *address, nexus_buffer_t *buffer)
{
    int node_num;

    nexus_get_int(buffer, &node_num, 1);
    nexus_get_global_pointer(buffer, &type_return_gps[node_num], 1);
    nexus_get_int(buffer, &master_procinfo[node_num].byte_order, 1);
    nexus_get_int(buffer, &master_procinfo[node_num].short_size, 1);
    nexus_get_int(buffer, &master_procinfo[node_num].int_size, 1);
    nexus_get_int(buffer, &master_procinfo[node_num].long_size, 1);
    nexus_get_int(buffer, &master_procinfo[node_num].float_size, 1);
    nexus_get_int(buffer, &master_procinfo[node_num].double_size, 1);
    nexus_get_int(buffer, &master_procinfo[node_num].float_type, 1);

#ifdef DEBUG
    nexus_printf("locking get types barrier\n");
#endif

    nexus_mutex_lock(&MPID_NEXUS_get_types_barrier.mutex);
    MPID_NEXUS_get_types_barrier.count--;
    if (MPID_NEXUS_get_types_barrier.count == 0)
    {
	nexus_cond_signal(&MPID_NEXUS_get_types_barrier.cond);
    }

#ifdef DEBUG
    nexus_printf("unlocking get types barrier\n");
#endif

    nexus_mutex_unlock(&MPID_NEXUS_get_types_barrier.mutex);
}


void MPID_NEXUS_finalize(void)
{
    /*
     * This should have some sort of blocking wait so that all the nodes
     * will be done before leaving the MPI environment.
     *
     * Possible implementation:
     *   Send an RSR to node one telling it that this node is done.
     *   When that node gets a message from all other nodes (including
     *   itself), send a message to all nodes that shutdown can continue
     *   without any more delay.
     */
#ifndef FORTRANM
    int i;

    for (i = 0; i < MPID_NEXUS_num_gps; i++)
    {
	nexus_destroy_global_pointer(&(MPID_NEXUS_nodes[i]));
    }
    NexusFree(MPID_NEXUS_nodes);

    nexus_shutdown_nonexiting();
#else
    NexusFree(MPID_NEXUS_nodes);
#endif /* FORTRANM */
}
