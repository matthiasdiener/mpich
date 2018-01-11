/*
 * nl_file.c
 *
 * Node locking, using a /tmp file.
 * This code should keep multiple nodes with the same name from
 * starting on the same machine.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/nl_file.c,v 1.21 1997/02/13 19:43:40 tuecke Exp $";

#include "internal.h"

#include <sys/types.h>
#include <fcntl.h>

#ifdef HAVE_FLOCK
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#endif

#ifdef NEXUS_USE_FCNTL_LOCK
#define NEXUS_HAVE_LOCK_METHOD
#endif

#ifdef HAVE_LOCKF
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_LOCKF_H
#include <sys/lockf.h>
#endif
#endif

#define ConstructFilename() \
{ \
    nexus_stdio_lock();\
    sprintf(nodelock_filename, "/tmp/nx_%s_%s_%d", _nx_master_id_string, \
	    _nx_my_node.name, _nx_my_node.number); \
    nexus_stdio_unlock(); \
}

static char nodelock_filename[1024] = "";
static int  nodelock_fd;

#define LOCATION_STATE	0
#define LOCATION_COUNT	sizeof(int)
#define LOCATION_SP	(2 * sizeof(int))

#define STATE_NORMAL	0
#define STATE_EXITING	1

typedef struct _nodelock_reply_t
{
    nexus_mutex_t	mutex;
    nexus_cond_t	cond;
    nexus_bool_t	done;
    int			value;
    nexus_startpoint_t  startpoint;
} nodelock_reply_t;

typedef struct _nodelock_shutdown_wait_t
{
    nexus_mutex_t	mutex;
    nexus_cond_t	cond;
    int			count;
} nodelock_shutdown_wait_t;

static nodelock_shutdown_wait_t shutdown_wait;
static nexus_endpoint_t nodelock_endpoint;

static void	write_state(int fd, int state);
static int	read_state(int fd);
static void	write_count(int fd, int count);
static int	read_count(int fd);
static void	write_sp(int fd, nexus_startpoint_t *sp);
static void	read_sp(int fd, nexus_startpoint_t *sp);
static void	lock_file(int fd);
static void	unlock_file(int fd);
static void	close_file(int fd);

static void _nx_nodelock_checkin_handler(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);

static void _nx_nodelock_reply_handler(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);

static nexus_handler_t nodelock_handlers[] =
{ \
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) _nx_nodelock_checkin_handler},
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) _nx_nodelock_reply_handler},
};



/*
 * _nx_nodelock_master_init()
 *
 * This is called only by the master node.  It should establish
 * this process for the node identified by _nx_my_node.
 */
void _nx_nodelock_master_init(void)
{
    ConstructFilename();
    nexus_debug_printf(1, ("_nx_nodelock_master_init(): Creating %s\n", nodelock_filename));

 retry:
    if ((nodelock_fd = open(nodelock_filename, O_RDWR|O_CREAT|O_EXCL, 0600)) == -1)
    {
	if (errno == EINTR)
	{
	    goto retry;
	}
	else if (errno == EEXIST)
	{
	    nexus_fatal("_nx_nodelock_master_init(): Master node lock file already exists: %s\n", nodelock_filename);
	}
	else
	{
	    nexus_fatal("_nx_nodelock_master_init(): Failed to open master node lock file %s, errno = %d\n", nodelock_filename, errno);
	}
    }
    else
    {
	/*
	 * The open succeeded.  So write the lock file.
	 * Initialize shutdown_wait.
	 */
	nexus_endpointattr_t nodelock_attr;
	nexus_startpoint_t nodelock_startpoint;
	nexus_endpointattr_init(&nodelock_attr);
	nexus_endpointattr_set_handler_table(&nodelock_attr,
					     nodelock_handlers,
					     2);
	nexus_endpoint_init(&nodelock_endpoint, &nodelock_attr);
	nexus_startpoint_bind(&nodelock_startpoint, &nodelock_endpoint);

	lock_file(nodelock_fd);

	write_state(nodelock_fd, STATE_NORMAL);
	write_count(nodelock_fd, 0);
	write_sp(nodelock_fd, &nodelock_startpoint);
	
	nexus_mutex_init(&(shutdown_wait.mutex), (nexus_mutexattr_t *) NULL);
	nexus_cond_init(&(shutdown_wait.cond), (nexus_condattr_t *) NULL);
	shutdown_wait.count = 0;
	
	unlock_file(nodelock_fd);
    }
    
} /* _nx_nodelock_master_init() */
	

/*
 * _nx_nodelock_check()
 *
 * See if there is already a node process on this machine for
 * my node (_nx_my_node).
 *
 * If so, then checkin with that process, set _nx_my_node.gp
 * to be the global pointer to that process, and return NEXUS_TRUE.
 *
 * Otherwise, do not change _nx_my_node.gp, and return NEXUS_FALSE.
 */
nexus_bool_t _nx_nodelock_check(void)
{
    int state, count;
    nexus_bool_t defering_to_other_process = NEXUS_FALSE;

    ConstructFilename();
    nexus_debug_printf(1, ("_nx_nodelock_check(): Checking %s\n", nodelock_filename));

 retry:
    if ((nodelock_fd = open(nodelock_filename, O_RDWR|O_CREAT, 0600)) == -1)
    {
	if (errno == EINTR)
	{
	    goto retry;
	}
	else
	{
	    nexus_fatal("_nx_nodelock_check(): Failed to open node lock file %d, errno = %d\n", nodelock_filename, errno);
	}
    }
    else
    {
	/*
	 * The open succeeded.
	 */
	int size;
	
	lock_file(nodelock_fd);

	/*
	 * Figure out how big the file is.
	 */
	while ((size = lseek(nodelock_fd, 0, SEEK_END)) == -1)
	{
	    if (errno != EINTR)
	    {
		nexus_fatal("_nx_nodelock_check(): lseek() failed, errno = %d\n", errno);
	    }
	}
	if (size == 0)
	{
	    /*
	     * I am the first process to open this file.
	     * Therefore, I am the process for this node.
	     * So write the lock file, and initialize shutdown_wait.
	     */
	    nexus_endpointattr_t nodelock_attr;
	    nexus_startpoint_t nodelock_startpoint;
	    nexus_endpointattr_init(&nodelock_attr);
	    nexus_endpointattr_set_handler_table(&nodelock_attr,
						 nodelock_handlers,
						 2);
	    nexus_endpoint_init(&nodelock_endpoint, &nodelock_attr);
	    nexus_startpoint_bind(&nodelock_startpoint, &nodelock_endpoint);

	    nexus_debug_printf(1, ("_nx_nodelock_check(): I'm the node\n"));
	    write_state(nodelock_fd, STATE_NORMAL);
	    write_count(nodelock_fd, 0);
	    write_sp(nodelock_fd, &nodelock_startpoint);
	    
	    nexus_mutex_init(&(shutdown_wait.mutex),
			     (nexus_mutexattr_t *) NULL);
	    nexus_cond_init(&(shutdown_wait.cond), (nexus_condattr_t *) NULL);
	    shutdown_wait.count = 0;
	
	    _nx_my_node.return_code = NEXUS_NODE_NEW;
	    defering_to_other_process = NEXUS_FALSE;
	    unlock_file(nodelock_fd);
	}
	else
	{
	    /*
	     * I am not the first process to open this file.
	     * Therefore, some other process is the node process.
	     */
	    state = read_state(nodelock_fd);

	    if (state == STATE_EXITING)
	    {
		/*
		 * I caught the node process trying to exit.
		 * So retry the check again from scratch.
		 */
		nexus_debug_printf(1, ("_nx_nodelock_check(): Found exiting node\n"));
		unlock_file(nodelock_fd);
		close_file(nodelock_fd);
		goto retry;
	    }
	    else /* (state == STATE_NORMAL) */
	    {
		/*
		 * Set the state so that the node process knows
		 * somebody is trying to connect to it.
		 */
		nodelock_reply_t reply;
		nexus_startpoint_t reply_sp;
		nexus_endpoint_t reply_ep;
		nexus_endpointattr_t reply_attr;
		nexus_buffer_t buffer;
		nexus_startpoint_t other_sp;
		int buf_size;

		nexus_debug_printf(1, ("_nx_nodelock_check(): Found normal node\n"));
		
		/* Bump the count of nodes trying to connect */
		count = read_count(nodelock_fd);
		write_count(nodelock_fd, count + 1);
		
		read_sp(nodelock_fd, &other_sp);
		unlock_file(nodelock_fd);

		/*
		 * Contact the node process and wait for a reply.
		 */
		nexus_mutex_init(&(reply.mutex), (nexus_mutexattr_t *) NULL);
		nexus_cond_init(&(reply.cond), (nexus_condattr_t *) NULL);
		reply.done = NEXUS_FALSE;

		nexus_endpointattr_init(&reply_attr);
		nexus_endpointattr_set_handler_table(&reply_attr,
						     nodelock_handlers,
						     2);
		nexus_endpoint_init(&reply_ep, &reply_attr);
		nexus_endpoint_set_user_pointer(&reply_ep, (void *) &reply);
		nexus_startpoint_bind(&reply_sp, &reply_ep);

		buf_size = nexus_sizeof_startpoint(&reply_sp, 1);
		nexus_buffer_init(&buffer, buf_size, 0);
		nexus_put_startpoint_transfer(&buffer, &reply_sp, 1);
		nexus_send_rsr(&buffer, &other_sp, 0, NEXUS_TRUE, NEXUS_FALSE);

		/* Wait for the reply */
		nexus_mutex_lock(&(reply.mutex));
		while(!reply.done)
		{
		    nexus_cond_wait(&(reply.cond), &(reply.mutex));
		}
		nexus_mutex_unlock(&(reply.mutex));
		nexus_mutex_destroy(&(reply.mutex));
		nexus_cond_destroy(&(reply.cond));
		nexus_startpoint_destroy(&other_sp);
		nexus_endpoint_destroy(&reply_ep);
		nexus_endpointattr_destroy(&reply_attr);
		
		nexus_debug_printf(1, ("_nx_nodelock_check(): Got reply.value = %d\n", reply.value));
		
		if (reply.value == 0)
		{
		    /*
		     * Yea!  Everything succeeded.
		     * So set _nx_my_node.gp to that other node's gp,
		     */
		    if (!nexus_startpoint_is_null(&(_nx_my_node.startpoint)))
		    {
			nexus_startpoint_destroy(&(_nx_my_node.startpoint));
		    }
		    _nx_my_node.startpoint = reply.startpoint;
		    _nx_my_node.return_code = reply.value;
		    defering_to_other_process = NEXUS_TRUE;
		    nodelock_filename[0] = '\0';
		}
		else
		{
		    /*
		     * The NexusAcquiredAsNode() function returned
		     * non-zero.  This signals that the node process
		     * no longer wishes to accept nodes checking into
		     * it, because it is calling
		     * nexus_destroy_current_context().
		     * So retry the whole process again.
		     */
		    goto retry;
		}
	    }
	}
    }

    return (defering_to_other_process);
} /* _nx_nodelock_check() */


/*
 * _nx_nodelock_checkin_handler()
 */
static void _nx_nodelock_checkin_handler(nexus_endpoint_t *endpoint,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    nexus_startpoint_t reply_sp;
    nexus_buffer_t reply_buffer;
    nexus_context_t *context;
    nexus_startpoint_t user_sp;
    int reply_value;
    int count;
    int buf_size;
    
    nexus_debug_printf(1, ("_nx_nodelock_checkin_handler(): Entering\n"));
    
    nexus_get_startpoint(buffer, &reply_sp, 1);

    nexus_mutex_lock(&(shutdown_wait.mutex));

    /* Decrement the count in the file */
    lock_file(nodelock_fd);
    count = read_count(nodelock_fd);
    write_count(nodelock_fd, count - 1);
    unlock_file(nodelock_fd);
		
    if (shutdown_wait.count > 0)
    {
	/*
	 * We are exiting.  So signal the main thread if all
	 * outstanding requests have completed.
	 * Return a non-zero value to the requesting node.
	 */
	nexus_debug_printf(1, ("_nx_nodelock_checkin_handler(): Waiting for shutdown\n"));
	if (--shutdown_wait.count <= 0)
	{
	    nexus_cond_signal(&(shutdown_wait.cond));
	}
	reply_value = -1;
    }
    else
    {
	/*
	 * Call NexusAcquiredAsNode()
	 */
	_nx_context(&context);

	reply_value = (NexusAcquiredAsNode)(&user_sp);
    }
    
    nexus_debug_printf(1, ("_nx_nodelock_checkin_handler(): Returning %d\n", reply_value));
    
    /*
     * Return the value.
     */
    buf_size = nexus_sizeof_int(1) + nexus_sizeof_startpoint(&user_sp, 1);
    nexus_buffer_init(&reply_buffer, buf_size, 0);
    nexus_put_int(&reply_buffer, &reply_value, 1);
    nexus_put_startpoint_transfer(&reply_buffer, &user_sp, 1);
    nexus_send_rsr(&reply_buffer,
		   &reply_sp,
		   1,
		   NEXUS_TRUE,
		   called_from_non_threaded_handler);
    
    nexus_mutex_unlock(&(shutdown_wait.mutex));

} /* _nx_nodelock_checkin_handler */


/*
 * _nx_nodelock_reply_handler()
 */
static void _nx_nodelock_reply_handler(nexus_endpoint_t *endpoint,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    nodelock_reply_t *reply;
    
    reply = (nodelock_reply_t *)nexus_endpoint_get_user_pointer(endpoint);
    nexus_mutex_lock(&(reply->mutex));
    reply->done = NEXUS_TRUE;
    nexus_get_int(buffer, &(reply->value), 1);
    nexus_get_startpoint(buffer, &(reply->startpoint), 1);
    nexus_debug_printf(1, ("_nx_nodelock_reply_handler(): Got reply->value=%d\n", reply->value));
    nexus_cond_signal(&(reply->cond));
    nexus_mutex_unlock(&(reply->mutex));
} /* _nx_nodelock_reply_handler */


/*
 * _nx_nodelock_shutdown()
 *
 * Shutdown this process as the node idenified by _nx_my_node.
 * 
 * This means that if any new node processes come along on this
 * machine with the same node name as this process, they will
 * not defer to this process.  
 */
void _nx_nodelock_shutdown(void)
{
    nexus_debug_printf(1, ("_nx_nodelock_shutdown(): File %s\n", nodelock_filename));
    
    /*
     * Set the file state to exiting, get the number of pending checkins,
     * and remove the file.
     */
    nexus_mutex_lock(&(shutdown_wait.mutex));
    lock_file(nodelock_fd);
    write_state(nodelock_fd, STATE_EXITING);
    shutdown_wait.count = read_count(nodelock_fd);
    unlock_file(nodelock_fd);
    close_file(nodelock_fd);
    remove(nodelock_filename);
    nodelock_filename[0] = '\0';

    /*
     * Wait for any remaining processes to checkin before terminating.
     */
    while (shutdown_wait.count > 0)
    {
	nexus_debug_printf(1, ("_nx_nodelock_shutdown(): Waiting for %d requests\n", shutdown_wait.count));
	nexus_cond_wait(&(shutdown_wait.cond), &(shutdown_wait.mutex));
    }
    nexus_mutex_unlock(&(shutdown_wait.mutex));
} /* _nx_nodelock_shutdown() */


/*
 * _nx_nodelock_cleanup()
 *
 * Cleanup any /tmp/nx* files that I created.
 */
void _nx_nodelock_cleanup(void)
{
    if(strlen(nodelock_filename) > (size_t)0)
    {
	remove(nodelock_filename);
	nodelock_filename[0] = '\0';
    }
} /* _nx_nodelock_cleanup() */


/*
 * write_state()
 */
static void write_state(int fd, int state)
{
    while (lseek(fd, LOCATION_STATE, SEEK_SET) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("write_state(): lseek() failed, errno = %d\n", errno);
	}
    }
    while (write(fd, &state, sizeof(int)) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("write_state(): write() failed, errno = %d\n", errno);
	}
    }
} /* write_state() */


/*
 * read_state()
 */
static int read_state(int fd)
{
    int state;
    while (lseek(fd, LOCATION_STATE, SEEK_SET) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("read_state(): lseek() failed, errno = %d\n", errno);
	}
    }
    while (read(fd, &state, sizeof(int)) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("read_state(): read() failed, errno = %d\n", errno);
	}
    }
    return (state);
} /* read_state() */


/*
 * write_count()
 */
static void write_count(int fd, int count)
{
    while (lseek(fd, LOCATION_COUNT, SEEK_SET) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("write_count(): lseek() failed, errno = %d\n", errno);
	}
    }
    while (write(fd, &count, sizeof(int)) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("write_count(): Write failed, errno = %d\n", errno);
	}
    }
} /* write_count() */


/*
 * read_count()
 */
static int read_count(int fd)
{
    int count;
    while (lseek(fd, LOCATION_COUNT, SEEK_SET) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("read_count(): lseek() failed, errno = %d\n", errno);
	}
    }
    while (read(fd, &count, sizeof(int)) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("read_count(): Read failed, errno = %d\n", errno);
	}
    }
    return (count);
} /* read_count() */


/*
 * write_sp()
 */
static void write_sp(int fd, nexus_startpoint_t *sp)
{
    while (lseek(fd, LOCATION_SP, SEEK_SET) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("write_sp(): lseek() failed, errno = %d\n", errno);
	}
    }
    if (_nx_write_startpoint(fd, sp) != 0)
    {
	nexus_fatal("write_sp(): Got unexpected EOF while writing startpoint\n");
    }
} /* write_sp() */


/*
 * read_sp()
 */
static void read_sp(int fd, nexus_startpoint_t *sp)
{
    while (lseek(fd, LOCATION_SP, SEEK_SET) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("read_sp(): lseek() failed, errno = %d\n", errno);
	}
    }
    if (_nx_read_startpoint(fd, sp) != 0)
    {
	nexus_fatal("read_sp(): Got unexpected EOF while reading startpoint\n");
    }
} /* write_sp() */


/*
 * lock_file()
 */
static void lock_file(int fd)
{
#ifdef HAVE_LOCKF
    while (lseek(fd, 0, SEEK_SET) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("lock_file(): lseek() failed, errno = %d\n", errno);
	}
    }
    while (lockf(fd, F_LOCK, 1L) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("lock_file(): lockf() failed, errno = %d\n", errno);
	}
    }
#endif /* HAVE_LOCKF */

#if defined(HAVE_FCNTL) && !defined (HAVE_FLOCK)
    struct flock l;
    l.l_type = F_WRLCK;
    l.l_whence = 0;
    l.l_start = 0;
    l.l_len = 0;
    while (fcntl(fd, F_SETLKW, (int) &l) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("lock_file(): fcntl() failed, errno = %d\n", errno);
	}
    }
#endif /* HAVE_FCNTL */

#ifdef HAVE_FLOCK
    while (flock(fd, LOCK_EX) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("lock_file(): flock() failed, errno = %d\n", errno);
	}
    }
#endif /* HAVE_FLOCK */    
} /* lock_file() */


/*
 * unlock_file()
 */
static void unlock_file(int fd)
{
#ifdef HAVE_LOCKF
    while (lseek(fd, 0, SEEK_SET) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("unlock_file(): lseek() failed, errno = %d\n", errno);
	}
    }
    while (lockf(fd, F_ULOCK, 1L) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("unlock_file(): lockf() failed, errno = %d\n", errno);
	}
    }
#endif /* HAVE_LOCKF */

#if defined(HAVE_FCNTL) && !defined(HAVE_FLOCK)
    struct flock l;
    l.l_type = F_UNLCK;
    l.l_whence = 0;
    l.l_start = 0;
    l.l_len = 0;
    while (fcntl(fd, F_SETLKW, (int) &l) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("lock_file(): fcntl() failed, errno = %d\n", errno);
	}
    }
#endif /* HAVE_FCNTL */

#ifdef HAVE_FLOCK
    while (flock(fd, LOCK_UN) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("unlock_file(): flock() failed, errno = %d\n", errno);
	}
    }
#endif /* HAVE_FLOCK */    
} /* unlock_file() */


/*
 * close_file()
 */
static void close_file(int fd)
{
    while (close(fd) == -1)
    {
	if (errno != EINTR)
	{
	    nexus_fatal("close_file(): close() failed, errno = %d\n", errno);
	}
    }
} /* close_file() */
