/*
 * Nexus
 * Authors:     Steven Tuecke
 *              Argonne National Laboratory
 *
 * pr_mpinx.c	- Protocol module for Hubertus Franke's threaded MPI/Nexus
 *			on the IBM SP2
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/pr_mpinx.c,v 1.9 1996/10/07 04:40:09 tuecke Exp $";

#include "internal.h"
#include "mpi.h"
#include "mpirsr.h"

#ifdef NEXUS_CRITICAL_PATH_TIMER
#include "perf/utp/UTP.h"
nexus_bool_t _nx_time_critical_path = NEXUS_FALSE;
nexus_bool_t _nx_critical_path_timer_started = NEXUS_FALSE;
int _nx_critical_path_start_timer = 0;
int _nx_critical_path_stop_timer = 0;
#define START_CRITICAL_PATH_TIMER() \
{ \
    if (_nx_time_critical_path) \
    { \
	_nx_critical_path_timer_started = NEXUS_TRUE; \
	UTP_start_timer(_nx_critical_path_start_timer); \
    } \
}
#define STOP_CRITICAL_PATH_TIMER() \
{ \
    if (_nx_critical_path_timer_started) \
    { \
	UTP_stop_timer(_nx_critical_path_stop_timer); \
	_nx_critical_path_timer_started = NEXUS_FALSE; \
    } \
}
#else
#define START_CRITICAL_PATH_TIMER()
#define STOP_CRITICAL_PATH_TIMER()
#endif


/*
 * Hash table size for the proto table
 */
#define PROTO_TABLE_SIZE 1021

/*
 * Only one thread is allowed to be in the mpinx code (and thus
 * mucking with data structures) at a time.
 */
static nexus_mutex_t		mpinx_mutex;
static nexus_cond_t		mpinx_cond;
static nexus_bool_t		mpinx_done;

#define mpinx_enter() nexus_mutex_lock(&mpinx_mutex);
#define mpinx_exit()  nexus_mutex_unlock(&mpinx_mutex);

#define mpinx_fatal mpinx_exit(); nexus_fatal


/*
 * Some forward typedef declarations...
 */
typedef struct _mpinx_buffer_t	mpinx_buffer_t;
typedef struct _mpinx_proto_t	mpinx_proto_t;


/*
 * A mpinx_buffer_t free list, to avoid malloc calls on the
 * main body of a message buffer.
 */
static mpinx_buffer_t *	buffer_free_list = (mpinx_buffer_t *) NULL;

#ifdef BUILD_DEBUG
#define MallocMpinxBuffer(Routine, Buf) \
    { \
	NexusMalloc(Routine, Buf, mpinx_buffer_t *, \
		    sizeof(struct _mpinx_buffer_t)); \
	Buf->magic = NEXUS_BUFFER_MAGIC; \
    }
#else  /* BUILD_DEBUG */
#define MallocMpinxBuffer(Routine, Buf) \
    { \
	NexusMalloc(Routine, Buf, mpinx_buffer_t *, \
		    sizeof(struct _mpinx_buffer_t)); \
    }
#endif /* BUILD_DEBUG */

#define GetMpinxBuffer(Routine, Buf) \
    if (buffer_free_list) \
    { \
	Buf = buffer_free_list; \
	buffer_free_list = buffer_free_list->next; \
    } \
    else \
    { \
	MallocMpinxBuffer(Routine, Buf); \
    }

#define FreeMpinxBuffer(Buf) \
{ \
    if ((Buf)->storage != (char *) NULL) \
	NexusFree((Buf)->storage); \
    (Buf)->next = buffer_free_list; \
    buffer_free_list = (Buf); \
}


/*********************************************************************
 * 		Buffer management macros
 *********************************************************************/
static nexus_sizeof_table_t mpinx_sizeof_table =
{
    sizeof(float),
    sizeof(double),
    sizeof(short),
    sizeof(unsigned short),
    sizeof(int),
    sizeof(unsigned int),
    sizeof(long),
    sizeof(unsigned long),
    sizeof(char),
    sizeof(unsigned char)
};

/*
 * PUT_*(mpinx_buffer_t *Buf, TYPE *Data, int Count)
 */
#ifdef BUILD_DEBUG
#define DO_PUT_ASSERTIONS(TYPE, Buf, Data, Count) \
    NexusBufferMagicCheck(MPINX_PUT_ ## TYPE, (nexus_buffer_t *) &(Buf)); \
    if (((Buf)->current + ((Count) * sizeof(TYPE))) > ((Buf)->storage + (Buf)->size)) \
    { \
	mpinx_fatal("nexus_put_" #TYPE ": Buffer overrun\n"); \
    } \
    if(NexusDebug(3)) \
    { \
        nexus_printf("MPINX_PUT_" #TYPE "(): storage:%x size:%lu current:%x limit:%x count:%d\n", \
		     (Buf)->storage, \
		     (Buf)->size, \
		     (Buf)->current, \
		     (Buf)->storage + (Buf)->size, \
		     (Count)); \
    }

#else /* BUILD_DEBUG */
#define DO_PUT_ASSERTIONS(TYPE, Buf, Data, Count)
#endif /* BUILD_DEBUG */

#define PUT(TYPE, Buf, Data, Count) \
{ \
    DO_PUT_ASSERTIONS(TYPE, Buf, Data, Count); \
    memcpy((Buf)->current, (Data), ((Count) * sizeof(TYPE))); \
    (Buf)->current += ((Count) * sizeof(TYPE)); \
}

#define PUT_FLOAT(Buf, Data, Count)	PUT(float, Buf, Data, Count)
#define PUT_DOUBLE(Buf, Data, Count)	PUT(double, Buf, Data, Count)
#define PUT_SHORT(Buf, Data, Count)	PUT(short, Buf, Data, Count)
#define PUT_U_SHORT(Buf, Data, Count)	PUT(u_short, Buf, Data, Count)
#define PUT_INT(Buf, Data, Count)	PUT(int, Buf, Data, Count)
#define PUT_U_INT(Buf, Data, Count)	PUT(u_int, Buf, Data, Count)
#define PUT_LONG(Buf, Data, Count)	PUT(long, Buf, Data, Count)
#define PUT_U_LONG(Buf, Data, Count)	PUT(u_long, Buf, Data, Count)
#define PUT_CHAR(Buf, Data, Count)	PUT(char, Buf, Data, Count)
#define PUT_U_CHAR(Buf, Data, Count)	PUT(u_char, Buf, Data, Count)


/*
 * GET_*(mpinx_buffer_t *Buf, TYPE *Dest, int Count)
 */
#ifdef BUILD_DEBUG
#define DO_GET_ASSERTIONS(TYPE, Buf, Dest, Count) \
    NexusBufferMagicCheck(MPINX_GET_ ## TYPE, (nexus_buffer_t *) &(Buf)); \
    NexusAssert2(!(Buf)->stashed, \
		 ("MPINX_GET_*(): Expected an un-stashed buffer\n")); \
    if (((Buf)->current + ((Count) * sizeof(TYPE))) > ((Buf)->storage + (Buf)->size)) \
    { \
	mpinx_fatal("nexus_get_" #TYPE ": Buffer overrun\n"); \
    } \
    if(NexusDebug(3)) \
    { \
        nexus_printf("MPINX_GET_" #TYPE "(): storage:%x size:%lu current:%x limit:%x count:%d\n", \
		     (Buf)->storage, \
		     (Buf)->size, \
		     (Buf)->current, \
		     (Buf)->storage + (Buf)->size, \
		     (Count)); \
    }

#else /* BUILD_DEBUG */
#define DO_GET_ASSERTIONS(TYPE, Buf, Dest, Count)
#endif /* BUILD_DEBUG */

#define GET(TYPE, Buf, Dest, Count) \
{ \
    DO_GET_ASSERTIONS(TYPE, Buf, Dest, Count); \
    memcpy((Dest), (Buf)->current, ((Count) * sizeof(TYPE))); \
    (Buf)->current += ((Count) * sizeof(TYPE)); \
}

#define GET_FLOAT(Buf, dest, Count)	GET(float, Buf, dest, Count)
#define GET_DOUBLE(Buf, dest, Count)	GET(double, Buf, dest, Count)
#define GET_SHORT(Buf, dest, Count)	GET(short, Buf, dest, Count)
#define GET_U_SHORT(Buf, dest, Count)	GET(u_short, Buf, dest, Count)
#define GET_INT(Buf, dest, Count)	GET(int, Buf, dest, Count)
#define GET_U_INT(Buf, dest, Count)	GET(u_int, Buf, dest, Count)
#define GET_LONG(Buf, dest, Count)	GET(long, Buf, dest, Count)
#define GET_U_LONG(Buf, dest, Count)	GET(u_long, Buf, dest, Count)
#define GET_CHAR(Buf, dest, Count)	GET(char, Buf, dest, Count)
#define GET_U_CHAR(Buf, dest, Count)	GET(u_char, Buf, dest, Count)


/*
 * GET_STASHED_*(mpinx_buffer_t *Buf, TYPE *Dest, int Count)
 */
#ifdef BUILD_DEBUG
#define DO_GET_STASHED_ASSERTIONS(TYPE, Buf, Dest, Count) \
    NexusBufferMagicCheck(MPINX_GET_STASHED_ ## TYPE, (nexus_buffer_t *) &(Buf));\
    NexusAssert2((Buf)->stashed, \
		 ("MPINX_GET_STASHED_*(): Expected a stashed buffer\n"));   \
    if (((Buf)->current + ((Count) * sizeof(TYPE))) > ((Buf)->storage + (Buf)->size)) \
    { \
	mpinx_fatal("nexus_get_stashed_" #TYPE ": Buffer overrun\n"); \
    } \
    if(NexusDebug(3)) \
    { \
        nexus_printf("MPINX_GET_STASHED_" #TYPE "(): storage:%x size:%lu current:%x limit:%x count:%d\n", \
		     (Buf)->storage, \
		     (Buf)->size, \
		     (Buf)->current, \
		     (Buf)->storage + (Buf)->size, \
		     (Count)); \
    }

#else /* BUILD_DEBUG */
#define DO_GET_STASHED_ASSERTIONS(TYPE, Buf, Dest, Count)
#endif /* BUILD_DEBUG */

#define GET_STASHED(TYPE, Buf, Dest, Count) \
{ \
    DO_GET_STASHED_ASSERTIONS(TYPE, Buf, Dest, Count); \
    memcpy((Dest), (Buf)->current, ((Count) * sizeof(TYPE))); \
    (Buf)->current += ((Count) * sizeof(TYPE)); \
}

#define GET_STASHED_FLOAT(Buf, dest, Count) \
    GET_STASHED(float, Buf, dest, Count)
#define GET_STASHED_DOUBLE(Buf, dest, Count) \
    GET_STASHED(double, Buf, dest, Count)
#define GET_STASHED_SHORT(Buf, dest, Count) \
    GET_STASHED(short, Buf, dest, Count)
#define GET_STASHED_U_SHORT(Buf, dest, Count) \
    GET_STASHED(u_short, Buf, dest, Count)
#define GET_STASHED_INT(Buf, dest, Count) \
    GET_STASHED(int, Buf, dest, Count)
#define GET_STASHED_U_INT(Buf, dest, Count) \
    GET_STASHED(u_int, Buf, dest, Count)
#define GET_STASHED_LONG(Buf, dest, Count) \
    GET_STASHED(long, Buf, dest, Count)
#define GET_STASHED_U_LONG(Buf, dest, Count) \
    GET_STASHED(u_long, Buf, dest, Count)
#define GET_STASHED_CHAR(Buf, dest, Count) \
    GET_STASHED(char, Buf, dest, Count)
#define GET_STASHED_U_CHAR(Buf, dest, Count) \
    GET_STASHED(u_char, Buf, dest, Count)


/*
 * mpinx_buffer_t
 *
 * This is an overload of nexus_buffer_t.  It adds the
 * mpinx specific information to that structure.
 *
 * This is only used for sending message.  mpinx_rsr_recv_t is
 * used as the nexus_buffer_t on the receiving side.
 */
struct _mpinx_buffer_t
{
#ifdef BUILD_DEBUG
    int				magic;
#endif
    nexus_buffer_funcs_t *	funcs;
    nexus_sizeof_table_t *	sizeof_table;
    mpinx_buffer_t *		next;
    mpinx_proto_t *		proto;
    char *			storage;
    char *			current;
    int				size;
    int				n_elements;
    unsigned long		context;
    unsigned long		address;
    int				handler_id;
    int				handler_name_length;
    char *			handler_name;
#ifdef BUILD_PROFILE
    int				source_node_id;
    int				source_context_id;
    int				dest_node_id;
    int				dest_context_id;
#endif	    
    
};


/*
 * mpinx_rsr_recv_t
 *
 * Extension to rsr_recv_t in MPI/Nexus.
 *
 * This is space the MPI/Nexus manages as part of a received message.
 * This is used as a nexus_buffer_t for receiving messages.
 */
typedef struct _mpinx_rsr_recv_t
{
    union
    {
	rsr_recv_t			base;
	struct /* recv */
	{
#ifdef BUILD_DEBUG
	    int				magic;
#endif
	    nexus_buffer_funcs_t *	funcs;
	    nexus_sizeof_table_t *	sizeof_table;
	} recv;
    } u;

    nexus_handler_type_t	handler_type;
    nexus_handler_func_t	handler_func;
    nexus_bool_t		stashed;
    char *			storage;
    char *			current;
    int				size;
#ifdef BUILD_PROFILE
    int				source_node_id;
    int				source_context_id;
#endif	    
    unsigned long		context;
    unsigned long		address;
    int				handler_id;
    char			handler_name[NEXUS_MAX_HANDLER_NAME_LENGTH];
} mpinx_rsr_recv_t;


/*
 * mpinx_rsr_recv_size
 *
 * Size of the mpinx_rsr_recv_t structure.  MPI_Init_rsr() may pad out
 * that structure and return a size that is larger than
 * sizeof(mpinx_rsr_recv_t), so we need to hang onto that size
 * for use in the mpinx_rsr_*_wrapper() functions.
 */
int mpinx_rsr_recv_size;


/*
 * mpinx_rsr_send_t
 *
 * Extension to rsr_send_t in MPI/Nexus.
 *
 * This is basically a message header that MPI/Nexus sends along
 * with the message.
 */
typedef struct _mpinx_rsr_send_t
{
    rsr_send_t		base;
    int			handler_id;
#ifdef BUILD_PROFILE
    int			source_node_id;
    int			source_context_id;
#endif	    
    unsigned long	context;
    unsigned long	address;
    char		handler_name[NEXUS_MAX_HANDLER_NAME_LENGTH];
} mpinx_rsr_send_t;


/*
 * mpinx_proto_t
 *
 * This is an overload of nexus_proto_t.  It adds the
 * mp specific information to that structure.
 */
struct _mpinx_proto_t
{
    nexus_proto_type_t	type;	/* NEXUS_PROTO_TYPE_MPINX */
    nexus_proto_funcs_t *funcs;
    int			destination;
    int			reference_count;
};


static int my_node;
static int n_nodes;


/*
 * Protocol table stuff.
 *
 * The protocol table is hashed on the destination. The table itself is an
 * array of header structures pointing to a linked list of buckets.
 *
 * This table is used to avoid creating multiple mpinx_proto_t
 * objects to the same context.  Multiple global pointers to the same
 * context share a mpinx_proto_t.
 */
typedef struct _proto_table_entry_t
{
    mpinx_proto_t *proto;
    struct _proto_table_entry_t *next;
} proto_table_entry_t;

struct _proto_table_entry_t	proto_table[PROTO_TABLE_SIZE];

static void			proto_table_init(void);
static void			proto_table_insert(mpinx_proto_t *proto);
static mpinx_proto_t *		proto_table_lookup(int *dest);


/*
 * Various forward declarations of procedures
 */
static nexus_proto_type_t	mpinx_proto_type(void);
static void		mpinx_init(void);
static void		mpinx_shutdown(nexus_bool_t shutdown_others);
static void		mpinx_abort(void);

static int		mpinx_send_rsr(nexus_buffer_t *buffer,
				       nexus_startpoint_t *sp_array,
				       int num_sp,
				       int handler_id,
				       nexus_bool_t destroy_buffer);
static void		mpinx_destroy_proto(nexus_proto_t *nexus_proto);
static void             mpinx_copy_proto(nexus_proto_t *proto_in,
					 nexus_proto_t **proto_out);
static nexus_mi_proto_t *mpinx_get_my_mi_proto(void);
static nexus_bool_t	mpinx_construct_from_mi_proto(nexus_proto_t **proto,
						  nexus_mi_proto_t *mi_proto);
static void		mpinx_get_creator_proto_params(char *buf,
						       int buf_size,
						       char *node_name,
						       int node_number);
static void		mpinx_construct_creator_proto(nexus_proto_t **proto);
static int		mpinx_compare_protos(nexus_proto_t *proto1,
					     nexus_proto_t *proto2);


static mpinx_proto_t *	construct_proto(int destination);
static void		free_proto(mpinx_proto_t *proto);

static void		mpinx_set_buffer_size(nexus_buffer_t *buffer,
					      int size,
					      int n_elements);
static int		mpinx_check_buffer_size(nexus_buffer_t *buffer,
						int slack,
						int increment);
static void		mpinx_free_buffer(nexus_buffer_t *buffer);
static void		mpinx_stash_buffer(nexus_buffer_t *buffer,
				       nexus_stashed_buffer_t *stashed_buffer);
static void		mpinx_free_stashed_buffer(
				       nexus_stashed_buffer_t *stashed_buffer);


static int	mpinx_sizeof_float	(nexus_buffer_t *buffer, int count);
static int	mpinx_sizeof_double	(nexus_buffer_t *buffer, int count);
static int	mpinx_sizeof_short	(nexus_buffer_t *buffer, int count);
static int	mpinx_sizeof_u_short	(nexus_buffer_t *buffer, int count);
static int	mpinx_sizeof_int	(nexus_buffer_t *buffer, int count);
static int	mpinx_sizeof_u_int	(nexus_buffer_t *buffer, int count);
static int	mpinx_sizeof_long	(nexus_buffer_t *buffer, int count);
static int	mpinx_sizeof_u_long	(nexus_buffer_t *buffer, int count);
static int	mpinx_sizeof_char	(nexus_buffer_t *buffer, int count);
static int	mpinx_sizeof_u_char	(nexus_buffer_t *buffer, int count);


static void	mpinx_put_float	(nexus_buffer_t *buffer,
				 float *data, int count);
static void	mpinx_put_double(nexus_buffer_t *buffer,
				 double *data, int count);
static void	mpinx_put_short	(nexus_buffer_t *buffer,
				 short *data, int count);
static void	mpinx_put_u_short(nexus_buffer_t *buffer,
				  unsigned short *data, int count);
static void	mpinx_put_int	(nexus_buffer_t *buffer,
				 int *data, int count);
static void	mpinx_put_u_int	(nexus_buffer_t *buffer,
				 unsigned int *data, int count);
static void	mpinx_put_long	(nexus_buffer_t *buffer,
				 long *data, int count);
static void	mpinx_put_u_long(nexus_buffer_t *buffer,
				 unsigned long *data, int count);
static void	mpinx_put_char	(nexus_buffer_t *buffer,
				 char *data, int count);
static void	mpinx_put_u_char(nexus_buffer_t *buffer,
				 unsigned char *data, int count);

static void	mpinx_get_float	(nexus_buffer_t *buffer,
				 float *dest, int count);
static void	mpinx_get_double(nexus_buffer_t *buffer,
				 double *dest, int count);
static void	mpinx_get_short	(nexus_buffer_t *buffer,
				 short *dest, int count);
static void	mpinx_get_u_short(nexus_buffer_t *buffer,
				  unsigned short *dest, int count);
static void	mpinx_get_int	(nexus_buffer_t *buffer,
				 int *dest, int count);
static void	mpinx_get_u_int	(nexus_buffer_t *buffer,
				 unsigned int *dest, int count);
static void	mpinx_get_long	(nexus_buffer_t *buffer,
				 long *dest, int count);
static void	mpinx_get_u_long(nexus_buffer_t *buffer,
				 unsigned long *dest, int count);
static void	mpinx_get_char	(nexus_buffer_t *buffer,
				 char *dest, int count);
static void	mpinx_get_u_char(nexus_buffer_t *buffer,
				 unsigned char *dest, int count);

static void	mpinx_get_stashed_float	(nexus_stashed_buffer_t *buffer,
					 float *dest, int count);
static void	mpinx_get_stashed_double(nexus_stashed_buffer_t *buffer,
					 double *dest, int count);
static void	mpinx_get_stashed_short	(nexus_stashed_buffer_t *buffer,
					 short *dest, int count);
static void	mpinx_get_stashed_u_short(nexus_stashed_buffer_t *buffer,
					  unsigned short *dest, int count);
static void	mpinx_get_stashed_int	(nexus_stashed_buffer_t *buffer,
					 int *dest, int count);
static void	mpinx_get_stashed_u_int	(nexus_stashed_buffer_t *buffer,
					 unsigned int *dest, int count);
static void	mpinx_get_stashed_long	(nexus_stashed_buffer_t *buffer,
					 long *dest, int count);
static void	mpinx_get_stashed_u_long(nexus_stashed_buffer_t *buffer,
					 unsigned long *dest, int count);
static void	mpinx_get_stashed_char	(nexus_stashed_buffer_t *buffer,
					 char *dest, int count);
static void	mpinx_get_stashed_u_char(nexus_stashed_buffer_t *buffer,
					 unsigned char *dest, int count);

static nexus_proto_funcs_t mpinx_proto_funcs =
{
    mpinx_proto_type,
    mpinx_init,
    mpinx_shutdown,
    mpinx_abort,
    NULL, /* mpinx_poll, */
    NULL, /* mpinx_blocking_poll, */
    mpinx_destroy_proto,
    mpinx_copy_proto,
    mpinx_get_my_mi_proto,
    mpinx_construct_from_mi_proto,
    mpinx_get_creator_proto_params,
    mpinx_construct_creator_proto,
    mpinx_compare_protos,
    NULL /* mpinx_test_proto */,
};

static nexus_buffer_funcs_t mpinx_buffer_funcs =
{
    mpinx_set_buffer_size,
    mpinx_check_buffer_size,
    mpinx_send_rsr,
    mpinx_free_buffer,
    mpinx_stash_buffer,
    mpinx_free_stashed_buffer,
    mpinx_sizeof_float,
    mpinx_sizeof_double,
    mpinx_sizeof_short,
    mpinx_sizeof_u_short,
    mpinx_sizeof_int,
    mpinx_sizeof_u_int,
    mpinx_sizeof_long,
    mpinx_sizeof_u_long,
    mpinx_sizeof_char,
    mpinx_sizeof_u_char,
    mpinx_put_float,
    mpinx_put_double,
    mpinx_put_short,
    mpinx_put_u_short,
    mpinx_put_int,
    mpinx_put_u_int,
    mpinx_put_long,
    mpinx_put_u_long,
    mpinx_put_char,
    mpinx_put_u_char,
    mpinx_get_float,
    mpinx_get_double,
    mpinx_get_short,
    mpinx_get_u_short,
    mpinx_get_int,
    mpinx_get_u_int,
    mpinx_get_long,
    mpinx_get_u_long,
    mpinx_get_char,
    mpinx_get_u_char,
    mpinx_get_stashed_float,
    mpinx_get_stashed_double,
    mpinx_get_stashed_short,
    mpinx_get_stashed_u_short,
    mpinx_get_stashed_int,
    mpinx_get_stashed_u_int,
    mpinx_get_stashed_long,
    mpinx_get_stashed_u_long,
    mpinx_get_stashed_char,
    mpinx_get_stashed_u_char,
};


/*
 * mpinx_rsr_lookup_func()
 *
 * Callback from MPI/Nexus.
 *
 * Fillin the mpinx_rsr_recv_t structure.
 * This is used to determine the handler type, and to pass on
 * to the handler wrapper function.
 */
static void mpinx_rsr_lookup_func(mpinx_rsr_send_t *send,
				  rsr_misc_t *misc,
				  mpinx_rsr_recv_t *recv )
{
    recv->handler_id = send->handler_id;
    if (send->handler_id < 0)
    {
	recv->u.base.rsr_type = RSR_HANDLER_TYPE_NON_THREADED;
	recv->u.base.rsr_type_arg.overloaded = NULL;
	return;
    }
    
    if (recv->handler_type == NEXUS_HANDLER_TYPE_THREADED)
    {
	recv->u.base.rsr_type = RSR_HANDLER_TYPE_THREADED;
	/* This should be filled in with a real pthread_attr */
	recv->u.base.rsr_type_arg.pthread_attr = NULL;
    }
    else
    {
	recv->u.base.rsr_type = RSR_HANDLER_TYPE_NON_THREADED;
	recv->u.base.rsr_type_arg.overloaded = NULL;
    }

    /* Save away my information */
    recv->size = send->base.u_datalen;
#ifdef BUILD_PROFILE
    recv->source_node_id = send->source_node_id;
    recv->source_context_id = send->source_context_id;
#endif	    
    recv->context = send->context;
    recv->address = send->address;
    if (!(recv->handler_func))
    {
	/*
	 * The handler was not recognized, so save away the
	 * handler_name for future use.
	 */
	strcpy(recv->handler_name, send->handler_name);
    }
} /* mpinx_rsr_lookup_func() */


/*
 * mpinx_rsr_threaded_wrapper()
 *
 * Callback from MPI/Nexus.
 *
 * Invoke the threaded handler using the info in 'buff'.
 */
static void mpinx_rsr_threaded_wrapper(char *buff)
{
    mpinx_rsr_recv_t *recv = (mpinx_rsr_recv_t *) (buff - mpinx_rsr_recv_size);
    nexus_handler_record_t *rec;
    nexus_threaded_handler_func_t func;
    nexus_stashed_buffer_t nexus_buf;

    if (recv->handler_id < 0)
    {
	STOP_CRITICAL_PATH_TIMER();
	_nx_exit_transient_process(0);
    }
    
    START_CRITICAL_PATH_TIMER();
    
    /*
     * recv->handler_func is really a nexus_handler_record_t
     * So extract the real function pointer.
     */
    rec = (nexus_handler_record_t *) recv->handler_func;
    func = (nexus_threaded_handler_func_t) rec->func;

    /*
     * Setup recv as a nexus_buffer_t
     */
#ifdef BUILD_DEBUG    
    recv->u.recv.magic = NEXUS_BUFFER_MAGIC;
#endif    
    recv->u.recv.funcs = &mpinx_buffer_funcs;
    recv->u.recv.sizeof_table = &mpinx_sizeof_table;
    recv->stashed = NEXUS_TRUE;
    recv->storage = buff;
    recv->current = buff;
	
    _nx_set_context(recv->context);
    
#ifdef BUILD_PROFILE	
    _nx_pablo_log_remote_service_request_receive(recv->source_node_id,
						 recv->source_context_id,
						 rec->name,
						 rec->id,
						 recv->size);
    if (_nx_pablo_count_remote_service_requests())
    {
	_nx_accumulate_rsr_profile_info((nexus_context_t *) recv->context,
					rec,
					recv->source_node_id,
					recv->source_context_id,
					recv->size);
    }
#endif /* BUILD_PROFILE */

    nexus_buf = (nexus_stashed_buffer_t) recv;
    
    (*func)((void *) recv->address, &nexus_buf);
} /* mpinx_rsr_threaded_wrapper() */


/*
 * mpinx_rsr_non_threaded_wrapper()
 *
 * Callback from MPI/Nexus.
 *
 * Invoke the non-threaded handler using the info in 'buff'.
 */
static void mpinx_rsr_non_threaded_wrapper(char *buff)
{
    mpinx_rsr_recv_t *recv = (mpinx_rsr_recv_t *) (buff - mpinx_rsr_recv_size);

    START_CRITICAL_PATH_TIMER();
    
    /*
     * Setup recv as a nexus_buffer_t
     */
#ifdef BUILD_DEBUG    
    recv->u.recv.magic = NEXUS_BUFFER_MAGIC;
#endif    
    recv->u.recv.funcs = &mpinx_buffer_funcs;
    recv->u.recv.sizeof_table = &mpinx_sizeof_table;
    recv->stashed = NEXUS_FALSE;
    recv->storage = buff;
    recv->current = buff;

    _nx_handle_message(recv->handler_id,
		       recv->endpoint,
#ifdef BUILD_PROFILE
		       recv->source_node_id,
		       recv->source_context_id,
		       recv->size,
#endif
		       (void *) recv);
} /* mpinx_rsr_non_threaded_wrapper() */


/*
 * _nx_pr_mpinx_info()
 *
 * Return the nexus_proto_funcs_t function table for this protocol module.
 *
 * This procedure is used for bootstrapping the protocol module.
 * The higher level Nexus code needs to call this routine to
 * retrieve the functions it needs to use this protocol module.
 */
void *_nx_pr_mpinx_info(void)
{
    return((void *) (&mpinx_proto_funcs));
} /* _nx_pr_*_info() */


/*
 * mpinx_proto_type()
 *
 * Return the nexus_proto_type_t for this protocol module.
 */
static nexus_proto_type_t mpinx_proto_type(void)
{
    return (NEXUS_PROTO_TYPE_MPINX);
} /* mpinx_proto_type() */


/*
 * mpinx_init()
 *
 * Initialize the MP protocol.
 */
static void mpinx_init(void)
{
    /* Initialize the rsr portion of MPI/Nexus */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_node);
    MPI_Comm_size(MPI_COMM_WORLD, &n_nodes);
    mpinx_rsr_recv_size = sizeof(mpinx_rsr_recv_t);
    MPI_Rsr_init(mpinx_rsr_threaded_wrapper,
		 mpinx_rsr_non_threaded_wrapper,
		 (rsr_handler_lookup_t) mpinx_rsr_lookup_func,
		 &mpinx_rsr_recv_size);

    proto_table_init();
    nexus_mutex_init(&mpinx_mutex, (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&mpinx_cond, (nexus_condattr_t *) NULL);
    mpinx_done = NEXUS_FALSE;

} /* mpinx_init() */


/*
 * mpinx_shutdown()
 *
 * This routine is called during normal shutdown of a process.
 */
static void mpinx_shutdown(nexus_bool_t shutdown_others)
{
    nexus_bool_t first_in;

    mpinx_enter();
    first_in = !mpinx_done;
    mpinx_done = NEXUS_TRUE;
    mpinx_exit();

    /*
     * If I'm supposed to shutdown other nodes, then
     * broadcast a shutdown command to all other nodes.
     */
    if (shutdown_others && first_in)
    {
	int i;
	mpinx_rsr_send_t send;
	int send_length;
	MPI_Request request;
	MPI_Status status;
	
	send_length = sizeof(rsr_send_t) + sizeof(int);

	for (i = 0; i < n_nodes; i++)
	{
	    if (i != my_node)
	    {
		send.base.service = RSR_HANDLER;
		send.handler_id = -1;
		MPI_Rsr(NULL, 0, MPI_BYTE,
			&send, send_length,
			i, &request);
		MPI_Wait(&request, &status);
	    }
	}
    }
} /* mpinx_shutdown() */


/*
 * mpinx_abort()
 *
 * This routine is called during the _abnormal_ shutdown of a process.
 */
static void mpinx_abort(void)
{
    MPI_Abort(MPI_COMM_WORLD,1);
} /* mpinx_abort() */


/*
 * mpinx_send_rsr()
 *
 * Generate a remote service request message to the node and context
 * saved in the 'nexus_buffer'.
 */
static int mpinx_send_rsr(nexus_buffer_t *nexus_buffer,
			  nexus_startpoint_t *sp_array,
			  int num_sp,
			  int handler_id,
			  nexus_bool_t destroy_buffer)
{
    mpinx_buffer_t *buf;
    mpinx_rsr_send_t send;
    int send_length;
    MPI_Request request;
    MPI_Status status;

    NexusBufferMagicCheck(mpinx_send_remote_service_request, nexus_buffer);

    buf = (mpinx_buffer_t *) *nexus_buffer;
    
    NexusAssert2((buf->proto->type == NEXUS_PROTO_TYPE_MPINX),
		 ("mpinx_send_remote_service_request(): Internal error: proto_type is not NEXUS_PROTO_TYPE_MPINX\n"));

    nexus_debug_printf(2,("mpinx_send_remote_service_request(): invoked with buffer:%x\n", nexus_buffer));

    if (buf->storage == (char *) NULL)
    {
	/*
	 * This is a zero length message.
	 */
	buf->size = 0;
    }
    else
    {
	buf->size = buf->current - buf->storage;
    }

    /*
     * Setup the mpinx_rsr_send_t header
     */
    send.base.service = RSR_HANDLER;
    send.context = buf->context;
    send.address = buf->address;
    send.handler_id = buf->handler_id;
    strcpy(send.handler_name, buf->handler_name);
    send_length = (sizeof(mpinx_rsr_send_t)
		   - NEXUS_MAX_HANDLER_NAME_LENGTH
		   + buf->handler_name_length + 1);
		   
#ifdef BUILD_PROFILE
    send.source_node_id = buf->source_node_id;
    send.source_context_id = buf->source_context_id;
    _nx_pablo_log_remote_service_request_send(buf->dest_node_id,
					      buf->dest_context_id,
					      buf->handler_name,
					      buf->handler_id,
					      buf->size);
#endif

    STOP_CRITICAL_PATH_TIMER();

    /* Perform the rsr */
    MPI_Rsr(buf->storage,
	    buf->size,
	    MPI_BYTE,
	    &send, send_length,
	    buf->proto->destination,
	    &request);
    MPI_Wait(&request, &status);

    /* Free up the send buffer */
    mpinx_enter();
    FreeMpinxBuffer(buf);
    mpinx_exit();
    
    nexus_poll();

    return(0);
	   
} /* mpinx_send_remote_service_request() */


/*
 * mpinx_send_urgent_remote_service_request()
 */
static int mpinx_send_urgent_remote_service_request(nexus_buffer_t *buffer)
{
    return(mpinx_send_remote_service_request(buffer));
} /* mpinx_send_urgent_remote_service_request() */


/*
 * mpinx_destroy_proto()
 *
 * Decrement the reference count for this proto.  If it goes to 0
 * then close the fd used by this proto.
 */
static void mpinx_destroy_proto(nexus_proto_t *nexus_proto)
{
    mpinx_proto_t *proto = (mpinx_proto_t *) nexus_proto;
    mpinx_enter();
    proto->reference_count--;
    NexusAssert2((proto->reference_count >= 0),
		 ("mpinx_destroy(): Internal error: Reference count < 0\n"));
    mpinx_exit();
} /* mpinx_destroy_proto() */

/*
 * mpinx_copy_proto()
 *
 * Increase the reference count on the associated proto and copy the
 * pointer to the nexus_proto_t
 *
 */
static void mpinx_copy_proto(nexus_proto_t *proto_in,
			     nexus_proto_t **proto_out)
{
    mpinx_proto_t *proto = (mpinx_proto_t *) proto_in;

    mpinx_enter();
    proto->reference_count++;
    *proto_out = proto_in;
    mpinx_exit();
} /* mpinx_copy_proto() */

/*
 * mpinx_get_my_mi_proto()
 *
 * Return the machine independent mp protocol information
 * for this protocol.
 */
static nexus_mi_proto_t *mpinx_get_my_mi_proto(void)
{
    nexus_mi_proto_t *mi_proto;
    NexusMalloc(mpinx_get_my_mi_proto(),
		mi_proto,
		nexus_mi_proto_t *,
		(sizeof(nexus_mi_proto_t)	/* base */
		 + sizeof(int)			/* space for ints */
		 ));
    mi_proto->proto_type = NEXUS_PROTO_TYPE_MPINX;
    mi_proto->n_ints = 1;
    mi_proto->n_strings = 0;
    mi_proto->ints = (int *) (((char *) mi_proto)
			      + sizeof(nexus_mi_proto_t));
    mi_proto->ints[0] = my_node;
    mi_proto->string_lengths = (int *) NULL;
    mi_proto->strings = (char **) NULL;
    return (mi_proto);
} /* mpinx_get_my_mi_proto() */


/*
 * mpinx_construct_from_mi_proto()
 *
 * The passed machine independent protocol, 'mi_proto', should
 * be a mp protocol that I can use to connect to that node:
 *  - If it is not a mp protocol, then fatal out.
 *  - If it is a mp protocol:
 *	- If I cannot use this protocol to attach to the node, then
 *		return NEXUS_FALSE.  (This option is useful if two nodes
 *		both speak a particular protocol, but they cannot
 *		talk to each other via that protocol.  For example,
 *		on two MPP, the nodes within a single MPP can
 *		talk to each other via the native messaging protocol,
 *		but cannot talk to the nodes on the other MPP
 *		using that native protocol.)
 *	- If this mp protocol points to myself, then set
 *		*proto=_nx_local_proto, and return NEXUS_TRUE.
 *	- Otherwise, construct a mp protocol object for this mi_proto
 *		and put it in *proto.  Then return NEXUS_TRUE.  
 */
static nexus_bool_t mpinx_construct_from_mi_proto(nexus_proto_t **proto,
						  nexus_mi_proto_t *mi_proto)
{
    int destination;
    nexus_bool_t result;
    
    NexusAssert2((mi_proto->proto_type == NEXUS_PROTO_TYPE_MPINX),
		 ("mpinx_construct_from_mi_proto(): Internal error: Was given wrong type mi_proto\n"));

    destination = mi_proto->ints[0];
    
    /*
     * Test to see if this proto points to myself.
     * If it does, then return the _nx_local_proto.
     */
    if (destination == my_node)
    {
	*proto = _nx_local_proto;
    }
    else
    {
	mpinx_enter();
	*proto = (nexus_proto_t *) construct_proto(destination);
	mpinx_exit();
    }
    return (NEXUS_TRUE);
} /* mpinx_construct_from_mi_proto() */


/*
 * mpinx_get_creator_proto_params()
 *
 * Encapsulate enough protocol information into a retained command
 * line argument so that the construct_creator_proto() routine
 * can reconstruct a proto to connect back to me.
 */
static void mpinx_get_creator_proto_params(char *buf,
					   int buf_size,
					   char *node_name,
					   int node_number)
{
    buf[0] = '\0';
} /* mpinx_get_creator_proto_params() */


/*
 * mpinx_construct_creator_proto()
 *
 * Use the stored command line arguments to construct a proto
 * to my creator.
 */
static void mpinx_construct_creator_proto(nexus_proto_t **proto)
{
    mpinx_enter();
    *proto = (nexus_proto_t *) construct_proto(0);
    mpinx_exit();
} /* mpinx_construct_creator_proto() */


/*
 * mpinx_compare_protos()
 */
static int mpinx_compare_protos(nexus_proto_t *proto1,
			     nexus_proto_t *proto2)
{
    mpinx_proto_t *p1 = (mpinx_proto_t *) proto1;
    mpinx_proto_t *p2 = (mpinx_proto_t *) proto2;
    return(p1->destination == p2->destination);
} /* mpinx_compare_protos() */


/*
 * construct_proto()
 *
 * Construct a mpinx_proto_t for the given destination. Look up in the
 * proto table to see if one already exists. If it does, bump its reference
 * count and return that one. Otherwise create one, insert into the
 * table with a reference count of 1 and return it.
 */
static mpinx_proto_t *construct_proto(int destination)
{
    mpinx_proto_t *proto;

    proto = proto_table_lookup(&destination);
    nexus_debug_printf(3,
		       ("construct_proto(): Table lookup returns proto=%x\n",
			proto));
    if (proto == (mpinx_proto_t *) NULL)
    {
	NexusMalloc(construct_proto(), proto, mpinx_proto_t *,
		    sizeof(mpinx_proto_t));

	proto->type = NEXUS_PROTO_TYPE_MPINX;
	proto->funcs = &mpinx_proto_funcs;
	proto->destination = destination;
	proto->reference_count = 1;
	
	proto_table_insert(proto);
    }
    else
    {
	proto->reference_count++;
    }
	
    return (proto);
} /* construct_proto() */


/*
 * free_proto()
 *
 * Free the passed 'proto'.
 */
static void free_proto(mpinx_proto_t *proto)
{
    NexusFree(proto);
} /* free_proto() */


/*
 * proto_table_init()
 *
 * Initialize the protocol table.
 */
static void proto_table_init(void)
{
    int i;

    for (i = 0; i < PROTO_TABLE_SIZE; i++)
    {
	proto_table[i].proto = (mpinx_proto_t *) NULL;
	proto_table[i].next = (proto_table_entry_t *) NULL;
    }
} /* proto_table_init() */


/*
 * proto_table_insert()
 *
 * Insert the given proto into the table, hashing on its destination.
 *
 * We assume that the entry is not present in the table.
 */
static void proto_table_insert(mpinx_proto_t *proto)
{
    int bucket;
    proto_table_entry_t *new_ent;

    bucket = (proto->destination % PROTO_TABLE_SIZE);

    if (proto_table[bucket].proto == (mpinx_proto_t *) NULL)
    {
	/* Drop it into the preallocated table entry */
	proto_table[bucket].proto = proto;
    }
    else
    {
	/*
	 * Need to allocate a new proto_table_entry_t and add it
	 * to the bucket
	 */
	NexusMalloc(proto_table_insert(), new_ent, proto_table_entry_t *,
		    sizeof(struct _proto_table_entry_t));

	new_ent->proto = proto;
	new_ent->next = proto_table[bucket].next;

	proto_table[bucket].next = new_ent;
    }

} /* proto_table_insert() */


/*
 * proto_table_lookup()
 *
 * Look up and return the mpinx_proto_t for the given destination.
 * Return NULL if none exists.
 */
static mpinx_proto_t *proto_table_lookup(int *dest)
{
    proto_table_entry_t *ent;
    int bucket;
    nexus_bool_t result;

    bucket = (*dest % PROTO_TABLE_SIZE);

    for (ent = &(proto_table[bucket]);
	 ent != (proto_table_entry_t *) NULL;
	 ent = ent->next)
    {
	if (ent->proto != (mpinx_proto_t *) NULL)
	{
	    if (*dest == ent->proto->destination)
	    {
		return (ent->proto);
	    }
	}
    }
    
    return ((mpinx_proto_t *) NULL);
} /* proto_table_lookup() */



/*********************************************************************
 * 		Buffer management code
 *********************************************************************/

/*
 * mpinx_set_buffer_size()
 * 
 * Allocate message buffer space for 'size' bytes, that will be used to
 * hold 'n_elements'.
 */
static void mpinx_set_buffer_size(nexus_buffer_t *buffer,
				  int size, int n_elements)
{
    mpinx_buffer_t *mpinx_buffer;
    char *storage;
    int total_size;

    NexusBufferMagicCheck(mpinx_check_buffer_size, buffer);

    mpinx_buffer = (mpinx_buffer_t *) *buffer;
    
    if (size > 0 )
    {
	NexusMalloc(mpinx_set_buffer_size(),
		    storage,
		    char *,
		    size);

	mpinx_buffer->size = size;
	mpinx_buffer->n_elements = n_elements;
	mpinx_buffer->storage = storage;
	mpinx_buffer->current = storage;
    }

} /* mpinx_set_buffer_size() */


/*
 * mpinx_check_buffer_size()
 *
 * Check that that passed message 'buffer' has at least 'slack'
 * bytes remaining; if not, increase size by 'increment' bytes
 * until there are enough bytes remaining.
 *
 * If no resizing is necessary, leave 'buffer' unchanged and
 * return NEXUS_TRUE.
 * If resizing is successful, modify 'buffer' to a new, larger
 * buffer and return NEXUS_TRUE.
 * Otherwise, if 'increment' is 0 and 'slack' bytes are not
 * available in teh buffer, then leave 'buffer' unchanged and
 * return NEXUS_FALSE.
 */
static int mpinx_check_buffer_size(nexus_buffer_t *buffer,
				   int slack, int increment)
{
    mpinx_buffer_t *mpinx_buffer;
    int used;
    int needed;

    NexusBufferMagicCheck(mpinx_check_buffer_size, buffer);

    mpinx_buffer = (mpinx_buffer_t *) *buffer;
    
    used = mpinx_buffer->current - mpinx_buffer->storage;
    needed = used + slack;

    if (mpinx_buffer->size == 0)
    {
	mpinx_set_buffer_size(buffer, slack, -1);
    }
    else if (needed > mpinx_buffer->size)
    {
	char *new_storage;
	int new_size;

	if (increment <= 0)
	{
	    return(NEXUS_FALSE);
	}

	new_size = mpinx_buffer->size;
	while (new_size < needed)
	{
	    new_size += increment;
	}

	NexusMalloc(mpinx_check_buffer_size(),
		    new_storage,
		    char *,
		    new_size);
	
	memcpy(new_storage, mpinx_buffer->storage, used);
	    
	NexusFree(mpinx_buffer->storage);

	mpinx_buffer->size = new_size;
	mpinx_buffer->storage = new_storage;
	mpinx_buffer->current = mpinx_buffer->storage + used;
    }
    
    return(NEXUS_TRUE);
} /* mpinx_check_buffer_size() */


/*
 * mpinx_free_buffer()
 *
 * Free the passed nexus_buffer_t.
 *
 * This should be called on the receiving end, after the handler
 * has completed.
 *
 * Note: The stashed flag could be set, since mpinx_stash_buffer()
 * just sets this flag and typecasts a buffer to a stashed buffer.
 * In this case, do not free the buffer.
 */
static void mpinx_free_buffer(nexus_buffer_t *buffer)
{
    mpinx_rsr_recv_t *mpinx_buffer;
    
    NexusAssert2((buffer),
		 ("mpinx_free_buffer(): Passed a NULL nexus_buffer_t *\n") );
    
    /* If the buffer was stashed, *buffer will have been set to NULL */
    if (!(*buffer))
    {
	return;
    }

    NexusBufferMagicCheck(mpinx_free_buffer, buffer);

    mpinx_buffer = (mpinx_rsr_recv_t *) *buffer;
    
    NexusAssert2((!mpinx_buffer->stashed),
		 ("mpinx_free_buffer(): Expected a non-stashed buffer\n"));

    free(mpinx_buffer);

    *buffer = (nexus_buffer_t) NULL;
    
} /* mpinx_free_buffer() */


/*
 * mpinx_stash_buffer()
 *
 * Convert 'buffer' to a stashed buffer.
 */
static void mpinx_stash_buffer(nexus_buffer_t *buffer,
			       nexus_stashed_buffer_t *stashed_buffer)
{
    mpinx_rsr_recv_t *mpinx_buffer;
    
    NexusBufferMagicCheck(mpinx_stash_buffer, buffer);

    mpinx_buffer = (mpinx_rsr_recv_t *) *buffer;
    NexusAssert2((!mpinx_buffer->stashed),
		 ("mpinx_stash_buffer(): Expected an un-stashed buffer\n"));
    
    mpinx_buffer->stashed = NEXUS_TRUE;
		  
    *stashed_buffer = (nexus_stashed_buffer_t) *buffer;
    
    *buffer = (nexus_buffer_t) NULL;
} /* mpinx_stash_buffer() */


/*
 * mpinx_free_stashed_buffer()
 *
 * Free the passed nexus_stashed_buffer_t that was stashed
 * by mpinx_stash_buffer().
 */
static void mpinx_free_stashed_buffer(nexus_stashed_buffer_t *stashed_buffer)
{
    mpinx_rsr_recv_t *mpinx_buffer;
    
    NexusAssert2((stashed_buffer),
		 ("mpinx_free_stashed_buffer(): Passed a NULL nexus_stashed_buffer_t *\n") );
    NexusBufferMagicCheck(mpinx_free_stashed_buffer,
			  (nexus_buffer_t *) stashed_buffer);
    
    mpinx_buffer = (mpinx_rsr_recv_t *) *stashed_buffer;
    
    NexusAssert2((mpinx_buffer->stashed),
		 ("mpinx_free_stashed_buffer(): Expected a stashed buffer\n"));

    free(mpinx_buffer);

    *stashed_buffer = (nexus_stashed_buffer_t) NULL;
    
} /* mpinx_free_stashed_buffer() */



/*
 * mpinx_sizeof_*()
 *
 * Return the size (in bytes) that 'count' elements of the given
 * type will require to be put into the 'buffer'.
 */
static int mpinx_sizeof_float(nexus_buffer_t *buffer, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buffer;
    return(mpinx_buf->sizeof_table->float_size * count);
}

static int mpinx_sizeof_double(nexus_buffer_t *buffer, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buffer;
    return(mpinx_buf->sizeof_table->double_size * count);
}

static int mpinx_sizeof_short(nexus_buffer_t *buffer, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buffer;
    return(mpinx_buf->sizeof_table->short_size * count);
}

static int mpinx_sizeof_u_short(nexus_buffer_t *buffer, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buffer;
    return(mpinx_buf->sizeof_table->u_short_size * count);
}

static int mpinx_sizeof_int(nexus_buffer_t *buffer, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buffer;
    return(mpinx_buf->sizeof_table->int_size * count);
}

static int mpinx_sizeof_u_int(nexus_buffer_t *buffer, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buffer;
    return(mpinx_buf->sizeof_table->u_int_size * count);
}

static int mpinx_sizeof_long(nexus_buffer_t *buffer, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buffer;
    return(mpinx_buf->sizeof_table->long_size * count);
}

static int mpinx_sizeof_u_long(nexus_buffer_t *buffer, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buffer;
    return(mpinx_buf->sizeof_table->u_long_size * count);
}

static int mpinx_sizeof_char(nexus_buffer_t *buffer, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buffer;
    return(mpinx_buf->sizeof_table->char_size * count);
}

static int mpinx_sizeof_u_char(nexus_buffer_t *buffer, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buffer;
    return(mpinx_buf->sizeof_table->u_char_size * count);
}




/*
 * mpinx_put_*()
 *
 * Put 'count' elements, starting at 'data', of the given type,
 * into 'buffer'.
 */
static void mpinx_put_float(nexus_buffer_t *buf,
			    float *data, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buf;
    PUT_FLOAT(mpinx_buf, data, count);
}

static void mpinx_put_double(nexus_buffer_t *buf,
			     double *data, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buf;
    PUT_DOUBLE(mpinx_buf, data, count);
}

static void mpinx_put_short(nexus_buffer_t *buf,
			    short *data, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buf;
    PUT_SHORT(mpinx_buf, data, count);
}

static void mpinx_put_u_short(nexus_buffer_t *buf,
			      unsigned short *data, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buf;
    PUT_U_SHORT(mpinx_buf, data, count);
}

static void mpinx_put_int(nexus_buffer_t *buf,
			  int *data, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buf;
    PUT_INT(mpinx_buf, data, count);
}

static void mpinx_put_u_int(nexus_buffer_t *buf,
			    unsigned int *data, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buf;
    PUT_U_INT(mpinx_buf, data, count);
}

static void mpinx_put_long(nexus_buffer_t *buf,
			   long *data, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buf;
    PUT_LONG(mpinx_buf, data, count);
}

static void mpinx_put_u_long(nexus_buffer_t *buf,
			     unsigned long *data, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buf;
    PUT_U_LONG(mpinx_buf, data, count);
}

static void mpinx_put_char(nexus_buffer_t *buf,
			   char *data, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buf;
    PUT_CHAR(mpinx_buf, data, count);
}

static void mpinx_put_u_char(nexus_buffer_t *buf,
			     unsigned char *data, int count)
{
    mpinx_buffer_t *mpinx_buf = (mpinx_buffer_t *) *buf;
    PUT_U_CHAR(mpinx_buf, data, count);
}



/*
 * mpinx_get_*()
 *
 * Get 'count' elements of the given type from 'buffer' and store
 * them into 'data'.
 */
static void mpinx_get_float(nexus_buffer_t *buf,
			    float *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_FLOAT(mpinx_buf, dest, count);
}

static void mpinx_get_double(nexus_buffer_t *buf,
			     double *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_DOUBLE(mpinx_buf, dest, count);
}

static void mpinx_get_short(nexus_buffer_t *buf,
			    short *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_SHORT(mpinx_buf, dest, count);
}

static void mpinx_get_u_short(nexus_buffer_t *buf,
			      unsigned short *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_U_SHORT(mpinx_buf, dest, count);
}

static void mpinx_get_int(nexus_buffer_t *buf,
			  int *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_INT(mpinx_buf, dest, count);
}

static void mpinx_get_u_int(nexus_buffer_t *buf,
			    unsigned int *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_U_INT(mpinx_buf, dest, count);
}

static void mpinx_get_long(nexus_buffer_t *buf,
			   long *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_LONG(mpinx_buf, dest, count);
}

static void mpinx_get_u_long(nexus_buffer_t *buf,
			     unsigned long *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_U_LONG(mpinx_buf, dest, count);
}

static void mpinx_get_char(nexus_buffer_t *buf,
			   char *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_CHAR(mpinx_buf, dest, count);
}

static void mpinx_get_u_char(nexus_buffer_t *buf,
			     unsigned char *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_U_CHAR(mpinx_buf, dest, count);
}



/*
 * mpinx_get_stashed_*()
 *
 * Get 'count' elements of the given type from the stashed 'buffer' and store
 * them into 'data'.
 */
static void mpinx_get_stashed_float(nexus_stashed_buffer_t *buf,
				    float *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_STASHED_FLOAT(mpinx_buf, dest, count);
}

static void mpinx_get_stashed_double(nexus_stashed_buffer_t *buf,
				     double *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_STASHED_DOUBLE(mpinx_buf, dest, count);
}

static void mpinx_get_stashed_short(nexus_stashed_buffer_t *buf,
				    short *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_STASHED_SHORT(mpinx_buf, dest, count);
}

static void mpinx_get_stashed_u_short(nexus_stashed_buffer_t *buf,
				      unsigned short *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_STASHED_U_SHORT(mpinx_buf, dest, count);
}

static void mpinx_get_stashed_int(nexus_stashed_buffer_t *buf,
				  int *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_STASHED_INT(mpinx_buf, dest, count);
}

static void mpinx_get_stashed_u_int(nexus_stashed_buffer_t *buf,
				    unsigned int *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_STASHED_U_INT(mpinx_buf, dest, count);
}

static void mpinx_get_stashed_long(nexus_stashed_buffer_t *buf,
				   long *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_STASHED_LONG(mpinx_buf, dest, count);
}

static void mpinx_get_stashed_u_long(nexus_stashed_buffer_t *buf,
				     unsigned long *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_STASHED_U_LONG(mpinx_buf, dest, count);
}

static void mpinx_get_stashed_char(nexus_stashed_buffer_t *buf,
				   char *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_STASHED_CHAR(mpinx_buf, dest, count);
}

static void mpinx_get_stashed_u_char(nexus_stashed_buffer_t *buf,
				     unsigned char *dest, int count)
{
    mpinx_rsr_recv_t *mpinx_buf = (mpinx_rsr_recv_t *) *buf;
    GET_STASHED_U_CHAR(mpinx_buf, dest, count);
}
