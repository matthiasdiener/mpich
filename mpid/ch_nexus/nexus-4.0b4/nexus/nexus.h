/*
 * nexus.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/nexus.h,v 1.117 1997/02/24 21:45:43 tuecke Exp $"
 *
 * This header contains the exported interface of Nexus.
 */

#ifndef _NEXUS_INCLUDE_NEXUS_H
#define _NEXUS_INCLUDE_NEXUS_H

/* EXTERN_C_BEGIN and EXTERN_C_END should surround any Nexus prototypes in
   nexus.h or Nexus .h files that are included by nexus.h.  This will
   allow C++ codes to include nexus.h and properly link against the
   Nexus library.
*/
#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

EXTERN_C_BEGIN

#define NEXUS_VERSION_MAJOR 4
#define NEXUS_VERSION_MINOR 0

#ifdef _ALL_SOURCE
#include <standards.h>
#endif

/*
 * nexus_path_type_t
 */
typedef enum _nexus_path_type_t
{
    NEXUS_PATH_TYPE_DATABASE = 1,
    NEXUS_PATH_TYPE_LISTENER = 2,
    NEXUS_PATH_MAXVAL
} nexus_path_type_t;

    
/*
 * NEXUS_PROTO_TYPE_*
 *
 * Each different Nexus protocol module should have a unique
 * type identifier for all times...
 */
typedef enum _nexus_proto_type_t
{
    NEXUS_PROTO_TYPE_LOCAL	= 0,
    NEXUS_PROTO_TYPE_TCP	= 1,
    NEXUS_PROTO_TYPE_SHMEM	= 2,
    NEXUS_PROTO_TYPE_INX	= 3,
    NEXUS_PROTO_TYPE_PVM	= 4,
    NEXUS_PROTO_TYPE_EUI	= 5,
    NEXUS_PROTO_TYPE_ATM	= 6,
    NEXUS_PROTO_TYPE_MPL	= 7,
    NEXUS_PROTO_TYPE_MPINX	= 8,
    NEXUS_PROTO_TYPE_UDP	= 9,
    NEXUS_PROTO_TYPE_MN_UDP	= 10,
    NEXUS_PROTO_TYPE_MAXVAL, /* do not assign a value.. should float to one plus maximum valid number */
    NEXUS_PROTO_TYPE_INTERNAL1	= 1000001,
    NEXUS_PROTO_TYPE_INTERNAL2	= 1000002
} nexus_proto_type_t;

EXTERN_C_END

#include <nexus_config.h>

#if defined(BUILD_LITE) && !defined(PORTS0_LITE)
#define PORTS0_LITE
#endif

typedef int		nexus_bool_t;

/*
 * Include the various Nexus Component Library headers.
 */
#include "nexus_dc.h"

/*********************************************************************
 * General defines and macros
 *********************************************************************/

EXTERN_C_BEGIN

#ifndef NEXUS_SUCCESS
#define NEXUS_SUCCESS 0
#define NEXUS_FAILURE 1
#endif

#ifndef NEXUS_TRUE
#define	NEXUS_TRUE		1
#define	NEXUS_FALSE		0
#endif

#define NEXUS_MAX(V1,V2) (((V1) > (V2)) ? (V1) : (V2))
#define NEXUS_MIN(V1,V2) (((V1) < (V2)) ? (V1) : (V2))

#ifdef NEXUS_DEFINE_GLOBALS
#define NEXUS_GLOBAL
#else
#define NEXUS_GLOBAL extern
#endif

#define NEXUS_MAX_LIBA_SIZE 64

#ifndef NEXUS_DEFAULT_LIBA_SIZE
#define NEXUS_DEFAULT_LIBA_SIZE (2*sizeof(unsigned long))
#endif

#define NEXUS_MAX_EXECUTABLE_PATH_LENGTH 1023

#define NEXUS_MAX_DEBUG_LEVELS 10

#ifdef BUILD_DEBUG
#define NexusAssert(assertion) \
    if (!(assertion)) \
    { \
        nexus_fatal("Assertion " #assertion " failed in file %s at line %d\n",\
		   __FILE__, __LINE__); \
    }
#define NexusAssert2(assertion, print_args) \
    if (!(assertion)) \
    { \
        nexus_fatal("Assertion " #assertion " failed in file %s at line %d: %s", \
		   __FILE__, __LINE__, nexus_assert_sprintf print_args); \
    }
#else /* BUILD_DEBUG */
#define NexusAssert(assertion)
#define NexusAssert2(assertion, print_args)
#endif /* BUILD_DEBUG */


#define NexusMalloc(Func, Var, Type, Size) \
{ \
    if ((Size) > 0) \
    { \
	if (((Var) = (Type) ports0_malloc (Size)) == (Type) NULL) \
	{ \
	    nexus_fatal("%s: malloc of size %d failed for %s %s in file %s line %d\n", \
                        #Func, (Size), #Type, #Var, __FILE__, __LINE__); \
	} \
    } \
    else \
    { \
	(Var) = (Type) NULL; \
    } \
}

#define NexusFree(Ptr) \
{ \
    if ((Ptr) != NULL) \
    { \
	ports0_free(Ptr); \
    } \
}


/*********************************************************************
 * Debug state stuff
 *********************************************************************/

typedef struct _nexus_debug_state_t
{
    unsigned long catagory;
    unsigned long module;
    unsigned long operation;
    unsigned long level;
} nexus_debug_state_t;

#define NXDBG_NONE	0x00000000
#define NXDBG_ALL	0xffffffff
#define NXDBG_USER	0x00000001
#define NXDBG_CORE	0x00000002
#define NXDBG_PR	0x00000004
#define NXDBG_ST	0x00000008
#define NXDBG_DB	0x00000010
#define NXDBG_TH	0x00000020

/* CORE modules */
#define NXDBG_C_INIT		0x00000001
#define NXDBG_C_NODELOCK	0x00000002
#define NXDBG_C_PROCESS		0x00000004
#define NXDBG_C_CONTEXT		0x00000008
#define NXDBG_C_HANDLER		0x00000010

/* PR modules */
#define NXDBG_PR_IFACE		0x00000001
#define NXDBG_PR_LOCAL		0x00000002
#define NXDBG_PR_TCP		0x00000004
#define NXDBG_PR_MPL		0x00000008
#define NXDBG_PR_INX		0x00000010
#define NXDBG_PR_RX		0x00000020
#define NXDBG_PR_MPINX		0x00000040

/* ST modules */
#define NXDBG_ST_IFACE		0x00000001
#define NXDBG_ST_FORK		0x00000002
#define NXDBG_ST_RSH		0x00000004
#define NXDBG_ST_IBMDS		0x00000008
#define NXDBG_ST_INX		0x00000010
#define NXDBG_ST_MPINX		0x00000020
#define NXDBG_ST_MPL		0x00000040
#define NXDBG_ST_SOLDL		0x00000080
#define NXDBG_ST_SS		0x00000100
#define NXDBG_ST_NS		0x00000200

/* DB modules */
#define NXDBG_DB_IFACE		0x00000001
#define NXDBG_DB_FILE		0x00000002

/* TH modules */


/*********************************************************************
 * Data tranform defines
 *********************************************************************/

#define NEXUS_MAX_TRANSFORM_INFO_SIZE 64

#define NEXUS_TRANSFORM_NONE    0
#define NEXUS_TRANSFORM_GZIP_ID 1
#define NEXUS_TRANSFORM_DES_CBC 2
#define NEXUS_TRANSFORM_DES_ECB 3

typedef void nexus_transformattr_t;
typedef void nexus_transformstate_t;


/*********************************************************************
 * Some forward typedef declarations...
 *********************************************************************/

typedef struct _nexus_proto_funcs_t	nexus_proto_funcs_t;
typedef struct _nexus_buffer_t *	nexus_buffer_t;
typedef struct _nexus_endpointattr_t    nexus_endpointattr_t;
typedef struct _nexus_endpoint_t        nexus_endpoint_t;
typedef struct _nexus_startpoint_t      nexus_startpoint_t;
typedef struct _nexus_proto_t		nexus_proto_t;
typedef struct _nexus_mi_proto_t	nexus_mi_proto_t;


/*********************************************************************
 * Handler stuff
 *********************************************************************/

typedef void (*nexus_handler_func_t)(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);
typedef void (*nexus_unknown_handler_func_t)(nexus_endpoint_t *endpoint,
					     nexus_buffer_t *buffer,
					     int handler_id);

typedef enum _nexus_handler_type_t
{
    NEXUS_HANDLER_TYPE_THREADED		= 0,
    NEXUS_HANDLER_TYPE_NON_THREADED	= 1
} nexus_handler_type_t;

typedef struct _nexus_handler_t {
    nexus_handler_type_t	type;
    nexus_handler_func_t	func;
} nexus_handler_t;

/* System handlers */
#define NEXUS_CONTEXT_CREATE_HANDLER_ID -1


/*
 * Other typedefs...
 */
typedef void *(*nexus_thread_func_t)(void *user_arg);

/*
 * JGG
 *  do we want to keep this?
 * JGG
 */
typedef void (*nexus_skip_poll_callback_t)(int *skip_poll,
					   nexus_proto_type_t proto,
					   nexus_bool_t message_handled);


/*********************************************************************
 * nexus_module_list_t
 *********************************************************************/

/*
 * The user must provide a variable containing an initial set of
 * modules that are staticly linked into the executable.
 * This variables contains an array of the triples:
 *	{family_name, module_name, info_function_ptr}
 * Where:
 *   family_name is:
 *	"startups"	startup module
 *	"protocols"	protocol module
 *	"database"	database module
 *	"transform"	transform module
 *   module_name is a unique (within the family) string name for the module.
 *   info_function_ptr is a function pointer to the module's info function.
 *
 * The general form of a module information function is
 *	_nx_<family_name>_<module_name>_info().
 * So the user might supply a table that looks like:
 *
 * extern void *_nx_st_rsh_info(void);
 * extern void *_nx_pr_tcp_info(void);
 * nexus_module_list_t nexus_module_list[] =
 * {
 *     {"startups", "rsh", _nx_st_rsh_info},
 *     {"protocols", "tcp", _nx_pr_tcp_info},
 *     {NULL, NULL, NULL},
 * };
 *
 * This would specify that the rsh startup module, and the tcp protocol
 * module are staticly linked into the program.
 *
 * Note: On machines that provide dynamic linking, this list can be
 * empty.  All necessary modules will be dynamically loaded.  However,
 * linking some in using this list may provide for quicker startup.
 * Also, some startup modules need to be staticly linked to bootstrap
 * properly.
 */
typedef struct _nexus_module_list_t
{
    char *family_name;
    char *module_name;
    void *(*info_func)(void);
} nexus_module_list_t;
/*
extern nexus_module_list_t nexus_module_list[];
*/

EXTERN_C_END


/*********************************************************************
 * Include ports0
 *********************************************************************/
#include "ports0.h"


/*********************************************************************
 * Include sanity checking header stuff
 *********************************************************************/
#include "nx_sanity.h"


/*********************************************************************
 * Include the thread module's external header, which should contain:
 *   1) #include's for system thread headers
 *   2) typedefs for external nexus types
 *   3) macros for external nexus routines
 *********************************************************************/
#ifndef BUILD_LITE

#ifndef NEXUS_TH_HEADER
#define NEXUS_TH_HEADER "th_p0.h"
#endif

#include NEXUS_TH_HEADER

#endif /* BUILD_LITE */


/*********************************************************************
 * Some typdefs and structures
 *********************************************************************/

EXTERN_C_BEGIN

#ifdef BUILD_LITE
/*
 * For NexusLite we need to dummy up some simple versions
 * of the mutex and cond stuff.
 */
typedef int	nexus_mutex_t;
typedef int	nexus_cond_t;
typedef int	nexus_mutexattr_t;
typedef int	nexus_condattr_t;
#endif /* BUILD_LITE */


/*
 * nexus_list_t
 *
 * A generic linked list structure
 */
typedef struct _nexus_list_t
{
    void *                    value;
    struct _nexus_list_t *    next;
} nexus_list_t;


/*
 * nexus_barrier_t
 *
 * A generic barrier structure
 */
typedef struct _nexus_barrier_t
{
    nexus_mutex_t	mutex;
    nexus_cond_t	cond;
    int			count;
} nexus_barrier_t;


/*
 * nexus_context_create_handle_t
 */
typedef struct _nexus_context_create_handle_t
{
    int				type;
    nexus_mutex_t		mutex;
    nexus_cond_t		cond;
    nexus_endpoint_t *          ep;
    int				total;
    int				checked_in;
    int				next_sp;
    nexus_startpoint_t **	sp;
    int **			rc;
} nexus_context_create_handle_t;


/*********************************************************************
 * Buffer management structures
 *********************************************************************/

/*
 * nexus_base_segment_t
 */
typedef struct _nexus_base_segment_t
{
    struct _nexus_base_segment_t *	next;
    unsigned long			size;
    unsigned long			size_used;
    nexus_byte_t *			storage;
    nexus_byte_t *			current;
    nexus_bool_t			storage_is_inline;
} nexus_base_segment_t;

/*
 * nexus_direct_info_t
 */
typedef struct _nexus_direct_info_t
{
    int			datatype;
    unsigned long	size;
    void *		data;
    int			action;
    unsigned long	custom_info;
} nexus_direct_info_t;

#define NEXUS_DIRECT_INFO_ACTION_INLINE 	0
#define NEXUS_DIRECT_INFO_ACTION_POINTER	1
#define NEXUS_DIRECT_INFO_ACTION_CUSTOM         2

/*
 * nexus_direct_segment_t
 */
typedef struct _nexus_direct_segment_t
{
    struct _nexus_direct_segment_t *	next;
    unsigned long			size;
    unsigned long			n_left;
    nexus_direct_info_t *		storage;
    nexus_direct_info_t *		current;
} nexus_direct_segment_t;

/*
 * nexus_buffer_t
 */
struct _nexus_buffer_t
{
    int				magic;
    nexus_proto_funcs_t *	funcs;
    nexus_buffer_t 		next;
    unsigned long               reserved_header_size;
    int				format;
    int				saved_state; /* NEXUS_BUFFER_SAVED_STATE_* */
    nexus_base_segment_t *	base_segments;
    nexus_base_segment_t *	current_base_segment;
    nexus_direct_segment_t *	direct_segments;
    nexus_direct_segment_t *	current_direct_segment;
    unsigned long		n_direct;
    nexus_byte_t *		direct_info;
    nexus_bool_t		iovec_formatted;
    nexus_barrier_t *		barrier;
};

enum
{
    NEXUS_BUFFER_SAVED_STATE_UNSAVED		= 0,
    NEXUS_BUFFER_SAVED_STATE_SAVED_IN_HANDLER	= 1,
    NEXUS_BUFFER_SAVED_STATE_SAVED		= 2,
    NEXUS_BUFFER_SAVED_STATE_FREED		= 3
};

/*********************************************************************
 * Protocol module typedefs and structures
 *********************************************************************/

/*
 * nexus_proto_t
 *
 * nexus_proto_t is similar to nexus_buffer_t.
 * Each protocol will have to add data to its version of a proto_t.
 * This structure is the common root to all of them -- all proto_t's
 * should start with these same fields.
 *
 * This part of the nexus_proto_t needs to live here so that
 * macros below can work.
 */
struct _nexus_proto_t
{
    nexus_proto_type_t	 type;
    nexus_proto_funcs_t *funcs;
};


/*
 * nexus_mi_proto_t
 *
 * This structure contains a "machine independent" representation
 * of the protocols that can be used by a global pointer (gp) to
 * deliver a message for that gp.
 *
 * The protocols are stored in the packed byte array.
 * The contents of this byte array are:
 *	node number (4 byte, big endian integer)
 *		The node number to which this mi_proto points.
 *	context differentiator (4 byte, big endian integer)
 *		When contexts are implemented as processes, it is
 *		possible for the node number, node name, and
 *		context address (stored in the gp->liba)
 *		to be identical between two gp's
 *		even though they really point to different
 * 		contexts.  This can happen because the context
 *		address which is stored in the gp->liba is _not_
 *		a unique context id within this node, but
 *		rather is only unique within a single process that
 *		is part of a node.
 *		The context differentiator disambiguates this situation.
 *		Generally it contains the process id of the process
 *		representing this mi_proto.
 *	node number (null terminated string)
 *	protocols: 0 or more entries of the following form
 *	    protocol type (2 byte, big endian integer)
 *	    size of next array (2 byte, big endian integer)
 *	    protocol specific byte array
 *		This contains information that a protocol module can
 *		use to deliver a message.
 *
 * The 'proto' field is an instantiated version on one of the protocols
 * in the byte 'array'.
 *
 * This structure is _not_ reference counted, though the nexus_proto_t
 * object stored in this structure is.  The nexus_proto_t reference
 * count is of the number of global pointers which point through this
 * nexus_mi_proto_t structure to the given nexus_proto_t object.
 */
struct _nexus_mi_proto_t
{
    nexus_proto_t *		proto;
    struct _nexus_mi_proto_t *	next;
    int				size;
    nexus_byte_t *		array;
};


/*********************************************************************
 * Miscellaneous typedefs and structures
 *********************************************************************/

#ifdef BUILD_PROFILE

#ifndef NEXUS_PROFILE_RSR_HASH_TABLE_SIZE
#define NEXUS_PROFILE_RSR_HASH_TABLE_SIZE       16
#endif

/*
 * nexus_profile_rsr_record_t
 *
 * This is used to keep cumulative totals of handler invocations
 */
typedef struct _nexus_profile_rsr_record_t
{
    int                                         node_id;
    int                                         context_id;
    int                                         count;
    int                                         size;
    struct _nexus_profile_rsr_record_t *        next;
} nexus_profile_rsr_record_t;

#endif /* BUILD_PROFILE */

/*
 * nexus_context_t
 *
 * Hold context info.
 *
 * The segment_list is a circular, doubly linked list, with a dummy
 * first segment.
 */
typedef struct _nexus_context_t
{
    nexus_mutex_t               mutex;
    int                         id;
    void *                      handle;
    int                         (*entry_point)(void *);
    void                        (*switchingfunc)(void *);
    int                         (*destructofunc)(struct _nexus_context_t *);
    int                         (*pfnNexusBoot)(nexus_startpoint_t *sp);
    int                         (*pfnNexusExit)(void);
    void                        (*pfnNexusUnknownHandler)(void *,
                                                          nexus_buffer_t *,
                                                          char *, int );
    int                         (*pfnNexusAcquiredAsNode)(nexus_startpoint_t *sp);
    int                         n_segments;
    struct _nexus_segment_t *   segment_list;
    nexus_bool_t                is_node;
    struct _nexus_context_t *   context_list_next;
#ifdef BUILD_PROFILE
    int                         snapshot_id;
    int                         rsr_profile_count;
    nexus_startpoint_t          master_sp;
    nexus_bool_t                waiting_on_master_sp;
    nexus_cond_t                cond;
#endif /* BUILD_PROFILE */
} nexus_context_t;

struct _nexus_endpointattr_t
{
    nexus_handler_t *			handler_table;
    int					handler_table_size;
    nexus_unknown_handler_func_t	unknown_handler;
    nexus_handler_type_t		unknown_handler_type;
    int					transform_id;
    nexus_transformattr_t *		transform_attr;
    nexus_context_t *			context;
};

struct _nexus_endpoint_t
{
    nexus_handler_t *			handler_table;
    int					handler_table_size;
    nexus_unknown_handler_func_t	unknown_handler;
    nexus_handler_type_t		unknown_handler_type;
    int					transform_id;
    nexus_transformstate_t *		transform_state;
    nexus_context_t *			context;
    void *				user_pointer;
    unsigned long			id;
    nexus_endpoint_t *			next;
    nexus_endpoint_t *			prev;
};

struct _nexus_startpoint_t
{
    nexus_mi_proto_t *  mi_proto;
    unsigned int	copy_locally	: 1; /* 0==sp_copy must roundtrip */
    unsigned int	destroy_locally	: 1; /* 0==sp_destroy must roundtrip */
    unsigned int        liba_is_inline  : 1; /* 0==array, 1==pointer */
    unsigned int	liba_size       : 8;
    unsigned int	transform_id	: 8;
    nexus_transformstate_t *	transform_state;
    unsigned long	endpoint_id;
    union
    {
        nexus_byte_t *  pointer;                        /* liba_is_inline==0*/
        nexus_byte_t    array[NEXUS_DEFAULT_LIBA_SIZE]; /* liba_is_inline==1*/
    } liba;
#ifdef BUILD_PROFILE
    int                 node_id;
    int                 context_id;
#endif /* BUILD_PROFILE */
};


/*
 * nexus_segment_t
 *
 * Holds one data segment in a context.
 */
typedef struct _nexus_segment_t
{
    void *                      data;
    unsigned int                size;
    struct _nexus_context_t *   context;
    struct _nexus_segment_t *   prev;
    struct _nexus_segment_t *   next;
} nexus_segment_t;

/*
 * nexus_node_t
 *
 * An array of these is returned by nexus_init() on the master node,
 * and by nexus_acquire_nodes().
 */
typedef struct _nexus_node_t
{
    char *			name;
    int				number;
    nexus_startpoint_t	        startpoint;
    int				return_code;
} nexus_node_t;

/* Values for nexus_node_t.return_code */
#define NEXUS_NODE_NEW		0
#define NEXUS_NODE_OLD_SAME_EXE	1
#define NEXUS_NODE_OLD_DIFF_EXE	2


#if defined(HAVE_THREAD_SAFE_STDIO) || defined(BUILD_LITE)
#define nexus_stdio_lock()
#define nexus_stdio_unlock()
#else
NEXUS_GLOBAL nexus_mutex_t nexus_stdio_mutex;
#define nexus_stdio_lock() nexus_mutex_lock(&nexus_stdio_mutex) 
#define nexus_stdio_unlock() nexus_mutex_unlock(&nexus_stdio_mutex)
#endif /* HAVE_THREAD_SAFE_STDIO */


/*********************************************************************
 * Protocol module function indirection table
 *********************************************************************/

struct _nexus_proto_funcs_t
{
    nexus_proto_type_t	(*proto_type)(void);
    void		(*init)(int *argc, char ***argv);
    void		(*shutdown)(nexus_bool_t shutdown_others);
    void		(*abort)(void);
    nexus_bool_t	(*poll)(void);
    void		(*poll_blocking)(void);
    void                (*increment_reference_count)(nexus_proto_t *proto);
    nexus_bool_t	(*decrement_reference_count)(nexus_proto_t *proto);
    void		(*get_my_mi_proto)(nexus_byte_t **array,
					   int *size);
    nexus_bool_t	(*construct_from_mi_proto)(nexus_proto_t **proto,
						   nexus_mi_proto_t *mi_proto,
						   nexus_byte_t *array,
						   int size);
    int			(*test_proto)(nexus_proto_t *proto);
    int			(*send_rsr)(nexus_buffer_t *buffer,
				nexus_startpoint_t *sp,
				int handler_id,
				nexus_bool_t destroy_buffer,
				nexus_bool_t called_from_non_threaded_handler);
    int                 (*direct_info_size)(void);
    int                 (*direct_get)(nexus_byte_t *dest,
				      size_t n_bytes,
				      int action,
				      unsigned long info);
};


/*********************************************************************
 * Declarations for functions that must be supplied by the package
 *********************************************************************/

extern int	NexusBoot(nexus_startpoint_t *startpoint);
extern int	NexusExit(void);
extern int	NexusAcquiredAsNode(nexus_startpoint_t *startpoint);


/*********************************************************************
 * Exported interface declarations
 *********************************************************************/

/*
 * TODO: The following functions seem not to be defined anywhere:
 */
extern int	nexus_user_iargc(void);
extern char *	nexus_user_getarg(int i);
extern int	nexus_package_iargc(void);
extern char *	nexus_package_getarg(int i);
extern void	nexus_get_argc_and_argv(int *argc, char ***argv);
/*
 * end undefined functions
 */

extern void     nexus_set_path(nexus_path_type_t, char *);

extern void	nexus_init(int *argc,
			   char ***argv,
			   char *args_env_variable,
			   char *package_designator,
			   int (*package_args_init_func)(int *argc,
							 char ***argv),
			   void (*usage_message_func)(void),
			   int (*new_process_params_func)(char *buf, int size),
			   nexus_module_list_t module_list[],
			   nexus_node_t **nodes,
			   int *n_nodes);
extern void	nexus_start(void);
extern void	nexus_start_nonblocking(void);
extern void	nexus_exit(int rc, int shutdown);
extern void	nexus_shutdown(void);
extern void	nexus_shutdown_nonexiting(void);
extern void	nexus_abort(void);

extern void	nexus_context_destroy(int called_from_non_threaded_handler);
extern void	nexus_node_acquire(char *node_name,
				   int node_number,
				   int count,
				   char *directory_path,
				   char *executable_path,
				   nexus_node_t **nodes,
				   int *n_nodes);

extern void	nexus_context_create_init(
				nexus_context_create_handle_t *contexts,
                                int n_contexts);
extern void	nexus_context_create(nexus_startpoint_t *node_sp,
				     char *executable_path,
				     nexus_startpoint_t *new_context_sp,
				     int *return_code,
				     nexus_context_create_handle_t *contexts);
extern void	nexus_context_create_wait(
				nexus_context_create_handle_t *contexts);
extern void *	nexus_malloc(size_t size);
extern void	nexus_free(void *data_segment);

/*
 * JGG
 * do we want to keep this function?
 * JGG
 */
extern void     nexus_register_skip_poll_callback(
				nexus_skip_poll_callback_t *func);
extern void	nexus_poll(void);
extern void	nexus_poll_profile(int id);
extern void	nexus_poll_blocking(void);

extern int      nexus_endpointattr_init(nexus_endpointattr_t *endpointattr);
extern int      nexus_endpointattr_destroy(nexus_endpointattr_t *endpointattr);
extern int      nexus_endpointattr_set_handler_table(
		                  nexus_endpointattr_t *endpointattr,
				  nexus_handler_t *handler_table,
				  int handler_table_size);
extern int      nexus_endpointattr_get_handler_table(
                                  nexus_endpointattr_t *endpointattr,
				  nexus_handler_t **handler_table,
				  int *handler_table_size);
extern int      nexus_endpointattr_set_unknown_handler(
				  nexus_endpointattr_t *endpointattr,
				  nexus_unknown_handler_func_t func,
				  nexus_handler_type_t type);
extern int      nexus_endpointattr_get_unknown_handler(
				  nexus_endpointattr_t *endpointattr,
				  nexus_unknown_handler_func_t *func,
				  nexus_handler_type_t *type);
extern int      nexus_endpointattr_set_transform(
				  nexus_endpointattr_t *endpointattr,
				  int transform_id,
				  void *transform_info);
extern int      nexus_endpointattr_get_transform(
				  nexus_endpointattr_t *endpointattr,
				  int *transform_id,
				  void **transform_info);

extern int      nexus_endpoint_init(nexus_endpoint_t *endpoint,
				    nexus_endpointattr_t *endpointattr);
extern int      nexus_endpoint_destroy(nexus_endpoint_t *endpoint);
extern int      nexus_endpoint_string(nexus_endpoint_t *endpoint,
				      char *string,
				      int length);

extern int      nexus_startpoint_bind(nexus_startpoint_t *startpoint,
				      nexus_endpoint_t *endpoint);
extern int      nexus_startpoint_copy(nexus_startpoint_t *dest_startpoint,
				      nexus_startpoint_t *src_startpoint);
extern int      nexus_startpoint_destroy(nexus_startpoint_t *startpoint);
extern int      nexus_startpoint_destroy_and_notify(nexus_startpoint_t *sp);
extern int      nexus_startpoint_equal_context(nexus_startpoint_t *sp1,
					       nexus_startpoint_t *sp2);
extern int      nexus_startpoint_equal(nexus_startpoint_t *sp1,
				       nexus_startpoint_t *sp2);
extern int      nexus_startpoint_to_current_context(nexus_startpoint_t *sp);
extern int	nexus_startpoint_get_endpoint(nexus_startpoint_t *sp,
					      nexus_endpoint_t **ep);
extern int      nexus_startpoint_string(nexus_startpoint_t *startpoint,
					char *string,
					int length);

extern int      nexus_put_startpoint_transfer(nexus_buffer_t *buffer,
					      nexus_startpoint_t *sp,
					      unsigned long count);
extern int      nexus_get_startpoint(nexus_buffer_t *buffer,
				     nexus_startpoint_t *sp,
				     unsigned long count);

extern int      nexus_sizeof_startpoint(nexus_startpoint_t *sp, int count);

extern int	nexus_buffer_init(nexus_buffer_t *buffer,
				  unsigned long buffer_size,
				  unsigned long num_direct_puts);
extern int	nexus_buffer_destroy(nexus_buffer_t *buffer);
extern int	nexus_check_buffer_size(nexus_buffer_t *buffer,
					int size_needed,
					int size_increment,
					int num_direct_puts_needed,
					int num_direct_puts_increment);


extern int	nexus_master_context(void);


/*********************************************************************
 * Fault tolerance stuff
 *********************************************************************/

extern void nexus_enable_fault_tolerance(int (*callback_func)(void *user_arg,
							      int fault_code),
					 void *callback_func_user_arg);
extern int	nexus_startpoint_test(nexus_startpoint_t *sp);

#define NEXUS_FAULT_NONE			0
#define NEXUS_FAULT_UNKNOWN			1
#define NEXUS_FAULT_PROCESS_DIED		2
#define NEXUS_FAULT_PROCESS_SHUTDOWN_ABNORMALLY	3
#define NEXUS_FAULT_PROCESS_SHUTDOWN_NORMALLY	4
#define NEXUS_FAULT_ACCEPT_ATTACH_FAILED	5
#define NEXUS_FAULT_CONNECT_FAILED		6
#define NEXUS_FAULT_BAD_PROTOCOL		7
#define NEXUS_FAULT_READ_FAILED			8
#define NEXUS_FAULT_BAD_URL			9

extern char *nexus_fault_strings[];


/*********************************************************************
 * Event driver interface
 *********************************************************************/

/* 
 * TODO: This should move into a separate Nexus Component Library
 */
struct iovec;

extern int              nexus_fd_init(int *argc, char ***argv);
extern int              nexus_fd_shutdown(void);
extern int              nexus_fd_open(char *path,
				      int flags,
				      int mode,
				      int *fd);
extern int              nexus_fd_setup(int fd);
extern int              nexus_fd_close(int fd);
extern int              nexus_fd_register_for_select(
				int fd,
				void (*read_callback_func)(void *arg,
							   int fd),
				void (*write_callback_func)(void *arg,
							    int fd),
				void (*except_callback_func)(void *arg,
							     int fd),
				void *callback_arg);
extern int		nexus_fd_register_for_read(
				int fd,
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
				void *callback_arg);
extern int		nexus_fd_register_for_write(
				int fd,
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
				void *callback_arg);
extern int		nexus_fd_register_for_writev(
				int fd,
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
				void *callback_arg);
extern int		nexus_fd_write_one_nonblocking(int fd, char c);
extern int              nexus_fd_unregister(int fd,
					    void (*callback_func)(void *arg,
								  int fd));
extern int              nexus_fd_handle_events(int poll_type,
					       int *message_handled);
#define NEXUS_FD_POLL_BLOCKING_ALL            0
#define NEXUS_FD_POLL_BLOCKING_ONCE           1
#define NEXUS_FD_POLL_BLOCKING_UNTIL_SHUTDOWN 2
#define NEXUS_FD_POLL_NONBLOCKING_ALL         3
#define NEXUS_FD_POLL_NONBLOCKING_ONCE        4
extern void	nexus_fd_set_handler_thread(nexus_bool_t i_am_handler_thread);
extern int              nexus_fd_create_listener(unsigned short *port,
			    int backlog,
			    void (*callback_func)(void *arg, int fd),
			    void *callback_arg);
extern int              nexus_fd_close_listener(unsigned short port);
extern int              nexus_fd_connect(char *host,
					 unsigned short port,
					 int *fd);
extern int              nexus_fd_defer(void (*add_fd)(int fd),
				       void (*remove_fd)(int fd));
extern int              nexus_fd_defer_callback(int fd);



/*********************************************************************
 * Function prototypes that are not part of the official interface
 *********************************************************************/

/*
 * args.c
 */
extern int	nexus_find_argument(int *argc,
				    char ***argv,
				    char *arg,
				    int count);
extern void	nexus_remove_arguments(int *argc,
				       char ***argv,
				       int arg_num,
				       int count);
extern void	nexus_get_retained_arguments(int **argc, char ****argv);


/*
 * attach.c
 */
extern int	nexus_allow_attach(unsigned short *port,
				   char **host,
				   int (*approval_func)(void *user_arg,
							char *url,
							nexus_startpoint_t *sp),
				   void *approval_func_user_arg);
extern void	nexus_disallow_attach(unsigned short port);
extern int	nexus_attach(char *url,
			     nexus_startpoint_t *sp);
extern int	nexus_split_nexus_url(char *url,
				      char **host,
				      unsigned short *port,
				      char ***specifiers);
extern void	nexus_split_nexus_url_free(char **host,
					   char ***specifiers);


/*
 * error.c
 */
extern void		nexus_silent_fatal(void);
#if defined(__STDC__) || defined(__cplusplus)
extern void		nexus_fatal(char *msg, ...);
extern void		nexus_error(char *msg, ...);
extern void		nexus_warning(char *msg, ...);
extern void		nexus_notice(char *msg, ...);
extern void		nexus_printf(char *msg, ...);
extern void		nexus_perror(char *msg, ...);
extern void		nexus_fatal_perror(char *msg, ...);
extern char *		nexus_assert_sprintf(char *msg, ...);
#else /* __STDC__ */
/*
 * These have variable number of parameters, so must be defined this way
 * to get the compiler to work correctly.
 */
extern void		nexus_fatal();
extern void		nexus_error();
extern void		nexus_warning();
extern void		nexus_notice();
extern void		nexus_printf();
extern void		nexus_perror();
extern void		nexus_fatal_perror();
extern char *		nexus_assert_sprintf();
#endif /* __STDC__ */

/*
 * md_*.c
 */
extern void		nexus_kill(int pid);
extern void 		nexus_usleep(long usecs);
extern double		nexus_wallclock(void);
extern void		nexus_pause(void);


/*
 * nexus_register_fd_notifiers() registers functions that
 * Nexus should call to register any fd's that it uses.
 *
 * Note: This is _not_ a general solution.  It exists so that nperl
 * does not have to use a patched version of nexus.  It _will_ change
 * sometime in the future.  You should not rely on this function.
 */
typedef void (*nexus_fd_notifier_func_t)(int fd);
extern void	nexus_register_fd_notifiers(nexus_fd_notifier_func_t add,
					    nexus_fd_notifier_func_t remove);

/*
 * rdb_iface.c
 */
extern char *   nexus_rdb_lookup(char *node_name, char *key);
extern void     nexus_rdb_free(char *value);


/*
 * th_*.c
 */
extern nexus_bool_t	nexus_preemptive_threads(void);


/*
 * utils.c
 */
extern void		nexus_ids(int *node_id,
				  int *context_id,
				  int *thread_id);



/*********************************************************************
 * Interface routines that can be implemented as macros
 *********************************************************************/

#define nexus_macro_set_package_id(package_id_base) \
    ports0_set_package_id(package_id_base)
#define nexus_macro_get_package_id() \
    ports0_get_package_id()
#define nexus_macro_send_rsr(buffer, startpoint, handler_id, destroy_buffer, called_from_non_threaded_handler) \
    ((startpoint)->mi_proto->proto->funcs->send_rsr(buffer, startpoint, \
					     handler_id, destroy_buffer, \
					     called_from_non_threaded_handler))
#define nexus_macro_endpoint_set_user_pointer(endpoint, address) \
    (endpoint)->user_pointer = (address)
#define nexus_macro_endpoint_get_user_pointer(endpoint) \
    (endpoint)->user_pointer
#define nexus_macro_startpoint_set_null(startpoint) \
    (startpoint)->mi_proto = (nexus_mi_proto_t *) NULL
#define nexus_macro_startpoint_is_null(startpoint) \
    ((startpoint)->mi_proto == (nexus_mi_proto_t *) NULL)

#ifndef USE_MACROS
    
extern int	nexus_set_package_id(char *package_id_base);
extern char *	nexus_get_package_id(void);
extern int	nexus_send_rsr(nexus_buffer_t *buffer,
			       nexus_startpoint_t *startpoint,
			       int handler_id,
			       nexus_bool_t destroy_buffer,
			       nexus_bool_t called_from_non_threaded_handler);
extern void     nexus_endpoint_set_user_pointer(nexus_endpoint_t *endpoint,
						void *address);
extern void *   nexus_endpoint_get_user_pointer(nexus_endpoint_t *endpoint);
extern void     nexus_startpoint_set_null(nexus_startpoint_t *startpoint);
extern int      nexus_startpoint_is_null(nexus_startpoint_t *startpoint);

#else  /* USE_MACROS */

#define nexus_set_package_id(package_id_base) \
    nexus_macro_set_package_id(package_id_base)
#define nexus_get_package_id() \
    nexus_macro_get_package_id()
#define nexus_send_rsr(buffer, startpoint, handler_id, destroy_buffer, called_from_non_threaded_handler) \
    nexus_macro_send_rsr(buffer, startpoint, handler_id, destroy_buffer, called_from_non_threaded_handler)
#define nexus_endpoint_set_user_pointer(endpoint, address) \
    nexus_macro_endpoint_set_user_pointer(endpoint, address)
#define nexus_endpoint_get_user_pointer(endpoint) \
    nexus_macro_endpoint_get_user_pointer(endpoint)
#define nexus_startpoint_set_null(startpoint) \
    nexus_macro_startpoint_set_null(startpoint)
#define nexus_startpoint_is_null(startpoint) \
    nexus_macro_startpoint_is_null(startpoint)

#endif /* USE_MACROS */

    
/*********************************************************************
 * nexus_sizeof_DATATYPE()
 *********************************************************************/

#define nexus_macro_sizeof_float(Count) \
    (nexus_dc_sizeof_float(Count))
#define nexus_macro_sizeof_double(Count) \
    (nexus_dc_sizeof_double(Count))
#define nexus_macro_sizeof_short(Count) \
    (nexus_dc_sizeof_short(Count))
#define nexus_macro_sizeof_u_short(Count) \
    (nexus_dc_sizeof_u_short(Count))
#define nexus_macro_sizeof_int(Count) \
    (nexus_dc_sizeof_int(Count))
#define nexus_macro_sizeof_u_int(Count) \
    (nexus_dc_sizeof_u_int(Count))
#define nexus_macro_sizeof_long(Count) \
    (nexus_dc_sizeof_long(Count))
#define nexus_macro_sizeof_u_long(Count) \
    (nexus_dc_sizeof_u_long(Count))
#define nexus_macro_sizeof_char(Count) \
    (nexus_dc_sizeof_char(Count))
#define nexus_macro_sizeof_u_char(Count) \
    (nexus_dc_sizeof_u_char(Count))
#define nexus_macro_sizeof_byte(Count) \
    (nexus_dc_sizeof_byte(Count))

#ifndef USE_MACROS
    
extern int	nexus_sizeof_float(int count);
extern int	nexus_sizeof_double(int count);
extern int	nexus_sizeof_short(int count);
extern int	nexus_sizeof_u_short(int count);
extern int	nexus_sizeof_int(int count);
extern int	nexus_sizeof_u_int(int count);
extern int	nexus_sizeof_long(int count);
extern int	nexus_sizeof_u_long(int count);
extern int	nexus_sizeof_char(int count);
extern int	nexus_sizeof_u_char(int count);
extern int	nexus_sizeof_byte(int count);

#else  /* USE_MACROS */

#define nexus_sizeof_float(count) \
    nexus_macro_sizeof_float(count)
#define nexus_sizeof_double(count) \
    nexus_macro_sizeof_double(count)
#define nexus_sizeof_short(count) \
    nexus_macro_sizeof_short(count)
#define nexus_sizeof_u_short(count) \
    nexus_macro_sizeof_u_short(count)
#define nexus_sizeof_int(count) \
    nexus_macro_sizeof_int(count)
#define nexus_sizeof_u_int(count) \
    nexus_macro_sizeof_u_int(count)
#define nexus_sizeof_long(count) \
    nexus_macro_sizeof_long(count)
#define nexus_sizeof_u_long(count) \
    nexus_macro_sizeof_u_long(count)
#define nexus_sizeof_char(count) \
    nexus_macro_sizeof_char(count)
#define nexus_sizeof_u_char(count) \
    nexus_macro_sizeof_u_char(count)
#define nexus_sizeof_byte(count) \
    nexus_macro_sizeof_byte(count)

#endif /* USE_MACROS */


/*********************************************************************
 * nexus_put_DATATYPE()
 *********************************************************************/

extern void nexus_put_float(nexus_buffer_t *buffer,
			    float *data,
			    int count);
extern void nexus_put_double(nexus_buffer_t *buffer,
			     double *data,
			     int count);
extern void nexus_put_short(nexus_buffer_t *buffer,
			    short *data,
			    int count);
extern void nexus_put_u_short(nexus_buffer_t *buffer,
			      unsigned short *data,
			      int count);
extern void nexus_put_int(nexus_buffer_t *buffer,
			  int *data,
			  int count);
extern void nexus_put_u_int(nexus_buffer_t *buffer,
			    unsigned int *data,
			    int count);
extern void nexus_put_long(nexus_buffer_t *buffer,
			   long *data,
			   int count);
extern void nexus_put_u_long(nexus_buffer_t *buffer,
			     unsigned long *data,
			     int count);
extern void nexus_put_char(nexus_buffer_t *buffer,
			   char *data,
			   int count);
extern void nexus_put_u_char(nexus_buffer_t *buffer,
			     unsigned char *data,
			     int count);
extern void nexus_put_byte(nexus_buffer_t *buffer,
			   nexus_byte_t *data,
			   int count);
extern void nexus_put_user(nexus_buffer_t *buffer,
			   nexus_byte_t *data,
			   int count);

/*********************************************************************
 * nexus_get_DATATYPE()
 *********************************************************************/

extern void nexus_get_float(nexus_buffer_t *buffer,
			    float *dest,
			    int count);
extern void nexus_get_double(nexus_buffer_t *buffer,
			     double *dest,
			     int count);
extern void nexus_get_short(nexus_buffer_t *buffer,
			    short *dest,
			    int count);
extern void nexus_get_u_short(nexus_buffer_t *buffer,
			      unsigned short *dest,
			      int count);
extern void nexus_get_int(nexus_buffer_t *buffer,
			  int *dest,
			  int count);
extern void nexus_get_u_int(nexus_buffer_t *buffer,
			    unsigned int *dest,
			    int count);
extern void nexus_get_long(nexus_buffer_t *buffer,
			   long *dest,
			   int count);
extern void nexus_get_u_long(nexus_buffer_t *buffer,
			     unsigned long *dest,
			     int count);
extern void nexus_get_char(nexus_buffer_t *buffer,
			   char *dest,
			   int count);
extern void nexus_get_u_char(nexus_buffer_t *buffer,
			     unsigned char *dest,
			     int count);
extern void nexus_get_byte(nexus_buffer_t *buffer,
			   nexus_byte_t *dest,
			   int count);
extern void nexus_get_user(nexus_buffer_t *buffer,
			   nexus_byte_t *dest,
			   int count);

/*********************************************************************
 * nexus_direct_put_DATATYPE()
 *********************************************************************/

extern void _nx_direct_put(nexus_buffer_t *buffer,
			   void *data,
			   unsigned long count,
			   int datatype,
			   int sizeof_datatype);

#define nexus_macro_direct_put_float(buffer, data, count) \
    _nx_direct_put(buffer, data, count, \
		   NEXUS_DC_DATATYPE_FLOAT, \
		   nexus_dc_sizeof_float(1))
#define nexus_macro_direct_put_double(buffer, data, count) \
    _nx_direct_put(buffer, data, count, \
		   NEXUS_DC_DATATYPE_DOUBLE, \
		   nexus_dc_sizeof_double(1))
#define nexus_macro_direct_put_short(buffer, data, count) \
    _nx_direct_put(buffer, data, count, \
		   NEXUS_DC_DATATYPE_SHORT, \
		   nexus_dc_sizeof_short(1))
#define nexus_macro_direct_put_u_short(buffer, data, count) \
    _nx_direct_put(buffer, data, count, \
		   NEXUS_DC_DATATYPE_U_SHORT, \
		   nexus_dc_sizeof_u_short(1))
#define nexus_macro_direct_put_int(buffer, data, count) \
    _nx_direct_put(buffer, data, count, \
		   NEXUS_DC_DATATYPE_INT, \
		   nexus_dc_sizeof_int(1))
#define nexus_macro_direct_put_u_int(buffer, data, count) \
    _nx_direct_put(buffer, data, count, \
		   NEXUS_DC_DATATYPE_U_INT, \
		   nexus_dc_sizeof_u_int(1))
#define nexus_macro_direct_put_long(buffer, data, count) \
    _nx_direct_put(buffer, data, count, \
		   NEXUS_DC_DATATYPE_LONG, \
		   nexus_dc_sizeof_long(1))
#define nexus_macro_direct_put_u_long(buffer, data, count) \
    _nx_direct_put(buffer, data, count, \
		   NEXUS_DC_DATATYPE_U_LONG, \
		   nexus_dc_sizeof_u_long(1))
#define nexus_macro_direct_put_char(buffer, data, count) \
    _nx_direct_put(buffer, data, count, \
		   NEXUS_DC_DATATYPE_CHAR, \
		   nexus_dc_sizeof_char(1))
#define nexus_macro_direct_put_u_char(buffer, data, count) \
    _nx_direct_put(buffer, data, count, \
		   NEXUS_DC_DATATYPE_U_CHAR, \
		   nexus_dc_sizeof_u_char(1))
#define nexus_macro_direct_put_byte(buffer, data, count) \
    _nx_direct_put(buffer, data, count, \
		   NEXUS_DC_DATATYPE_BYTE, \
		   nexus_dc_sizeof_byte(1))
#define nexus_macro_direct_put_user(buffer, data, count) \
    _nx_direct_put(buffer, data, count, \
		   NEXUS_DC_DATATYPE_BYTE, \
		   nexus_dc_sizeof_byte(1))

#ifndef USE_MACROS
    
extern void nexus_direct_put_float(nexus_buffer_t *buffer,
				   float *data,
				   int count);
extern void nexus_direct_put_double(nexus_buffer_t *buffer,
				    double *data,
				    int count);
extern void nexus_direct_put_short(nexus_buffer_t *buffer,
				   short *data,
				   int count);
extern void nexus_direct_put_u_short(nexus_buffer_t *buffer,
				     unsigned short *data,
				     int count);
extern void nexus_direct_put_int(nexus_buffer_t *buffer,
				 int *data,
				 int count);
extern void nexus_direct_put_u_int(nexus_buffer_t *buffer,
				   unsigned int *data,
				   int count);
extern void nexus_direct_put_long(nexus_buffer_t *buffer,
				  long *data, int count);
extern void nexus_direct_put_u_long(nexus_buffer_t *buffer,
				    unsigned long *data,
				    int count);
extern void nexus_direct_put_char(nexus_buffer_t *buffer,
				  char *data,
				  int count);
extern void nexus_direct_put_u_char(nexus_buffer_t *buffer,
				    unsigned char *data,
				    int count);
extern void nexus_direct_put_byte(nexus_buffer_t *buffer,
				  nexus_byte_t *data,
				  int count);
extern void nexus_direct_put_user(nexus_buffer_t *buffer,
				  nexus_byte_t *data,
				  int count);

#else  /* USE_MACROS */

#define nexus_direct_put_float(buffer, data, count) \
    nexus_macro_direct_put_float(buffer, data, count)
#define nexus_direct_put_double(buffer, data, count) \
    nexus_macro_direct_put_double(buffer, data, count)
#define nexus_direct_put_short(buffer, data, count) \
    nexus_macro_direct_put_short(buffer, data, count)
#define nexus_direct_put_u_short(buffer, data, count) \
    nexus_macro_direct_put_u_short(buffer, data, count)
#define nexus_direct_put_int(buffer, data, count) \
    nexus_macro_direct_put_int(buffer, data, count)
#define nexus_direct_put_u_int(buffer, data, count) \
    nexus_macro_direct_put_u_int(buffer, data, count)
#define nexus_direct_put_long(buffer, data, count) \
    nexus_macro_direct_put_long(buffer, data, count)
#define nexus_direct_put_u_long(buffer, data, count) \
    nexus_macro_direct_put_u_long(buffer, data, count)
#define nexus_direct_put_char(buffer, data, count) \
    nexus_macro_direct_put_char(buffer, data, count)
#define nexus_direct_put_u_char(buffer, data, count) \
    nexus_macro_direct_put_u_char(buffer, data, count)
#define nexus_direct_put_byte(buffer, data, count) \
    nexus_macro_direct_put_byte(buffer, data, count)
#define nexus_direct_put_user(buffer, data, count) \
    nexus_macro_direct_put_user(buffer, data, count)
    
#endif /* USE_MACROS */

    
/*********************************************************************
 * nexus_direct_get_DATATYPE()
 *********************************************************************/

extern void nexus_direct_get_float(nexus_buffer_t *buffer,
				   float *dest,
				   int count);
extern void nexus_direct_get_double(nexus_buffer_t *buffer,
				    double *dest,
				    int count);
extern void nexus_direct_get_short(nexus_buffer_t *buffer,
				   short *dest,
				   int count);
extern void nexus_direct_get_u_short(nexus_buffer_t *buffer,
				     unsigned short *dest,
				     int count);
extern void nexus_direct_get_int(nexus_buffer_t *buffer,
				 int *dest,
				 int count);
extern void nexus_direct_get_u_int(nexus_buffer_t *buffer,
				   unsigned int *dest,
				   int count);
extern void nexus_direct_get_long(nexus_buffer_t *buffer,
				  long *dest,
				  int count);
extern void nexus_direct_get_u_long(nexus_buffer_t *buffer,
				    unsigned long *dest,
				    int count);
extern void nexus_direct_get_char(nexus_buffer_t *buffer,
				  char *dest,
				  int count);
extern void nexus_direct_get_u_char(nexus_buffer_t *buffer,
				    unsigned char *dest,
				    int count);
extern void nexus_direct_get_byte(nexus_buffer_t *buffer,
				  nexus_byte_t *dest,
				  int count);
extern void nexus_direct_get_user(nexus_buffer_t *buffer,
				  nexus_byte_t *dest,
				  int count);

/*********************************************************************
 * nexus_user_put_DATATYPE()
 *********************************************************************/

#define nexus_macro_user_put_float(User_buffer, Data, Count) \
    nexus_dc_put_float(User_buffer, Data, Count)
#define nexus_macro_user_put_double(User_buffer, Data, Count) \
    nexus_dc_put_double(User_buffer, Data, Count)
#define nexus_macro_user_put_short(User_buffer, Data, Count) \
    nexus_dc_put_short(User_buffer, Data, Count)
#define nexus_macro_user_put_u_short(User_buffer, Data, Count) \
    nexus_dc_put_u_short(User_buffer, Data, Count)
#define nexus_macro_user_put_int(User_buffer, Data, Count) \
    nexus_dc_put_int(User_buffer, Data, Count)
#define nexus_macro_user_put_u_int(User_buffer, Data, Count) \
    nexus_dc_put_u_int(User_buffer, Data, Count)
#define nexus_macro_user_put_long(User_buffer, Data, Count) \
    nexus_dc_put_long(User_buffer, Data, Count)
#define nexus_macro_user_put_u_long(User_buffer, Data, Count) \
    nexus_dc_put_u_long(User_buffer, Data, Count)
#define nexus_macro_user_put_char(User_buffer, Data, Count) \
    nexus_dc_put_char(User_buffer, Data, Count)
#define nexus_macro_user_put_u_char(User_buffer, Data, Count) \
    nexus_dc_put_u_char(User_buffer, Data, Count)
#define nexus_macro_user_put_byte(User_buffer, Data, Count) \
    nexus_dc_put_byte(User_buffer, Data, Count)

extern int  nexus_user_put_startpoint_transfer(nexus_byte_t **user_buffer,
					       nexus_startpoint_t *sp,
					       unsigned long count);

#ifndef USE_MACROS
    
extern void nexus_user_put_float(nexus_byte_t **user_buffer,
				 float *data,
				 unsigned long count);
extern void nexus_user_put_double(nexus_byte_t **user_buffer,
				  double *data,
				  unsigned long count);
extern void nexus_user_put_short(nexus_byte_t **user_buffer,
				 short *data,
				 unsigned long count);
extern void nexus_user_put_u_short(nexus_byte_t **user_buffer,
				   unsigned short *data,
				   unsigned long count);
extern void nexus_user_put_int(nexus_byte_t **user_buffer,
			       int *data,
			       unsigned long count);
extern void nexus_user_put_u_int(nexus_byte_t **user_buffer,
				 unsigned int *data,
				 unsigned long count);
extern void nexus_user_put_long(nexus_byte_t **user_buffer,
				long *data,
				unsigned long count);
extern void nexus_user_put_u_long(nexus_byte_t **user_buffer,
				  unsigned long *data,
				  unsigned long count);
extern void nexus_user_put_char(nexus_byte_t **user_buffer,
				char *data,
				unsigned long count);
extern void nexus_user_put_u_char(nexus_byte_t **user_buffer,
				  unsigned char *data,
				  unsigned long count);
extern void nexus_user_put_byte(nexus_byte_t **user_buffer,
				nexus_byte_t *data,
				unsigned long count);

#else /* USE_MACROS */

#define nexus_user_put_float(User_buffer, Data, Count) \
    nexus_macro_user_put_float(User_buffer, Data, Count)
#define nexus_user_put_double(User_buffer, Data, Count) \
    nexus_macro_user_put_double(User_buffer, Data, Count)
#define nexus_user_put_short(User_buffer, Data, Count) \
    nexus_macro_user_put_short(User_buffer, Data, Count)
#define nexus_user_put_u_short(User_buffer, Data, Count) \
    nexus_macro_user_put_u_short(User_buffer, Data, Count)
#define nexus_user_put_int(User_buffer, Data, Count) \
    nexus_macro_user_put_int(User_buffer, Data, Count)
#define nexus_user_put_u_int(User_buffer, Data, Count) \
    nexus_macro_user_put_u_int(User_buffer, Data, Count)
#define nexus_user_put_long(User_buffer, Data, Count) \
    nexus_macro_user_put_long(User_buffer, Data, Count)
#define nexus_user_put_u_long(User_buffer, Data, Count) \
    nexus_macro_user_put_u_long(User_buffer, Data, Count)
#define nexus_user_put_char(User_buffer, Data, Count) \
    nexus_macro_user_put_char(User_buffer, Data, Count)
#define nexus_user_put_u_char(User_buffer, Data, Count) \
    nexus_macro_user_put_u_char(User_buffer, Data, Count)
#define nexus_user_put_byte(User_buffer, Data, Count) \
    nexus_macro_user_put_byte(User_buffer, Data, Count)

#endif /* USE_MACROS */

/*********************************************************************
 * nexus_user_get_DATATYPE()
 *********************************************************************/

#define nexus_macro_user_get_float(User_buffer, Data, Count, Format) \
    nexus_dc_get_float(User_buffer, Data, Count, Format)
#define nexus_macro_user_get_double(User_buffer, Data, Count, Format) \
    nexus_dc_get_double(User_buffer, Data, Count, Format)
#define nexus_macro_user_get_short(User_buffer, Data, Count, Format) \
    nexus_dc_get_short(User_buffer, Data, Count, Format)
#define nexus_macro_user_get_u_short(User_buffer, Data, Count, Format) \
    nexus_dc_get_u_short(User_buffer, Data, Count, Format)
#define nexus_macro_user_get_int(User_buffer, Data, Count, Format) \
    nexus_dc_get_int(User_buffer, Data, Count, Format)
#define nexus_macro_user_get_u_int(User_buffer, Data, Count, Format) \
    nexus_dc_get_u_int(User_buffer, Data, Count, Format)
#define nexus_macro_user_get_long(User_buffer, Data, Count, Format) \
    nexus_dc_get_long(User_buffer, Data, Count, Format)
#define nexus_macro_user_get_u_long(User_buffer, Data, Count, Format) \
    nexus_dc_get_u_long(User_buffer, Data, Count, Format)
#define nexus_macro_user_get_char(User_buffer, Data, Count, Format) \
    nexus_dc_get_char(User_buffer, Data, Count, Format)
#define nexus_macro_user_get_u_char(User_buffer, Data, Count, Format) \
    nexus_dc_get_u_char(User_buffer, Data, Count, Format)
#define nexus_macro_user_get_byte(User_buffer, Data, Count, Format) \
    nexus_dc_get_byte(User_buffer, Data, Count, Format)

extern int  nexus_user_get_startpoint(nexus_byte_t **user_buffer,
				      nexus_startpoint_t *sp,
				      unsigned long count,
				      int format);

#ifndef USE_MACROS

extern void nexus_user_get_float(nexus_byte_t **user_buffer,
				 float *data,
				 unsigned long count,
				 int format);
extern void nexus_user_get_double(nexus_byte_t **user_buffer,
				  double *data,
				  unsigned long count,
				  int format);
extern void nexus_user_get_short(nexus_byte_t **user_buffer,
				 short *data,
				 unsigned long count,
				 int format);
extern void nexus_user_get_u_short(nexus_byte_t **user_buffer,
				   unsigned short *data,
				   unsigned long count,
				   int format);
extern void nexus_user_get_int(nexus_byte_t **user_buffer,
			       int *data,
			       unsigned long count,
			       int format);
extern void nexus_user_get_u_int(nexus_byte_t **user_buffer,
				 unsigned int *data,
				 unsigned long count,
				 int format);
extern void nexus_user_get_long(nexus_byte_t **user_buffer,
				long *data,
				unsigned long count,
				int format);
extern void nexus_user_get_u_long(nexus_byte_t **user_buffer,
				  unsigned long *data,
				  unsigned long count,
				  int format);
extern void nexus_user_get_char(nexus_byte_t **user_buffer,
				char *data,
				unsigned long count,
				int format);
extern void nexus_user_get_u_char(nexus_byte_t **user_buffer,
				  unsigned char *data,
				  unsigned long count,
				  int format);
extern void nexus_user_get_byte(nexus_byte_t **user_buffer,
				nexus_byte_t *data,
				unsigned long count,
				int format);

#else /* USE_MACROS */
    
#define nexus_user_get_float(User_buffer, Data, Count, Format) \
    nexus_macro_user_get_float(User_buffer, Data, Count, Format)
#define nexus_user_get_double(User_buffer, Data, Count, Format) \
    nexus_macro_user_get_double(User_buffer, Data, Count, Format)
#define nexus_user_get_short(User_buffer, Data, Count, Format) \
    nexus_macro_user_get_short(User_buffer, Data, Count, Format)
#define nexus_user_get_u_short(User_buffer, Data, Count, Format) \
    nexus_macro_user_get_u_short(User_buffer, Data, Count, Format)
#define nexus_user_get_int(User_buffer, Data, Count, Format) \
    nexus_macro_user_get_int(User_buffer, Data, Count, Format)
#define nexus_user_get_u_int(User_buffer, Data, Count, Format) \
    nexus_macro_user_get_u_int(User_buffer, Data, Count, Format)
#define nexus_user_get_long(User_buffer, Data, Count, Format) \
    nexus_macro_user_get_long(User_buffer, Data, Count, Format)
#define nexus_user_get_u_long(User_buffer, Data, Count, Format) \
    nexus_macro_user_get_u_long(User_buffer, Data, Count, Format)
#define nexus_user_get_char(User_buffer, Data, Count, Format) \
    nexus_macro_user_get_char(User_buffer, Data, Count, Format)
#define nexus_user_get_u_char(User_buffer, Data, Count, Format) \
    nexus_macro_user_get_u_char(User_buffer, Data, Count, Format)
#define nexus_user_get_byte(User_buffer, Data, Count, Format) \
    nexus_macro_user_get_byte(User_buffer, Data, Count, Format)

#endif /* USE_MACROS */

/*********************************************************************
 * NexusLite dummy mutex and cond macros
 *********************************************************************/

#ifdef BUILD_LITE
/*
 * For NexusLite we need to dummy up some simple versions
 * of the mutex and cond stuff.
 */

#define nexus_macro_mutex_init(M,A)	(*(M) = 0)
#define nexus_macro_mutex_destroy(M)	(*(M) = 0)
#define nexus_macro_mutex_lock(M) \
    ( (*(M)) \
     ? (nexus_fatal("nexus_mutex_lock(): Deadlock detected in file %s at line %d. The mutex at address 0x%lu is already locked.\n", __FILE__, __LINE__, (unsigned long) (M)), 1) \
     : ( ((*(M)) = 1), 0 ) )
#define nexus_macro_mutex_unlock(M)	( ((*(M)) = 0), 0 )
#define nexus_macro_mutex_trylock(M)    nexus_macro_mutex_lock(M)
#define nexus_macro_cond_init(C,A)	(*(C) = 0)
#define nexus_macro_cond_destroy(C)	(*(C) = 0)
#define nexus_macro_cond_wait(C,M) \
    ( ((*(M)) = 0), nexus_poll_blocking(), ((*(M)) = 1), 0 )
#define nexus_macro_cond_signal(C)	(*(C) = 0)
#define nexus_macro_cond_broadcast(C)	(*(C) = 0)
#define nexus_macro_thread_yield()

#ifndef USE_MACROS
extern int		nexus_mutex_init(nexus_mutex_t *mutex,
					 nexus_mutexattr_t *attr);
extern int		nexus_mutex_destroy(nexus_mutex_t *mutex);
extern int		nexus_mutex_lock(nexus_mutex_t *mutex);
extern int		nexus_mutex_unlock(nexus_mutex_t *mutex);
extern int              nexus_mutex_trylock(nexus_mutex_t *mutex);
extern int		nexus_cond_init(nexus_cond_t *cond,
					nexus_condattr_t *attr);
extern int		nexus_cond_destroy(nexus_cond_t *cond);
extern int		nexus_cond_wait(nexus_cond_t *cond,
					nexus_mutex_t *mutex);
extern int		nexus_cond_signal(nexus_cond_t *cond);
extern int		nexus_cond_broadcast(nexus_cond_t *cond);
extern void		nexus_thread_yield(void);
#else  /* USE_MACROS */
#define nexus_mutex_init(M,A) nexus_macro_mutex_init(M,A)
#define nexus_mutex_destroy(M) nexus_macro_mutex_destroy(M)
#define nexus_mutex_lock(M) nexus_macro_mutex_lock(M)
#define nexus_mutex_unlock(M) nexus_macro_mutex_unlock(M)
#define nexus_mutex_trylock(M) nexus_macro_mutex_trylock(M)
#define nexus_cond_init(C,A) nexus_macro_cond_init(C,A)
#define nexus_cond_destroy(C) nexus_macro_cond_destroy(C)
#define nexus_cond_wait(C,M) nexus_macro_cond_wait(C,M)
#define nexus_cond_signal(C) nexus_macro_cond_signal(C)
#define nexus_cond_broadcast(C) nexus_macro_cond_broadcast(C)
#define nexus_thread_yield() nexus_macro_thread_yield()
#endif /* USE_MACROS */

#endif /* BUILD_LITE */


EXTERN_C_END

/* 
 * At this point, we have defined all of the macros that can effect
 * the size of structures in nexus.h
 */
#include "nx_ver.h"
#endif /* _NEXUS_INCLUDE_NEXUS_H */
