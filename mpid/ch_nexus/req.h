#ifndef MPIR_REQUEST_COOKIE

#include "util.h"

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

/*
 * Locking info for request queues.
 */
nexus_mutex_t message_queue_lock;

/*
 * Modified definitions of the request.  The "device" handle has been 
 * integrated.
 * 
 * Should consider separating persistent from non-persistent requests.
 * Note that much of the information a send request is not needed
 * once the send envelope is sent, unless it is a persistent request.
 * This includes the contextid, dest, tag, and datatype.  The comm
 * might be needed for error handling, but maybe not.
 *
 * If persistent is a handle_type, we save another store.
 */

/* MPIR_COMMON is a subset of the handle structures that contains JUST
   the handle type and the Cookie.
 */
/* User not yet supported */
typedef enum {
    MPIR_SEND,
    MPIR_RECV,
    MPIR_PERSISTENT_SEND,
    MPIR_PERSISTENT_RECV /*,
    MPIR_USER */
} MPIR_OPTYPE;

#define MPIR_REQUEST_COOKIE 0xe0a1beaf
typedef struct {
    MPIR_OPTYPE handle_type;
    MPIR_COOKIE                 /* Cookie to help detect valid item */
    int is_complete;
    nexus_endpoint_t endpoint;
    } MPIR_COMMON;

/*
   In the case of a send handle, the information is provided separately.
   All we need is enough information to dispatch the message and to 
   deliver the data (in a rendezvous/get setting).  Some sophisticated
   devices may be able to use an struct MPIR_DATATYPE directly; they should
   add an struct MPIR_DATATYPE field if they need it.
 */
typedef struct _MPIR_SHANDLE MPIR_SHANDLE;
struct _MPIR_SHANDLE {
    MPIR_OPTYPE  handle_type;    
    MPIR_COOKIE                   /* Cookie to help detect valid item */
    int          is_complete;     /* Indicates if complete */
    nexus_endpoint_t endpoint;
    int          start; /* ??? */
    void *       bsend; /* ??? */
    int          errval;          /* Holds any error code; 0 for none */
    struct MPIR_COMMUNICATOR     *comm;            /* Do we need this?  */

    /* Device data */
    nexus_cond_t   *cond;     /* condition thread is waiting on to be
                               * notified of the request's finish.  NULL
                               * indicates that no thread is waiting on
			       * the request. 
                               */
};

/* 
 * A receive request is VERY different from a send.  We need to 
 * keep the information about the message we want in the request,
 * as well as how to accept it.
 */
typedef struct _MPIR_RHANDLE MPIR_RHANDLE;
struct _MPIR_RHANDLE {
    MPIR_OPTYPE  handle_type;    
    MPIR_COOKIE                /* Cookie to help detect valid item */
    int          is_complete;  /* Indicates is complete */
    nexus_endpoint_t endpoint;
    MPI_Status   s;            /* Status of data */
    int          context_id;  
    void         *buf;         /* address of buffer */
    int          len;          /* length of buffer at bufadd in bytes */

    /* Device data */
    struct MPIR_DATATYPE *type;        /* basic or derived datatype */
    int          count;
    nexus_bool_t is_freed;
    nexus_startpoint_t sp;   /* for synchronous sends, this is the
			       * sp that should be used to notify the
			       * sender of the completion of the
			       * receive.  NULL indicates a
			       * non-synchronous send.
			       */
    nexus_startpoint_t* sptr; /* This will either be a pointer to "sp" or
				NULL. It is used to test for a synchronous
				send. */
    
    nexus_cond_t   *cond;     /* condition thread is waiting on to be
                               * notified of the request's finish.  NULL
                               * indicates that no thread is waiting on
			       * the request. 
                               */
    nexus_buffer_t recv_buf;  /* Nexus data */
};

typedef struct {
    MPIR_RHANDLE rhandle;
    int          active;
    int          perm_tag, perm_source, perm_count;
    void         *perm_buf;
    struct MPIR_DATATYPE *perm_datatype;
    struct MPIR_COMMUNICATOR     *perm_comm;
    } MPIR_PRHANDLE;

typedef struct {
    MPIR_SHANDLE shandle;
    int          active;
    int          perm_tag, perm_dest, perm_count;
    void         *perm_buf;
    struct MPIR_DATATYPE *perm_datatype;
    struct MPIR_COMMUNICATOR     *perm_comm;
    void         (*send) ANSI_ARGS((struct MPIR_COMMUNICATOR *, void *, int, struct MPIR_DATATYPE *, 
				    int, int, int, int, MPI_Request, int *));
                    /* IsendDatatype, IssendDatatype, Ibsend, IrsendDatatype */
    } MPIR_PSHANDLE;
	
/* This is an "extension" handle and is NOT part of the MPI standard.
   Defining it, however, introduces no problems with the standard, and
   it allows us to easily extent the request types.

   Note that this is not yet compatible with the essential fields of
   MPIR_COMMON.
 */
typedef struct {
    MPIR_OPTYPE handle_type;    
    MPIR_COOKIE                 /* Cookie to help detect valid item */
    int         is_complete;    /* Is request complete? */
    int         active;         /* Should this be ignored? */
    int         (*create_ureq) ANSI_ARGS((MPI_Request));
    int         (*free_ureq)   ANSI_ARGS((MPI_Request));
    int         (*wait_ureq)   ANSI_ARGS((MPI_Request));
    int         (*test_ureq)   ANSI_ARGS((MPI_Request));
    int         (*start_ureq)  ANSI_ARGS((MPI_Request));
    int         (*cancel_ureq) ANSI_ARGS((MPI_Request));
    void        *private_data;
} MPIR_UHANDLE;

#define MPIR_HANDLES_DEFINED

union MPIR_HANDLE {
    MPIR_OPTYPE   handle_type;   
    MPIR_COMMON   chandle;       /* common fields */
    MPIR_SHANDLE  shandle;
    MPIR_RHANDLE  rhandle;
    MPIR_PSHANDLE persistent_shandle;
    MPIR_PRHANDLE persistent_rhandle;
    MPIR_UHANDLE  uhandle;
};

#define MPID_Request_init( ptr, in_type ) { \
		      memset(ptr,0,sizeof(*(ptr)));\
		      (ptr)->handle_type = in_type;\
		      MPIR_SET_COOKIE((ptr),MPIR_REQUEST_COOKIE);}
#endif
