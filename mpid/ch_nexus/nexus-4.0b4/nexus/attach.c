/*
 * Nexus
 * Authors:     Steven Tuecke and Robert Olson
 *              Argonne National Laboratory
 *
 * attach.c		- TCP attachment
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/attach.c,v 1.2 1996/12/11 18:14:23 tuecke Exp $";

#include "internal.h"

#include <netdb.h>

#define MAX_ATTACH_BUFFER 4096

#ifdef BUILD_LITE
#define attach_enter()
#define attach_exit()
#else  /* BUILD_LITE */
static nexus_mutex_t	attach_mutex;
#define attach_enter()	nexus_mutex_lock(&attach_mutex);
#define attach_exit()	nexus_mutex_unlock(&attach_mutex);
#endif /* BUILD_LITE */

static char		attach_local_host[MAXHOSTNAMELEN];

/*
 * attach_listener_t
 *
 * One of these is allocated for each file descriptor that is
 * used for listening on a port.
 */
typedef struct _attach_listener_t
{
    struct _attach_listener_t *	next;
    unsigned short		port;
    nexus_context_t *		approval_func_context;
    void *			approval_func_user_arg;
    int		(*approval_func)(void *user_arg,
				 char *url,
				 nexus_startpoint_t *sp);
} attach_listener_t;

static attach_listener_t *listener_list;


/*
 * attach_state_t
 */
typedef struct _attach_state_t
{
    int				state;
    int				fd;
    nexus_mutex_t		mutex;
    nexus_cond_t		cond;
    nexus_bool_t		done;
    int				rc;
    int				format;
    nexus_startpoint_t *	sp;
    attach_listener_t *		listener;
    int				buf_length;
    nexus_byte_t		buf[MAX_ATTACH_BUFFER];
} attach_state_t;

#define ATTACH_STATE_WRITE		0
#define ATTACH_STATE_READ_FORMAT	1
#define ATTACH_STATE_READ_LENGTH	2
#define ATTACH_STATE_READ_BODY		3


/*
 * accept_attach_state_t
 */
typedef struct _accept_attach_state_t
{
    int				state;
    int				fd;
    int				format;
    int				buf_length;
    nexus_byte_t		buf[MAX_ATTACH_BUFFER];
    int				url_length;
    char			url[MAX_ATTACH_BUFFER];
    attach_listener_t		listener;
} accept_attach_state_t;

#define ACCEPT_ATTACH_STATE_READ_FORMAT		0
#define ACCEPT_ATTACH_STATE_READ_LENGTH		1
#define ACCEPT_ATTACH_STATE_READ_BODY		2
#define ACCEPT_ATTACH_STATE_WRITE		3


static void accept_attach_callback(void *arg,
				   int fd);
static void accept_attach_read_callback(void *arg,
					int fd,
					char *buf,
					size_t nbytes_read,
					char **new_buf,
					size_t *new_max_nbytes,
					size_t *new_wait_for_nbytes);
static void accept_attach_write_callback(void *arg,
					 int fd,
					 char *buf,
					 size_t nbytes);
static void accept_attach_failure(accept_attach_state_t *state);
static void accept_attach_error_callback(void *arg,
					 int fd,
					 char *buf,
					 size_t nbytes,
					 int error);

static void attach_write_callback(void *arg,
				  int fd,
				  char *buf,
				  size_t nbytes);
static void attach_read_callback(void *arg,
				 int fd,
				 char *buf,
				 size_t nbytes_read,
				 char **new_buf,
				 size_t *new_max_nbytes,
				 size_t *new_wait_for_nbytes);
static void attach_state_signal(attach_state_t *state,
				int rc);
static void attach_error_callback(void *arg,
				  int fd,
				  char *buf,
				  size_t nbytes,
				  int error);




/*
 * _nx_attach_usage_message()
 */
void _nx_attach_usage_message(void)
{
} /* _nx_attach_usage_message() */


/*
 * _nx_attach_new_process_params()
 *
 * Add arguments to 'buf', returning the number
 * of characters that were added.
 *
 * Return: The total number of characters added to 'buf'.
 */
int _nx_attach_new_process_params(char *buf, int size)
{
    return(0);
} /* _nx_attach_new_process_params() */


/*
 * _nx_attach_init()
 */
void _nx_attach_init(int *argc, char ***argv)
{
#ifndef BUILD_LITE
    nexus_mutex_init(&attach_mutex, (nexus_mutexattr_t *) NULL);
#endif /* BUILD_LITE */
    listener_list = (attach_listener_t *) NULL;
    _nx_md_gethostname(attach_local_host, MAXHOSTNAMELEN);
} /* _nx_attach_init() */


/* TODO: Add _nx_attach_stutdwon() */


/*
 * nexus_allow_attach()
 *
 * Allow other nexus clients to attach this one.
 *
 * If *port==0, then a port number is allocated and returned in *port.
 * If *port!=0, then this port number will be used.
 *
 * '*host' is set to point to a string that contains the hostname
 * that should be used.  This string should _not_ be freed.
 *
 * This function may be called multiple times to allow multiple ports to
 * be used for attachment.
 *
 * Return:	0 on success
 *		non-0 on failure:
 *			1: the port is in use
 *			2: invalid, port, host, or approval_func
 */
int nexus_allow_attach(unsigned short *port,
		       char **host,
		       int (*approval_func)(void *user_arg,
					    char *url,
					    nexus_startpoint_t *sp),
		       void *approval_func_user_arg)
{
    int rc;
    attach_listener_t *listener;

    if (!port || !host || !approval_func)
    {
	return(2);
    }
    
    NexusMalloc(nexus_allow_attach(),
		listener,
		attach_listener_t *,
		sizeof(attach_listener_t));
    listener->approval_func = approval_func;
    listener->approval_func_user_arg = approval_func_user_arg;
    _nx_context(&(listener->approval_func_context));

    if ((rc = nexus_fd_create_listener(port,
				       -1,
				       accept_attach_callback,
				       (void *) listener)))
    {
	if (rc == EADDRINUSE)
	{
	    /* The port is in use */
	    return(1);
	}
	else
	{
	    nexus_fatal("nexus_allow_attach(): nexus_fd_create_listener() failed, rc=%d\n", rc);
	}
    }

    listener->port = *port;
    *host = attach_local_host;

    attach_enter();
    listener->next = listener_list;
    listener_list = listener;
    attach_exit();
	
    return(0);
} /* nexus_allow_attach() */


/*
 * accept_attach_callback()
 */
static void accept_attach_callback(void *arg,
				   int fd)
{
    attach_listener_t *listener = (attach_listener_t *) arg;
    accept_attach_state_t *state;

    NexusMalloc(accept_attach_callback(),
		state,
		accept_attach_state_t *,
		sizeof(accept_attach_state_t));
    state->state = ACCEPT_ATTACH_STATE_READ_FORMAT;
    state->fd = fd;
    state->buf_length = 1;
    state->listener = *listener;
    nexus_fd_register_for_read(state->fd,
			       (char *) state->buf,
			       1,
			       1,
			       accept_attach_read_callback,
			       accept_attach_error_callback,
			       (void *) state);

} /* accept_attach_callback() */


/*
 * accept_attach_read_callback()
 */
static void accept_attach_read_callback(void *arg,
					int fd,
					char *buf,
					size_t nbytes_read,
					char **new_buf,
					size_t *new_max_nbytes,
					size_t *new_wait_for_nbytes)
{
    accept_attach_state_t *state = (accept_attach_state_t *) arg;
    nexus_byte_t *b = state->buf;

    switch(state->state)
    {
    case ACCEPT_ATTACH_STATE_READ_FORMAT:
	state->format = *b;
	if (nexus_dc_is_valid_format(state->format))
	{
	    state->state = ACCEPT_ATTACH_STATE_READ_LENGTH;
	    *new_buf = (char *) state->buf;
	    *new_max_nbytes
		= nexus_dc_sizeof_remote_int(1, state->format);
	    *new_wait_for_nbytes = *new_max_nbytes;
	}
	else
	{
	    accept_attach_failure(state);
	}
	break;
    case ACCEPT_ATTACH_STATE_READ_LENGTH:
	nexus_dc_get_int(&b,
			 &(state->buf_length),
			 1,
			 state->format);
	if (   (state->buf_length > 0)
	    && (state->buf_length <= MAX_ATTACH_BUFFER))
	{
	    state->state = ACCEPT_ATTACH_STATE_READ_BODY;
	    *new_buf = (char *) state->buf;
	    *new_max_nbytes = state->buf_length;
	    *new_wait_for_nbytes = *new_max_nbytes;
	}
	else
	{
	    accept_attach_failure(state);
	}
	break;
    case ACCEPT_ATTACH_STATE_READ_BODY:
	/* TODO: Need to make sure this is safe.
	 * For example, can url_length be sabotaged to cause an overrun?
	 */
	nexus_dc_get_int(&b,
			 &(state->url_length),
			 1,
			 state->format);
	if (   (state->url_length > 0)
	    && (state->url_length < MAX_ATTACH_BUFFER)
	    && (state->url_length < state->buf_length) )
	{
	    nexus_context_t *save_context;
	    attach_listener_t *listener = &(state->listener);
	    int rc;
	    nexus_startpoint_t sp;
	    int body_length;
	    int buf_length;
	    
	    nexus_dc_get_char(&b,
			      state->url,
			      state->url_length,
			      state->format);
	    state->url[state->url_length] = '\0';
	    
	    nexus_startpoint_set_null(&sp);
	    _nx_context(&save_context);
	    _nx_set_context(listener->approval_func_context);
	    nexus_debug_printf(1,("accept_attach_read_callback(): calling approval_func\n"));
	    rc = (*listener->approval_func)(listener->approval_func_user_arg,
					    state->url,
					    &sp);
	    nexus_debug_printf(1,("accept_attach_read_callback(): approval_func returned %d\n", rc));
	    _nx_set_context(save_context);

	    if (rc == 0 && nexus_startpoint_is_null(&sp))
	    {
		rc = NEXUS_FAULT_UNKNOWN;
	    }

	    state->state = ACCEPT_ATTACH_STATE_WRITE;
	    b = state->buf;
	    body_length = nexus_sizeof_int(1); /* rc */
	    if (rc == 0)
	    {
		body_length += nexus_sizeof_startpoint(&sp, 1);
	    }
	    buf_length = 1 + nexus_sizeof_int(1) + body_length;
	    if (buf_length > MAX_ATTACH_BUFFER)
	    {
		rc = NEXUS_FAULT_UNKNOWN;
		body_length = nexus_sizeof_int(1); /* rc */
		buf_length = 1 + nexus_sizeof_int(1) + body_length;
	    }
	    *b++ = (nexus_byte_t) nexus_dc_format();
	    nexus_user_put_int(&b, &body_length, 1);
	    nexus_user_put_int(&b, &rc, 1);
	    if (rc == 0)
	    {
		nexus_user_put_startpoint_transfer(&b, &sp, 1);
	    }
	    state->buf_length = buf_length;
	    
	    /* Write the outgoing message */
	    nexus_fd_register_for_write(fd,
					(char *) state->buf,
					state->buf_length,
					accept_attach_write_callback,
					accept_attach_error_callback,
					(void *) state);

	}
	else
	{
	    accept_attach_failure(state);
	}
	break;
    }
} /* accept_attach_read_callback() */


/*
 * accept_attach_write_callback()
 */
static void accept_attach_write_callback(void *arg,
					 int fd,
					 char *buf,
					 size_t nbytes)
{
    accept_attach_state_t *state = (accept_attach_state_t *) arg;
    nexus_fd_close(state->fd);
    NexusFree(state);
} /* accept_attach_write_callback() */


/*
 * accept_attach_failure()
 */
static void accept_attach_failure(accept_attach_state_t *state)
{
    nexus_fd_close(state->fd);
    NexusFree(state);
    if (_nx_fault_detected(NEXUS_FAULT_ACCEPT_ATTACH_FAILED) != 0)
    {
	nexus_fatal("accept_attach_failure(): Attachment attempt failed.\n");
    }
} /* accept_attach_failure() */


/*
 * accept_attach_error_callback()
 */
static void accept_attach_error_callback(void *arg,
					 int fd,
					 char *buf,
					 size_t nbytes,
					 int error)
{
    accept_attach_state_t *state = (accept_attach_state_t *) arg;
    accept_attach_failure(state);
} /* accept_attach_error_callback() */


/*
 * nexus_disallow_attach()
 *
 * Disallow other nexus clients to attach this one using the passed 'port'.
 */
void nexus_disallow_attach(unsigned short port)
{
    attach_listener_t *listener;
    attach_listener_t *listener_last;

    attach_enter();
    for (listener = listener_list, listener_last = (attach_listener_t *) NULL;
	 listener && listener->port != port;
	 listener_last = listener, listener = listener->next)
	; /* empty body */

    if (listener)
    {
	/* Remove listener from listener_list */
	if (listener_last)
	{
	    listener_last = listener->next;
	}
	else
	{
	    listener_list = listener->next;
	}

	attach_exit();

	/* Close the listener */
	nexus_fd_close_listener(port);

	NexusFree(listener);
    }
    else
    {
	/* There is no listener with this port */
	attach_exit();
    }
} /* nexus_disallow_attach() */


/*
 * nexus_attach()
 *
 * Use 'url', of the form "x-nexus://<host>:<port>/...", and
 * use it to attach this nexus client to the nexus server
 * specified by this url.
 *
 * Return:	0 if successful, and fill in 'sp' with a
 *			startpoint to the nexus server.
 *		non-0 on failure:
 *			1: invalid url
 *			2: failed to connect
 *			3: other end died unexpectedly
 */
int nexus_attach(char *url, nexus_startpoint_t *sp)
{
    char *host;
    unsigned short port;
    int fd = -1;
    int rc;
    int url_length;
    int body_length;
    int buf_length;
    attach_state_t state;
    nexus_byte_t *b;

    if (nexus_split_nexus_url(url, &host, &port, NULL) != 0)
    {
	/* Invalid url */
	rc = NEXUS_FAULT_BAD_URL;
	goto abort;
    }

    rc = nexus_fd_connect(host, port, &fd);
    nexus_split_nexus_url_free(&host, NULL);
    if (rc < 0)
    {
	/* Failed to connect */
	rc = NEXUS_FAULT_CONNECT_FAILED;
	fd = -1;
	goto abort;
    }

    /* Put together the outgoing message containing the URL */
    url_length = strlen(url);
    body_length = nexus_sizeof_int(1) + nexus_sizeof_char(url_length);
    buf_length = 1 + nexus_sizeof_int(1) + body_length;
    if (buf_length > MAX_ATTACH_BUFFER)
    {
	/* URL too long */
	rc = NEXUS_FAULT_BAD_URL;
	goto abort;
    }
    state.state = ATTACH_STATE_WRITE;
    state.fd = fd;
    nexus_mutex_init(&(state.mutex), (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(state.cond), (nexus_condattr_t *) NULL);
    state.done = NEXUS_FALSE;
    state.rc = 0;
    state.sp = sp;
    state.buf_length = buf_length;
    b = state.buf;
    *b++ = (nexus_byte_t) nexus_dc_format();
    nexus_user_put_int(&b, &body_length, 1);
    nexus_user_put_int(&b, &url_length, 1);
    nexus_user_put_char(&b, url, url_length);

    /* Write the outgoing message */
    nexus_fd_register_for_write(fd,
				(char *) state.buf,
				state.buf_length,
				attach_write_callback,
				attach_error_callback,
				(void *) &state);

    /* Wait for the attach to complete */
    nexus_mutex_lock(&(state.mutex));
    while (!state.done)
    {
	nexus_cond_wait(&(state.cond), &(state.mutex));
    }
    nexus_mutex_unlock(&(state.mutex));

    fd = state.fd;
    rc = state.rc;
    nexus_mutex_destroy(&(state.mutex));
    nexus_cond_destroy(&(state.cond));
    
abort:
    nexus_fd_close(fd);
    return (rc);
    
} /* nexus_attach() */


/*
 * attach_write_callback()
 */
static void attach_write_callback(void *arg,
				  int fd,
				  char *buf,
				  size_t nbytes)
{
    attach_state_t *state = (attach_state_t *) arg;
    state->state = ATTACH_STATE_READ_FORMAT;
    nexus_fd_register_for_read(state->fd,
			       (char *) state->buf,
			       1,
			       1,
			       attach_read_callback,
			       attach_error_callback,
			       arg);
} /* attach_write_callback() */


/*
 * attach_read_callback()
 */
static void attach_read_callback(void *arg,
				 int fd,
				 char *buf,
				 size_t nbytes_read,
				 char **new_buf,
				 size_t *new_max_nbytes,
				 size_t *new_wait_for_nbytes)
{
    attach_state_t *state = (attach_state_t *) arg;
    nexus_byte_t *b = state->buf;
    int rc;

    switch(state->state)
    {
    case ATTACH_STATE_READ_FORMAT:
	state->format = *b;
	if (nexus_dc_is_valid_format(state->format))
	{
	    state->state = ATTACH_STATE_READ_LENGTH;
	    *new_buf = (char *) state->buf;
	    *new_max_nbytes
		= nexus_dc_sizeof_remote_int(1, state->format);
	    *new_wait_for_nbytes = *new_max_nbytes;
	}
	else
	{
	    attach_state_signal(state, NEXUS_FAULT_BAD_PROTOCOL);
	}
	break;
    case ATTACH_STATE_READ_LENGTH:
	nexus_dc_get_int(&b,
			 &(state->buf_length),
			 1,
			 state->format);
	if (   (state->buf_length > 0)
	    && (state->buf_length <= MAX_ATTACH_BUFFER))
	{
	    state->state = ATTACH_STATE_READ_BODY;
	    *new_buf = (char *) state->buf;
	    *new_max_nbytes = state->buf_length;
	    *new_wait_for_nbytes = *new_max_nbytes;
	}
	else
	{
	    attach_state_signal(state, NEXUS_FAULT_BAD_PROTOCOL);
	}
	break;
    case ATTACH_STATE_READ_BODY:
	/* TODO: Need to add error checking to this */
	nexus_user_get_int(&b,
			   &rc,
			   1,
			   state->format);
	nexus_user_get_startpoint(&b,
				  state->sp,
				  1,
				  state->format);
	attach_state_signal(state, rc);
	break;
    }
} /* attach_read_callback() */


/*
 * attach_state_signal()
 */
static void attach_state_signal(attach_state_t *state,
				int rc)
{
    state->rc = rc;
    nexus_mutex_lock(&(state->mutex));
    state->done = NEXUS_TRUE;
    nexus_cond_signal(&(state->cond));
    nexus_mutex_unlock(&(state->mutex));
} /* attach_state_signal() */


/*
 * attach_error_callback()
 */
static void attach_error_callback(void *arg,
				  int fd,
				  char *buf,
				  size_t nbytes,
				  int error)
{
    attach_state_t *state = (attach_state_t *) arg;
    attach_state_signal(state, NEXUS_FAULT_BAD_PROTOCOL);
} /* attach_error_callback() */


/*
 * nexus_split_nexus_url()
 *
 * Take 'url', of the form:
 *	x-nexus://<host>:<port>/specifier1/specifier2/...
 * and split out the various components, and fill them into
 * the 'host', 'port', and 'specifiers' arguments.
 *
 * The 'host' is pointer to a string with the hostname.
 * The 'specifiers' is an array of character pointers, pointing
 * to the list of specifiers.
 *
 * 'host', 'port', and/or 'specifiers' may be NULL pointers,
 * indicating that the caller does not care about those fields.
 *
 * The values returned in 'host' and 'specifiers' should be freed
 * by calling nexus_split_nexus_url_free(host,specifiers).
 *
 * Return:	0 if successful
 *		non-0 if it is an invalid url
 */
int nexus_split_nexus_url(char *url,
			  char **host,
			  unsigned short *port,
			  char ***specifiers)
{
    char *start;
    char *end;
    char *specifier_start;
    char *s;
    int length;
    char tmp[16];
    int i;
    int n_specifiers;
    int done;

    if (host)
    {
	*host = (char *) NULL;
    }
    if (specifiers)
    {
	*specifiers = (char **) NULL;
    }

    /*
     * Check the url for a valid prefix
     */
    if (strncmp(url, "x-nexus://", 10) != 0)
    {
	/* invalid url: invalid prefix */
	goto badurl;
    }
    start = url + 10;

    /*
     * Pull the host out of the url
     */
    if ((end = strchr(start, ':')) == (char *) NULL)
    {
	/* invalid url: no ':' after host */
	goto badurl;
    }
    if (host)
    {
	length = end - start;
	NexusMalloc(nexus_attach(),
		    *host,
		    char *,
		    length + 1);
	strncpy(*host, start, length);
	(*host)[length] = '\0';
    }

    /* Pull the port out of the url */
    start = end + 1;
    if ((end = strchr(start, '/')) == (char *) NULL)
    {
	end = strchr(start, '\0');
    }
    if (   ((end - start) > 15)
	|| ((end - start) == 0) )
    {
	/*
	 * invalid url: the port section is either empty or is
	 *		way to big to be a valid port
	 */
	goto badurl;
    }
    for (i = 0; start+i < end; i++)
    {
	tmp[i] = *(start + i);
	if (!isdigit(tmp[i]))
	{
	    /* invalid url: port section contains a non-digit */
	    goto badurl;
	}
    }
    tmp[i] = '\0';
    if (port)
    {
	*port = (short) atoi(tmp);
    }

    if (*end == '\0')
    {
	/* There is no specifiers section, so we're done */
	return(0);
    }
    
    if (!specifiers)
    {
	/* We don't care about the specifiers, so we're done */
	return(0);
    }

    /* Count the number of specifiers */
    n_specifiers = 0;
    start = end + 1;
    specifier_start = start;
    for (done = NEXUS_FALSE; !done; )
    {
	end = strchr(start, '/');
	if (end == (char *) NULL)
	{
	    /* We've hit the end of the url, without a trailing / */
	    n_specifiers++;
	    done = NEXUS_TRUE;
	}
	else if (*(end - 1) == '\\')
	{
	    /* This is an escaped slash, so keep looking */
	    start = end + 1;
	}
	else
	{
	    /* We found the end of a specifier */
	    start = end + 1;
	    n_specifiers++;
	    if (*start == '\0')
	    {
		/* We've hit the end of the url, with a trailing / */
		done = NEXUS_TRUE;
	    }
	}
    }

    if (n_specifiers == 0)
    {
	/* No specifiers, so we're done */
	return(0);
    }
    
    /* Allocate the specifiers array */
    NexusMalloc(nexus_split_nexus_url(),
		*specifiers,
		char **,
		(sizeof(char *) * (n_specifiers + 1)) );
    for (i = 0; i <= n_specifiers; i++)
    {
	(*specifiers)[i] = (char *) NULL;
    }

    /* Split out the specifiers */
    start = specifier_start;
    i = 0;
    for (done = NEXUS_FALSE; !done; )
    {
	end = strchr(start, '/');
	if (end == (char *) NULL)
	{
	    /* We've hit the end of the url, without a trailing / */
	    length = strlen(specifier_start);
	    NexusMalloc(nexus_split_nexus_url(),
			s,
			char *,
			length + 1);
	    strncpy(s, specifier_start, length);
	    s[length] = '\0';
	    (*specifiers)[i] = s;
	    i++;
	    
	    done = NEXUS_TRUE;
	}
	else if (*(end - 1) == '\\')
	{
	    /* This is an escaped slash, so keep looking */
	    start = end + 1;
	}
	else
	{
	    /* We found the end of a specifier */
	    length = end - specifier_start;
	    NexusMalloc(nexus_split_nexus_url(),
			s,
			char *,
			length + 1);
	    strncpy(s, specifier_start, length);
	    s[length] = '\0';
	    (*specifiers)[i] = s;
	    i++;
	    
	    start = end + 1;
	    specifier_start = start;

	    if (*start == '\0')
	    {
		/* We've hit the end of the url, with a trailing / */
		done = NEXUS_TRUE;
	    }
	}
    }

    return(0);

 badurl:
    nexus_split_nexus_url_free(host, specifiers);
    return(1);
    
} /* nexus_split_nexus_url() */


/*
 * nexus_split_nexus_url_free()
 *
 * Free the 'host' and 'specifiers' values that were
 * previously returned by nexus_split_nexus_url().
 */
void nexus_split_nexus_url_free(char **host,
				char ***specifiers)
{
    int i;
    
    if (host && *host)
    {
	NexusFree(*host);
    }

    if (specifiers && *specifiers)
    {
	for (i = 0; (*specifiers)[i] != (char *) NULL; i++)
	{
	    NexusFree((*specifiers)[i]);
	}
	NexusFree(*specifiers);
    }
} /* nexus_split_nexus_url_free() */
