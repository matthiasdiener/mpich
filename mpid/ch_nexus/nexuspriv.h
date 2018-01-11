#ifndef _NEXUSPRIV_H
#define _NEXUSPRIV_H

#include <nexus.h>

/*
 * To avoid namespace pollution that the user might try to use, we
 * probably change the names of these typedef's and so forth.
 */

typedef enum
{
    MPI_CTRL_TAG,
    MPI_DATA_TAG
} MPI_recv_tag_t;
    
typedef struct _MPI_dest_t
{
    int value;
} MPI_dest_t;

typedef struct _MPI_control_t
{
#ifdef JGG
    int source;
    int data_size;
#else
    struct
    {
	int mode:32;
	int src:32;
	int context_id:32;
	int lrank:32;
	int tag:32;
	int len:32;
	int xdr:32;
    } header;
#endif
#if JGG
    nexus_stashed_buffer_t data;
#else
    nexus_buffer_t data;
#endif
    struct _MPI_control_t *next;
    struct _MPI_control_t *prev;
} MPI_control_t;

typedef struct _MPI_data_t
{
    int source;
    int data_size;
    nexus_stashed_buffer_t data;
    struct _MPI_data_t *next;
    struct _MPI_data_t *prev;
} MPI_data_t;

typedef struct _controlQ_t
{
    MPI_control_t *head;
    MPI_control_t *tail;
} controlQ_t;

typedef struct _dataQ_t
{
    MPI_data_t *head;
    MPI_data_t *tail;
} dataQ_t;

typedef struct _barrier_t
{
    int count;
    nexus_mutex_t mutex;
    nexus_cond_t cond;
} barrier_t;

#define MPI_PutQ(q, element) \
{ \
    if ((q)->head) \
    { \
	(q)->tail->next = (element); \
	(element)->prev = (q)->tail; \
	(q)->tail = (element); \
    } \
    else \
    { \
	(q)->head = (q)->tail = (element); \
	(element)->prev = NULL; \
    } \
    (element)->next = NULL; \
}

#define MPI_GetQ(q, element) \
{ \
    /* element is head of Q */ \
    if ((element) == (q)->head) \
    { \
	(q)->head = (q)->head->next; \
    } \
 \
    /* element is tail of Q */ \
    if ((element) == (q)->tail) \
    { \
	(q)->tail = (q)->tail->prev; \
    } \
 \
    /* set element's predesessor's next element */ \
    if ((element)->prev) \
    { \
	(element)->prev->next = (element)->next; \
    } \
 \
    /* set element's next's predesessor element */ \
    if ((element)->next) \
    { \
	(element)->next->prev = (element)->prev; \
    } \
}

#define CONTROL_PACKET "get_control"
#define DATA_PACKET    "get_data"
#define INITIAL_NODES  "get_initial_nodes"
#define ADD_NODES      "add_nodes"
#define SEND_TYPES     "master_get_types"
#define RETURN_TYPES   "slave_get_types"

#define CONTROL_PACKET_HASH 163
#define DATA_PACKET_HASH    116
#define INITIAL_NODES_HASH  772
#define ADD_NODES_HASH      929
#define SEND_TYPES_HASH     706
#define RETURN_TYPES_HASH   593

/* -1 is used for MPI_PROC_NULL */
#define ANY_NODE        -2
#define NO_NODE_WAITING -3

extern void MPID_NEXUS_receive(MPI_recv_tag_t type,
			       int *source,
			       MPID_PKT_T *buffer,
			       int buf_size);

extern void MPID_NEXUS_send_control(MPI_dest_t dest,
				    MPID_PKT_T *buffer,
				    int size);

extern void MPID_NEXUS_send_data(MPI_dest_t dest,
				 void *buffer,
				 int size);

extern void MPID_NEXUS_setup_nodes(nexus_node_t *orig_nodes,
				   int n_orig_nodes);

extern void MPID_NEXUS_publicize_nodes(int *argc, char ***argv);

extern int _MPID_NEXUS_my_id(void);
extern int _MPID_NEXUS_num_gps(void);
extern void _MPID_NEXUS_nodes(nexus_global_pointer_t **nodes, int *num);
extern nexus_global_pointer_t *MPID_NEXUS_nodes;
extern int MPID_NEXUS_control(void);

#endif /* _NEXUSPRIV_H */
