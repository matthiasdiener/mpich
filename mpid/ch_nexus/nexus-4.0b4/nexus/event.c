/*
 * event.c
 *
 * File descriptor and timed event driver
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/event.c,v 1.33 1997/03/01 07:07:40 tuecke Exp $";

#include "internal.h"

#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#if defined(HAVE_DCE_CMA_UX_H) && defined(HAVE_THREAD_SAFE_SELECT) && defined(TARGET_ARCH_AIX)
#include <dce/cma_ux.h>
#endif

#ifdef BUILD_LITE
#undef HAVE_THREAD_SAFE_SELECT
#undef HAVE_THREAD_SAFE_READ
#undef HAVE_THREAD_SAFE_WRITE
#undef HAVE_THREAD_SAFE_WRITEV
#endif /* BUILD_LITE */

#undef USE_BLOCKING_SELECT
#undef USE_BLOCKING_READ
#undef USE_BLOCKING_WRITE
#undef USE_BLOCKING_WRITEV
#undef USE_THREAD_SAFE_FD_CALLS

#ifndef NEXUS_FD_SET_CAST
#ifdef NEXUS_ARCH_HPUX
#define NEXUS_FD_SET_CAST (int *)
#else
#define NEXUS_FD_SET_CAST (fd_set *)
#endif
#endif

/*
 * On an old version of HP/UX (7.something, I believe), if
 * you did a non-blocking write of a buffer that was greater
 * than 4096 bytes, the write would return a "Message too long"
 * error, instead of writing what it could and then returning.
 *
 * If you ever hit a situation like this again, set
 * NEXUS_MAX_WRITE_PACKET to the maximum number of bytes that you
 * want passed to write() at a time.  For example:
 *	#define NEXUS_MAX_WRITE_PACKET 4096
 *
 * If NEXUS_MAX_WRITE_PACKET==0, then it will always just write the
 * full buffer, no matter how long it is.
 *
 * This can also be useful if you want to break write operations
 * into blocks for, say, performance reasons.
 */
#ifndef NEXUS_MAX_WRITE_PACKET
#define NEXUS_MAX_WRITE_PACKET 0
#endif

/*
 * *_info_t
 *
 * Callback information structures for select, listener, read, write, writev
 */
typedef struct _select_info_t
{
    int				fd;
    nexus_bool_t		is_listener;
    void	(*read_callback)(void *arg, int fd);
    void *	read_arg;
    void	(*write_callback)(void *arg, int fd);
    void *	write_arg;
    void	(*except_callback)(void *arg, int fd);
    void *	except_arg;
} select_info_t;

typedef struct _listener_info_t
{
    unsigned short		port;
    int				fd;
    struct _listener_info_t *	next;
    struct _listener_info_t *	prev;
    void *			arg;
    void	(*callback)(void *arg, int fd);
} listener_info_t;

typedef struct _read_info_t
{
    char *			buf;
    size_t			max_nbytes;
    size_t			wait_for_nbytes;
    size_t			nbytes_read;
    void *			arg;
    void	(*callback)(void *arg,
			    int fd,
			    char *buf,
			    size_t nbytes_read,
			    char **new_buf,
			    size_t *new_max_nbytes,
			    size_t *new_wait_for_nbytes);
    void	(*error_callback)(void *arg,
				  int fd,
				  char *buf,
				  size_t nbytes,
				  int error);
} read_info_t;

typedef struct _write_info_t
{
    char *			buf;
    size_t			nbytes;
    size_t			nbytes_wrote;
    void *			arg;
    void	(*callback)(void *arg,
			    int fd,
			    char *buf,
			    size_t nbytes);
    void	(*error_callback)(void *arg,
				  int fd,
				  char *buf,
				  size_t nbytes,
				  int error);
} write_info_t;

typedef struct _writev_info_t
{
    struct iovec *	orig_iov;
    size_t		orig_iovcnt;
    struct iovec *	iov;
    size_t		iovcnt;
    size_t		nbytes_wrote;
    void *		arg;
    void	(*callback)(void *arg,
			    int fd,
			    struct iovec *iov,
			    size_t iovcnt,
			    size_t nbytes);
    void	(*error_callback)(void *arg,
				  int fd,
				  struct iovec *iov,
				  size_t iovcnt,
				  size_t nbytes,
				  int error);
} writev_info_t;


/*
 * Stuff for keeping track of file descriptors.
 *   fd_tablesize:	Maximum number of fds that system supports
 *   fd_table:		Array of pointers to select_info_t's. This is used
 *				to map from an fd to the appropriate
 *				proto or incoming data structure.
 *   n_fds_in_use:	Number of entries in fd_table that are in use
 *   *_fds:		The current fd_set that should be used for select().
 *				* = read, write, or except
 *   fd_table_modified:	Set to NEXUS_TRUE whenever the fd table
 *				changes.  This signals the
 *				handler thread which is blocked in
 *				a select that the fd_table is
 *				different than when it was called,
 *				and therefore should not be trusted.
 */
static int		fd_tablesize;
static select_info_t **	fd_table;
static int		n_fds_in_use;
static fd_set		read_fds;
static fd_set		write_fds;
static fd_set		except_fds;
static int		n_fds_set;
static nexus_bool_t	fd_table_modified;

#ifdef HAVE_SYSCONF
#define NUM_FDS sysconf(_SC_OPEN_MAX)
#else
#define NUM_FDS getdtablesize()
#endif


/*
 * Variables global to this module.
 */
static nexus_bool_t		shutdown_called;
static listener_info_t *	listener_head;
static listener_info_t *	listener_tail;

static int pipe_to_self[2]; /* 0 = read, 1 = write */

#ifndef BUILD_LITE
static nexus_bool_t	   handle_events_in_progress;
static nexus_thread_key_t  i_am_handler_thread_key;
#define I_Am_Handler_Thread() \
    (nexus_thread_getspecific(i_am_handler_thread_key) ? 1 : 0)
#endif /* !BUILD_LITE */

static nexus_bool_t defer_events;
static void (*defer_add_func)(int fd);
static void (*defer_remove_func)(int fd);

#ifdef BUILD_LITE
#define event_enter()
#define event_exit()
#else  /* BUILD_LITE */
static nexus_mutex_t	event_mutex;
#define event_enter()	nexus_mutex_lock(&event_mutex);
#define event_exit()	nexus_mutex_unlock(&event_mutex);
#endif /* BUILD_LITE */

/*
 * Function prototypes
 */
static int	fd_close(int fd);
static void	fd_register_for_read(int fd,
				     void (*callback_func)(void *arg,
							   int fd),
				     void *callback_arg);
static void	fd_register_for_write(int fd,
				      void (*callback_func)(void *arg,
							    int fd),
				      void *callback_arg);
static void	fd_register_for_except(int fd,
				       void (*callback_func)(void *arg,
							     int fd),
				       void *callback_arg);

static void	listener_accept(void *arg, int fd);

static void	fd_table_add(int fd);
static void	fd_table_remove(int fd,
				select_info_t *select_info);

/* callbacks */
static void read_callback(void *arg, int fd);
static void write_callback(void *arg, int fd);
static void writev_callback(void *arg, int fd);
static void pipe_to_self_callback(void *arg, int fd);


/*
 * nexus_fd_init()
 *
 *  Init all the global variables and setup mutexes.
 */
int nexus_fd_init(int *argc, char ***argv)
{
    int i;
    
    shutdown_called = NEXUS_FALSE;
    listener_head = listener_tail = NULL;
#ifndef BUILD_LITE
    nexus_mutex_init(&event_mutex, (nexus_mutexattr_t *) NULL);
#endif

    /* Initialize the fd tables and fd_set's */
    fd_tablesize = NUM_FDS;
    NexusMalloc(nexus_fd_init(),
		fd_table,
		select_info_t **,
		(sizeof(select_info_t *) * fd_tablesize));
    for (i = 0; i < fd_tablesize; i++)
    {
	fd_table[i] = (select_info_t *) NULL;
    }
    fd_table_modified = NEXUS_FALSE;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);
    n_fds_set = 0;
    
    /*
     * Create a pipe to myself, so that I can wake up the thread that is
     * blocked on a select()
     */
    if (pipe(pipe_to_self) != 0)
    {
	return(-1);
    }
    fd_table_add(pipe_to_self[0]);
    fd_register_for_read(pipe_to_self[0], pipe_to_self_callback, NULL);

    defer_events = NEXUS_FALSE;
    defer_add_func = defer_remove_func = NULL;

#ifndef BUILD_LITE
    handle_events_in_progress = NEXUS_FALSE;
    nexus_thread_key_create(&i_am_handler_thread_key, NULL);
#endif
    
    return(0);
} /* nexus_fd_init() */


/*
 * nexus_fd_set_handler_thread()
 */
void nexus_fd_set_handler_thread(nexus_bool_t i_am_handler_thread)
{
#ifndef BUILD_LITE
    nexus_thread_setspecific(i_am_handler_thread_key,
			     (void *) i_am_handler_thread);
#endif
} /* nexus_fd_set_handler_thread() */


/*
 * nexus_fd_tablesize()
 *
 * Return the file descriptor table size
 */
int nexus_fd_tablesize()
{
    return(fd_tablesize);
} /* nexus_fd_tablesize() */


/*
 * nexus_fd_shutdown()
 *
 * Make sure the select() wakes up and set the shutdown_called flag to TRUE
 */
int nexus_fd_shutdown(void)
{
    listener_info_t *cur_listener;
    listener_info_t *next_listener;
    char close_flag;

    nexus_debug_printf(3, ("nexus_fd_shutdown(): entering\n"));
    event_enter();
    shutdown_called = NEXUS_TRUE;

    /*
     * Close the pipe_to_self to wakeup the handler thread from the select().
     */
    close_flag = '\0';
    while (   (write(pipe_to_self[1], &close_flag, 1) == -1)
	   && (errno == EINTR))
	/* do nothing */ ;

    while (   (close(pipe_to_self[1]) == -1)
	   && (errno == EINTR))
	/* do nothing */ ;

    for (cur_listener = listener_head;
	 cur_listener;
	 cur_listener = next_listener)
    {
	next_listener = cur_listener->next;
	nexus_fd_close_listener(cur_listener->port);
    }

    event_exit();
    nexus_debug_printf(3, ("nexus_fd_shutdown(): exiting\n"));
    
    return(0);
} /* nexus_fd_shutdown() */


/*
 * nexus_fd_open()
 *
 * Open the fd in non-blocking mode
 */
int nexus_fd_open(char *path, int flags, int mode, int *fd)
{
    int real_flags;
    nexus_bool_t done = NEXUS_FALSE;
    int rc = 0;
    int save_errno;

    nexus_debug_printf(3, ("nexus_fd_open(): entering\n"));
    event_enter();
	
    real_flags = flags | O_NDELAY | ~O_SYNC;

    while (!done)
    {
        *fd = open(path, real_flags, mode);
	save_errno = errno;
	
        if (*fd < 0)
        {
	    if (save_errno != EINTR)
	    {
	        done = NEXUS_TRUE;
	        rc = -save_errno;
	    }
	}
	else
	{
	    done = NEXUS_TRUE;
	}
    }

    fd_table_add(*fd);
    
    event_exit();
    nexus_debug_printf(3, ("nexus_fd_open(): exiting\n"));
    return(rc);
} /* nexus_fd_open() */


/*
 * fd_setup()
 *
 * Set the passed fd to be non-blocking so that the event driver
 * can use it properly.
 * 
 * TODO: Can we call setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, ...) since we
 * don't know if the fd is a socket or a named pipe?
 *
 * Also, do we need the F_SETFD to set the close-on-exec bit?
 */
int fd_setup(int fd)
{
    int save_errno;
    int flags;
    int rc = 0;

    if ((flags = fcntl(fd, F_GETFL, 0)) < 0)
    {
	save_errno = errno;
	rc = -save_errno;
	goto setup_abort;
    }

    flags |= O_NDELAY;
    
    if (fcntl(fd, F_SETFL, flags) == -1)
    {
	save_errno = errno;
	rc = -save_errno;
	goto setup_abort;
    }

    fd_table_add(fd);

setup_abort:
    return(rc);
} /* fd_setup() */


/*
 * fd_setup_socket()
 *
 * Set the passed fd to be non-blocking so that the event driver
 * can use it properly.  Also set the socket for TCP_NODELAY
 * and SO_KEEPALIVE.
 *
 * Also, do we need the F_SETFD to set the close-on-exec bit?
 */
int fd_setup_socket(int fd)
{
    int one = 1;
    int save_errno;
    int rc = 0;

#ifdef TCP_NODELAY
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &one,
		   sizeof(one)) < 0)
    {
	save_errno = errno;
	rc = -save_errno;
	goto setup_abort;
    }
#endif
    
#ifdef SO_KEEPALIVE
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &one,
		   sizeof(one)) < 0)
    {
	save_errno = errno;
	rc = -save_errno;
	goto setup_abort;
    }
#endif
    
    rc = fd_setup(fd);

setup_abort:
    return(rc);
} /* fd_setup_socket() */


/*
 * nexus_fd_setup()
 */
int nexus_fd_setup(int fd)
{
    int rc;
    nexus_debug_printf(3, ("nexus_fd_setup(): entering\n"));
    event_enter();
    rc = fd_setup(fd);
    event_exit();
    nexus_debug_printf(3, ("nexus_fd_setup(): exiting\n"));
    return(rc);
} /* nexus_fd_setup() */


/*
 * nexus_fd_setup_socket()
 */
int nexus_fd_setup_socket(int fd)
{
    int rc;
    nexus_debug_printf(3, ("nexus_fd_setup_socket(): entering\n"));
    event_enter();
    rc = fd_setup_socket(fd);
    event_exit();
    nexus_debug_printf(3, ("nexus_fd_setup_socket(): exiting\n"));
    return(rc);
} /* nexus_fd_setup() */


/*
 * fd_close()
 *
 * Close the passed fd.
 *
 * This assumes event_enter() has already been called (if necessary).
 */
static int fd_close(int fd)
{
    nexus_bool_t done = NEXUS_FALSE;
    int save_errno;
    int rc = 0;

    while (!done)
    {
        if (close(fd))
        {
	    save_errno = errno;
	    if (save_errno != EINTR)
	    {
		done = NEXUS_TRUE;
	        rc = -save_errno;
	    }
        }
	else
	{
	    done = NEXUS_TRUE;
	}
    }

    return(rc);
} /* fd_close() */


/*
 * nexus_fd_close()
 *
 * Close the passed fd.
 */
int nexus_fd_close(int fd)
{
    int rc;

    nexus_debug_printf(3, ("nexus_fd_close(): entering\n"));
#ifndef USE_THREAD_SAFE_FD_CALLS
    event_enter();
#endif

    rc = fd_close(fd);

#ifndef USE_THREAD_SAFE_FD_CALLS
    event_exit();
#endif
    nexus_debug_printf(3, ("nexus_fd_close(): exiting\n"));
    return(rc);
} /* nexus_fd_close() */


/*
 * fd_register_for_read()
 */
static void fd_register_for_read(int fd,
				 void (*callback_func)(void *arg,
						       int fd),
				 void *callback_arg)
{
    select_info_t *select_info;

    fd_table_add(fd);
    select_info = fd_table[fd];

    if (select_info->read_callback)
    {
	FD_CLR(fd, &read_fds);
	n_fds_set--;
	select_info->read_callback = NULL;
	select_info->read_arg = NULL;
	fd_table_modified = NEXUS_TRUE;
    }
    
    if (callback_func)
    {
	FD_SET(fd, &read_fds);
	n_fds_set++;
	select_info->read_callback = callback_func;
	select_info->read_arg = callback_arg;
	fd_table_modified = NEXUS_TRUE;
    }

} /* fd_register_for_read() */


/*
 * fd_register_for_write()
 */
static void fd_register_for_write(int fd,
				  void (*callback_func)(void *arg,
							int fd),
				  void *callback_arg)
{
    select_info_t *select_info;

    fd_table_add(fd);
    select_info = fd_table[fd];

    if (select_info->write_callback)
    {
	FD_CLR(fd, &write_fds);
	n_fds_set--;
	select_info->write_callback = NULL;
	select_info->write_arg = NULL;
	fd_table_modified = NEXUS_TRUE;
    }
    
    if (callback_func)
    {
	FD_SET(fd, &write_fds);
	n_fds_set++;
	select_info->write_callback = callback_func;
	select_info->write_arg = callback_arg;
	fd_table_modified = NEXUS_TRUE;
    }

} /* fd_register_for_write() */


/*
 * fd_register_for_except()
 */
static void fd_register_for_except(int fd,
				   void (*callback_func)(void *arg,
							 int fd),
				   void *callback_arg)
{
    select_info_t *select_info;

    select_info = fd_table[fd];

    if (select_info->except_callback)
    {
	FD_CLR(fd, &except_fds);
	n_fds_set--;
	select_info->except_callback = NULL;
	select_info->except_arg = NULL;
	fd_table_modified = NEXUS_TRUE;
    }
    
    if (callback_func)
    {
	FD_SET(fd, &except_fds);
	n_fds_set++;
	select_info->except_callback = callback_func;
	select_info->except_arg = callback_arg;
	fd_table_modified = NEXUS_TRUE;
    }
} /* fd_register_for_except() */


/*
 * nexus_fd_register_for_select()
 *
 * Register the passed funcs to be called when the fd is able to
 * to read/write on it.
 */
int nexus_fd_register_for_select(int fd,
			         void (*read_callback_func)(void *arg,
							    int fd),
			         void (*write_callback_func)(void *arg,
							     int fd),
			         void (*except_callback_func)(void *arg,
				 			      int fd),
			         void *callback_arg)
{
    nexus_debug_printf(3, ("nexus_fd_register_for_select(): entering\n"));
    event_enter();

    fd_table_add(fd);

    fd_register_for_read(fd, read_callback_func, callback_arg);
    fd_register_for_write(fd, read_callback_func, callback_arg);
    fd_register_for_except(fd, read_callback_func, callback_arg);
			 
    event_exit();
    nexus_debug_printf(3, ("nexus_fd_register_for_select(): exiting\n"));
    return(0);
} /* nexus_fd_register_for_select() */


/*
 * nexus_fd_register_for_read()
 *
 * read on fd, filling buf with nbytes of data, calling the proper callback
 * when done
 *
 * This function can do one of two things:
 *   1. Call select() until this fd is used and then call the appropriate
 *        callback, or
 *   2. Create a new thread that does a blocking read on the fd.
 */
int nexus_fd_register_for_read(int fd,
			       char *buf,
			       size_t max_nbytes,
			       size_t wait_for_nbytes,
			       void (*callback_func)(
				                  void *arg,
						  int fd,
						  char *buf,
						  size_t nbytes,
						  char **new_buf,
						  size_t *new_max_nbytes,
						  size_t *new_wait_for_nbytes),
			       void (*error_callback_func)(void *arg,
							   int fd,
							   char *buf,
							   size_t nbytes,
							   int error),
			       void *callback_arg)
{
    read_info_t *read_info;

    nexus_debug_printf(3, ("nexus_fd_register_for_read(): entering\n"));

    NexusMalloc(nexus_fd_register_for_read(),
		read_info,
		read_info_t *,
		sizeof(read_info_t));
    
    read_info->buf = buf;
    read_info->max_nbytes = max_nbytes;
    read_info->wait_for_nbytes = wait_for_nbytes;
    read_info->nbytes_read = 0;
    read_info->arg = callback_arg;
    read_info->callback = callback_func;
    read_info->error_callback = error_callback_func;

    event_enter();
    fd_table_add(fd);
    fd_register_for_read(fd,
			 read_callback,
			 (void *) read_info);
    event_exit();

    nexus_debug_printf(3, ("nexus_fd_register_for_read(): exiting\n"));
    return(0);
} /* nexus_fd_register_for_read() */


/*
 * read_callback()
 *
 * Callback to do work for nexus_fd_register_for_read()
 */
static void read_callback(void *arg, int fd)
{
    read_info_t *read_info = (read_info_t *) arg;
    long n_read;
    int save_errno;
    nexus_byte_t done;

    nexus_debug_printf(5,("read_callback(): entering\n"));
    for (done = NEXUS_FALSE; !done; )
    {
#ifndef USE_THREAD_SAFE_FD_CALLS
	event_enter();
#endif

	nexus_debug_printf(5,("read_callback(): calling read, fd=%i, buf=%lx, size=%i\n", fd, (read_info->buf + read_info->nbytes_read), (read_info->max_nbytes - read_info->nbytes_read)));
	n_read = read(fd,
		      (read_info->buf + read_info->nbytes_read),
		      (read_info->max_nbytes - read_info->nbytes_read));
	save_errno = errno;
	nexus_debug_printf(5,("read_callback(): read returned n_read=%li\n", n_read));

#ifdef BUILD_DEBUG
	if (NexusDebug(7))
	{
	    int i;
	    unsigned char *start;
	    
	    start = (unsigned char *)(read_info->buf
	    			      + read_info->nbytes_read);

	    nexus_printf("Received %d bytes of data on fd %d\n",
	    		 n_read,
			 fd);

	    nexus_printf("First %d bytes:  ", NEXUS_MIN(n_read, 100));
	    nexus_stdio_lock();
	    for (i = 0; i < NEXUS_MIN(n_read, 100); i++)
	    {
		printf("%2x  ", *(start + i));
	    }
	    printf("\n"); fflush(stdout);
	    nexus_stdio_unlock();
	}
#endif

#ifndef USE_THREAD_SAFE_FD_CALLS
	event_exit();
#endif
	
	/*
	 * n_read: is > 0 if it successfully read some bytes
	 *         is < 0 on error -- need to check errno
	 *         is 0 on EOF
	 */
	
	if (n_read > 0)
	{
	    read_info->nbytes_read += n_read;
	    if (read_info->nbytes_read >= read_info->wait_for_nbytes)
	    {
		char *new_buf = (char *) NULL;
		size_t new_max_nbytes = 0;
		size_t new_wait_for_nbytes = 0;
		(*read_info->callback)(read_info->arg,
				       fd,
				       read_info->buf,
				       read_info->nbytes_read,
				       &new_buf,
				       &new_max_nbytes,
				       &new_wait_for_nbytes);
		if (   new_buf
		       && (new_max_nbytes > 0)
		       && (new_wait_for_nbytes > 0))
		{
		    read_info->buf = new_buf;
		    read_info->max_nbytes = new_max_nbytes;
		    read_info->wait_for_nbytes = new_wait_for_nbytes;
		    read_info->nbytes_read = 0;
		}
		else
		{
		    NexusFree(read_info);
		    done = NEXUS_TRUE;
		}
	    }
	    else
	    {
		fd_register_for_read(fd,
				     read_callback,
				     (void *)read_info);
	        done = NEXUS_TRUE;
	    }
	}
	else if (n_read == 0)
	{
	    (*read_info->error_callback)(read_info->arg,
					 fd,
					 read_info->buf,
					 read_info->nbytes_read,
					 0);
	    NexusFree(read_info);
	    done = NEXUS_TRUE;
	}
	else /* n_read < 0 */
	{
	    if (save_errno == EINTR)
	    {
		/* Try again */
	    }
	    else if (save_errno == EAGAIN || save_errno == EWOULDBLOCK)
	    {
		/* We've read all we can for now.  So repost the read. */
		event_enter();
		fd_register_for_read(fd,
				     read_callback,
				     (void *) read_info);
		event_exit();
		done = NEXUS_TRUE;
	    }
	    else
	    {
		(*read_info->error_callback)(read_info->arg,
					     fd,
					     read_info->buf,
					     read_info->nbytes_read,
					     save_errno);
		NexusFree(read_info);
		done = NEXUS_TRUE;
	    }
	}
    }

    nexus_debug_printf(5,("read_callback(): exiting\n"));
} /* read_callback */


/*
 * nexus_fd_register_for_write()
 *
 * This function can do one of two things:
 *   1. Call select() until this fd is ready, and then call the appropriate
 *        callback, or
 *   2. Create a new thread that does a blocking write on the fd.
 */
int nexus_fd_register_for_write(int fd,
				char *buf,
				size_t nbytes,
				void (*callback_func)(void *arg,
						      int fd,
						      char *buf,
						      size_t nbytes),
				void (*error_callback_func)(void *arg,
							    int fd,
							    char *buf,
							    size_t nbytes,
							    int error),
				void *callback_arg)
{
    write_info_t *write_info;
    
    nexus_debug_printf(3, ("nexus_fd_register_for_write(): entering\n"));

    NexusMalloc(nexus_fd_register_for_write(),
		write_info,
		write_info_t *,
		sizeof(write_info_t));
    write_info->buf = buf;
    write_info->nbytes = nbytes;
    write_info->nbytes_wrote = 0;
    write_info->arg = callback_arg;
    write_info->callback = callback_func;
    write_info->error_callback = error_callback_func;

    event_enter();
    fd_table_add(fd);
    fd_register_for_write(fd,
			  write_callback,
			  (void *) write_info);
    event_exit();

    nexus_debug_printf(3, ("nexus_fd_register_for_write(): exiting\n"));
    return(0);
} /* nexus_fd_register_for_write() */


/*
 * write_callback()
 *
 * Callback to do work for nexus_fd_register_for_write()
 */
static void write_callback(void *arg, int fd)
{
    write_info_t *write_info = (write_info_t *) arg;
    long n_wrote;
    int save_errno;
    nexus_bool_t done;
    size_t nbytes;
    char *buf;

    for (done = NEXUS_FALSE; !done; )
    {
	buf = (write_info->buf + write_info->nbytes_wrote);
	nbytes = (write_info->nbytes - write_info->nbytes_wrote);
#if NEXUS_MAX_WRITE_PACKET > 0
	nbytes = MIN(NEXUS_MAX_WRITE_PACKET, nbytes);
#endif
	
#ifndef USE_THREAD_SAFE_FD_CALLS
	event_enter();
#endif
	
	n_wrote = write(fd, buf, nbytes);
	save_errno = errno;

#ifdef BUILD_DEBUG
	if (NexusDebug(7))
	{
	    int i;

	    nexus_printf("Sent %d bytes of data on fd %d\n",
	    		 n_wrote,
			 fd);

	    nexus_printf("First %d bytes:  ", NEXUS_MIN(n_wrote, 100));
	    nexus_stdio_lock();
	    for (i = 0; i < NEXUS_MIN(n_wrote, 100); i++)
	    {
		printf("%2x  ", *((unsigned char *)(buf + i)));
	    }
	    printf("\n"); fflush(stdout);
	    nexus_stdio_unlock();
	}
#endif
	
#ifndef USE_THREAD_SAFE_FD_CALLS
	event_exit();
#endif
	
	/*
	 * n_wrote: is > 0 on success -- number of bytes written
	 *          is < 0 on error -- need to check errno
	 *          is 0 (SysV) or (-1 && errno==EWOULDBLOCK) (BSD)
	 *              if the write would block without writing anything
	 */

	if (n_wrote > 0)
	{
	    write_info->nbytes_wrote += n_wrote;
	    if (write_info->nbytes_wrote >= write_info->nbytes)
	    {
		(*write_info->callback)(write_info->arg,
					fd,
					write_info->buf,
					write_info->nbytes_wrote);
		NexusFree(write_info);
		done = NEXUS_TRUE;
	    }
	}
	else if (   (n_wrote == 0)
		 || (n_wrote < 0 && save_errno == EWOULDBLOCK))
	{
	    fd_register_for_write(fd,
				  write_callback,
				  (void *) write_info);
	    done = NEXUS_TRUE;
	}
	else /* n_wrote < 0 */
	{
	    if (save_errno == EINTR)
	    {
		/* Try again */
	    }
	    else
	    {
		(*write_info->error_callback)(write_info->arg,
					      fd,
					      write_info->buf,
					      write_info->nbytes_wrote,
					      save_errno);
		NexusFree(write_info);
		done = NEXUS_TRUE;
	    }
	}
    }
} /* write_callback() */


/*
 * nexus_fd_write_one_nonblocking()
 *
 * Try to write the one character, c, to the fd.
 * But do not block on the write.  This function should
 * return immediately whether the write succeeds or fails.
 */
int nexus_fd_write_one_nonblocking(int fd, char c)
{
    long n_wrote;
    int save_errno;
    nexus_bool_t done;
    size_t nbytes;
    char *buf;
    int rc;

    nexus_debug_printf(3, ("nexus_fd_write_one_nonblocking(): entering\n"));
#ifndef USE_THREAD_SAFE_FD_CALLS
    event_enter();
#endif
	
retry:
    n_wrote = write(fd, &c, 1);
    save_errno = errno;
    if ((n_wrote < 0) && (save_errno) == EINTR)
    {
	goto retry;
    }
    rc = (n_wrote > 0 ? 1 : 0);
    
#ifndef USE_THREAD_SAFE_FD_CALLS
    event_exit();
#endif
    nexus_debug_printf(3, ("nexus_fd_write_one_nonblocking(): exiting\n"));
    return(rc);
} /* write_callback() */


/*
 * nexus_fd_register_for_writev()
 *
 * This function can do one of two things:
 *   1. Call select() until this fd is used and then call the appropriate
 *        callback, or
 *   2. Create a new thread that does a blocking write on the fd.
 */
int nexus_fd_register_for_writev(int fd,
				 struct iovec *iov,
				 size_t iovcnt,
				 void callback_func(void *arg,
						    int fd,
						    struct iovec *iov,
						    size_t iovcnt,
						    size_t nbytes),
				 void error_callback_func(void *arg,
							  int fd,
							  struct iovec *iov,
							  size_t iovcnt,
							  size_t nbytes,
							  int error),
				 void *callback_arg)
{
    writev_info_t *writev_info;

    nexus_debug_printf(3, ("nexus_fd_register_for_writev(): entering\n"));

    NexusMalloc(nexus_fd_register_for_write(),
		writev_info,
		writev_info_t *,
		sizeof(writev_info_t));
    writev_info->orig_iov = iov;
    writev_info->orig_iovcnt = iovcnt;
    NexusMalloc(nexus_fd_register_for_write(),
		writev_info->iov,
		struct iovec *,
		(sizeof(struct iovec) * iovcnt));
    writev_info->iovcnt = iovcnt;
    writev_info->arg = callback_arg;
    writev_info->callback = callback_func;
    writev_info->error_callback = error_callback_func;

    event_enter();
    fd_table_add(fd);
    fd_register_for_write(fd,
			  writev_callback,
			  (void *) writev_info);
    event_exit();

    nexus_debug_printf(3, ("nexus_fd_register_for_writev(): exiting\n"));
    return(0);
} /* nexus_fd_register_for_writev() */


/*
 * writev_callback()
 *
 * Callback to do work for nexus_fd_register_for_writev()
 */
static void writev_callback(void *arg, int fd)
{
    writev_info_t *writev_info = (writev_info_t *) arg;
    long n_wrote;
    int save_errno;
    nexus_bool_t done;

    for (done = NEXUS_FALSE; !done; )
    {
#ifndef USE_THREAD_SAFE_FD_CALLS
	event_enter();
#endif
	
	n_wrote = writev(fd,
			 writev_info->iov,
			 writev_info->iovcnt);
	save_errno = errno;
	
#ifndef USE_THREAD_SAFE_FD_CALLS
	event_exit();
#endif
	
	/*
	 * n_wrote: is > 0 on success -- number of bytes written
	 *          is < 0 on error -- need to check errno
	 *          is 0 (SysV) or (-1 && errno==EWOULDBLOCK) (BSD)
	 *              if the write would block without writing anything
	 */

	if (n_wrote > 0)
	{
	    writev_info->nbytes_wrote += n_wrote;
	    while (n_wrote > 0)
	    {
		if (n_wrote >= writev_info->iov->iov_len)
		{
		    n_wrote -= writev_info->iov->iov_len;
		    writev_info->iov++;
		    writev_info->iovcnt--;
		}
		else
		{
		    writev_info->iov->iov_base =
			(void *) (((nexus_byte_t *) writev_info->iov->iov_base)
			          + n_wrote);
		    writev_info->iov->iov_len -= n_wrote;
		    n_wrote = 0;
		}
	    }
	    if (writev_info->iovcnt <= 0)
	    {
		(*writev_info->callback)(writev_info->arg,
					 fd,
					 writev_info->orig_iov,
					 writev_info->orig_iovcnt,
					 writev_info->nbytes_wrote);
		NexusFree(writev_info);
		done = NEXUS_TRUE;
	    }
	}
	else if (   (n_wrote == 0)
		 || (n_wrote < 0 && save_errno == EWOULDBLOCK))
	{
	    fd_register_for_write(fd,
				  writev_callback,
				  (void *) writev_info);
	    done = NEXUS_TRUE;
	}
	else /* n_wrote < 0 */
	{
	    if (save_errno == EINTR)
	    {
		/* Try again */
	    }
	    else
	    {
		(*writev_info->error_callback)(writev_info->arg,
					       fd,
					       writev_info->orig_iov,
					       writev_info->orig_iovcnt,
					       writev_info->nbytes_wrote,
					       save_errno);
		NexusFree(writev_info);
		done = NEXUS_TRUE;
	    }
	}
    }
} /* writev_callback() */


/*
 * nexus_fd_unregister()
 */
int nexus_fd_unregister(int fd,
			void callback_func(void *arg, int fd))
{
    select_info_t *select_info;
    nexus_debug_printf(3, ("nexus_fd_unregister(): entering\n"));
    event_enter();
    select_info = fd_table[fd];
    if (select_info)
    {
	if (callback_func)
	{
	    /* TODO: Do something better with callback_func() */
	    if (select_info->read_arg)
	    {
		(*callback_func)(select_info->read_arg, fd);
	    }
	    if (select_info->write_arg)
	    {
		(*callback_func)(select_info->write_arg, fd);
	    }
	    if (select_info->except_arg)
	    {
		(*callback_func)(select_info->except_arg, fd);
	    }
	}
	fd_register_for_read(fd, NULL, NULL);
	fd_register_for_write(fd, NULL, NULL);
	fd_register_for_except(fd, NULL, NULL);
    }
    event_exit();
    nexus_debug_printf(3, ("nexus_fd_unregister(): exiting\n"));
    return(0);
} /* nexus_fd_unregister() */


/*
 * nexus_fd_handle_events()
 *
 * Perform a select() on the registered fds and call the proper callbacks.
 */
int nexus_fd_handle_events(int poll_type, int *message_handled)
{
    fd_set read_set;
    fd_set write_set;
    fd_set except_set;
    struct timeval timeout;
    nexus_bool_t done;
    nexus_bool_t use_timeout;
    int n_ready;
    int n_checked;
    int fd;
    int select_errno;
    select_info_t *select_info;
    int rc = 0;
    void *arg;
    void (*callback)(void *, int);

    nexus_debug_printf(5, ("nexus_fd_handle_events(): entering, poll_type=%i\n",poll_type));
    event_enter();

#ifndef BUILD_LITE
    if (handle_events_in_progress)
    {
	if (!I_Am_Handler_Thread())
	{
	    event_exit();
	    if (message_handled)
		*message_handled = 0;
	    return(0);
	}
	/* Let a handler thread recurse */
    }
    else
    {
	handle_events_in_progress = NEXUS_TRUE;
    }
#endif /* !BUILD_LITE */

    done = NEXUS_FALSE;
    while (!done && !shutdown_called)
    {
	if (   (n_fds_set <= 0)
	    && (poll_type != NEXUS_FD_POLL_BLOCKING_UNTIL_SHUTDOWN) )
	{
	    done = NEXUS_TRUE;
	    continue;
	}

	fd_table_modified = NEXUS_FALSE;
	
	read_set = read_fds;
	write_set = write_fds;
	except_set = except_fds;

	switch (poll_type)
	{
	case NEXUS_FD_POLL_BLOCKING_ALL:
	    use_timeout = NEXUS_FALSE;
	    poll_type = NEXUS_FD_POLL_NONBLOCKING_ALL;
	    break;
	case NEXUS_FD_POLL_BLOCKING_ONCE:
	case NEXUS_FD_POLL_BLOCKING_UNTIL_SHUTDOWN:
	    use_timeout = NEXUS_FALSE;
	    break;
	case NEXUS_FD_POLL_NONBLOCKING_ALL:
	case NEXUS_FD_POLL_NONBLOCKING_ONCE:
	    use_timeout = NEXUS_TRUE;
	    timeout.tv_sec = 0L;
	    timeout.tv_usec = 0L;
	    break;
	}
    
	nexus_debug_printf(5,("nexus_fd_handle_events(): Calling select()\n"));
	n_ready = select(fd_tablesize,
			 NEXUS_FD_SET_CAST &read_set,
			 NEXUS_FD_SET_CAST &write_set,
			 NEXUS_FD_SET_CAST &except_set,
			 (use_timeout ? &timeout : NULL));
	select_errno = errno;
	nexus_debug_printf(5,("nexus_fd_handle_events(): select() returned\n"));

	if (shutdown_called)
	    break;
	
	if (n_ready < 0)
	{
	    if (select_errno == EINTR)
	    {
		continue;
	    }
	    else
	    {
		rc = -1;
		goto handle_abort;
	    }
	}
	else if (n_ready > 0)
	{
	    if (message_handled)
	    {
		*message_handled = 1;
	    }
    
	    for (n_checked = 0, fd = 0;
		 n_checked < n_ready;
		 fd++)
	    {
		if (FD_ISSET(fd, &read_set))
		{
		    select_info = fd_table[fd];
		    callback = select_info->read_callback;
		    arg = select_info->read_arg;
		    if (select_info->is_listener)
		    {
			listener_accept(arg, fd);
		    }
		    else
		    {
			fd_register_for_read(fd, NULL, NULL);
			event_exit();
			(*callback)(arg, fd);
			event_enter();
		    }
		    if (fd_table_modified)
			break;
		    else
			n_checked++;
		}
		if (FD_ISSET(fd, &write_set))
		{
		    select_info = fd_table[fd];
		    callback = select_info->write_callback;
		    arg = select_info->write_arg;
		    fd_register_for_write(fd, NULL, NULL);
		    event_exit();
		    (*callback)(arg, fd);
		    event_enter();
		    if (fd_table_modified)
			break;
		    else
			n_checked++;
		}
		if (FD_ISSET(fd, &except_set))
		{
		    select_info = fd_table[fd];
		    callback = select_info->except_callback;
		    arg = select_info->except_arg;
		    fd_register_for_except(fd, NULL, NULL);
		    event_exit();
		    (*callback)(arg, fd);
		    event_enter();
		    if (fd_table_modified)
			break;
		    else
			n_checked++;
		}
	    } /* for */
	} /* endif */

	if (   poll_type == NEXUS_FD_POLL_NONBLOCKING_ONCE
	    || poll_type == NEXUS_FD_POLL_BLOCKING_ONCE
	    || (   (n_ready == 0)
		&& (   (poll_type == NEXUS_FD_POLL_NONBLOCKING_ALL)
		    || (poll_type == NEXUS_FD_POLL_BLOCKING_ALL) ) ) )
	{
	    done = NEXUS_TRUE;
	}
    }

handle_abort:
#ifndef BUILD_LITE
    handle_events_in_progress = NEXUS_TRUE;
#endif /* BUILD_LITE */
    event_exit();
    nexus_debug_printf(5, ("nexus_fd_handle_events(): exiting\n"));
    return(0);
} /* nexus_fd_handle_events */


/*
 * nexus_fd_create_listener()
 *
 * Create a listener on port that calls callback_func() when someone
 * tries to connect.
 */
int nexus_fd_create_listener(unsigned short *port,
			     int backlog,
			     void (*callback_func)(void *arg,
						   int fd),
			     void *callback_arg)
{
    struct sockaddr_in my_addr;
    int listen_fd;
    int len;
    int save_errno;
    listener_info_t *listener;

    nexus_debug_printf(3, ("nexus_fd_create_listener(): entering\n"));

    len = sizeof(my_addr);
    memset(&my_addr, len, '\0');

    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(*port);

    event_enter();
    
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
	save_errno = errno;
	event_exit();
	return(save_errno);
    }

#if defined(SO_REUSEADDR) && !defined(TARGET_ARCH_SUNOS41) && !defined(TARGET_ARCH_NEXT040)
    /*
     * Set the port so it can be reused immediately if this
     * process dies.
     *
     * Under SunOS4.1 and NeXTStep, there apparently is a bug in this.
     * With this option set, a bind() will succeed on a port even if
     * that port is still in active use by another process.
     */
    if (*port != 0)
    {
	int one = 1;

	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)) < 0)
	{
	    save_errno = errno;
	    nexus_warning("setup_listener(): setsockopt SO_REUSEADDR failed: %s\n", _nx_md_system_error_string(save_errno));
	}
    }
#endif /* SO_REUSEADDR */
    
    if (bind(listen_fd, (struct sockaddr *)&my_addr, len) < 0)
    {
	save_errno = errno;
	close(listen_fd);
	event_exit();
        return(save_errno);
    }

    if (listen(listen_fd, (backlog < 0 ? SOMAXCONN : backlog)) < 0)
    {
	save_errno = errno;
	close(listen_fd);
	event_exit();
	return(save_errno);
    }

    if (getsockname(listen_fd, (struct sockaddr *)&my_addr, &len) < 0)
    {
	save_errno = errno;
	close(listen_fd);
	event_exit();
	return(save_errno);
    }

    *port = ntohs(my_addr.sin_port);

    NexusMalloc(nexus_fd_create_listener(),
		listener,
		listener_info_t *,
		sizeof(listener_info_t));
    listener->port = *port;
    listener->fd = listen_fd;
    listener->callback = callback_func;
    listener->arg = callback_arg;
    listener->next = NULL;

    if (listener_head)
    {
	listener->prev = listener_tail;
	listener_tail->next = listener;
	listener_tail = listener;
    }
    else
    {
	listener_head = listener_tail = listener;
	listener->prev = NULL;
    }

    fd_setup(listen_fd);
    fd_register_for_read(listen_fd,
			 callback_func,
			 (void *) listener);
    fd_table[listen_fd]->is_listener = NEXUS_TRUE;

    event_exit();
    nexus_debug_printf(3, ("nexus_fd_create_listener(): exiting\n"));
    return(0);
} /* nexus_fd_create_listener() */


/*
 * listener_accept()
 *
 *  a fd that we are listening to has someone trying to connect.  The first
 *  argument points to the listener information.  After accepting the
 *  connection, call the function registered by the listener with its
 *  argument.
 */
static void listener_accept(void *arg, int fd)
{
    listener_info_t *listener = (listener_info_t *) arg;
    struct sockaddr addr;
    int addrlen;
    int new_fd;
    int rc;
    void (*user_callback)(void *, int);
    void *user_arg;

    addrlen = sizeof(struct sockaddr);
    new_fd = accept(fd, &addr, &addrlen);
    if (new_fd < 0)
    {
	/* TODO: We should probably deal with this error better */
	return;
    }
    if ((rc = fd_setup_socket(new_fd)) != 0)
    {
	/* TODO: We should probably deal with this error better */
	close(new_fd);
	return;
    }

    user_callback = listener->callback;
    user_arg = listener->arg;
    
    event_exit();
    (*user_callback)(user_arg, new_fd);
    event_enter();

} /* listener_accept() */
    

/*
 * nexus_fd_close_listener()
 *
 *  stop listening at the passed port
 */
int nexus_fd_close_listener(unsigned short port)
{
    listener_info_t *cur_listener;

    nexus_debug_printf(3, ("nexus_fd_close_listener(): entering\n"));
    event_enter();
    
    for (cur_listener = listener_head;
	 cur_listener && cur_listener->port != port;
	 cur_listener = cur_listener->next)
	; /* empty body */
    
    if (!cur_listener)
    {
	event_exit();
	return(-1);
    }
    
    if (cur_listener == listener_head)
    {
	listener_head = cur_listener->next;
    }
    if (cur_listener == listener_tail)
    {
	listener_tail = cur_listener->prev;
    }

    if (cur_listener->prev)
    {
	cur_listener->prev->next = cur_listener->next;
    }
    if (cur_listener->next)
    {
	cur_listener->next->prev = cur_listener->prev;
    }

    fd_register_for_read(cur_listener->fd, NULL, NULL);
    fd_close(cur_listener->fd);
    NexusFree(cur_listener);

    event_exit();
    nexus_debug_printf(3, ("nexus_fd_close_listener(): exiting\n"));
    return(0);
} /* nexus_fd_close_listener() */


/*
 * nexus_fd_connect()
 *
 *  connect to host/port and return the new descriptor in *fd
 */
int nexus_fd_connect(char *host, unsigned short port, int *fd)
{
    struct sockaddr_in his_addr, use_his_addr;
    nexus_bool_t connect_succeeded;
    struct hostent *hp;
    int save_errno;
#ifdef HAVE_GETHOSTBYNAME_R_5
    struct hostent hp2;
    char hp_tsdbuffer[500];
    int hp_errnop;
#elif HAVE_GETHOSTBYNAME_R_3
    struct hostent hp2;
    struct hostent_data hp_data;
#endif
    
    nexus_debug_printf(3, ("nexus_fd_connect(): entering\n"));

#ifdef HAVE_GETHOSTBYNAME_R_5
    hp = gethostbyname_r(host, &hp2, hp_tsdbuffer, 500, &hp_errnop);
#elif defined(HAVE_GET_HOSTBYNAME_R_3)
    rc = gethostbyname_r(host, &hp2, &hp_data);
    if (rc == 0)
    {
	hp = &hp2;
    }
    else
    {
	hp = NULL;
    }
#else
    /*
     * On SunOS 4.1.X, gethostbyname() calls sprintf() :(
     */
    nexus_stdio_lock();
    hp = gethostbyname(host);
    nexus_stdio_unlock();
#endif

    memset(&his_addr, sizeof(his_addr), '\0');
    his_addr.sin_port = htons(port);

    if (hp == NULL)
    {
	/*
	 * gethostbyname() on many machines does the right thing for IP
	 * addresses (e.g., "140.221.7.13").  But on some machines
	 * (e.g., SunOS 4.1.x) it doesn't.  So hack it in this case.
	 */
	if (isdigit(host[0]))
	{
	    unsigned long i1, i2, i3, i4;
	    int rc;

	    nexus_stdio_lock();
	    rc = sscanf(host, "%lu.%lu.%lu.%lu", &i1, &i2, &i3, &i4);
	    nexus_stdio_unlock();

	    if (rc != 4)
	    {
		return(-1);
	    }

	    his_addr.sin_addr.s_addr = (  (i1 << 24)
					  | (i2 << 16)
					  | (i3 << 8)
					  | i4);
	    his_addr.sin_family = AF_INET;
	}
	else
	{
	    return(-1);
	}
    }
    else
    {
	memcpy(&his_addr.sin_addr, hp->h_addr, hp->h_length);
	his_addr.sin_family = hp->h_addrtype;
    }

#ifndef USE_THREAD_SAFE_FD_CALLS
    event_enter();
#endif
    
    connect_succeeded = NEXUS_FALSE;
    while (!connect_succeeded)
    {
	if ((*fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    save_errno = errno;
#ifndef USE_THREAD_SAFE_FD_CALLS
	    event_exit();
#endif
	    return(-save_errno);
	}

	use_his_addr = his_addr;

	if (connect(*fd, (struct sockaddr *)&use_his_addr,
		    sizeof(use_his_addr)) == 0)
	{
	    connect_succeeded = NEXUS_TRUE;
	}
	else
	{
	    save_errno = errno;
	    if (save_errno == EINPROGRESS)
	    {
		/*
		 * man connect: EINPROGRESS:
		 *   The socket is non-blocking and the connection cannot be
		 *   completed immediately.  It is possible to select(2) for
		 *   completion by selecting the socket for writing.
		 * So this connect has, for all practical purposes, succeeded.
		 */
		connect_succeeded = NEXUS_TRUE;
	    }
	    else if (save_errno == EINTR)
	    {
#ifdef TARGET_ARCH_SUNOS41
		/*
		 * With SunOS 4.1.3 (at least when used
		 * with FSU pthreads), connect() has
		 * an annoying little bug.  It may return EINTR, but
		 * still succeed.  When using round robin scheduling,
		 * where SIGALRM is used to timeslice between threads,
		 * this bug actually occurs with surprising frequency.
		 *
		 * My first attempted solution was to call getpeername(),
		 * with the idea that if it succeeds then the connect()
		 * succeeded, and if it failed then the connect() failed.
		 * This didn't work; getpeername() would sometime fail
		 * yet the connect() would still succeed.
		 *
		 * But it appears that selecting the file descriptor
		 * for writing causes it to wait until things are sane.
		 * (This idea comes from EINPROGRESS.)  And it also
		 * appears that when connect() returns EINTR it
		 * nonetheless always succeeds.
		 *
		 * Note that sometimes the select() returns EINTR,
		 * so we need to retry the select() in that case.
		 * And the EBADF check is just for good measure, as
		 * this would seem to catch the (as yet unobserved)
		 * case where the interrupted connect() actually
		 * failed.
		 *
		 * -Steve Tuecke
		 */
		fd_set my_fd_set;
		int select_rc;
		int select_loop_done = NEXUS_FALSE;
		while (!select_loop_done)
		{
		    FD_ZERO(&my_fd_set);
		    FD_SET(*fd, &my_fd_set);
		    select_rc = select((*fd)+1,
				       NEXUS_FD_SET_CAST NULL,
				       NEXUS_FD_SET_CAST &my_fd_set,
				       NEXUS_FD_SET_CAST NULL,
				       NULL);
		    if (select_rc == 1)
		    {
			select_loop_done = NEXUS_TRUE;
			connect_succeeded = NEXUS_TRUE;
		    }
		    else
		    {
			save_errno = errno;
			if (save_errno == EBADF)
			{
			    close(*fd);
			    select_loop_done = NEXUS_TRUE;
			}
			else if (save_errno != EINTR)
			{
			    /*
			     * connect() returned EINTR,
			     * but selecting on the socket failed.
			     * Guess its time to try another hack
			     * around this connect() bug.
			     */
#ifndef USE_THREAD_SAFE_FD_CALLS
			    event_exit();
#endif
			    return(-1);
			}
		    }
		}
#else /* TARGET_ARCH_SUNOS41 */
		/*
		 * Do nothing.  Just try again.
		 */
		close(*fd);
#endif
	    }
	    else if (save_errno == ETIMEDOUT)
	    {
		close(*fd);
#ifndef BUILD_LITE
		/*
		 * Might as well give other threads a chance to run before
		 * trying again.
		 */
	        nexus_thread_yield();
#endif
	    }
	    else
	    {
		close(*fd);
#ifndef USE_THREAD_SAFE_FD_CALLS
		event_exit();
#endif
		return(-2);
	    }
	}
    }

#ifndef USE_THREAD_SAFE_FD_CALLS
    event_exit();
#endif

    if (nexus_fd_setup_socket(*fd) != 0)
    {
	close(*fd);
	return(-3);
    }

    nexus_debug_printf(3, ("nexus_fd_connect(): exiting\n"));
    return(0);
} /* nexus_fd_connect() */


/*
 * nexus_fd_defer()
 *
 *  let another event driver handle the detection of events.  The two
 *  functions passed here are called whenever a new fd is registered to be
 *  select() or read() from
 */
int nexus_fd_defer(void (*add_fd)(int fd),
		   void (*remove_fd)(int fd))
{
    event_enter();
    defer_events = NEXUS_TRUE;
    defer_add_func = add_fd;
    defer_remove_func = remove_fd;
    event_exit();

    /* TODO: Defer any currently registered fds */
    
    return(0);
} /* nexus_fd_defer() */


/*
 * nexus_fd_defer_callback()
 *
 *  when another event driver detects an event on a fd, we need to call the
 *  proper callbacks
 */
int nexus_fd_defer_callback(int fd)
{
    select_info_t *select_info;

    if (fd < 0)
    {
	/* TODO: Change this so that it calls a select */
	return(-1);
    }

    event_enter();
    select_info = fd_table[fd];
    event_exit();

    if (!select_info)
    {
	return(-1);
    }
    
    if (select_info->read_callback)
    {
	(*select_info->read_callback)(select_info->read_arg, fd);
    }
    if (select_info->write_callback)
    {
	(*select_info->write_callback)(select_info->write_arg, fd);
    }
    if (select_info->except_callback)
    {
	(*select_info->except_callback)(select_info->except_arg, fd);
    }

    return(0);
} /* nexus_fd_defer_callback() */


/*
 * pipe_to_self_callback()
 *
 *  This is the function that gets called when the pipe to self gets signaled
 *  during shutdown.  We don't really want to do anything except to guarantee
 *  that select() will wake after shutdown_called is set to true.
 */
static void pipe_to_self_callback(void *arg, int fd)
{
    return;
} /* pipe_to_self_callback() */


/*
 * fd_table_add()
 */
static void fd_table_add(int fd)
{
    select_info_t *select_info;

    if (fd_table[fd])
	return;
    
    NexusMalloc(fd_table_add(),
		select_info,
		select_info_t *,
		sizeof(select_info_t));
    select_info->fd = fd;
    select_info->is_listener = NEXUS_FALSE;
    select_info->read_callback = NULL;
    select_info->read_arg = NULL;
    select_info->write_callback = NULL;
    select_info->write_arg = NULL;
    select_info->except_callback = NULL;
    select_info->except_arg = NULL;

    fd_table[fd] = select_info;
    n_fds_in_use++;
    fd_table_modified = NEXUS_TRUE;

    if (defer_events)
    {
	defer_add_func(fd);
    }
} /* fd_table_add() */


/*
 * fd_table_remove()
 */
static void fd_table_remove(int fd, select_info_t *select_info)
{
    fd_table[fd] = (void *) NULL;
    n_fds_in_use--;
    fd_table_modified = NEXUS_TRUE;

    if (defer_events)
    {
	defer_remove_func(fd);
    }
    NexusFree(select_info);

} /* fd_table_remove() */
