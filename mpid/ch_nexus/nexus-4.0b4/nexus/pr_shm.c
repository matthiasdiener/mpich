/*
 * Nexus
 * Author:     Cheryl DeMatteis
 *              The Aerospace Corporation
 *
 * pr_shm.c		- shared memory protocol module
 *
 *  TODO:
 *    - add feature which allocates another shared memory segment if the
 *      amount of available shared memory is exhausted
 */

#include "internal.h"
#include "nexus.h"

/* shared memory specific include files */
#include "pr_shm.h"

/* hack for debugging shm_abort stuff */
int shm_abort_flag = 0;

/* shared memory specific variables */
int shmid;
void* shmptr;
static int *shm_counter;	/* get rid of this */
static nexus_bool_t     shm_done;
static nexus_bool_t     using_handler_thread;
static nexus_bool_t     handler_thread_done;
static nexus_mutex_t    handler_thread_done_mutex;
static nexus_cond_t     handler_thread_done_cond;

static void *shm_handler_thread( void *arg);

extern int  same_gp( nexus_global_pointer_t*, nexus_global_pointer_t*,
		    nexus_bool_t );

/*
 * Only one thread is allowed to be in the shared memory protocol code
 * (and thus 
 * mucking with data structures) at a time.
 */
#ifdef BUILD_LITE
#define shm_enter()
#define shm_exit()
#else  /* BUILD_LITE */
static nexus_mutex_t  shm_mutex;
#define shm_enter()	nexus_mutex_lock(&shm_mutex);
#define shm_exit()	nexus_mutex_unlock(&shm_mutex);
#endif /* BUILD_LITE */

/*
 * Some forward typedef declarations...
 */
typedef struct _shm_buffer_t	     shm_buffer_t;
typedef struct _shm_proto_t	     shm_proto_t;
typedef struct _shm_buffer_header_t  shm_buffer_header_t;


/*
 * Thread is handler?
 *
 * Thread specific storage is used to keep track if the current
 * thread is a handler thread or not.
 */

#ifndef BUILD_LITE
static nexus_thread_key_t i_am_shm_handler_thread_key;

#define _nx_set_i_am_shm_handler_thread() \
    nexus_thread_setspecific(i_am_shm_handler_thread_key, (void *) 1)
#define _nx_i_am_shm_handler_thread(Result) \
    *(Result) = (nexus_bool_t)nexus_thread_getspecific(i_am_shm_handler_thread_key)
#endif /* BUILD_LITE */


#ifdef BUILD_DEBUG
/*
static char tmpbuf1[10240];
static char tmpbuf2[10240];
*/
#endif

/*
 * shm_proto_t
 *
 * This is an overload of nexus_proto_t.  It adds the
 * shared memory specific information to that structure.
 */
struct _shm_proto_t
{
    nexus_proto_type_t	type;	/* NEXUS_PROTO_TYPE_SHM */
    nexus_proto_funcs_t *funcs;
    int                 node_id;
    int                 context_id;
    int                 pid;
};


/*
 * shm_buffer_t
 *
 * This is an overload of nexus_buffer_t.  It adds the
 * shared memory specific information to that structure.
 */
struct _shm_buffer_t
{
#ifdef BUILD_DEBUG
    int				magic;
#endif /* BUILD_DEBUG */
    nexus_buffer_funcs_t *	funcs;
    shm_buffer_t *		next;
    unsigned long		context;
    unsigned long		address;
#ifdef NEXUS_PROFILE
    int				node_id;
    int				context_id;
    int				message_length;
#endif /* NEXUS_PROFILE */    
    unsigned long		size;
    int				n_elements;
    nexus_bool_t		stashed;
    char *			base_pointer;
    char *			current_pointer;
    int				handler_id;
    char			handler_name[NEXUS_MAX_HANDLER_NAME_LENGTH+1];
    shm_buffer_header_t         *hdr;
};


/* the entries in this structure will have to be flushed out.  Some of
 * them are here because I plan on using them later, some of them are
 * old and outdated
 */

struct _shm_buffer_header_t
{
   int                     node_id;
   int                     context_id;
   int                     pid;
   key_t                   shmid;
   int                     shmsize;
   char                   *shmptr;
   Malloc                 *shm_top;
   nexus_mutex_t           header_mutex;
   shm_buffer_t           *buffer_free_list;
   nexus_mutex_t	   buffer_free_list_mutex;
   nexus_mutex_t	   message_queue_mutex;
   nexus_cond_t            message_arrival_cond;
   nexus_mutex_t           message_arrival_mutex;
   shm_buffer_header_t    *next;
   shm_buffer_t *	   message_queue_head;
   shm_buffer_t *	   message_queue_tail;
};



/*
 * header of the message queus
 */

shm_buffer_header_t* shm_message_header;


/*
 *  Using shared memory requires using a mutex for anything held
 *  in shared memory, whether or not we are using threads
 */

#define lock_header(mutex) nexus_mutex_lock(&mutex)
#define unlock_header(mutex) nexus_mutex_unlock(&mutex)
#define lock_buffer_free_list(mutex)   nexus_mutex_lock(&mutex)
#define unlock_buffer_free_list(mutex) nexus_mutex_unlock(&mutex)
#define lock_buffer_mem(mutex) nexus_mutex_lock(&mutex)
#define unlock_buffer_mem(mutex) nexus_mutex_unlock(&mutex)
#define lock_message_queue(mutex) nexus_mutex_lock(&mutex)
#define unlock_message_queue(mutex) nexus_mutex_unlock(&mutex)


#ifdef BUILD_DEBUG
#define MallocShmBuffer(Routine, Head, Buf) \
{ \
    lock_header( shm_message_header->header_mutex ); \
    Buf = (shm_buffer_t*) mn_malloc((Head)->shm_top, sizeof(struct _shm_buffer_t)); \
    unlock_header( shm_message_header->header_mutex );  \
    if( Buf == NULL ) \
      nexus_fatal("mn_malloc failed"); \
    Buf->magic = NEXUS_BUFFER_MAGIC; \
}
#else  /* BUILD_DEBUG */
#define MallocShmBuffer(Routine, Head, Buf) \
{ \
    lock_header( shm_message_header->header_mutex ); \
    Buf = (shm_buffer_t*) mn_malloc((Head)->shm_top, sizeof(struct _shm_buffer_t)); \
    unlock_header( shm_message_header->header_mutex); \
    if( Buf == NULL ) \
       nexus_fatal("mn_malloc failed"); \
}
#endif /* BUILD_DEBUG */


#define FreeShmBuffer(Buf) \
{ \
     lock_buffer_free_list((Buf)->hdr->buffer_free_list_mutex); \
     (Buf)->next = (Buf)->hdr->buffer_free_list; \
     (Buf)->hdr->buffer_free_list = (Buf); \
     mn_free( (Buf)->hdr->shm_top, (Buf)->base_pointer ); \
     unlock_buffer_free_list((Buf)->hdr->buffer_free_list_mutex); \
}


#define GetShmBuffer(Routine, Head, Buf) \
{ \
    lock_buffer_free_list((Head)->buffer_free_list_mutex); \
    if ((Head)->buffer_free_list) \
    { \
        Buf = (Head)->buffer_free_list; \
	(Head)->buffer_free_list = (Head)->buffer_free_list->next; \
	unlock_buffer_free_list((Head)->buffer_free_list_mutex); \
    } \
    else \
    { \
	unlock_buffer_free_list((Head)->buffer_free_list_mutex); \
	MallocShmBuffer(Routine, Head, Buf); \
    } \
    Buf->hdr = Head; \
}

void DequeueMessage( shm_buffer_t **buf) 
{ 
    shm_buffer_header_t* header;
    int context_id;
    int pid;


    _nx_context_id( &context_id );
    pid = _nx_md_getpid();
    header = shm_message_header; 

    while( _nx_my_node.number != header->node_id ||
	   context_id != header->context_id ||
	  pid != header->pid )
       header = header->next;

    /* protects against more than one handler takig stuff off the queue
     what should be done is to use a mutex that exists on the message
     header itself.
     */
    if( header->message_queue_head == NULL )
    {
       *buf = NULL;
       return;
    }
    
/*    lock_message_queue( header->message_queue_mutex ); */
    *buf = header->message_queue_head; 
    header->message_queue_head = 
     header->message_queue_head->next; 
/*    unlock_message_queue( buf->hdr->message_queue_mutex );*/
    nexus_debug_printf(4,("DequeueMessage(): buf [ 0x%x ]\n", buf));

    
} /* DequeueMessage() */


/*
 * This is a message queue containing all messages that have
 * been sent via shared memory and are waiting to be handled.
 *
 * In the single-threaded version, the sending thread will
 * call handle_enqueued_messages() to handle messages on this queue.
 *
 * In the multi-threaded version, a separate handler thread will
 * call handle_enqueued_messages() to handle messages on this queue.
 * A lock and condition is used to awaken the handler thread when
 * there is a message to be delivered.
 */

/* xxx need to lock the list to enqueue and dequeue */
#define EnqueueMessage(buf) \
{ \
    if ((buf)->hdr->message_queue_head == (shm_buffer_t *) NULL) \
    { \
      (buf)->hdr->message_queue_head = (buf)->hdr->message_queue_tail = buf; \
    } \
    else \
    { \
	(buf)->hdr->message_queue_tail->next = buf; \
	(buf)->hdr->message_queue_tail = buf; \
    } \
    buf->next = (shm_buffer_t *) NULL; \
}


/*
 *  there is a linked list of message queues...they all must be
 *  checked.  Too complicated to be a macro...
 */

int MessagesEnqueued() 
{
   int context_id;
   int pid;

   shm_buffer_header_t *header = shm_message_header;
   _nx_context_id( &context_id );
   pid = _nx_md_getpid();

   while( header && (( header->node_id != _nx_my_node.number )
		     || ( header->context_id != context_id )
		     || ( header->pid != pid) ))
   {
      header = header->next;
   }

   if( header == NULL || header->message_queue_head == (shm_buffer_t*)NULL )
      return NEXUS_FALSE;
   else
   {
      return NEXUS_TRUE;
   }
} /* MessagesEnqueued */


static nexus_bool_t	handle_in_progress;


static void handle_enqueued_messages(void);


/*
 * Various forward declarations of procedures
 */
static nexus_proto_type_t	shm_proto_type(void);
static void		shm_init(int *argc, char ***argv);
static void             shm_poll( void );
static void             shm_get_my_mi_proto( nexus_byte_t **array,
                                             int *size);
static void		shm_shutdown(nexus_bool_t shutdown_others);
static void		shm_abort(void);

static void		shm_init_remote_service_request(
					    nexus_buffer_t *buffer,
					    nexus_global_pointer_t *gp,
					    char *handler_name,
					    int handler_id);
static int		shm_send_remote_service_request(
						   nexus_buffer_t *buffer);
static int		shm_send_urgent_remote_service_request(
						   nexus_buffer_t *buffer);
static nexus_bool_t	shm_construct_from_mi_proto(nexus_proto_t **proto,
						    nexus_mi_proto_t *mi_proto,
						      nexus_byte_t *array,
						      int size);

static shm_proto_t *construct_proto( int context_id, int node_id, int pid );


static void		shm_set_buffer_size(nexus_buffer_t *buffer,
					      int size, int n_elements);
static int		shm_check_buffer_size(nexus_buffer_t *buffer,
						int slack, int increment);
static void		shm_free_buffer(nexus_buffer_t *buffer);
static void		shm_stash_buffer(nexus_buffer_t *buffer,
				       nexus_stashed_buffer_t *stashed_buffer);
static void		shm_free_stashed_buffer(
				       nexus_stashed_buffer_t *stashed_buffer);


static int	shm_sizeof_float	(nexus_buffer_t *buffer, int count);
static int	shm_sizeof_double	(nexus_buffer_t *buffer, int count);
static int	shm_sizeof_short	(nexus_buffer_t *buffer, int count);
static int	shm_sizeof_u_short	(nexus_buffer_t *buffer, int count);
static int	shm_sizeof_int	(nexus_buffer_t *buffer, int count);
static int	shm_sizeof_u_int	(nexus_buffer_t *buffer, int count);
static int	shm_sizeof_long	(nexus_buffer_t *buffer, int count);
static int	shm_sizeof_u_long	(nexus_buffer_t *buffer, int count);
static int	shm_sizeof_char	(nexus_buffer_t *buffer, int count);
static int	shm_sizeof_u_char	(nexus_buffer_t *buffer, int count);
static int	shm_sizeof_byte	(nexus_buffer_t *buffer, int count);


static void	shm_put_float	(nexus_buffer_t *buffer,
				 float *data, int count);
static void	shm_put_double(nexus_buffer_t *buffer,
				 double *data, int count);
static void	shm_put_short	(nexus_buffer_t *buffer,
				 short *data, int count);
static void	shm_put_u_short(nexus_buffer_t *buffer,
				 unsigned short *data, int count);
static void	shm_put_int	(nexus_buffer_t *buffer,
				 int *data, int count);
static void	shm_put_u_int	(nexus_buffer_t *buffer,
				 unsigned int *data, int count);
static void	shm_put_long	(nexus_buffer_t *buffer,
				 long *data, int count);
static void	shm_put_u_long(nexus_buffer_t *buffer,
				 unsigned long *data, int count);
static void	shm_put_char	(nexus_buffer_t *buffer,
				 char *data, int count);
static void	shm_put_u_char(nexus_buffer_t *buffer,
				 unsigned char *data, int count);
static void	shm_put_byte(nexus_buffer_t *buffer,
			       unsigned char *data, int count);

static void	shm_get_float	(nexus_buffer_t *buffer,
				 float *dest, int count);
static void	shm_get_double(nexus_buffer_t *buffer,
				 double *dest, int count);
static void	shm_get_short	(nexus_buffer_t *buffer,
				 short *dest, int count);
static void	shm_get_u_short(nexus_buffer_t *buffer,
				 unsigned short *dest, int count);
static void	shm_get_int	(nexus_buffer_t *buffer,
				 int *dest, int count);
static void	shm_get_u_int	(nexus_buffer_t *buffer,
				 unsigned int *dest, int count);
static void	shm_get_long	(nexus_buffer_t *buffer,
				 long *dest, int count);
static void	shm_get_u_long(nexus_buffer_t *buffer,
				 unsigned long *dest, int count);
static void	shm_get_char	(nexus_buffer_t *buffer,
				 char *dest, int count);
static void	shm_get_u_char(nexus_buffer_t *buffer,
				 unsigned char *dest, int count);
static void	shm_get_byte(nexus_buffer_t *buffer,
			       unsigned char *dest, int count);

static void	shm_get_stashed_float	(nexus_stashed_buffer_t *buffer,
					 float *dest, int count);
static void	shm_get_stashed_double(nexus_stashed_buffer_t *buffer,
					 double *dest, int count);
static void	shm_get_stashed_short	(nexus_stashed_buffer_t *buffer,
					 short *dest, int count);
static void	shm_get_stashed_u_short(nexus_stashed_buffer_t *buffer,
					 unsigned short *dest, int count);
static void	shm_get_stashed_int	(nexus_stashed_buffer_t *buffer,
					 int *dest, int count);
static void	shm_get_stashed_u_int	(nexus_stashed_buffer_t *buffer,
					 unsigned int *dest, int count);
static void	shm_get_stashed_long	(nexus_stashed_buffer_t *buffer,
					 long *dest, int count);
static void	shm_get_stashed_u_long(nexus_stashed_buffer_t *buffer,
					 unsigned long *dest, int count);
static void	shm_get_stashed_char	(nexus_stashed_buffer_t *buffer,
					 char *dest, int count);
static void	shm_get_stashed_u_char(nexus_stashed_buffer_t *buffer,
					 unsigned char *dest, int count);
static void	shm_get_stashed_byte(nexus_stashed_buffer_t *buffer,
				       unsigned char *dest, int count);



static nexus_proto_funcs_t shm_proto_funcs =
{
    shm_proto_type,
    shm_init,
    shm_shutdown,
    shm_abort,
    shm_poll,
    NULL /* shm_blocking_poll */,
    shm_init_remote_service_request,
    NULL /* shm_increment_reference_count */,
    NULL /* shm_decrement_reference_count */,
    shm_get_my_mi_proto,
    shm_construct_from_mi_proto,
    NULL /* shm_test_proto */,
};

static nexus_buffer_funcs_t shm_buffer_funcs =
{
    shm_set_buffer_size,
    shm_check_buffer_size,
    shm_send_remote_service_request,
    shm_send_urgent_remote_service_request,
    shm_free_buffer,
    shm_stash_buffer,
    shm_free_stashed_buffer,
    shm_sizeof_float,
    shm_sizeof_double,
    shm_sizeof_short,
    shm_sizeof_u_short,
    shm_sizeof_int,
    shm_sizeof_u_int,
    shm_sizeof_long,
    shm_sizeof_u_long,
    shm_sizeof_char,
    shm_sizeof_u_char,
    shm_sizeof_byte,
    shm_put_float,
    shm_put_double,
    shm_put_short,
    shm_put_u_short,
    shm_put_int,
    shm_put_u_int,
    shm_put_long,
    shm_put_u_long,
    shm_put_char,
    shm_put_u_char,
    shm_put_byte,
    shm_get_float,
    shm_get_double,
    shm_get_short,
    shm_get_u_short,
    shm_get_int,
    shm_get_u_int,
    shm_get_long,
    shm_get_u_long,
    shm_get_char,
    shm_get_u_char,
    shm_get_byte,
    shm_get_stashed_float,
    shm_get_stashed_double,
    shm_get_stashed_short,
    shm_get_stashed_u_short,
    shm_get_stashed_int,
    shm_get_stashed_u_int,
    shm_get_stashed_long,
    shm_get_stashed_u_long,
    shm_get_stashed_char,
    shm_get_stashed_u_char,
    shm_get_stashed_byte,
};


/*
 * _nx_pr_shm_info()
 *
 * Return the nexus_proto_funcs_t function table for this protocol module.
 *
 * This procedure is used for bootstrapping the protocol module.
 * The higher level Nexus code needs to call this routine to
 * retrieve the functions it needs to use this protocol module.
 */
void *_nx_pr_shm_info(void)
{
    return((void *) (&shm_proto_funcs));
} /* _nx_pr_shm_info() */


/*
 * shm_proto_type()
 *
 * Return the nexus_proto_type_t for this protocol module.
 */
static nexus_proto_type_t shm_proto_type(void)
{
    return (NEXUS_PROTO_TYPE_SHM);
} /* shm_proto_type() */


/*
 * shm_init()
 *
 * Initialize the SHM protocol.
 */
static void shm_init(int *argc, char ***argv)
{
   key_t shmkey;
   int shmsize;
   int arg_n;
   char *key_string;
   char *size_string;
   int my_node_id;
   shm_buffer_header_t *header_ptr;
   Malloc *mem;
   int context_id;

   nexus_printf("shm_init(): inside\n");
#ifndef BUILD_LITE
   
   nexus_mutex_init(&shm_mutex, (nexus_mutexattr_t *) NULL);
   nexus_cond_init(&handler_thread_done_cond, (nexus_condattr_t*) NULL);   

#endif /* BUILD_LITE */

   shm_done = NEXUS_FALSE;


    /* get the shmkey and size from the command-line, if not there
     * look for it in the .resource_database, if not
     * found, use the default values: (defined in pr_shm.h)
     *   NEXUS_SHM_KEY 
     *   NEXUS_SHM_SIZE
     */

#ifndef BUILD_LITE   
   nexus_thread_key_create(&i_am_shm_handler_thread_key, NULL);
#endif /* BUILD_LITE */
      
   if( (arg_n = nexus_find_argument( argc, argv, "shmkey", 2)) >= 0 )
   {
      shmkey = atoi((*argv)[arg_n + 1]);
   }
   else if(key_string = nexus_rdb_lookup(_nx_my_node.name, "shm_key"))
   {
      shmkey = atoi( key_string );
      nexus_rdb_free( key_string );
   }
   else
   {
      shmkey = NEXUS_SHM_KEY;
   }

   if( (arg_n = nexus_find_argument( argc, argv, "shmsize", 2)) >= 0 )
   {
      shmsize = atoi((*argv)[arg_n + 1]);
   }
   else if(size_string = nexus_rdb_lookup(_nx_my_node.name, "shm_size"))
   {
      shmsize = atoi( size_string );
      nexus_rdb_free( size_string );
   }
   else
   {
      shmsize = NEXUS_SHM_SIZE;
   }

   /* above lookups tested, they work with and without entries in
    * the .resource_database and using command-line arguments
    * Note: command-line arguments take precedence
    */

   shmid = shmget( shmkey, shmsize, IPC_CREAT | SHM_R | SHM_W );
   if( shmid < 0 )
   {
      nexus_fatal("shm_init(): shmget failed errno [ %d ]\n", errno);
   }
   
#ifdef SOLARIS			/* really for the ultrasparc */
   shmptr = shmat( shmid, NULL, SHM_SHARE_MMU );
#endif /* SOLARIS */
   shmptr = shmat( shmid, NULL, 0 );
   if( (int)shmptr == -1 )
   {
      nexus_fatal("shm_init(): shmat failed errno [ %d ]\n", errno);
   }

   /* make sure that the headers are allocated for this node */


   /* shm_counter will be the first value in the shm segment */
   shm_counter = (int*)shmptr;

   /* shm_message_header will be the second structure in the shm segment */
   shm_message_header = (shm_buffer_header_t*)((char*)shmptr + sizeof(int));

   /* get current context_id */
   _nx_context_id( &context_id );

   if( _nx_master_node )
   {
      mem = (Malloc*) ((char*)shmptr + sizeof(int) +
		       sizeof( shm_buffer_header_t));
      construct_mn_Malloc( &mem,
			  (caddr_t)shmptr + sizeof( shm_buffer_header_t)
			  + sizeof( Malloc),
			  shmsize -
			  (sizeof( shm_buffer_header_t) + sizeof(Malloc))
			  );

      /* malloc and initialize beginning node of the message headers */
      shm_message_header->next               = (shm_buffer_header_t*)NULL;
      shm_message_header->message_queue_head = (shm_buffer_t*)NULL;
      shm_message_header->message_queue_tail = (shm_buffer_t*)NULL;
      shm_message_header->shmid              = shmid;
      shm_message_header->shm_top            = mem;
      shm_message_header->buffer_free_list   = (shm_buffer_t*)NULL;

      shm_message_header->node_id            = _nx_my_node.number;
      shm_message_header->context_id         = context_id;
      shm_message_header->pid                = _nx_md_getpid();

      /* initialize the mutexes associated with this node */
      nexus_mutex_init( &(shm_message_header->buffer_free_list_mutex),
		       (nexus_mutexattr_t*) NULL);
      nexus_mutex_init( &(shm_message_header->header_mutex),
		       (nexus_mutexattr_t*) NULL);
      nexus_mutex_init( &(shm_message_header->message_queue_mutex),
		       (nexus_mutexattr_t*) NULL);
      nexus_cond_init( &(shm_message_header->message_arrival_cond),
		       (nexus_condattr_t*) NULL);
      nexus_printf("shm_init(): created header node[ %d ] context [ %d ] pid[ %d ]\n",
		   shm_message_header->node_id,
		   shm_message_header->context_id,
		   shm_message_header->pid);
   }
   else
   {
      if( shm_message_header == (shm_buffer_header_t*)NULL )
	 nexus_fatal("shm_message_header not initialized");
      /* can I assume that the master node always gets into init first ? */
      header_ptr = shm_message_header;

      /* I don't know if the order of the shm_init will be guaranteed...
       * therefore have to be careful traversing this list, I don't know
       * if all the headers have been initialized yet
       */

      while( header_ptr->next != NULL )
      {
	 header_ptr = header_ptr->next;
      }

      lock_header( shm_message_header->header_mutex );
      header_ptr->next =
	 (shm_buffer_header_t*)mn_malloc( shm_message_header->shm_top,
					 sizeof( shm_buffer_header_t));
      unlock_header( shm_message_header->header_mutex );    
      if( header_ptr->next == (shm_buffer_header_t*)NULL )
	 nexus_fatal("shm_init(): mn_malloc failed\n");
      header_ptr->next->message_queue_head = (shm_buffer_t*)NULL;
      header_ptr->next->message_queue_tail = (shm_buffer_t*)NULL;
      header_ptr->next->shm_top = shm_message_header->shm_top;	    
      header_ptr = header_ptr->next;
      header_ptr->node_id = _nx_my_node.number;
      header_ptr->context_id = context_id;
      header_ptr->pid = _nx_md_getpid();
      header_ptr->shmid = shmid;
      header_ptr->buffer_free_list = (shm_buffer_t*)NULL;
      header_ptr->next = (shm_buffer_header_t*)NULL;
      /* initialize the mutexes associated with this node */
      nexus_mutex_init( &(header_ptr->buffer_free_list_mutex),
		       (nexus_mutexattr_t*) NULL);
      nexus_mutex_init( &(header_ptr->message_queue_mutex),
		       (nexus_mutexattr_t*) NULL);		       
      nexus_cond_init( &(header_ptr->message_arrival_cond),
		      (nexus_condattr_t*) NULL);
      nexus_printf("shm_init(): created header node[ %d ] context [ %d ] pid[ %d ]\n",
		   header_ptr->node_id,
		   header_ptr->context_id,
		   header_ptr->pid);
      
   }
   
   /*   message_queue_head = message_queue_tail = (shm_buffer_t *) NULL;*/
   handle_in_progress = NEXUS_FALSE;

#ifndef BUILD_LITE   
   if( nexus_preemptive_threads() )
   {
      nexus_thread_t thread;
      using_handler_thread = NEXUS_TRUE;

      handler_thread_done = NEXUS_FALSE;

      nexus_thread_create( &thread,
			   (nexus_thread_attr_t*) NULL,
			   shm_handler_thread,
			   (void*) NULL
			  );
      shm_proto_funcs.poll = NULL;
      shm_proto_funcs.blocking_poll = NULL;
   }
   else
   {
      using_handler_thread = NEXUS_FALSE;
   }
#endif /* BUILD_LITE */

   nexus_printf("shm_init(): leaving\n");
    
} /* shm_init() */



/*
 * shm_handler_thread()
 *
 * The entry point for the handler thread (In the multithreaded version)
 *
 */
static void *shm_handler_thread( void *arg)
{
   shm_buffer_header_t *header = shm_message_header;

   _nx_set_i_am_shm_handler_thread();

   nexus_printf("shm_handler_thread():inside\n");
   while( 1 )
   {
      /*
      nexus_mutex_lock( &(header->message_arrival_mutex) );
      nexus_cond_wait( &(header->message_arrival_cond), &(header->message_arrival_mutex) );
*/
      shm_enter();
      handle_enqueued_messages();
      shm_exit();

/*
      nexus_mutex_unlock( &header->message_arrival_mutex );
*/
      if(shm_done)
      {
	 nexus_printf("shm_handler_thread(): shm_done\n");
	 break;
      }
   }


   nexus_printf("shm_handler_thread(): getting lock\n");
   
   /* check to see if I'm the handler thread or not here */
   nexus_mutex_lock(&handler_thread_done_mutex);
   handler_thread_done = NEXUS_TRUE;
   nexus_cond_signal(&handler_thread_done_cond);
   nexus_mutex_unlock(&handler_thread_done_mutex);

   nexus_printf("shm_handler_thread(): all done\n");
   return ((void *) NULL);

} /* shm_handler_thread() */

/*
 * shm_poll()
 *
 *
 */

static void shm_poll( void )
{
   shm_buffer_t *buf;

   /* this should change...to be more efficient. */

   if( MessagesEnqueued() )
   {
      shm_enter();
      handle_enqueued_messages();
      shm_exit();
   }

} /* shm_poll() */


/*
 *  shm_get_my_mi_proto
 */

static void  shm_get_my_mi_proto(nexus_byte_t **array,
				 int *size)
{
   int my_node_number;
   int my_context_id;
   int my_pid;
   int i;
   
   *size = 12;/* should this be sizeof(int) */
   i = 0;

   NexusMalloc(shm_get_my_mi_proto(),
	       *array,
	       nexus_byte_t *,
	       *size);

   my_node_number = _nx_my_node.number;
   _nx_context_id( &my_context_id );
   my_pid = _nx_md_getpid();

   nexus_printf("shm_get_my_mi_proto(): context[ %d ] node[ %d ] pid [ %d ]\n",
		my_context_id, my_node_number, my_pid);

   PackInt4(*array, i, my_context_id);
   PackInt4(*array, i, my_node_number);
   PackInt4(*array, i, my_pid);
} /* shm_get_my_mi_proto */


/*
 * shm_shutdown()
 *
 * This routine is called during normal shutdown of a process.
 */
static void shm_shutdown(nexus_bool_t shutdown_others)
{
   struct shmid_ds sp;
   int shmid;

   shm_enter();
   shm_done = NEXUS_TRUE;
   shm_exit();
   if( using_handler_thread )
   {
      nexus_bool_t i_am_shm_handler_thread;
      
      _nx_i_am_shm_handler_thread(&i_am_shm_handler_thread);

      if( ! i_am_shm_handler_thread )
      {
	 nexus_mutex_lock(&handler_thread_done_mutex);
	 while( !handler_thread_done )
	 {
	    nexus_cond_wait( &handler_thread_done_cond,
			    &handler_thread_done_mutex );
	 }
	 nexus_mutex_unlock(&handler_thread_done_mutex);	 
      }
   }
   
   /* need to stash this id, once the segment is detached the
    *  shared memory will be unavailable
    */
   
   shmid = shm_message_header->shmid;
   
   /*
    * this has to change, the starting address of the shm segment
    * should be kept track of some other way
    */
   
   if( shmdt( (char*)shm_counter ) )
   {
     nexus_fatal("shm_shutdown(): shmdt failed");
   }
   
   if( shmctl( shmid, IPC_STAT, &sp ) )
   {
     nexus_fatal("shm_shutdown(): shmctl failed");
   }
   else
   {
      if( sp.shm_nattch == 0 )
      {
	 if( shmctl( shmid, IPC_RMID,
		    (struct shmid_ds*)NULL) )
	 {
	    nexus_fatal("shm_shutdown(): IPC_RMID failed");
	 }
      }
      else
      {
	 nexus_printf("shm_shutdown(): shm_nattch [ %d ]\n", sp.shm_nattch);
      }
   }
   
} /* shm_shutdown() */


/*
 * shm_abort()
 *
 * This routine is called during the _abnormal_ shutdown of a process.
 */
static void shm_abort(void)
{
   struct shmid_ds sp;

   if( shm_abort_flag )
     return;

   shm_abort_flag = 1;
   
   if( shmdt( (char*)shm_counter ) )
   {
      nexus_printf("shm_abort(): shmdt failed errno [ %d ]", errno);
   }

   if( shmctl( shm_message_header->shmid, IPC_STAT, &sp ) )
   {
      nexus_printf("shm_abort(): shmctl failed errno [ %d ]", errno);
   }
   else
   {
      if( sp.shm_nattch == 0 )
      {
	 nexus_printf("shm_abort(): shm_nattch is 0");
	 if( shmctl( shm_message_header->shmid, IPC_RMID,
		    (struct shmid_ds*)NULL) )
	 {
	    nexus_printf("shm_abort(): IPC_RMID failed errno [ %d ]", errno);
	 }
      }
      else
	 nexus_printf("shm_abort(): shm_nattch [ %d ]\n", sp.shm_nattch);
   }
   
} /* shm_abort() */


/*
 * shm_dump_buffer()
 */
static void shm_dump_buffer(shm_buffer_t *buf)
{
    char *ptr;
    char *outputbuf;
    char temp2buf[30];
    int curr_printed=0;
    unsigned long l;

    NexusMalloc(shm_dump_buffer(), outputbuf, char *,
		(3 * buf->size + 20));


    outputbuf[0]='\0';
    ptr = buf->base_pointer;

    do
    {
	memcpy(&l, ptr, sizeof(unsigned long));
	if (   (ptr+sizeof(unsigned long) >= buf->current_pointer)
	    && (curr_printed==0) )
	{
	    sprintf(temp2buf, "%8.8lx <<%d<< ", l, buf->current_pointer - ptr);
	    strcat(outputbuf, temp2buf);
	    curr_printed=1;
	}
	else
	{
	    sprintf(temp2buf, "%8.8lx ", l);
	    strcat(outputbuf, temp2buf);
	}
	ptr += sizeof(unsigned long);
    } while(ptr < ((buf->base_pointer) + (buf->size)));

    nexus_printf("shm_dump_buffer(): base_pointer:%x current_pointer:%x size:%d data:%s\n", 
		 buf->base_pointer, buf->current_pointer, 
		 buf->size, outputbuf);
} /* shm_dump_buffer() */


/*
 * shm_init_remote_service_request()
 *
 * Initiate a shared memory remote service request (i.e. to the same context
 * that is calling this).
 *
 * Note: The things pointed to by 'gp' and 'handler_name' should
 * NOT change between this init_rsr call and the subsequent send_rsr.
 * Only the pointers are stashed away, and not the full data.
 *
 * Return: Fill in 'buffer' with a nexus_buffer_t.
 */
static void shm_init_remote_service_request(nexus_buffer_t *buffer,
					      nexus_global_pointer_t *gp,
					      char *handler_name,
					      int handler_id)
{
    shm_buffer_t *shm_buffer;
    int handler_name_length;
    shm_buffer_header_t* shm_msg_hdr;
    shm_proto_t* proto;
    unsigned char liba_flag;
    unsigned long endpoint_id;
    nexus_gp_endpoint_t *endpoint;

    shm_msg_hdr = shm_message_header;

    nexus_debug_printf(4,("shm_init_remote_service_request(): inside\n"));
    

    NexusAssert2((gp->mi_proto->proto->type == NEXUS_PROTO_TYPE_SHM),
		 ("shm_init_remote_service_request(): Internal error: proto_type is not NEXUS_PROTO_TYPE_SHM\n"));

    NEXUS_INTERROGATE(gp, _NX_GLOBAL_POINTER_T, "shm_init_remote_service_request");

    proto = (shm_proto_t*) gp->mi_proto->proto;

    nexus_debug_printf(4,("shm_init_remote_service_request(): context_id [ %d ] node_id [ %d ] pid [ %d ]\n",
		 proto->context_id, proto->node_id, proto->pid));

    while(( shm_msg_hdr != NULL ) &&
	  (shm_msg_hdr->context_id != proto->context_id) ||
	  (shm_msg_hdr->node_id != proto->node_id ) ||
	  (shm_msg_hdr->pid != proto->pid) )
    {
       shm_msg_hdr = shm_msg_hdr->next;
    }
    NexusAssert( shm_msg_hdr != (shm_buffer_header_t*) NULL );

    nexus_debug_printf(4,("shm_init_remote_service_request(): found the header node[ %d ] context[ %d ] pid[ %d ]\n",
		 shm_msg_hdr->node_id, shm_msg_hdr->context_id,
		 shm_msg_hdr->pid));

    GetShmBuffer(shm_init_remote_service_request(), shm_msg_hdr ,shm_buffer);

    shm_buffer->funcs = &shm_buffer_funcs;
    shm_buffer->next = (shm_buffer_t *) NULL;
    /* UnpackLibaFromGP(gp, shm_buffer->context, shm_buffer->address); */
    UnpackLibaFromStartpoint(gp, liba_flag, endpoint_id, shm_buffer->address);
    if (endpoint = ENDPOINT_FROM_ENDPOINT_ID(liba_flag, 
			endpoint_id, NEXUS_FALSE))
	shm_buffer->context = endpoint->context;
    else
    {
nexus_fatal("shm_init_remote_service_request(): extracted NULL endpoint from LIBA\n");
    } /* endif */
    nexus_debug_printf(4,("shm_init_remote_service_request(): context [ 0x%x ] address [0x%x ]\n",
		 shm_buffer->context, shm_buffer->address));
#ifdef NEXUS_PROFILE
    /* Set node_id and context_id to the _destination_ node and context */
    shm_buffer->context_id = gp->context_id;
#endif
    shm_buffer->size = 0;
    shm_buffer->n_elements = -1;
    shm_buffer->stashed = NEXUS_FALSE;
    shm_buffer->base_pointer = (char *) NULL;
    shm_buffer->current_pointer = (char *) NULL;
    shm_buffer->handler_id = handler_id;
    
    handler_name_length = strlen(handler_name);
    NexusAssert2((handler_name_length < NEXUS_MAX_HANDLER_NAME_LENGTH),
		 ("shm_init_remote_service_request(): Handler name exceeds maximum length (%d): %s\n", NEXUS_MAX_HANDLER_NAME_LENGTH, handler_name));
    strcpy(shm_buffer->handler_name,handler_name);

    nexus_debug_printf(2, ("shm_init_remote_service_request(): to:(%lu,%lu) %s-%d\n", shm_buffer->context, shm_buffer->address, handler_name, handler_id));

    *buffer = (nexus_buffer_t) shm_buffer;
    nexus_debug_printf(4,("shm_init_remote_service_request(): leaving\n"));
} /* shm_init_remote_service_request() */


/*
 * shm_send_remote_service_request()
 *
 * Generate a shm remote service request for 'nexus_buffer'.
 */
static int shm_send_remote_service_request(nexus_buffer_t *nexus_buffer)
{
    shm_buffer_t *buf;

    nexus_debug_printf(4,("shm_send_remote_service_request(): inside\n"));
    NexusBufferMagicCheck(shm_send_remote_service_request, nexus_buffer);

    buf = (shm_buffer_t *) *nexus_buffer;

#ifdef NEXUS_PROFILE
    buf->message_length = buf->current_pointer - buf->base_pointer;
    _nx_pablo_log_remote_service_request_send(buf->node_id,
					      buf->context_id,
					      buf->handler_name,
					      buf->handler_id,
					      buf->message_length);
    /* Reset node_id and context_id to the _source_ node and context */
    _nx_node_id(&(buf->node_id));
    _nx_context_id(&(buf->context_id));
#endif /* NEXUS_PROFILE */
    
    /* Reset the current_pointer for the future get operations */
    buf->current_pointer = buf->base_pointer;
#ifdef BUILD_DEBUG
    if (NexusDebug(2))
    {
	/*
	int i;
	tmpbuf1[0] = '\0';
	for (i = 0; i < buf->size; i++) {
	    sprintf(tmpbuf2, "%u ", (unsigned int)((buf->base_pointer)[i]));
	    strcat(tmpbuf1, tmpbuf2);
	}
	nexus_printf("shm_send_remote_service_request(): Beginning of message: %s\n",tmpbuf1);
	*/
	nexus_debug_printf(4,
			   ("shm_send_remote_service_request(): sending buffer:%x\n",
			    buf->base_pointer));
	if (buf && buf->base_pointer)
	{
	    shm_dump_buffer(buf);
	}
    }
#endif

    EnqueueMessage(buf);

    if (!handle_in_progress)
    {
	/*
	 * Handle the message, if this isn't already being called
	 * from a handler
	 */
	handle_enqueued_messages();
    }


    nexus_poll();

    nexus_debug_printf(4,("shm_send_remote_service_request(): leaving\n"));
    
    return(0);
    
} /* shm_send_remote_service_request() */


/*
 * shm_send_urgent_remote_service_request()
 */
static int shm_send_urgent_remote_service_request(nexus_buffer_t *buffer)
{
    return(shm_send_remote_service_request(buffer));
} /* shm_send_urgent_remote_service_request() */


/*
 * handle_enqueued_messages()
 *
 * Handle all messages that are enqueued.
 */
static void handle_enqueued_messages(void)
{
    shm_buffer_t *buf;
    nexus_handler_func_t handler_func;
    nexus_handler_type_t handler_type;

    handle_in_progress = NEXUS_TRUE;
    while (MessagesEnqueued())
    {
	DequeueMessage(&buf);
	if( buf == NULL )
	{
	   nexus_printf("handle_enqueued_messages(): returning got NULL\n");
	   return;
	}

#ifdef BUILD_DEBUG
    if (NexusDebug(2))
    {
	nexus_printf("DequeueMessage(): received buffer:%x\n",buf->base_pointer);
	if (buf && buf->base_pointer)
	{
	    shm_dump_buffer(buf);
	}
    }
#endif


	
	nexus_debug_printf(3, ("shm_handle_enqueued_messages(): calling %s with data base_pointer:%x and current_pointer:%x\n", buf->handler_name, buf->base_pointer, buf->current_pointer));

	_nx_lookup_handler(buf->context,
			   buf->handler_name,
			   buf->handler_id,
			   &handler_type,
			   &handler_func);
	if (handler_type == NEXUS_HANDLER_TYPE_THREADED)
	{
	    buf->stashed = NEXUS_TRUE;
	}
	shm_exit();
	_nx_handle_message(buf->handler_name,
			   buf->handler_id,
			   handler_type,
			   handler_func,
			   buf->context,
			   buf->address,
#ifdef NEXUS_PROFILE
			   buf->node_id,
			   buf->context_id,
			   buf->message_length,
#endif
			   (void *) buf);
	shm_enter();
    }
    handle_in_progress = NEXUS_FALSE;
/*    nexus_printf("handle_enqueued_messages(): leaving\n");    */
    
} /* handle_enqueued_messages() */


/*
 * shm_construct_from_mi_proto()
 *
 * Return my proto.  This is called only during initialization by
 * pr_iface.c:_nx_proto_init(), so that it can cache this proto.
 */
static nexus_bool_t shm_construct_from_mi_proto(nexus_proto_t **proto,
						  nexus_mi_proto_t *mi_proto,
						  nexus_byte_t *array,
						  int size)
{
    shm_proto_t *shm_proto;
    int i;
    int tmp_int;
    int context_id;
    int node_id;
    int pid;

    
    /* get the queue id */
    i = 0;
    UnpackInt4(array, i, tmp_int);
    context_id = tmp_int;
    UnpackInt4(array, i, tmp_int);
    node_id = tmp_int;
    UnpackInt4(array, i, tmp_int);
    pid = tmp_int;


    shm_enter();
    *proto = (nexus_proto_t *) construct_proto( context_id, node_id, pid );
    shm_exit();
    
    return (NEXUS_TRUE);
} /* shm_construct_from_mi_proto() */


static shm_proto_t *construct_proto( int context_id, int node_id, int pid )
{
   shm_proto_t *shm_proto;

    NexusMalloc(shm_construct_proto(),
		shm_proto,
		shm_proto_t *,
		sizeof(shm_proto_t) );
    shm_proto->type = NEXUS_PROTO_TYPE_SHM;
    shm_proto->funcs = &shm_proto_funcs;

   shm_proto->context_id = context_id;
   shm_proto->node_id = node_id;
   shm_proto->pid = pid;
    
    return shm_proto;

 } /* construct proto */
   

/*********************************************************************
 * 		Buffer management code
 *********************************************************************/

#ifdef NEXUS_SANITY_CHECK
#define SANITY_TYPE_CHECK_SIZE (sizeof(unsigned long) + sizeof(int))
#else  /* NEXUS_SANITY_CHECK */
#define SANITY_TYPE_CHECK_SIZE 0
#endif /* NEXUS_SANITY_CHECK */

/*
 * shm_set_buffer_size()
 * 
 * Allocate message buffer space for 'size' bytes, that will be used to
 * hold 'n_elements'.
 */
static void shm_set_buffer_size(nexus_buffer_t *buffer,
				  int size, int n_elements)
{
    shm_buffer_t *shm_buffer;
    char *storage;

    NexusBufferMagicCheck(shm_check_buffer_size, buffer);
    
#ifdef NEXUS_SANITY_CHECK
    if (n_elements != -1)
    {
	size += SANITY_TYPE_CHECK_SIZE * n_elements;
    }
#endif

    if (size > 0)
    {
	shm_buffer = (shm_buffer_t *) *buffer;
    
	/*NexusMalloc(shm_set_buffer_size(), storage, char *, size);*/
	lock_header( shm_message_header->header_mutex );	
	storage = (char*)mn_malloc( shm_message_header->shm_top, size );
	unlock_header( shm_message_header->header_mutex );	
        if( storage == NULL )
	   nexus_fatal("mn_malloc failed");
	shm_buffer->size = size;
	shm_buffer->n_elements = n_elements;
	shm_buffer->base_pointer = storage;
	shm_buffer->current_pointer = shm_buffer->base_pointer;
    }

} /* shm_set_buffer_size() */


/*
 * shm_check_buffer_size()
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
static int shm_check_buffer_size(nexus_buffer_t *buffer,
				   int slack, int increment)
{
    shm_buffer_t *shm_buffer;
    int used;
    int needed;

    NexusBufferMagicCheck(shm_check_buffer_size, buffer);

    shm_buffer = (shm_buffer_t *) *buffer;
    
    used = (shm_buffer->current_pointer - shm_buffer->base_pointer);
    needed = used + slack;

    if (shm_buffer->size == 0)
    {
	shm_set_buffer_size(buffer, slack, -1);
    }
    else if (needed > shm_buffer->size)
    {
	char *new_storage;
	int new_size;

	if (increment <= 0)
	    return(NEXUS_FALSE);

	new_size = shm_buffer->size;
	while (new_size < needed)
	{
	    new_size += increment;
	}

/*	NexusMalloc(shm_check_buffer_size(), new_storage, char *, new_size);*/
	lock_header( shm_message_header->header_mutex );	
	new_storage = (char*)mn_malloc( shm_message_header->shm_top, new_size);
	unlock_header( shm_message_header->header_mutex );	
        if( new_storage == NULL )
	   nexus_fatal("mn_malloc failed");
	
	/*memcpy(new_storage, shm_buffer->base_pointer, used);*/
	bcopy(shm_buffer->base_pointer, new_storage, used);
	    
/*	NexusFree(shm_buffer->base_pointer);*/
	lock_header( shm_message_header->header_mutex );	
	mn_free( shm_message_header->shm_top, shm_buffer->base_pointer );
	unlock_header( shm_message_header->header_mutex );	

	shm_buffer->size = new_size;
	shm_buffer->base_pointer = new_storage;
	shm_buffer->current_pointer = (shm_buffer->base_pointer + used);
    }
    
    return(NEXUS_TRUE);
} /* shm_check_buffer_size() */


/*
 * shm_free_buffer()
 *
 * Free the passed nexus_buffer_t.
 *
 * This should be called on the receiving end, after the handler
 * has completed.
 *
 * Note: The stashed flag could be set, since shm_stash_buffer()
 * just sets this flag and typecasts a buffer to a stashed buffer.
 * In this case, do not free the buffer.
 */
static void shm_free_buffer(nexus_buffer_t *buffer)
{
    shm_buffer_t *shm_buffer;
    shm_buffer_header_t *shm_msg_hdr;

    NexusAssert2((buffer),
		 ("shm_free_buffer(): Passed a NULL nexus_buffer_t *\n") );

    /* If the buffer was stashed, *buffer will have been set to NULL */
    if (!(*buffer))
	return;

    NexusBufferMagicCheck(shm_free_buffer, buffer);

    shm_buffer = (shm_buffer_t *) *buffer;

    NexusAssert2((!shm_buffer->stashed),
		 ("shm_free_buffer(): Expected a non-stashed buffer\n"));

    FreeShmBuffer(shm_buffer);

    *buffer = (nexus_buffer_t) NULL;
    
} /* shm_free_buffer() */


/*
 * shm_stash_buffer()
 *
 * Convert 'buffer' to a stashed buffer.
 */
static void shm_stash_buffer(nexus_buffer_t *buffer,
			       nexus_stashed_buffer_t *stashed_buffer)
{
    shm_buffer_t *shm_buffer;
    
    NexusBufferMagicCheck(shm_stash_buffer, buffer);

    shm_buffer = (shm_buffer_t *) *buffer;
    NexusAssert2((!shm_buffer->stashed),
		 ("shm_stash_buffer(): Expected an un-stashed buffer\n"));
    
    shm_buffer->stashed = NEXUS_TRUE;
		  
    *stashed_buffer = (nexus_stashed_buffer_t) *buffer;

    *buffer = (nexus_buffer_t) NULL;
} /* shm_stash_buffer() */


/*
 * shm_free_stashed_buffer()
 *
 * Free the passed nexus_stashed_buffer_t that was stashed
 * by shm_stash_buffer().
 */
static void shm_free_stashed_buffer(nexus_stashed_buffer_t *stashed_buffer)
{
    shm_buffer_t *shm_buffer;
    
    NexusAssert2((stashed_buffer),
		 ("shm_free_stashed_buffer(): Passed a NULL nexus_stashed_buffer_t *\n") );
    NexusBufferMagicCheck(shm_free_stashed_buffer,
			  (nexus_buffer_t *) stashed_buffer);
    
    shm_buffer = (shm_buffer_t *) *stashed_buffer;
    
    NexusAssert2((shm_buffer->stashed),
		 ("shm_free_stashed_buffer(): Expected a stashed buffer\n"));

    FreeShmBuffer(shm_buffer);

    *stashed_buffer = (nexus_stashed_buffer_t) NULL;
    
} /* shm_free_stashed_buffer() */




/*
 * shm_sizeof_*()
 *
 * Return the size (in bytes) that 'count' elements of the given
 * type will require to be put into the 'buffer'.
 */
static int shm_sizeof_float(nexus_buffer_t *buffer, int count)
{
    return (sizeof(float) * count + SANITY_TYPE_CHECK_SIZE);
}

static int shm_sizeof_double(nexus_buffer_t *buffer, int count)
{
    return (sizeof(double) * count + SANITY_TYPE_CHECK_SIZE);
}

static int shm_sizeof_short(nexus_buffer_t *buffer, int count)
{
    return (sizeof(short) * count + SANITY_TYPE_CHECK_SIZE);
}

static int shm_sizeof_u_short(nexus_buffer_t *buffer, int count)
{
    return (sizeof(u_short) * count + SANITY_TYPE_CHECK_SIZE);
}

static int shm_sizeof_int(nexus_buffer_t *buffer, int count)
{
    return (sizeof(int) * count + SANITY_TYPE_CHECK_SIZE);
}

static int shm_sizeof_u_int(nexus_buffer_t *buffer, int count)
{
    return (sizeof(u_int) * count + SANITY_TYPE_CHECK_SIZE);
}

static int shm_sizeof_long(nexus_buffer_t *buffer, int count)
{
    return (sizeof(long) * count + SANITY_TYPE_CHECK_SIZE);
}

static int shm_sizeof_u_long(nexus_buffer_t *buffer, int count)
{
    return (sizeof(u_long) * count + SANITY_TYPE_CHECK_SIZE);
}

static int shm_sizeof_char(nexus_buffer_t *buffer, int count)
{
    return (sizeof(char) * count + SANITY_TYPE_CHECK_SIZE);
}

static int shm_sizeof_u_char(nexus_buffer_t *buffer, int count)
{
    return (sizeof(u_char) * count + SANITY_TYPE_CHECK_SIZE);
}

static int shm_sizeof_byte(nexus_buffer_t *buffer, int count)
{
    return (sizeof(u_char) * count + SANITY_TYPE_CHECK_SIZE);
}


#ifdef NEXUS_ARCH_AIX
#define DO_TRACEBACK _nx_traceback()
#else
#define DO_TRACEBACK
#endif


/*
 * shm_put_*()
 *
 * Put 'count' elements, starting at 'data', of the given type,
 * into 'buffer'.
 */

#ifdef BUILD_DEBUG
#define DO_PUT_ASSERTIONS(TYPE) \
    NexusBufferMagicCheck(shm_put_ ## TYPE, buf); \
    if (((shm_buf->current_pointer - shm_buf->base_pointer) + count * sizeof(TYPE) + SANITY_TYPE_CHECK_SIZE) > shm_buf->size) \
    { \
        DO_TRACEBACK; \
	nexus_fatal("nexus_put_" #TYPE ": Buffer overrun\n"); \
    } \
    if(NexusDebug(3)) \
    { \
        nexus_printf("shm_put_" #TYPE "(): base:%x size:%lu current:%x limit:%x count:%d\n", \
		     shm_buf->base_pointer, \
		     shm_buf->size, \
		     shm_buf->current_pointer, \
		     shm_buf->base_pointer + shm_buf->size, \
		     count); \
    }

#else /* BUILD_DEBUG */
#define DO_PUT_ASSERTIONS(TYPE)
#endif /* BUILD_DEBUG */

#ifdef NEXUS_SANITY_CHECK
#define PUT(TYPE) \
    shm_buffer_t *shm_buf = (shm_buffer_t *) *buf; \
    DO_PUT_ASSERTIONS(TYPE); \
    { \
    unsigned long sanity_flag_long   = 0x10ca100a;  \
    unsigned long sanity_flag_int    = 0x10ca100b;  \
    unsigned long sanity_flag_char   = 0x10ca100c;  \
    unsigned long sanity_flag_double = 0x10ca100d;  \
    unsigned long sanity_flag_short  = 0x10ca100e;  \
    unsigned long sanity_flag_float  = 0x10ca100f;  \
                                                    \
    unsigned long sanity_flag_u_long  = 0x10ca101a; \
    unsigned long sanity_flag_u_int   = 0x10ca101b; \
    unsigned long sanity_flag_u_char  = 0x10ca101c; \
    unsigned long sanity_flag_byte    = 0x10ca101d; \
                                                    \
    unsigned long sanity_flag_u_short = 0x10ca101e; \
                                                    \
    bcopy( & sanity_flag_ ## TYPE, shm_buf->current_pointer, sizeof(unsigned long)); \
    shm_buf->current_pointer += sizeof(unsigned long);                               \
    bcopy(& count, shm_buf->current_pointer, sizeof(int));                          \
    shm_buf->current_pointer += sizeof(int);                                         \
    bcopy(data, shm_buf->current_pointer, count * sizeof(TYPE));                    \
    shm_buf->current_pointer += (count * sizeof(TYPE));                              \
    }
#else  /* NEXUS_SANITY_CHECK */
#define PUT(TYPE) \
    shm_buffer_t *shm_buf = (shm_buffer_t *) *buf; \
    DO_PUT_ASSERTIONS(TYPE); \
    bcopy(data, shm_buf->current_pointer, count * sizeof(TYPE)); \
    shm_buf->current_pointer += count * sizeof(TYPE);
#endif /* NEXUS_SANITY_CHECK */


static void shm_put_float(nexus_buffer_t *buf, float *data, int count)
{
    PUT(float);
}

static void shm_put_double(nexus_buffer_t *buf, double *data, int count)
{
    PUT(double);
}

static void shm_put_short(nexus_buffer_t *buf, short *data, int count)
{
    PUT(short);
}

static void shm_put_u_short(nexus_buffer_t *buf, unsigned short *data,
			    int count)
{
    PUT(u_short);
}

static void shm_put_int(nexus_buffer_t *buf, int *data, int count)
{
    PUT(int);
}

static void shm_put_u_int(nexus_buffer_t *buf, unsigned int *data, int count)
{
    PUT(u_int);
}

static void shm_put_long(nexus_buffer_t *buf, long *data, int count)
{
    PUT(long);
}

static void shm_put_u_long(nexus_buffer_t *buf, unsigned long *data,
			     int count)
{
    PUT(u_long);
}

static void shm_put_char(nexus_buffer_t *buf, char *data, int count)
{
    PUT(char);
}

static void shm_put_u_char(nexus_buffer_t *buf, unsigned char *data,
			     int count)
{
    PUT(u_char);
}

static void shm_put_byte(nexus_buffer_t *buf, unsigned char *data,
			   int count)
{
    PUT(u_char);
}



/*
 * shm_get_*()
 *
 * Get 'count' elements of the given type from 'buffer' and store
 * them into 'data'.
 */

#ifdef BUILD_DEBUG
#define DO_GET_ASSERTIONS(TYPE) \
    NexusBufferMagicCheck(shm_get_ ## TYPE, buf); \
    NexusAssert2(!shm_buf->stashed, \
		 ("shm_get_stashed_*(): Expected an un-stashed buffer\n")); \
    if (shm_buf->current_pointer > (shm_buf->base_pointer + shm_buf->size)) \
    { \
        DO_TRACEBACK; \
	nexus_fatal("nexus_get_" #TYPE ": Buffer overrun\n"); \
    } \
    if(NexusDebug(3)) \
    { \
        nexus_printf("shm_get_" #TYPE "(): base:%x size:%lu current:%x limit:%x count:%d\n", \
		     shm_buf->base_pointer, \
		     shm_buf->size, \
		     shm_buf->current_pointer, \
		     shm_buf->base_pointer + shm_buf->size, \
		     count); \
    }

#else /* BUILD_DEBUG */
#define DO_GET_ASSERTIONS(TYPE)
#endif /* BUILD_DEBUG */

#ifdef NEXUS_SANITY_CHECK
#define GET(TYPE) \
    shm_buffer_t *shm_buf = (shm_buffer_t *) *buf; \
    DO_GET_ASSERTIONS(TYPE); \
    { \
        unsigned long sanity_flag_long   = 0x10ca100a;  \
        unsigned long sanity_flag_int    = 0x10ca100b;  \
        unsigned long sanity_flag_char   = 0x10ca100c;  \
        unsigned long sanity_flag_double = 0x10ca100d;  \
        unsigned long sanity_flag_short  = 0x10ca100e;  \
        unsigned long sanity_flag_float  = 0x10ca100f;  \
                                                        \
        unsigned long sanity_flag_u_long  = 0x10ca101a; \
        unsigned long sanity_flag_u_int   = 0x10ca101b; \
        unsigned long sanity_flag_u_char  = 0x10ca101c; \
        unsigned long sanity_flag_byte    = 0x10ca101d; \
                                                        \
        unsigned long sanity_flag_u_short = 0x10ca101e; \
                                                        \
        unsigned long buffer_sanity_value; \
        int buffer_sanity_count; \
        \
        bcopy( shm_buf->current_pointer, &buffer_sanity_value, sizeof(unsigned long)); \
        if(  buffer_sanity_value != sanity_flag_ ## TYPE ) { \
    nexus_printf("shm_get_" #TYPE ": warning: expected type of %x but found %x\n", \
		         sanity_flag_ ## TYPE , \
		         buffer_sanity_value); \
	    shm_dump_buffer( shm_buf ); \
	    DO_TRACEBACK; \
        } \
        shm_buf->current_pointer += sizeof(unsigned long); \
        bcopy( shm_buf->current_pointer, &buffer_sanity_count, sizeof(int)); \
        if( buffer_sanity_count != count ) { \
         nexus_printf("shm_get_" #TYPE ": warning: expected count of %x but found %x\n", \
		         count, \
		         buffer_sanity_count); \
	    shm_dump_buffer( shm_buf ); \
	    DO_TRACEBACK; \
        } \
        shm_buf->current_pointer += sizeof(int); \
    } \
    bcopy(shm_buf->current_pointer, dest, sizeof(TYPE) * count); \
    shm_buf->current_pointer += count * sizeof(TYPE);
#else  /* NEXUS_SANITY_CHECK */
#define GET(TYPE) \
    shm_buffer_t *shm_buf = (shm_buffer_t *) *buf; \
    DO_GET_ASSERTIONS(TYPE); \
    bcopy(shm_buf->current_pointer, dest, sizeof(TYPE) * count); \
    shm_buf->current_pointer += count * sizeof(TYPE);
#endif /* NEXUS_SANITY_CHECK */



static void shm_get_float(nexus_buffer_t *buf, float *dest, int count)
{
    GET(float);
}

static void shm_get_double(nexus_buffer_t *buf, double *dest, int count)
{
    GET(double);
}

static void shm_get_short(nexus_buffer_t *buf, short *dest, int count)
{
    GET(short);
}

static void shm_get_u_short(nexus_buffer_t *buf, unsigned short *dest,
			    int count)
{
    GET(u_short);
}

static void shm_get_int(nexus_buffer_t *buf, int *dest, int count)
{
    GET(int);
}

static void shm_get_u_int(nexus_buffer_t *buf, unsigned int *dest, int count)
{
    GET(u_int);
}

static void shm_get_long(nexus_buffer_t *buf, long *dest, int count)
{
    GET(long);
}

static void shm_get_u_long(nexus_buffer_t *buf, unsigned long *dest,
			     int count)
{
    GET(u_long);
}

static void shm_get_char(nexus_buffer_t *buf, char *dest, int count)
{
    GET(char);
}

static void shm_get_u_char(nexus_buffer_t *buf, unsigned char *dest,
			     int count)
{
    GET(u_char);
}

static void shm_get_byte(nexus_buffer_t *buf, unsigned char *dest,
			   int count)
{
    GET(u_char);
}



/*
 * shm_get_stashed_*()
 *
 * Get 'count' elements of the given type from the stashed 'buffer' and store
 * them into 'data'.
 */

#ifdef BUILD_DEBUG
#define DO_GET_STASHED_ASSERTIONS(TYPE) \
    NexusBufferMagicCheck(shm_get_stashed_ ## TYPE, (nexus_buffer_t *) buf); \
    NexusAssert2(shm_buf->stashed, \
		 ("shm_get_stashed_*(): Expected a stashed buffer\n")); \
    if (shm_buf->current_pointer > (shm_buf->base_pointer + shm_buf->size)) \
    { \
        DO_TRACEBACK; \
	nexus_fatal("nexus_get_stashed_" #TYPE ": Buffer overrun\n"); \
    } \
    if(NexusDebug(3)) \
    { \
        nexus_printf("shm_get_stashed_" #TYPE "(): base:%x size:%lu current:%x limit:%x count:%d\n", \
		     shm_buf->base_pointer, \
		     shm_buf->size, \
		     shm_buf->current_pointer, \
		     shm_buf->base_pointer + shm_buf->size, \
		     count); \
    }

#else /* BUILD_DEBUG */
#define DO_GET_STASHED_ASSERTIONS(TYPE)
#endif /* BUILD_DEBUG */

#ifdef NEXUS_SANITY_CHECK
#define GET_STASHED(TYPE) \
    shm_buffer_t *shm_buf = (shm_buffer_t *) *buf; \
    DO_GET_STASHED_ASSERTIONS(TYPE); \
    { \
        unsigned long sanity_flag_long   = 0x10ca100a;  \
        unsigned long sanity_flag_int    = 0x10ca100b;  \
        unsigned long sanity_flag_char   = 0x10ca100c;  \
        unsigned long sanity_flag_double = 0x10ca100d;  \
        unsigned long sanity_flag_short  = 0x10ca100e;  \
        unsigned long sanity_flag_float  = 0x10ca100f;  \
                                                        \
        unsigned long sanity_flag_u_long  = 0x10ca101a; \
        unsigned long sanity_flag_u_int   = 0x10ca101b; \
        unsigned long sanity_flag_u_char  = 0x10ca101c; \
        unsigned long sanity_flag_byte    = 0x10ca101d; \
                                                        \
        unsigned long sanity_flag_u_short = 0x10ca101e; \
                                                        \
        unsigned long buffer_sanity_value; \
        int buffer_sanity_count; \
        \
        memcpy( &buffer_sanity_value, shm_buf->current_pointer, sizeof(unsigned long)); \
        if(  buffer_sanity_value != sanity_flag_ ## TYPE ) { \
           nexus_printf("shm_get_stashed_" #TYPE ": warning: expected type of %x but found %x\n", \
		         sanity_flag_ ## TYPE , \
		         buffer_sanity_value); \
	    shm_dump_buffer( shm_buf ); \
	    DO_TRACEBACK; \
        } \
        shm_buf->current_pointer += sizeof(unsigned long); \
        memcpy( &buffer_sanity_count, shm_buf->current_pointer, sizeof(int)); \
        if( buffer_sanity_count != count ) { \
         nexus_printf("shm_get_stashed_" #TYPE ": warning: expected count of %x but found %x\n", \
		         count, \
		         buffer_sanity_count); \
	    shm_dump_buffer( shm_buf ); \
	    DO_TRACEBACK; \
        } \
        shm_buf->current_pointer += sizeof(int); \
    } \
    memcpy(dest, shm_buf->current_pointer, sizeof(TYPE) * count); \
    shm_buf->current_pointer += count * sizeof(TYPE);
#else  /* NEXUS_SANITY_CHECK */
#define GET_STASHED(TYPE) \
    shm_buffer_t *shm_buf = (shm_buffer_t *) *buf; \
    DO_GET_STASHED_ASSERTIONS(TYPE); \
    memcpy(dest, shm_buf->current_pointer, sizeof(TYPE) * count); \
    shm_buf->current_pointer += count * sizeof(TYPE);
#endif /* NEXUS_SANITY_CHECK */



static void shm_get_stashed_float(nexus_stashed_buffer_t *buf,
				    float *dest, int count)
{
    GET_STASHED(float);
}

static void shm_get_stashed_double(nexus_stashed_buffer_t *buf,
				     double *dest, int count)
{
    GET_STASHED(double);
}

static void shm_get_stashed_short(nexus_stashed_buffer_t *buf,
				    short *dest, int count)
{
    GET_STASHED(short);
}

static void shm_get_stashed_u_short(nexus_stashed_buffer_t *buf,
				      unsigned short *dest, int count)
{
    GET_STASHED(u_short);
}

static void shm_get_stashed_int(nexus_stashed_buffer_t *buf,
				  int *dest, int count)
{
    GET_STASHED(int);
}

static void shm_get_stashed_u_int(nexus_stashed_buffer_t *buf,
				    unsigned int *dest, int count)
{
    GET_STASHED(u_int);
}

static void shm_get_stashed_long(nexus_stashed_buffer_t *buf,
				   long *dest, int count)
{
    GET_STASHED(long);
}

static void shm_get_stashed_u_long(nexus_stashed_buffer_t *buf,
				     unsigned long *dest, int count)
{
    GET_STASHED(u_long);
}

static void shm_get_stashed_char(nexus_stashed_buffer_t *buf,
				   char *dest, int count)
{
    GET_STASHED(char);
}

static void shm_get_stashed_u_char(nexus_stashed_buffer_t *buf,
				     unsigned char *dest, int count)
{
    GET_STASHED(u_char);
}

static void shm_get_stashed_byte(nexus_stashed_buffer_t *buf,
				   unsigned char *dest, int count)
{
    GET_STASHED(u_char);
}


/*
 * _nx_shm_usage_message()
 */

void _nx_shm_usage_message(void)
{
   printf("    -shmkey                   : key for shmget call,\n");
   printf("    -shmsize                  : size of shm segment to allocate.\n");
} /* _nx_shm_usage_message() */
