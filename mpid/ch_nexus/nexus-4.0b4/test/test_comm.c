#include <stdio.h>

#include "nexus.h"

/* uncomment the following line if you want verbose output */
/* #define VERBOSE */

#include "nexus_startup_code.h"

#define TEST_DIR "resource_database:test_dir"
#define TEST_EXE "resource_database:test_exe"

#define ATTACH_SERVER_FINISH_HANDLER_HASH 910
#define COMM_TEST_INIT_HANDLER_HASH       289
#define COMM_TEST_RECEIVE_HANDLER_HASH    592
#define COMM_TEST_FINISH_HANDLER_HASH     494
#define COMM_TEST_REPLY_HANDLER_HASH      409

#define MAX(V1,V2) (((V1) > (V2)) ? (V1) : (V2))
#define MIN(V1,V2) (((V1) < (V2)) ? (V1) : (V2))

#ifdef USE_THREADED_HANDLER
typedef nexus_stashed_buffer_t  ctrh_buffer_t;
#define CTRH_TYPE               NEXUS_HANDLER_TYPE_THREADED
#define CTRH_GET_GLOBAL_POINTER nexus_get_stashed_global_pointer
#define CTRH_GET_INT            nexus_get_stashed_int
#else
typedef nexus_buffer_t          ctrh_buffer_t;
#define CTRH_TYPE               NEXUS_HANDLER_TYPE_NON_THREADED
#define CTRH_GET_GLOBAL_POINTER nexus_get_global_pointer
#define CTRH_GET_INT            nexus_get_int
#endif

int float_type_tag  =  1;
int double_type_tag =  2;
int short_type_tag  =  3;
int ushort_type_tag =  4;
int int_type_tag    =  5;
int uint_type_tag   =  6;
int long_type_tag   =  7;
int ulong_type_tag  =  8;
int char_type_tag   =  9;
int uchar_type_tag  = 10;

int beginning_buffer_size;
int ending_buffer_size;
int step_size;



/* things for the communications test routines */

typedef struct _comm_test_reply_done_t
{
    nexus_mutex_t mutex;
    nexus_cond_t  cond;
    nexus_bool_t  done;
} comm_test_reply_done_t;

static comm_test_reply_done_t comm_test_reply_done;

nexus_global_pointer_t reply_gp;

static void comm_test_init_handler (void *address, ctrh_buffer_t *buffer);
static void comm_test_receive_handler (void *address, ctrh_buffer_t *buffer);
static void comm_test_finish_handler (void *address, ctrh_buffer_t *buffer);
static void comm_test_reply_handler (void *address, nexus_buffer_t *buffer);

static nexus_handler_t comm_test_handlers[] =
{ \
  {"comm_test_init_handler",
   COMM_TEST_INIT_HANDLER_HASH,
   CTRH_TYPE,
   (nexus_handler_func_t) comm_test_init_handler},
  {"comm_test_receive_handler",
   COMM_TEST_RECEIVE_HANDLER_HASH,
   CTRH_TYPE,
   (nexus_handler_func_t) comm_test_receive_handler},
  {"comm_test_finish_handler",
   COMM_TEST_FINISH_HANDLER_HASH,
   CTRH_TYPE,
   (nexus_handler_func_t) comm_test_finish_handler},
  {"comm_test_reply_handler",
   COMM_TEST_REPLY_HANDLER_HASH,
   NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) comm_test_reply_handler},
  {(char *) NULL,
   0,
   NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) NULL},
};



/* things for the attach test routines */

typedef struct
{
    nexus_mutex_t mutex;
    nexus_cond_t	 cond;
    nexus_bool_t	 done;
} attach_server_monitor_t;

static attach_server_monitor_t attach_server_monitor;

static nexus_bool_t   attach_server_arg = NEXUS_FALSE;
static nexus_bool_t   attach_client_arg = NEXUS_FALSE;
static char           attach_client_host[100];
static unsigned short attach_client_port;
static nexus_bool_t   attach_client_leave_server = NEXUS_FALSE;
static nexus_bool_t   attach_client_die = NEXUS_FALSE;
static unsigned short attach_server_port;

static void attach_server_finish_handler (void *address,
                                          nexus_buffer_t *buffer);

static nexus_handler_t attach_test_handlers[] =
{ \
  {"attach_server_finish_handler",
   ATTACH_SERVER_FINISH_HANDLER_HASH,
   NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) attach_server_finish_handler},
  {(char *) NULL,
   0,
   NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) NULL},
};



nexus_bool_t all_tests_okay;



int NexusBoot (void)
/*
** DESCRIPTION: This function is automatically invoked when a context is
** created and is used to do initialization activities such as registering
** handlers required by the context.
**
** This function must return a return code of zero for the context to
** be successfully initialized.
*/
{
    int rc;


#ifdef VERBOSE
    nexus_printf ("NexusBoot(): entering\n");
#endif
    nexus_register_handlers (system_handlers);
    nexus_register_handlers(comm_test_handlers);
    nexus_register_handlers(attach_test_handlers);
    rc = 0;
#ifdef VERBOSE
    nexus_printf("NexusBoot(): exiting, returning %d\n", rc);
#endif
    return (rc);
} /* NexusBoot() */





void comm_test_init_handler (void *address, ctrh_buffer_t *buffer)
/*
** DESCRIPTION: This function is a handler that is used at the beginning of
** the communications tests to transfer a global pointer for the test
** initiator to the remote communications test nodes.  This is necessary so
** the remote nodes can respond to the main process at the end of the test.
*/
{
    void *initial_segment;


#ifdef VERBOSE
    nexus_printf ("comm_test_remote_handler(): entering\n");
#endif

    /* take a look at the context's master data segment */
    initial_segment = nexus_context_initial_segment ();
#ifdef VERBOSE
    nexus_printf ("comm_test_remote_handler(): master segment at 0x%x\n",
		  (unsigned long) initial_segment);
#endif
    if (initial_segment)
    {
	*((int *) initial_segment) = 42;
    }

    /* get the reply global pointer */
    CTRH_GET_GLOBAL_POINTER (buffer, &reply_gp, 1);
} /* comm_test_init_handler() */





static void comm_test_receive_handler (void *address, ctrh_buffer_t *buffer)
/*
** DESCRIPTION: This function is a handler that receives and checks data sent
** from the originator of the communications test.  The data begins with two
** integers that represent a tag value for the type of data being sent and
** a value for the number of elements being sent.  The function then attempts
** to read number_of_elements values of the type specified by the tag from the
** sender and verifies that they are all the agreed-upon expected value.
*/
{
    char           received_char;
    double         received_double;
    float          received_float;
    int            data_type_tag;
    int            i;
    int            number_of_elements;
    int            received_int;
    long           received_long;
    nexus_bool_t   received_okay;
    short          received_short;
    unsigned char  received_uchar;
    unsigned int   received_uint;
    unsigned long  received_ulong;
    unsigned short received_ushort;


    nexus_get_int (buffer, &data_type_tag, 1);
    nexus_get_int (buffer, &number_of_elements, 1);

    received_okay = NEXUS_TRUE;

    switch (data_type_tag)
    {
    case 1:   /* float */
    {
	for (i = 0; i < number_of_elements; i++)
	{
            nexus_get_float (buffer, &received_float, 1);
            received_okay = received_okay && (received_float == 42.0);
	}

	if (received_okay)
	{
#ifdef VERBOSE
            nexus_printf ("comm_test_receive_handler(): float buffer success (size = %d)\n", number_of_elements);
#endif
	}
	else
	{
            nexus_printf ("comm_test_receive_handler(): ERROR: float buffer failure (size = %d)\n", number_of_elements);
            all_tests_okay = NEXUS_FALSE;
	}
    }
    break;

    case 2:   /* double */
    {
	for (i = 0; i < number_of_elements; i++)
	{
            nexus_get_double (buffer, &received_double, 1);
            received_okay = received_okay && (received_double == 42.0);
	}

	if (received_okay)
	{
#ifdef VERBOSE
            nexus_printf ("comm_test_receive_handler(): double buffer success (size = %d)\n", number_of_elements);
#endif
	}
	else
	{
            nexus_printf ("comm_test_receive_handler(): ERROR: double buffer failure (size = %d)\n", number_of_elements);
            all_tests_okay = NEXUS_FALSE;
	}
    }
    break;

    case 3:   /* short */
    {
	for (i = 0; i < number_of_elements; i++)
	{
            nexus_get_short (buffer, &received_short, 1);
	}

	if (received_okay)
	{
#ifdef VERBOSE
            nexus_printf ("comm_test_receive_handler(): short buffer success (size = %d)\n", number_of_elements);
#endif
	}
	else
	{
            nexus_printf ("comm_test_receive_handler(): ERROR: short buffer failure (size = %d)\n", number_of_elements);
            all_tests_okay = NEXUS_FALSE;
	}
    }
    break;

    case 4:   /* unsigned short */
    {
	for (i = 0; i < number_of_elements; i++)
	{
            nexus_get_u_short (buffer, &received_ushort, 1);
            received_okay = received_okay && (received_ushort == 42);
	}

	if (received_okay)
	{
#ifdef VERBOSE
            nexus_printf ("comm_test_receive_handler(): unsigned short buffer success (size = %d)\n", number_of_elements);
#endif
	}
	else
	{
            nexus_printf ("comm_test_receive_handler(): ERROR: unsigned short buffer failure (size = %d)\n", number_of_elements);
            all_tests_okay = NEXUS_FALSE;
	}
    }
    break;

    case 5:   /* int */
    {
	for (i = 0; i < number_of_elements; i++)
	{
            nexus_get_int (buffer, &received_int, 1);
            received_okay = received_okay && (received_int == 42);
	}

	if (received_okay)
	{
#ifdef VERBOSE
            nexus_printf ("comm_test_receive_handler(): int buffer success (size = %d)\n", number_of_elements);
#endif
	}
	else
	{
            nexus_printf ("comm_test_receive_handler(): ERROR: int buffer failure (size = %d)\n", number_of_elements);
            all_tests_okay = NEXUS_FALSE;
	}
    }
    break;

    case 6:   /* unsigned int */
    {
	for (i = 0; i < number_of_elements; i++)
	{
            nexus_get_u_int (buffer, &received_uint, 1);
            received_okay = received_okay && (received_uint == 42);
	}

	if (received_okay)
	{
#ifdef VERBOSE
            nexus_printf ("comm_test_receive_handler(): unsigned int buffer success (size = %d)\n", number_of_elements);
#endif
	}
	else
	{
            nexus_printf ("comm_test_receive_handler(): ERROR: unsigned int buffer failure (size = %d)\n", number_of_elements);
            all_tests_okay = NEXUS_FALSE;
	}
    }
    break;

    case 7:   /* long */
    {
	for (i = 0; i < number_of_elements; i++)
	{
            nexus_get_long (buffer, &received_long, 1);
            received_okay = received_okay && (received_long == 42);
	}

	if (received_okay)
	{
#ifdef VERBOSE
            nexus_printf ("comm_test_receive_handler(): long buffer success (size = %d)\n", number_of_elements);
#endif
	}
	else
	{
            nexus_printf ("comm_test_receive_handler(): ERROR: long buffer failure (size = %d)\n", number_of_elements);
            all_tests_okay = NEXUS_FALSE;
	}
    }
    break;

    case 8:   /* unsigned long */
    {
	for (i = 0; i < number_of_elements; i++)
	{
            nexus_get_u_long (buffer, &received_ulong, 1);
            received_okay = received_okay && (received_ulong == 42);
	}

	if (received_okay)
	{
#ifdef VERBOSE
            nexus_printf ("comm_test_receive_handler(): unsigned long buffer success (size = %d)\n", number_of_elements);
#endif
	}
	else
	{
            nexus_printf ("comm_test_receive_handler(): ERROR: unsigned long buffer failure (size = %d)\n", number_of_elements);
            all_tests_okay = NEXUS_FALSE;
	}
    }
    break;

    case 9:   /* char */
    {
	for (i = 0; i < number_of_elements; i++)
	{
            nexus_get_char (buffer, &received_char, 1);
            received_okay = received_okay && (received_char == 42);
	}

	if (received_okay)
	{
#ifdef VERBOSE
            nexus_printf ("comm_test_receive_handler(): char buffer success (size = %d)\n", number_of_elements);
#endif
	}
	else
	{
            nexus_printf ("comm_test_receive_handler(): ERROR: char buffer failure (size = %d)\n", number_of_elements);
            all_tests_okay = NEXUS_FALSE;
	}
    }
    break;

    case 10:   /* unsigned char */
    {
	for (i = 0; i < number_of_elements; i++)
	{
            nexus_get_u_char (buffer, &received_uchar, 1);
            received_okay = received_okay && (received_uchar == 42);
	}

	if (received_okay)
	{
#ifdef VERBOSE
            nexus_printf ("comm_test_receive_handler(): unsigned char buffer success (size = %d)\n", number_of_elements);
#endif
	}
	else
	{
            nexus_printf ("comm_test_receive_handler(): ERROR: unsigned char buffer failure (size = %d)\n", number_of_elements);
            all_tests_okay = NEXUS_FALSE;
	}
    }
    break;

    default:
    {
	nexus_fatal ("comm_test_receive_handler(): ERROR: bad tag type received!\n");
    }
    }
} /* comm_test_receive_handler() */





static void comm_test_finish_handler (void *address, nexus_buffer_t *buffer)
/*
** DESCRIPTION: This function is a handler that is used to end the test
** routines on the remote nodes.  The function reads an integer value from
** the sender which is used as a flag for whether the context of the remote
** node should be destroyed.
*/
{
    int            destroy_context;
    nexus_buffer_t reply_buffer;


    /* get destroy_context */
    CTRH_GET_INT (buffer, &destroy_context, 1);
#ifdef VERBOSE
    nexus_printf ("comm_test_remote_handler(): destroy_context=%d\n",
		  destroy_context);
#endif

    /* reply to the original context */
    nexus_init_remote_service_request (&reply_buffer, &reply_gp,
				       "comm_test_reply_handler",
				       COMM_TEST_REPLY_HANDLER_HASH);

#ifdef USE_THREADED_HANDLER    
#ifdef VERBOSE
    nexus_printf ("comm_test_remote_handler(): freeing stashed buffer\n");
#endif
    nexus_free_stashed_buffer(buffer);
#endif
    
#ifdef VERBOSE
    nexus_printf ("comm_test_remote_handler(): sending reply rsr\n");
#endif
    if (nexus_send_remote_service_request (&reply_buffer) != 0)
    {
	nexus_printf ("comm_test_remote_handler(): ERROR: send failed\n");
	nexus_printf ("comm_test_remote_handler(): ERROR: nexus_test_gp()=%d\n",
		      nexus_test_global_pointer (&reply_gp));
	all_tests_okay = NEXUS_FALSE;
    }

    if (destroy_context)
    {
#ifdef VERBOSE
	nexus_printf ("comm_test_remote_handler(): destroy current context\n");
#endif
	nexus_destroy_current_context (
#ifdef USE_THREADED_HANDLER
	    NEXUS_FALSE
#else
	    NEXUS_TRUE
#endif
	    );
    }
} /* comm_test_finish_handler() */





static void comm_test_reply_handler (void *address, nexus_buffer_t *buffer)
/*
** DESCRIPTION: This function is a handler that is invoked by the remote
** communications test nodes to signal the originator that they have completed.
*/
{
    struct _comm_test_reply_done_t *reply_done;


#ifdef VERBOSE
    nexus_printf ("comm_test_reply_handler(): entering\n");
#endif

    /* the address should be to the comm_test_reply_done structure */
    if (address == (void *) (&comm_test_reply_done))
    {
	reply_done = (comm_test_reply_done_t *) address;
#ifdef VERBOSE
	nexus_printf ("comm_test_reply_handler(): address ok\n");
#endif
    }
    else
    {
	reply_done = &comm_test_reply_done;
	nexus_printf ("comm_test_reply_handler(): ERROR: bad address %lu, should be %lu\n", (unsigned long) address, (unsigned long) (&comm_test_reply_done));
	all_tests_okay = NEXUS_FALSE;
    }
    
    /* awaken the main thread */
#ifdef VERBOSE
    nexus_printf ("comm_test_reply_handler(): entering monitor\n");
#endif
    nexus_mutex_lock (&reply_done->mutex);
    reply_done->done = NEXUS_TRUE;
#ifdef VERBOSE
    nexus_printf ("comm_test_reply_handler(): signaling monitor\n");
#endif
    nexus_cond_signal (&reply_done->cond);
    nexus_mutex_unlock (&reply_done->mutex);
   
#ifdef VERBOSE
    nexus_printf ("comm_test_reply_handler(): exiting\n");
#endif
} /* comm_test_reply_handler() */





static void comm_test (nexus_global_pointer_t *remote_gp, int destroy_context)
/*
** DESCRIPTION: This function is the main driver for the communications tests.
** It allocates space for buffers of the desired size for each Nexus type
** and then proceeds to send the requested number of data elements to the
** remote context pointed to by remote_gp.
*/
{
    char                   *char_array;
    double                 *double_array;
    float                  *float_array;
    int                    gp_size;
    int                    gp_n_elements;
    int                    buffer_size;
    int                    *int_array;
    int                    i;
    long                   *long_array;
    nexus_global_pointer_t reply_done_gp;
    nexus_buffer_t         buffer;
    short                  *short_array;
    unsigned char          *uchar_array;
    unsigned int           *uint_array;
    unsigned long          *ulong_array;
    unsigned short         *ushort_array;

    
#ifdef VERBOSE
    nexus_printf ("comm_test(): starting\n");
#endif

    nexus_mutex_init (&comm_test_reply_done.mutex, (nexus_mutexattr_t *) NULL);
    nexus_cond_init (&comm_test_reply_done.cond, (nexus_condattr_t *) NULL);
    nexus_mutex_lock (&comm_test_reply_done.mutex);
    comm_test_reply_done.done = NEXUS_FALSE;
    nexus_mutex_unlock (&comm_test_reply_done.mutex);

    /* set up the reply global pointer to point at the reply done monitor */
    nexus_global_pointer (&reply_done_gp, (void *) &comm_test_reply_done);

    /* invoke handler with a known (but bogus) address */
    nexus_init_remote_service_request (&buffer, remote_gp,
				       "comm_test_init_handler",
				       COMM_TEST_INIT_HANDLER_HASH);
    gp_size = nexus_sizeof_global_pointer (&buffer, &reply_done_gp, 1,
					   &gp_n_elements);
    nexus_set_buffer_size (&buffer, gp_size, gp_n_elements);
    nexus_put_global_pointer (&buffer, &reply_done_gp, 1);
    if (nexus_send_remote_service_request (&buffer) != 0)
    {
	nexus_printf ("comm_test(): ERROR: send failed\n");
	all_tests_okay = NEXUS_FALSE;
    }

    /* float */
    float_array  = (float *) malloc (ending_buffer_size * sizeof (float));
    for (i = 0; i < ending_buffer_size; i++)
    {
	float_array[i] = 42.0;
    }

    for (i = beginning_buffer_size; i <= ending_buffer_size; i = i + step_size)
    {                                         
	nexus_init_remote_service_request (&buffer, remote_gp,
					   "comm_test_receive_handler",
					   COMM_TEST_RECEIVE_HANDLER_HASH);
	buffer_size = nexus_sizeof_float (&buffer, i) +
	    nexus_sizeof_int (&buffer, 2);
	nexus_set_buffer_size (&buffer, buffer_size, 3);
	nexus_put_int (&buffer, &float_type_tag, 1);  /* send tag for type */
	nexus_put_int (&buffer, &i, 1);  /* send number of elements */
	nexus_put_float (&buffer, float_array, i);
	if (nexus_send_remote_service_request (&buffer) != 0)
	{
	    nexus_printf ("comm_test(): ERROR: send failed\n");
	    all_tests_okay = NEXUS_FALSE;
	}
    }

    free (float_array);

    /* double */
    double_array = (double *) malloc (ending_buffer_size * sizeof (double));
    for (i = 0; i < ending_buffer_size; i++)
    {
	double_array[i] = 42.0;
    }

    for (i = beginning_buffer_size; i <= ending_buffer_size; i = i + step_size)
    {
	nexus_init_remote_service_request (&buffer, remote_gp,
					   "comm_test_receive_handler",
					   COMM_TEST_RECEIVE_HANDLER_HASH);
	buffer_size = nexus_sizeof_double (&buffer, i) +
	    nexus_sizeof_int (&buffer, 2);
	nexus_set_buffer_size (&buffer, buffer_size, 3);
	nexus_put_int (&buffer, &double_type_tag, 1);  /* send tag for type */
	nexus_put_int (&buffer, &i, 1);  /* send number of elements */
	nexus_put_double (&buffer, double_array, i);
	if (nexus_send_remote_service_request (&buffer) != 0)
	{
	    nexus_printf ("comm_test(): ERROR: send failed\n");
	    all_tests_okay = NEXUS_FALSE;
	}
    }

    free (double_array);

    /* short */
    short_array  = (short *) malloc (ending_buffer_size * sizeof (short));
    for (i = 0; i < ending_buffer_size; i++)
    {
	short_array[i] = 42;
    }

    for (i = beginning_buffer_size; i <= ending_buffer_size; i = i + step_size)
    {
	nexus_init_remote_service_request (&buffer, remote_gp,
					   "comm_test_receive_handler",
					   COMM_TEST_RECEIVE_HANDLER_HASH);
	buffer_size = nexus_sizeof_short (&buffer, i) +
	    nexus_sizeof_int (&buffer, 2);
	nexus_set_buffer_size (&buffer, buffer_size, 3);
	nexus_put_int (&buffer, &short_type_tag, 1);  /* send tag for type */
	nexus_put_int (&buffer, &i, 1);  /* send number of elements */
	nexus_put_short (&buffer, short_array, i);
	if (nexus_send_remote_service_request (&buffer) != 0)
	{
	    nexus_printf ("comm_test(): ERROR: send failed\n");
	    all_tests_okay = NEXUS_FALSE;
	}
    }

    free (short_array);

    /* unsigned short */
    ushort_array = (unsigned short *) malloc (ending_buffer_size * sizeof(unsigned short));
    for (i = 0; i < ending_buffer_size; i++)
    {
	ushort_array[i] = 42;
    }

    for (i = beginning_buffer_size; i <= ending_buffer_size; i = i + step_size)
    {
	nexus_init_remote_service_request (&buffer, remote_gp,
					   "comm_test_receive_handler",
					   COMM_TEST_RECEIVE_HANDLER_HASH);
	buffer_size = nexus_sizeof_u_short (&buffer, i) +
	    nexus_sizeof_int (&buffer, 2);
	nexus_set_buffer_size (&buffer, buffer_size, 3);
	nexus_put_int (&buffer, &ushort_type_tag, 1);  /* send tag for type */
	nexus_put_int (&buffer, &i, 1);  /* send number of elements */
	nexus_put_u_short (&buffer, ushort_array, i);
	if (nexus_send_remote_service_request (&buffer) != 0)
	{
	    nexus_printf ("comm_test(): ERROR: send failed\n");
	    all_tests_okay = NEXUS_FALSE;
	}
    }

    free (ushort_array);

    /* int */
    int_array = (int *) malloc (ending_buffer_size * sizeof (int));
    for (i = 0; i < ending_buffer_size; i++)
    {
	int_array[i] = 42;
    }

    for (i = beginning_buffer_size; i <= ending_buffer_size; i = i + step_size)
    {
	nexus_init_remote_service_request (&buffer, remote_gp,
					   "comm_test_receive_handler",
					   COMM_TEST_RECEIVE_HANDLER_HASH);
	buffer_size = nexus_sizeof_int (&buffer, i) +
	    nexus_sizeof_int (&buffer, 2);
	nexus_set_buffer_size (&buffer, buffer_size, 3);
	nexus_put_int (&buffer, &int_type_tag, 1);  /* send tag for type */
	nexus_put_int (&buffer, &i, 1);  /* send number of elements */
	nexus_put_int (&buffer, int_array, i);
	if (nexus_send_remote_service_request (&buffer) != 0)
	{
	    nexus_printf ("comm_test(): ERROR: send failed\n");
	    all_tests_okay = NEXUS_FALSE;
	}
    }

    free (int_array);

    /* unsigned int */
    uint_array = (unsigned int *) malloc (ending_buffer_size * sizeof (unsigned int));
    for (i = 0; i < ending_buffer_size; i++)
    {
	uint_array[i] = 42;
    }

    for (i = beginning_buffer_size; i <= ending_buffer_size; i = i + step_size)
    {
	nexus_init_remote_service_request (&buffer, remote_gp,
					   "comm_test_receive_handler",
					   COMM_TEST_RECEIVE_HANDLER_HASH);
	buffer_size = nexus_sizeof_u_int (&buffer, i) +
	    nexus_sizeof_int (&buffer, 2);
	nexus_set_buffer_size (&buffer, buffer_size, 3);
	nexus_put_int (&buffer, &uint_type_tag, 1);  /* send tag for type */
	nexus_put_int (&buffer, &i, 1);  /* send number of elements */
	nexus_put_u_int (&buffer, uint_array, i);
	if (nexus_send_remote_service_request (&buffer) != 0)
	{
	    nexus_printf ("comm_test(): ERROR: send failed\n");
	    all_tests_okay = NEXUS_FALSE;
	}
    }

    free (uint_array);

    /* long */
    long_array = (long *) malloc (ending_buffer_size * sizeof (long));
    for (i = 0; i < ending_buffer_size; i++)
    {
	long_array[i] = 42.;
    }

    for (i = beginning_buffer_size; i <= ending_buffer_size; i = i + step_size)
    {
	nexus_init_remote_service_request (&buffer, remote_gp,
					   "comm_test_receive_handler",
					   COMM_TEST_RECEIVE_HANDLER_HASH);
	buffer_size = nexus_sizeof_long (&buffer, i) +
	    nexus_sizeof_int (&buffer, 2);
	nexus_set_buffer_size (&buffer, buffer_size, 3);
	nexus_put_int (&buffer, &long_type_tag, 1);  /* send tag for type */
	nexus_put_int (&buffer, &i, 1);  /* send number of elements */
	nexus_put_long (&buffer, long_array, i);
	if (nexus_send_remote_service_request (&buffer) != 0)
	{
	    nexus_printf ("comm_test(): ERROR: send failed\n");
	    all_tests_okay = NEXUS_FALSE;
	}
    }

    free (long_array);

    /* unsigned long */
    ulong_array  = (unsigned long *) malloc (ending_buffer_size * sizeof (unsigned long));
    for (i = 0; i < ending_buffer_size; i++)
    {
	ulong_array[i] = 42;
    }

    for (i = beginning_buffer_size; i <= ending_buffer_size; i = i + step_size)
    {
	nexus_init_remote_service_request (&buffer, remote_gp,
					   "comm_test_receive_handler",
					   COMM_TEST_RECEIVE_HANDLER_HASH);
	buffer_size = nexus_sizeof_u_long (&buffer, i) +
	    nexus_sizeof_int (&buffer, 2);
	nexus_set_buffer_size (&buffer, buffer_size, 3);
	nexus_put_int (&buffer, &ulong_type_tag, 1);  /* send tag for type */
	nexus_put_int (&buffer, &i, 1);  /* send number of elements */
	nexus_put_u_long (&buffer, ulong_array, i);
	if (nexus_send_remote_service_request (&buffer) != 0)
	{
	    nexus_printf ("comm_test(): ERROR: send failed\n");
	    all_tests_okay = NEXUS_FALSE;
	}
    }

    free (ulong_array);

    /* char */
    char_array   = (char *) malloc (ending_buffer_size * sizeof (char));
    for (i = 0; i < ending_buffer_size; i++)
    {
	char_array[i] = 42;
    }

    for (i = beginning_buffer_size; i <= ending_buffer_size; i = i + step_size)
    {
	nexus_init_remote_service_request (&buffer, remote_gp,
					   "comm_test_receive_handler",
					   COMM_TEST_RECEIVE_HANDLER_HASH);
	buffer_size = nexus_sizeof_char (&buffer, i) +
	    nexus_sizeof_int (&buffer, 2);
	nexus_set_buffer_size (&buffer, buffer_size, 3);
	nexus_put_int (&buffer, &char_type_tag, 1);  /* send tag for type */
	nexus_put_int (&buffer, &i, 1);  /* send number of elements */
	nexus_put_char (&buffer, char_array, i);
	if (nexus_send_remote_service_request (&buffer) != 0)
	{
	    nexus_printf ("comm_test(): ERROR: send failed\n");
	    all_tests_okay = NEXUS_FALSE;
	}
    }

    free (char_array);

    /* unsigned char */
    uchar_array  = (unsigned char *) malloc (ending_buffer_size * sizeof (unsigned char));
    for (i = 0; i < ending_buffer_size; i++)
    {
	uchar_array[i] = 42;
    }

    for (i = beginning_buffer_size; i <= ending_buffer_size; i = i + step_size)
    {
	nexus_init_remote_service_request (&buffer, remote_gp,
					   "comm_test_receive_handler",
					   COMM_TEST_RECEIVE_HANDLER_HASH);
	buffer_size = nexus_sizeof_u_char (&buffer, i) +
	    nexus_sizeof_int (&buffer, 2);
	nexus_set_buffer_size (&buffer, buffer_size, 3);
	nexus_put_int (&buffer, &uchar_type_tag, 1);  /* send tag for type */
	nexus_put_int (&buffer, &i, 1);  /* send number of elements */
	nexus_put_u_char (&buffer, uchar_array, i);
	if (nexus_send_remote_service_request (&buffer) != 0)
	{
	    nexus_printf ("comm_test(): ERROR: send failed\n");
	    all_tests_okay = NEXUS_FALSE;
	}
    }

    free (uchar_array);

#ifdef ATTACH_TEST
    /* die unexpectedly to test fault tolerance */
    if (attach_client_die)
    {
	exit (0);
    }
#endif /* ATTACH_TEST */

    nexus_init_remote_service_request (&buffer, remote_gp,
				       "comm_test_finish_handler",
				       COMM_TEST_FINISH_HANDLER_HASH);

    buffer_size = nexus_sizeof_int (&buffer, 1);
    nexus_set_buffer_size (&buffer, buffer_size, 1);
    nexus_put_int (&buffer, &destroy_context, 1);
    if (nexus_send_remote_service_request (&buffer) != 0)
    {
	nexus_printf ("comm_test(): ERROR: send failed\n");
	all_tests_okay = NEXUS_FALSE;
    }

    /* suspend main thread until handler completes */
#ifdef VERBOSE
    nexus_printf ("comm_test(): entering monitor\n");
#endif
    nexus_mutex_lock (&comm_test_reply_done.mutex);
#ifdef VERBOSE
    nexus_printf ("comm_test(): waiting on monitor\n");
#endif
    while (!comm_test_reply_done.done)
    {
	nexus_cond_wait (&comm_test_reply_done.cond,
			 &comm_test_reply_done.mutex);
    }
#ifdef VERBOSE
    nexus_printf ("comm_test(): awoke from monitor and exiting\n");
#endif
    nexus_mutex_unlock (&comm_test_reply_done.mutex);

    nexus_mutex_destroy (&comm_test_reply_done.mutex);
    nexus_cond_destroy (&comm_test_reply_done.cond);
    nexus_destroy_global_pointer (&reply_done_gp);

#ifdef VERBOSE
    nexus_printf ("comm_test(): complete\n");
#endif
} /* comm_test() */





static void local_comm_test (void)
/*
** DESCRIPTION: This function invokes comm_test() with a global pointer that
** points to the local context.  This has the effect of running a
** communications test inside the current context and node.
*/
{
    nexus_global_pointer_t gp;


#ifdef VERBOSE
    nexus_printf ("local_comm_test(): starting\n");
#endif
    nexus_global_pointer (&gp, NULL);
    comm_test (&gp, NEXUS_FALSE);
    nexus_destroy_global_pointer (&gp);    
#ifdef VERBOSE
    nexus_printf ("local_comm_test(): complete\n");
#endif
} /* local_comm_test() */





static void node_comm_test (nexus_node_t *nodes, int n_nodes)
/*
** DESCRIPTION: This function invokes comm_test() with each of the global
** pointers in the nodes[] structure.  This has the effect of running a
** communications test to each of the nodes specified to Nexus at startup
** in the package arguments.
*/
{
    int i;


#ifdef VERBOSE
    nexus_printf ("node_comm_test(): starting\n");
#endif

    /* run comm_test() to each node's main context */
    for (i = 0; i < n_nodes; i++)
    {
#ifdef VERBOSE
	nexus_printf ("node_comm_test(): running to node %d\n", i);
#endif
	comm_test (&nodes[i].gp, NEXUS_FALSE);
#ifdef VERBOSE
	nexus_printf ("node_comm_test(): completed to node %d\n", i);
#endif
    }

#ifdef VERBOSE
    nexus_printf ("node_comm_test(): complete\n");
#endif
} /* node_comm_test() */





static void remote_comm_test (nexus_node_t *nodes, int n_nodes)
/*
** DESCRIPTION: This function creates a new context on each node in the
** nodes[] array and then invokes comm_test() for each of these new contexts.
*/
{
    int                           i;
    int                           n_contexts;
    int                           rc[256];
    nexus_create_context_handle_t contexts;
    nexus_global_pointer_t        gp[256];


#ifdef VERBOSE
    nexus_printf ("remote_comm_test(): starting\n");
#endif

    /* create one context on each node */
    n_contexts = MIN (256, n_nodes);

    nexus_init_create_context_handle (&contexts, n_contexts);
    for (i = 0; i < n_contexts; i++)
    {
#ifdef VERBOSE
	nexus_printf ("remote_comm_test(): creating context %d\n", i);
#endif
	nexus_create_context(&nodes[i].gp, TEST_EXE, i, &gp[i], &rc[i], &contexts);
    }

    nexus_create_context_wait (&contexts);
#ifdef VERBOSE
    nexus_printf ("remote_comm_test(): all contexts created\n");
#endif

#ifdef SHUTDOWN_NODES_BEFORE_CONTEXTS    
#ifdef VERBOSE
    nexus_printf ("remote_comm_test(): shutting down nodes\n");
#endif
    if (n_nodes > 1)
    {
	shutdown_nodes(nodes, n_nodes);
    }
#endif

    /* check return codes and run comm_test() to each context */
    for (i = 0; i < n_contexts; i++) 
    {
	if (rc[i] != 0)
	{
	    nexus_printf ("remote_comm_test(): ERROR: bad return code (%d) for context %d\n", rc[i], i);
	    all_tests_okay = NEXUS_FALSE;
	}
	else
	{
#ifdef VERBOSE
	    nexus_printf ("remote_comm_test(): running on context %d\n", i);
#endif
	    comm_test (&gp[i], NEXUS_TRUE);
#ifdef VERBOSE
	    nexus_printf ("remote_comm_test(): completed on context %d\n", i);
#endif
	}
    }

#ifdef VERBOSE
    nexus_printf ("remote_comm_test(): complete\n");
#endif
} /* remote_comm_test() */





int fault_callback (void *user_arg, int fault_code)
/*
** DESCRIPTION: This is the callback routine for the
** nexus_enable_fault_tolerance() call.
*/
{
#ifdef VERBOSE
    nexus_printf ("fault_callback(): fault_code=%d: %s\n",
		  fault_code, nexus_fault_strings[fault_code]);
#endif
    return (0);
} /* fault_callback() */





int attach_requested (void *user_arg, char *url, nexus_global_pointer_t *gp)
/*
** DESCRIPTION: This is the attach callback handler used in
** nexus_allow_attach() when an attachment to the current Nexus computation
** is made.
*/
{
    char           *host;
    unsigned short port;
    char           **specs;
    int            i;

    
#ifdef VERBOSE
    nexus_printf ("attach_requested(): entering\n");
    nexus_printf ("attach_requested(): url=%s\n", url);
#endif

    nexus_split_nexus_url (url, &host, &port, &specs);
#ifdef VERBOSE
    nexus_printf ("attach_requested(): host=%s\n", host);
    nexus_printf ("attach_requested(): port=%hu\n", port);
#endif
    for (i = 0; specs[i]; i++)
    {
#ifdef VERBOSE
	nexus_printf ("attach_requested(): spec %d: <%s>\n", i, specs[i]);
#endif
    }
    nexus_split_nexus_url_free (&host, &specs);
    
    nexus_global_pointer (gp, (void *) 12345);
    
#ifdef VERBOSE
    nexus_printf ("attach_requested(): exiting\n");
#endif
    return (0);
} /* attach_requested() */





static void attach_server (void)
/*
** DESCRIPTION: This function is the server for the attach test.  The function
** allows attachments to the computation and then waits for the attach test
** monitor to signal that the server can end.  While the attach server is
** waiting, the attach client will presumably invoke handlers within the
** server's context.
*/
{
    char           *host;
    char           *host2;
    int            rc;
    unsigned short port;


#ifdef VERBOSE
    nexus_printf ("attach_server(): entering\n");
#endif
    nexus_enable_fault_tolerance (fault_callback, NULL);
    nexus_mutex_init(&attach_server_monitor.mutex, (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&attach_server_monitor.cond, (nexus_condattr_t *) NULL);
    attach_server_monitor.done = NEXUS_FALSE;

#ifdef VERBOSE
    nexus_printf ("attach_server(): allowing attachment\n");
#endif
    port = attach_server_port;
    if ((rc = nexus_allow_attach (&port, &host, &attach_requested, NULL)) != 0)
    {
	nexus_printf ("attach_server(): ERROR: nexus_allow_attach() did not return 0: rc=%d\n", rc);
	all_tests_okay = NEXUS_FALSE;
    }
    if ((rc = nexus_allow_attach (&port, &host2, &attach_requested, NULL)) != 1)
    {
	nexus_printf ("attach_server(): ERROR: second nexus_allow_attach() did not return 1: rc=%d\n\n", rc);
	all_tests_okay = NEXUS_FALSE;
    }

    nexus_printf ("attach_server(): ATTACH TO: %s %hu\n", host, port);

    nexus_printf ("attach_server(): waiting on monitor\n");
    nexus_mutex_lock (&attach_server_monitor.mutex);
    while (!attach_server_monitor.done)
    {
	nexus_cond_wait (&attach_server_monitor.cond,
			 &attach_server_monitor.mutex);
    }
    nexus_mutex_unlock (&attach_server_monitor.mutex);
#ifdef VERBOSE
    nexus_printf ("attach_server(): awoke from monitor\n");
#endif

    nexus_disallow_attach (port);

    nexus_mutex_destroy (&attach_server_monitor.mutex);
    nexus_cond_destroy (&attach_server_monitor.cond);

#ifdef VERBOSE
    nexus_printf ("attach_server(): exiting\n");
#endif
} /* attach_server() */





static void attach_client (void)
/*
** DESCRIPTION: This function is the client for the attach test.  It creates
** a url for the server computation to attach to and then calls nexus_attach()
** to make the attachment.  Once the attachment has been made, it calls
** comm_test() to exercise the connection.  Finally, the client invokes a
** remote service request on the server to signal it to shut down at the end
** of the test.
*/
{
    char                   url[1024];
    int                    rc;
    nexus_buffer_t         buffer;
    nexus_global_pointer_t gp;


#ifdef VERBOSE
    nexus_printf ("attach_client(): entering\n");
#endif

    nexus_enable_fault_tolerance (fault_callback, NULL);

    sprintf (url, "x-nexus://%s:%hu/spec1//spec\\/3/spec4", attach_client_host,
	     attach_client_port);

#ifdef VERBOSE
    nexus_printf ("attach_client(): attaching to url <%s>\n", url);
#endif
    rc = nexus_attach (url, &gp);
    if (rc == 0)
    {
#ifdef VERBOSE
	nexus_printf ("attach_client(): running comm_test() to server\n");
#endif
	comm_test (&gp, NEXUS_FALSE);
#ifdef VERBOSE
	nexus_printf ("attach_client(): done running comm_test() to server\n");
#endif

	if (!attach_client_leave_server)
	{
	    /* send rsr to shut the server down */
#ifdef VERBOSE
	    nexus_printf ("attach_client(): sending finish message to server\n");
#endif
	    nexus_init_remote_service_request(&buffer, &gp,
					      "attach_server_finish_handler",
					      ATTACH_SERVER_FINISH_HANDLER_HASH);
	    if (nexus_send_remote_service_request (&buffer) != 0)
	    {
		nexus_printf ("attach_client(): ERROR: send failed\n");
		all_tests_okay = NEXUS_FALSE;
	    }
	}

#ifdef VERBOSE
	nexus_printf ("attach_client(): destroying gp to server\n");
#endif
	nexus_destroy_global_pointer (&gp);
    }
    else
    {
	nexus_printf ("attach_client(): ERROR: nexus_attach() failed with rc=%d\n", rc);
	all_tests_okay = NEXUS_FALSE;
    }

#ifdef VERBOSE
    nexus_printf ("attach_client(): exiting\n");
#endif
} /* attach_client() */





static void attach_server_finish_handler (void *address,
                                          nexus_buffer_t *buffer)
/*
** DESCRIPTION: This function is a handler that is invoked by the attach client
** to signal the attach server that it should shut down because the attach
** test has completed.
*/
{
#ifdef VERBOSE
    nexus_printf ("attach_server_finish_handler(): entering\n");
#endif

#ifdef VERBOSE
    nexus_printf ("attach_server_finish_handler(): signaling\n");
#endif
    nexus_mutex_lock (&attach_server_monitor.mutex);
    attach_server_monitor.done = NEXUS_TRUE;
    nexus_cond_signal (&attach_server_monitor.cond);
    nexus_mutex_unlock (&attach_server_monitor.mutex);

#ifdef VERBOSE
    nexus_printf ("attach_server_finish_handler(): exiting\n");
#endif
} /* attach_server_finish_handler() */





void main (int argc, char **argv)
/*
** PROGRAMMER: Unknown
** UPDATED:    Greg Koenig, Argonne National Laboratory, 06-FEB-1996
**
** DESCRIPTION: This program exercises the communications routines in Nexus.
** The program consists of three main components: a local communications test,
** a server for an attach test, and a client for an attach test.  The user
** indicates which of the three main tests should be invoked by supplying
** command-line parameters when the executable is started.
**
** After selecting the test to run, the program executes a communications
** test routine which sends buffers of various sizes to the context(s)
** appropriate for the test selected.  These buffers contain variables of
** each of the types supported by Nexus.
**
** Invocation of this program is as follows:
**
**    First, the user can specify the first parameter as "testcomm" to signal
**    the program to execute basic communications tests.  If this test is
**    selected, the program expects to receive a beginning buffer size, an
**    ending buffer size, and a step size for the sizes of buffers desired by
**    the user.  For example:
**
**                % test_comm testcomm 1 1001 100
**
**    Second, the user can specify the first parameter as "server" to signal
**    the program that it is to act as an attach test server.  If this test is
**    selected, the program expects to receive a TCP port number that the
**    server is to listen for attachments on.  For example:
**
**                % test_comm server 7000
**
**    Third, the user can specify the first parameter as "client" to signal
**    the program that it is to act as an attach test client.  If this test is
**    selected, the program expects to receive a hostname and TCP port number
**    for the machine where the server is running, and a beginning buffer size,
**    and ending buffer size, and a step size for the sizes of buffers desired
**    the the user.  For example:
**
**                % test_comm client dalek.mcs.anl.gov 7000 1 1001 100
**
** If the user completely disregards command-line parameters, the program
** assumes a basic communications test with simple default values.
*/
{
    char         **my_argv;
    int          i;
    int          my_argc = 0;
    int          n_nodes;
    nexus_node_t *nodes;


    nexus_init (&my_argc,
		&my_argv,
		"NEXUS_ARGS",
		"nx",
		NULL,
		NULL,
		NULL,
		NULL,
		&nodes,
		&n_nodes);

    nexus_start();

    all_tests_okay = NEXUS_TRUE;

    /* print out the nodes array */
#ifdef VERBOSE
    for (i = 0; i < n_nodes; i++)
    {
	nexus_printf ("nodes[%d]: %s#%d, rc=%d\n",
		      i, nodes[i].name, nodes[i].number, nodes[i].return_code);
    }
#endif

    if (my_argc == 1)
    {
	/* the user didn't specify any parameters, assume some defaults */
	beginning_buffer_size = 1;
	ending_buffer_size = 1001;
	step_size = 100;

	local_comm_test ();
	node_comm_test (nodes, n_nodes);
	remote_comm_test (nodes, n_nodes);
    }
    else
    {
	/* the user specified something, so let's go with that */
	if (!strcmp (argv[1], "server"))
	{
            if (argc == 2)
	    {
		attach_server_port = 7000;
	    }
	    else
	    {
		attach_server_port = (short) atoi (argv[2]);
	    }
	    attach_server ();
	}
	else
	{
	    if (!strcmp (argv[1], "client"))
	    {
		strcpy (attach_client_host, argv[2]);
		attach_client_port = (short) atoi (argv[3]);
		if (argc == 4)
		{
		    beginning_buffer_size = 1;
		    ending_buffer_size = 1001;
		    step_size = 100;
		}
		else
		{
		    beginning_buffer_size = atoi (argv[4]);
		    ending_buffer_size = atoi (argv[5]);
		    step_size = atoi (argv[6]);
		}
		attach_client ();
	    }
	    else
	    {
		if (!strcmp (argv[1], "testcomm"))
		{
		    beginning_buffer_size = atoi (argv[2]);
		    ending_buffer_size = atoi (argv[3]);
		    step_size = atoi (argv[4]);

		    local_comm_test ();
		    node_comm_test (nodes, n_nodes);
		    remote_comm_test (nodes, n_nodes);
		}
	    }
	}
    }
    
    for (i = 0; i < n_nodes; i++)
    {
	nexus_free (nodes[i].name);
	nexus_destroy_global_pointer (&nodes[i].gp);
    }
    nexus_free (nodes);

#ifndef VERBOSE
    if (all_tests_okay)
    {
	printf ("All tests in this module completed successfully.\n");
	nexus_exit (0, NEXUS_TRUE);
    }
    else
    {
	printf ("There were errors in some of the tests in this module.\n");
	nexus_exit (1, NEXUS_TRUE);
    }
#endif

#ifdef VERBOSE
    nexus_destroy_current_context (NEXUS_FALSE);
#endif

    printf("main(): ERROR: we should never get here\n");
} /* main() */
