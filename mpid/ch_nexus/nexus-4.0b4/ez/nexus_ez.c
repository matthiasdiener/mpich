#include "nexus_ez.h"
#include <ctype.h>

#define PUTTING 0
#define GETTING 1
#define CREATE_LIST 2
#define INCREMENT 1000	/* used in nexus_check_buffer() */

struct s_node_t		/* a single node in the linked list that */
{			/* handles the reply in a rpc and asynch */
    void *info;
    struct s_node_t *next;
};

static int parse_num(char *format, int *index);
static void *find_next_argp(struct s_node_t ** linked_list, va_list * ap);
static void advance_node(struct s_node_t ** single_node);
static int format_parser(nexus_buffer_t * buffer, char *format, int op_code,
			     struct s_node_t * linked_list, va_list * ap);
static void nexus_ez_rpc_reply_handler(nexus_endpoint_t *endpoint, 
			nexus_buffer_t *buffer, 
			nexus_bool_t called_from_non_threaded_handler); 
static int rpc_general(nexus_startpoint_t *sp, int handler_id, 
			nexus_ez_rpchandle_t * rpchandle,
			nexus_bool_t called_from_nonthread_h,
		        char *format, va_list * ap);
static int init_endpt(nexus_endpoint_t *endp);


static nexus_handler_t nexus_ez_handlers[] =
{\
    { NEXUS_HANDLER_TYPE_NON_THREADED,
    (nexus_handler_func_t) nexus_ez_rpc_reply_handler}
};




/*
 * nexus_ez_rpc_unpack_1sided()
 * 
 * called within a handler function for a one-sided rpc, to extract data from a
 * buffer into a variable number of arguments specified by a format string
 */
#ifdef __STDC__
int nexus_ez_rpc_unpack_1sided(nexus_buffer_t * buffer, char *format,...)
#else
int nexus_ez_rpc_unpack_1sided(buffer, format, va_alist)
nexus_buffer_t *buffer;
char *format;
va_dcl
#endif

{
    int errorcode = 0;
    va_list ap;

#ifdef __STDC__
    va_start(ap, format);
#else
    va_start(ap);
#endif
    errorcode = format_parser(buffer, format, GETTING, NULL, &ap);
    va_end(ap);
    return errorcode;
}	/* nexus_ez_rpc_unpack_1sided() */




/*
 * nexus_ez_rpc_1sided()
 * 
 * One-sided rpc to handler named by handler_id.  It passes a variable number of
 * arguments specified by a format string
 */
#ifdef __STDC__
int nexus_ez_rpc_1sided(nexus_startpoint_t *sp, int handler_id, 
			nexus_bool_t called_from_nonthread_h,
			char *format,...) 
#else
int nexus_ez_rpc_1sided(sp, handler_id,	called_from_nonthread_h, format, 
			va_alist)
nexus_startpoint_t *sp;
int handler_id;
nexus_bool_t called_from_nonthread_h;
char *format;
va_dcl
#endif

{
    nexus_buffer_t buffer;
    int errorcode = 0;
    va_list ap;
    nexus_bool_t destroy_buffer;
    int rc;

    destroy_buffer= NEXUS_TRUE;
    nexus_buffer_init(&buffer, INCREMENT, 0);
#ifdef __STDC__
    va_start(ap, format);
#else
    va_start(ap);
#endif
    errorcode = format_parser(&buffer, format, PUTTING, NULL, &ap);
    if (errorcode != 0)
	return errorcode;
    va_end(ap);
    
    rc= nexus_send_rsr(&buffer, sp, handler_id, destroy_buffer,
				called_from_nonthread_h);
    return errorcode;
}	/* nexus_ez_rpc_1sided() */




/*
 * nexus_ez_rpc_unpack()
 * 
 * This function is called within a handler function that has been invoked by
 * nexus_ez_rpc or nexus_ez_asynch.  It must be used in conjunction with
 * these funtions.  If and only if nexus_ez_rpc or nexus_ez_asynch is called
 * should unpack be used.  It extracts data from the buffer into the
 * variables in the variable argument list specified by the format string.
 */
#ifdef __STDC__
int nexus_ez_rpc_unpack(nexus_buffer_t * buffer,
			    nexus_startpoint_t *reply_sp,
			    char *format,...)
#else
int nexus_ez_rpc_unpack(buffer, reply_sp, format, va_alist)
nexus_buffer_t *buffer;
nexus_startpoint_t *reply_sp;
char *format;
va_dcl
#endif

{
    int errorcode = 0;
    va_list ap;

    nexus_get_startpoint(buffer, reply_sp, 1);
#ifdef __STDC__
    va_start(ap, format);
#else
    va_start(ap);
#endif
    errorcode = format_parser(buffer, format, GETTING, NULL, &ap);
    va_end(ap);
    return errorcode;
}	/* nexus_ez_rpc_unpack() */




/*
 * nexus_ez_rpc_reply_handler()
 * 
 * The handler function that is invoked when an nexus_ez_rpc_1sided is called
 * within a handler that was invoked with a nexus_ez_rpc or a
 * nexus_ez_asynch.  This function fills the variables that are returned to
 * the original handler that called nexus_ez_rpc or nexus_ez_asynch.
 */
static void nexus_ez_rpc_reply_handler(nexus_endpoint_t *endpoint, 
			nexus_buffer_t *buffer,
			nexus_bool_t called_from_non_threaded_handler) 
{
    int rc;
    nexus_ez_rpchandle_t *rpchandle;

    rpchandle=(nexus_ez_rpchandle_t *)(nexus_endpoint_get_user_pointer(endpoint)); 
    rc = format_parser(buffer, rpchandle->reply_format, GETTING, rpchandle->node, NULL);
    /* sets done to true and signals the condition variable */
    nexus_mutex_lock(&(rpchandle->mutex));
    rpchandle->done = NEXUS_TRUE;
    rc = nexus_cond_signal(&(rpchandle->cond));
    nexus_mutex_unlock(&(rpchandle->mutex));
}	/* nexus_ez_rpc_reply_handler() */




/*
 * nexus_ez_rpc_wait()
 * 
 * Blocks until the rpc represented by rpchandle has completed
 */
int nexus_ez_rpc_wait(nexus_ez_rpchandle_t *rpchandle)
{
    nexus_mutex_lock(&(rpchandle->mutex));
    while (rpchandle->done == NEXUS_FALSE)
    {
	nexus_cond_wait(&(rpchandle->cond), &(rpchandle->mutex));
    }
    nexus_mutex_unlock(&(rpchandle->mutex));
    return 0;
}			/* nexus_ez_rpc_wait() */




/*
 * int nexus_ez_rpc_probe()
 * 
 * Returns status= true if rpc represented by rpchandle has been completed, and
 * status= false otherwise
 */
int nexus_ez_rpc_probe(nexus_ez_rpchandle_t rpchandle, int *status)
{
    nexus_mutex_lock(&(rpchandle.mutex));
    *status = rpchandle.done;
    nexus_mutex_unlock(&(rpchandle.mutex));
    return 0;
}	/* nexus_ez_rpc_proble() */




/*
 * init_endpt(nexus_endpoint_t *endp)
 * 
 * Initializes the endpoint used for the reply startpoint in nexus_ez_rpc 
 * and nexus_ez_asynch
 */
static int init_endpt(nexus_endpoint_t *endp)
{
    nexus_endpointattr_t epattr;

    nexus_endpointattr_init(&epattr);
    nexus_endpointattr_set_handler_table(&epattr, nexus_ez_handlers, 1);
    nexus_endpoint_init(endp, &epattr);
    nexus_endpointattr_destroy(&epattr);
    return (0);
} 	/* init_endpt() */




/*
 * rpc_general()
 * 
 * This function initializes the rpchandle, sets up the reply_sp, sets up the
 * rpc to the handler, and sets up the linked list to hold the reply from the
 * handler for nexus_ez_rpc and nexus_ez_asynch
 */
static int rpc_general(nexus_startpoint_t *sp, int handler_id, 
			nexus_ez_rpchandle_t *rpchandle,
			nexus_bool_t called_from_nonthread_h,
		        char *format, va_list * ap)
{
    nexus_buffer_t buffer;
    int errorcode = 0;
    nexus_bool_t destroy_buffer;
    nexus_startpoint_t reply_sp;

    destroy_buffer= NEXUS_TRUE;

    /* initialize rpchandle */
    nexus_mutex_init(&(rpchandle->mutex), (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(rpchandle->cond), (nexus_condattr_t *) NULL);
    rpchandle->done = NEXUS_FALSE;

    /* set up reply_sp */
    init_endpt(&(rpchandle->endpoint));
    nexus_endpoint_set_user_pointer(&(rpchandle->endpoint), (void *) 
               rpchandle);
    nexus_startpoint_bind(&reply_sp, &(rpchandle->endpoint));

    /* set up rpc to handler */
    nexus_buffer_init(&buffer, nexus_sizeof_startpoint(&reply_sp, 1), 0);
    nexus_put_startpoint_transfer(&buffer, &reply_sp, 1);
    errorcode = format_parser(&buffer, format, PUTTING, NULL, ap);
    if (errorcode != 0)
	return errorcode;

    /* set up linked list to hold reply from handler */
    rpchandle->reply_format = va_arg(*ap, char *);
    rpchandle->node = (struct s_node_t *) malloc(sizeof(struct s_node_t));
    rpchandle->node->next = NULL;
    errorcode = format_parser(&buffer, rpchandle->reply_format, CREATE_LIST, rpchandle->node, ap);
    if (errorcode != 0)
	return errorcode;

    nexus_send_rsr(&buffer, sp, handler_id, destroy_buffer, 
			called_from_nonthread_h);
    return errorcode;
}	/* rpc_general() */




/*
 * nexus_ez_asynch()
 * 
 * Regular rpc to handler named by handler_id, passing arguments specified by
 * format and contained in the variable argument list.  It returns arguments
 * specified by format2.  It returns immediately after constructing an
 * rpchandle, which can then be passed to wait or probe.
 */
#ifdef __STDC__
int nexus_ez_asynch(nexus_startpoint_t *sp, int handler_id, 
			nexus_ez_rpchandle_t * rpchandle,
			nexus_bool_t called_from_nonthread_h,
		        char *format,...)
#else
int nexus_ez_asynch(sp, handler_id, rpchandle, called_from_nonthread_h, 
			format, va_alist)
nexus_startpoint_t *sp;
int handler_id;
nexus_ez_rpchandle_t *rpchandle;
nexus_bool_t called_from_nonthread_h;
char *format;
va_dcl
#endif

{
    int errorcode = 0;
    va_list ap;

#ifdef __STDC__
    va_start(ap, format);
#else
    va_start(ap);
#endif
    errorcode = rpc_general(sp, handler_id, rpchandle, 
			 called_from_nonthread_h, format, &ap);
    va_end(ap);
    return errorcode;
}	/* nexus_ez_asynch() */





/*
 * nexus_ez_rpc()
 * 
 * This is a regular rpc which blocks until reply to handler named by
 * handler_id, passing arguments specified by format contained in the
 * variable argument list and placing return values as specified by format2
 * to variables contained in the variable argument list
 */
#ifdef __STDC__
int nexus_ez_rpc(nexus_startpoint_t *sp, int handler_id, 
			nexus_bool_t called_from_nonthread_h,
			char *format,...)
#else
int nexus_ez_rpc(sp,handler_id, called_from_nonthread_h, format, va_alist) 
nexus_startpoint_t *sp;
int handler_id;
nexus_bool_t called_from_nonthread_h;
char *format;
va_dcl
#endif

{
    int errorcode = 0;
    va_list ap;
    nexus_ez_rpchandle_t rpchandle;

#ifdef __STDC__
    va_start(ap, format);
#else
    va_start(ap);
#endif
    errorcode = rpc_general(sp, handler_id, &rpchandle,
			 called_from_nonthread_h, format, &ap);
    va_end(ap);
    if (errorcode != 0)
	return 0;
    nexus_ez_rpc_wait(&rpchandle);	/* block until reply */
    return errorcode;
}	/* nexus_ez_rpc() */




/*
 * nexus_ez_destroy_rpchandle()
 * 
 * Frees space malloced in creating the linked list to hold the reply
 * information in an rpc or async
 */
void nexus_ez_destroy_rpchandle(nexus_ez_rpchandle_t *rpchandle)
{
    struct s_node_t *single_node;

    nexus_endpoint_destroy(&(rpchandle->endpoint));
    nexus_mutex_destroy(&(rpchandle->mutex));
    nexus_cond_destroy(&(rpchandle->cond));
    while (rpchandle->node != NULL)
    {
	single_node = rpchandle->node;
	rpchandle->node = rpchandle->node->next;
	free(single_node);
    }
}	/* nexus_ez_destroy_rpchandle() */




/*
 * parse_num()
 * 
 * Used to parse the format string: if the user enters a fixed number, for
 * instance "%45i", parse_num gets the number from the string: 45
 */
static int parse_num(char *format, int *index)
{
    int count = 0;

    count = atoi(format + *index);
    while (isdigit((*(format + (*index)))))
	(*index)++;
    return count;
}	/* parse_num() */




/*
 * void * find_next_argp()
 * 
 * Finds the pointer to the next argument in the linked list or variable
 * argument list, called by format_parser
 */
static void *find_next_argp(struct s_node_t ** linked_list, va_list * ap)
{
    void *hold;

    if (*linked_list == NULL)
    {
        return va_arg(*ap, void *);
    }
    else
    {
	hold = (*linked_list)->info;
	*(linked_list) = (*linked_list)->next;
	return hold;
    }
}	/* find_next_argp() */




/*
 * advance_node()
 * 
 * Creates a new node for the linked list points single_node to the newly
 * malloced node
 */
static void advance_node(struct s_node_t ** single_node)
{
    (*single_node)->next = (struct s_node_t *) malloc(sizeof(struct s_node_t));
    *(single_node) = (*single_node)->next;
    (*single_node)->next = NULL;
}	/* advance_node() */




/*
 * format_parser()
 * 
 * Parses the format string and performs the operations for a get, put, or a
 * create_list
 */
static int format_parser(nexus_buffer_t * buffer, char *format,
	           int op_code, struct s_node_t * linked_list, va_list * ap)
{
    int errorcode, i, num;
    nexus_bool_t allocate;	/* whether it is necessary to allocate memory */
    int **hold_int, *hold;
    double **hold_double;
    long **hold_long;
    unsigned **hold_unsigned;
    float **hold_float;
    nexus_byte_t **hold_byte;
    nexus_startpoint_t **hold_sp, *sp;
    char **hold_char;
    unsigned long **hold_u_long;
    unsigned short **hold_u_short;
    short **hold_short;

    errorcode = 0;
    i = 0;

    while (*(format + i) != '\0')
    {
	allocate = NEXUS_FALSE;
	while (isspace(*(format + i)))	/* skips white space */
	    i++;
	if (*(format + i) == '%')
	    i++;
	else if (*(format + i) == '\0')	/* handle white space at end */
	    continue;
	else
	    return 1;		/* parse error */
	while (isspace(*(format + i)))	/* skips white space */
	    i++;

	/*
	 * gets the size (a number, a V, a v, or default 1)
	 */
	if (isdigit(*(format + i)))
        {
	    num = parse_num(format, &i);
        }
	else if (*(format + i) == 'V')
	{
	    allocate = NEXUS_TRUE;
	    i++;
	    switch (op_code)
	    {
	      case PUTTING:
		num = va_arg(*ap, int);
		nexus_check_buffer_size(buffer,nexus_sizeof_int(1),INCREMENT,0,0);
		nexus_put_int(buffer, &num, 1);
		break;
	      case GETTING:
		hold = (int *) find_next_argp(&linked_list, ap);
		nexus_get_int(buffer, &num, 1);
		if (hold != NULL)
		{
		    *hold = num;
		}
		break;
	      case CREATE_LIST:
		linked_list->info = (void *) (va_arg(*ap, int *));
		advance_node(&linked_list);
		break;
	    }
	}
	else if (*(format + i) == 'v')
	{
	    switch (op_code)
	    {
	      case PUTTING:
		num = va_arg(*ap, int);
		nexus_check_buffer_size(buffer,nexus_sizeof_int(1),INCREMENT,0,0);
		nexus_put_int(buffer, &num, 1);
		break;
	      case GETTING:
		hold = (int *) find_next_argp(&linked_list, ap);
		nexus_get_int(buffer, &num, 1);
		if (hold != NULL)
		{
		    *hold = num;
		}
		break;
	      case CREATE_LIST:
		linked_list->info = (void *) (va_arg(*ap, int *));
		advance_node(&linked_list);
		break;
	    }
	    i++;
	}
	else
	    num = 1;
	while (isspace(*(format + i)))	/* skips white space */
	    i++;
	if (num < 0)
	    return 4;

	/* parse the operation */
	switch (*(format + i++))
	{
	  case 'd':		/* integer */
	  case 'i':
	    switch (op_code)
	    {
	      case GETTING:
		if (allocate)
		{
		    hold_int = (int **) find_next_argp(&linked_list, ap);
		    *hold_int = (int *) malloc(num * sizeof(int));
		    nexus_get_int(buffer, *hold_int, num);
		}
		else
		{
		    nexus_get_int(buffer, (int *)find_next_argp(&linked_list, ap), num);
		}
		break;
	      case PUTTING:
		nexus_check_buffer_size(buffer,
				  nexus_sizeof_int(num), INCREMENT, 0, 0);
		nexus_put_int(buffer, va_arg(*ap, int *), num);
		break;
	      case CREATE_LIST:
		if (allocate)
		    linked_list->info = (void *) (va_arg(*ap, int **));
		else
		    linked_list->info = (void *) (va_arg(*ap, int *));
		break;
	    }
	    break;
	  case 'h':		/* short */
	    switch (*(format + i++))
	    {
	      case 'd':	/* short integer */
	      case 'i':
		switch (op_code)
		{
		  case GETTING:
		    if (allocate)
		    {
			hold_short = (short **) find_next_argp(&linked_list, ap);
			*hold_short = (short *) malloc(num * sizeof(short));
			nexus_get_short(buffer, *hold_short, num);
		    }
		    else
		    {
			nexus_get_short(buffer,
			   (short *) find_next_argp(&linked_list, ap), num);
   		    }
		    break;
		  case PUTTING:
		    nexus_check_buffer_size(buffer,
				nexus_sizeof_short(num), INCREMENT, 0, 0);
		    nexus_put_short(buffer, va_arg(*ap, short *), num);
		    break;
		  case CREATE_LIST:
		    if (allocate)
			linked_list->info = (void *) (va_arg(*ap, short **));
		    else
			linked_list->info = (void *) (va_arg(*ap, short *));
		    break;
		}
		break;
	      case 'u':	/* short unsigned */
		switch (op_code)
		{
		  case GETTING:
		    if (allocate)
		    {
			hold_u_short = (unsigned short **)
			    find_next_argp(&linked_list, ap);
			*hold_u_short = (unsigned short *) malloc(num *
						    sizeof(unsigned short));
			nexus_get_u_short(buffer, *hold_u_short, num);
		    }
		    else
		    {
			nexus_get_u_short(buffer, (unsigned short *)
				     find_next_argp(&linked_list, ap), num);
		    }
		    break;
		  case PUTTING:
		    nexus_check_buffer_size(buffer,
			      nexus_sizeof_u_short(num), INCREMENT, 0, 0);
		    nexus_put_u_short(buffer,
				      va_arg(*ap, unsigned short *), num);
		    break;
		  case CREATE_LIST:
		    if (allocate)
			linked_list->info = (void *)
			    (va_arg(*ap, short unsigned **));
		    else
			linked_list->info = (void *)
			    (va_arg(*ap, short unsigned *));
		    break;
		}
		break;
	      default:
		errorcode = 1;
	    }
	    break;
	  case 'l':		/* long */
	    switch (*(format + i++))
	    {
	      case 'd':	/* long integer */
	      case 'i':
		switch (op_code)
		{
		  case GETTING:
		    if (allocate)
		    {
			hold_long = (long **)
			    find_next_argp(&linked_list, ap);
			*hold_long = (long *) malloc(num * sizeof(long));
			nexus_get_long(buffer, *hold_long, num);
		    }
		    else
		    {
			nexus_get_long(buffer, (long *)
				     find_next_argp(&linked_list, ap), num);
		    }
		    break;
		  case PUTTING:
		    nexus_check_buffer_size(buffer,
				 nexus_sizeof_long(num), INCREMENT, 0, 0);
		    nexus_put_long(buffer,
				   va_arg(*ap, long *), num);
		    break;
		  case CREATE_LIST:
		    if (allocate)
			linked_list->info = (void *) (va_arg(*ap, long **));
		    else
			linked_list->info = (void *) (va_arg(*ap, long *));
		    break;
		}
		break;
	      case 'f':	/* double */
		switch (op_code)
		{
		  case GETTING:
		    if (allocate)
		    {
			hold_double = (double **)
			    find_next_argp(&linked_list, ap);
			*hold_double =
			    (double *) malloc(num * sizeof(double));
			nexus_get_double(buffer, *hold_double, num);
		    }
		    else
		    {
			nexus_get_double(buffer, (double *)
				     find_next_argp(&linked_list, ap), num);
   		    }
		    break;
		  case PUTTING:
		    nexus_check_buffer_size(buffer,
			       nexus_sizeof_double(num), INCREMENT, 0, 0);
		    nexus_put_double(buffer,
				     va_arg(*ap, double *), num);
		    break;
		  case CREATE_LIST:
		    if (allocate)
			linked_list->info =
			    (void *) (va_arg(*ap, double **));
		    else
			linked_list->info =
			    (void *) (va_arg(*ap, double *));
		    break;
		}
		break;
	      case 'u':	/* long unsigned */
		switch (op_code)
		{
		  case GETTING:
		    if (allocate)
		    {
			hold_u_long = (unsigned long **)
			    find_next_argp(&linked_list, ap);
			*hold_u_long =
			    (unsigned long *) malloc(num * sizeof(unsigned long));
			nexus_get_u_long(buffer, *hold_u_long, num);
		    }
		    else
		    {
			nexus_get_u_long(buffer, (unsigned long *)
				     find_next_argp(&linked_list, ap), num);
		    }
		    break;
		  case PUTTING:
		    nexus_check_buffer_size(buffer,
			       nexus_sizeof_u_long(num), INCREMENT, 0, 0);
		    nexus_put_u_long(buffer,
				     va_arg(*ap, unsigned long *), num);
		    break;
		  case CREATE_LIST:
		    if (allocate)
			linked_list->info =
			    (void *) (va_arg(*ap, unsigned long **));
		    else
			linked_list->info =
			    (void *) (va_arg(*ap, unsigned long *));
		    break;
		}
		break;
	      default:
		errorcode = 1;
	    }
	    break;
	  case 'c':		/* character */
	    switch (op_code)
	    {
	      case GETTING:
		if (allocate)
		{
		    hold_char = (char **) find_next_argp(&linked_list, ap);
		    *hold_char = (char *) malloc((num + 1) * sizeof(char));
		    nexus_get_char(buffer, *hold_char, num);
		    *(*hold_char + num) = '\0';
		}
		else
		{
		    nexus_get_char(buffer, (char *)
				   find_next_argp(&linked_list, ap), num);
		}
		break;
	      case PUTTING:
		nexus_check_buffer_size(buffer,
				 nexus_sizeof_char(num), INCREMENT, 0, 0);
		nexus_put_char(buffer, va_arg(*ap, char *), num);
		break;
	      case CREATE_LIST:
		if (allocate)
		    linked_list->info = (void *) (va_arg(*ap, char **));
		else
		    linked_list->info = (void *) (va_arg(*ap, char *));
		break;
	    }
	    break;
	  case 'f':		/* float */
	    switch (op_code)
	    {
	      case GETTING:
		if (allocate)
		{
		    hold_float = (float **) find_next_argp(&linked_list, ap);
		    *hold_float = (float *) malloc(num * sizeof(float));
		    nexus_get_float(buffer, *hold_float, num);
		}
		else
		{
		    nexus_get_float(buffer, (float *)
				    find_next_argp(&linked_list, ap), num);
		}
		break;
	      case PUTTING:
		nexus_check_buffer_size(buffer,
				nexus_sizeof_float(num), INCREMENT, 0, 0);
		nexus_put_float(buffer, va_arg(*ap, float *), num);
		break;
	      case CREATE_LIST:
		if (allocate)
		    linked_list->info = (void *) (va_arg(*ap, float **));
		else
		    linked_list->info = (void *) (va_arg(*ap, float *));
		break;
	    }
	    break;
	  case 'b':		/* byte */
	    switch (op_code)
	    {
	      case GETTING:
		if (allocate)
		{
		    hold_byte = (nexus_byte_t **) find_next_argp(&linked_list, ap);
		    *hold_byte = (nexus_byte_t *) malloc(num * sizeof(char));
		    nexus_get_byte(buffer, *hold_byte, num);
		}
		else
		{
		    nexus_get_byte(buffer, (nexus_byte_t *)
				   find_next_argp(&linked_list, ap), num);
		}
		break;
	      case PUTTING:
		nexus_check_buffer_size(buffer,
				 nexus_sizeof_byte(num), INCREMENT, 0, 0);
		nexus_put_byte(buffer, va_arg(*ap, nexus_byte_t *),
			       num);
		break;
	      case CREATE_LIST:
		if (allocate)
		    linked_list->info = (void *) (va_arg(*ap, nexus_byte_t **));
		else
		    linked_list->info = (void *) (va_arg(*ap, nexus_byte_t *));
		break;
	    }
	    break;
	  case 'p':		/* startpoint */
	    switch (op_code)
	    {
	      case GETTING:
		if (allocate)
		{
		    hold_sp = (nexus_startpoint_t **)
			find_next_argp(&linked_list, ap);
		    *hold_sp = (nexus_startpoint_t *) malloc(num *
					    sizeof(nexus_startpoint_t));
		    nexus_get_startpoint(buffer, *hold_sp, num);
		}
		else
		{
		    nexus_get_startpoint(buffer, (nexus_startpoint_t *)
				find_next_argp(&linked_list,ap), num);
		}
		break;
	      case PUTTING:
		sp = va_arg(*ap, nexus_startpoint_t *);
		nexus_check_buffer_size(buffer,
		     nexus_sizeof_startpoint(sp, num), INCREMENT, 0, 0);
		nexus_put_startpoint_transfer(buffer, sp, num);
		break;
	      case CREATE_LIST:
		if (allocate)
		    linked_list->info = (void *)
			(va_arg(*ap, nexus_startpoint_t **));
		else
		    linked_list->info = (void *)
			(va_arg(*ap, nexus_startpoint_t *));
		break;
	    }
	    break;
	  case 'u':		/* unsigned integer */
	    switch (op_code)
	    {
	      case GETTING:
		if (allocate)
		{
		    hold_unsigned = (unsigned **) find_next_argp(&linked_list, ap);
		    *hold_unsigned = (unsigned *) malloc(num * sizeof(unsigned));
		    nexus_get_u_int(buffer, *hold_unsigned, num);
		}
		else
		{
		    nexus_get_u_int(buffer, (unsigned *)
				    find_next_argp(&linked_list, ap), num);
		}
		break;
	      case PUTTING:
		nexus_check_buffer_size(buffer,
				nexus_sizeof_u_int(num), INCREMENT, 0, 0);
		nexus_put_u_int(buffer, va_arg(*ap, unsigned *),
				num);
		break;
	      case CREATE_LIST:
		if (allocate)
		    linked_list->info = (void *) (va_arg(*ap, unsigned int **));
		else
		    linked_list->info = (void *) (va_arg(*ap, unsigned int *));
		break;
	    }
	    break;
	  default:
	    errorcode = 1;
	}

	if (op_code == CREATE_LIST)
	    advance_node(&linked_list);
    }
    return errorcode;
}	/* format_parser() */
