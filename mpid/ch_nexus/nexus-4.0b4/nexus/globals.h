/*
 * globals.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/globals.h,v 1.39 1996/10/09 23:45:22 tuecke Exp $"
 */

#ifndef _NEXUS_INCLUDE_GLOBALS_H
#define _NEXUS_INCLUDE_GLOBALS_H

/*
 * _nx_my_mi_proto
 *
 * initialized in pr_iface.c and used in gp.c
 */
NEXUS_GLOBAL nexus_mi_proto_t *_nx_my_mi_proto;

/*
 * _nx_NexusBoot
 * _nx_NexusExit
 * _nx_NexusAcquiredAsNode
 *
 * These function pointers are set in initwrap.c:nexus_init() to
 * point to the proper user provided functions.
 *
 * We cannot just call them directly from within Nexus because
 * that results in unresolved symbols in libnexus, which prohibits
 * it from becoming a shared library under AIX.
 */
NEXUS_GLOBAL int (*_nx_NexusBoot)(nexus_startpoint_t *startpoint);
NEXUS_GLOBAL int (*_nx_NexusExit)(void);
NEXUS_GLOBAL int (*_nx_NexusAcquiredAsNode)(nexus_startpoint_t *startpoint);

/*
 * _nx_my_process_params
 *
 * When a process is started, there are a set of parameters that
 * it can take in the form of a string.  This strings is created
 * by _nx_new_process_params().  On the newly created
 * process, _nx_my_process_params should be set
 * to that parameters string by the code that starts up the process.
 *
 * Note: Some startup methods such as rsh will allow the parameters
 * to be passed as part of the command line arguments.  In this case,
 * _nx_my_process_params can be left empty, since those parameters
 * will be picked up as part of argc/argv.
 *
 * This variable is defined and statically initialized to NULL in args.c
 */
extern char *		_nx_my_process_params;


/*
 * This is used by the Nexus print routines for output.
 */
NEXUS_GLOBAL FILE 	*_nx_stdout;


/*
 * _nx_master_node
 *
 * NEXUS_TRUE if my process is the master node.
 */
NEXUS_GLOBAL nexus_bool_t	_nx_master_node;


/*
 * _nx_process_is_context
 *
 * NEXUS_TRUE if my process is for a context, not a node
 */
NEXUS_GLOBAL nexus_bool_t	_nx_process_is_context;


/*
 * _nx_my_node
 *
 * This is the canonical name for my node.
 */
NEXUS_GLOBAL nexus_node_t	_nx_my_node;


/*
 * _nx_my_node_id
 *
 * _nx_my_node_id is the unique integer node id for this process' node.
 */
NEXUS_GLOBAL int	_nx_my_node_id;


/*
 * _nx_master_id_string
 *
 * This is a string that is supplied by the master node which
 * uniquely identifies this Nexus computation.
 */
NEXUS_GLOBAL char *	_nx_master_id_string;


#ifdef BUILD_LITE

/*
 * _nx_lite_context
 *
 * Since NexusLite has only one context in a process, and no threads,
 * the nexus_context_t* is just stored in a global.
 */
NEXUS_GLOBAL nexus_context_t *_nx_lite_context;

#else  /* BUILD_LITE */

/*
 * _nx_context_key
 *
 * This key is used to store the context pointer in thread specific storage.
 */
NEXUS_GLOBAL nexus_thread_key_t _nx_context_key;

#endif /* BUILD_LITE */


/*
 * _nx_context_list
 *
 * This is a list of all the contexts within this nexus process
 */
NEXUS_GLOBAL nexus_context_t *_nx_context_list;


/*
 * _nx_initial_context
 * 
 * This is the first context that is created during process
 * initialization.
 */
NEXUS_GLOBAL nexus_context_t *_nx_initial_context;

/*
 * _nx_path_array
 *
 * Used to hold paths to files
 */
NEXUS_GLOBAL char _nx_path_array[NEXUS_PATH_MAXVAL][NEXUS_MAX_EXECUTABLE_PATH_LENGTH+1];

/*
 * Various debug flags...
 */
#ifdef BUILD_DEBUG
NEXUS_GLOBAL int	_nx_debug_level;
#endif


/*
 * For pause points.
 *   _nx_pause_on_fatal==NEXUS_TRUE is Nexus should pause when it hits
 * 	a fatal error.
 *   _nx_pausing_for_fatal==NEXUS_TRUE after we have paused on fatal (used
 *	to avoid looping in the pause.
 */
NEXUS_GLOBAL nexus_bool_t	_nx_pause_on_fatal;
NEXUS_GLOBAL nexus_bool_t	_nx_pausing_for_fatal;
NEXUS_GLOBAL nexus_bool_t	_nx_pause_on_startup;
NEXUS_GLOBAL nexus_bool_t	_nx_pausing_for_startup;

/*
 * If NEXUS_TRUE then print the commands that would start up
 * new processes, but do not actually start the new processes.
 */
NEXUS_GLOBAL nexus_bool_t	_nx_dont_start_processes;

#ifdef BUILD_DEBUG
NEXUS_GLOBAL int                  _nx_num_debug_levels;
NEXUS_GLOBAL nexus_debug_state_t  _nx_debug_levels[NEXUS_MAX_DEBUG_LEVELS];
#endif /* BUILD_DEBUG */

/*
 * These are used to keep things from hanging, if one thread exits while
 * another thread has a context or node creation outstanding.
 */
NEXUS_GLOBAL int                  _nx_num_outstanding_creates;
NEXUS_GLOBAL nexus_mutex_t        _nx_orphan_mutex;
NEXUS_GLOBAL nexus_cond_t         _nx_orphan_cond;

/*
 * _nx_fault_tolerant
 */
NEXUS_GLOBAL nexus_bool_t	_nx_fault_tolerant;

/*
 * If NEXUS_TRUE then _nx_executable_name() will not expand
 * an executable name that is a relative path to an absolute path.
 */
NEXUS_GLOBAL nexus_bool_t	_nx_dont_expand_executables;

NEXUS_GLOBAL int		_nx_skip_poll_count;


/*
 * _nx_transform_table
 */
NEXUS_GLOBAL nexus_transform_table_t
			_nx_transform_table[NEXUS_TRANSFORM_TABLE_SIZE];

#endif /* _NEXUS_INCLUDE_GLOBALS_H */
